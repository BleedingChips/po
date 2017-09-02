#pragma once
#include "dx11_frame.h"
#include <typeindex>
#include <set>
#include <memory>
namespace PO
{
	namespace Dx11
	{

		bool add_shader_path(std::type_index ti, const std::u16string& path);
		template<typename T> bool add_shader_path(const std::u16string& s) { return add_shader_path(typeid(T), s); }

		class property_proxy_map;

		template<typename ...T>
		struct make_property_info_set
		{
			operator const std::set<std::type_index> &() {
				static const std::set<std::type_index> set{ typeid(typename T::renderer_data)... };
				return set;
			}
		};

		namespace Implement
		{

			template<template<typename ...> class imp> struct base_interface
			{
				const std::type_index original_info;
				const std::type_index real_info;
			public:
				base_interface(const std::type_index& original, const std::type_index& real) : original_info(original), real_info(real) {}
				const std::type_index& id() const { return original_info; }
				const std::type_index& real_id() const { return real_info; }
				template<typename T> bool is() const {
					using type = std::decay_t<T>;
					return original_info == typeid(type);
				}
				template<typename T> std::decay_t<T>& cast() {
					using type = std::decay_t<T>;
					return static_cast<type&>(static_cast<imp<type>&>(*this));
				}

				template<typename T> const std::decay_t<T>& cast() const {
					using type = std::decay_t<T>;
					return static_cast<const type&>(static_cast<const imp<type>&>(*this));
				}

				template<typename T, typename ...AT> bool cast(T&& t, AT&& ...at)
				{
					using funtype = Tmp::pick_func<typename Tmp::degenerate_func<Tmp::extract_func_t<T>>::type>;
					static_assert(funtype::size == 1, "only receive one parameter");
					using true_type = std::decay_t<typename funtype::template out<Tmp::itself>::type>;
					//static_assert(std::is_base_of<base_interface, true_type>::value, "need derived form base_interface.");
					if (is<true_type>())
						return (t(cast<true_type>(), std::forward<AT>(at)...), true);
					return false;
				}
			};


			template<typename T> class property_implement;
		}

		class property_interface : public Implement::base_interface<Implement::property_implement>
		{
		public:
			using Implement::base_interface<Implement::property_implement>::base_interface;
			virtual ~property_interface();
		};

		struct stage_requirement
		{
			template<typename T, typename K>
			struct stage_requirement_picker
			{
				static_assert(
					(std::is_same_v<stage_context, std::decay_t<T>> || std::is_same_v<stage_context, std::decay_t<K>>) && !std::is_same_v<std::decay_t<T>, std::decay_t<K>>
					, "");
				using type = std::conditional_t<std::is_same_v<stage_context, std::decay_t<T>>, std::decay_t<K>, std::decay_t<T>>;
				using function_type = void(*)(T, K);
				static bool update_requirement(stage_context& sc, property_interface& pi, void* data)
				{
					function_type fun_ptr = static_cast<function_type>(data);
					return pi.cast([&](type& de) {
						if constexpr(std::is_same_v<stage_context, std::decay_t<T>>)
							(*fun_ptr)(sc, de);
						else
							(*fun_ptr)(de, sc);
					});
				}
			};

			using func_type = std::tuple<bool(*)(stage_context&, property_interface&, void*), void*>;
			using map_t = std::map<std::type_index, func_type>;

			template<typename F> map_t::value_type make_mapping_pair(F&& f)
			{
				using funtype = Tmp::pick_func<typename Tmp::degenerate_func<Tmp::extract_func_t<F>>::type>;
				static_assert(funtype::size == 2, "only receive one parameter");
				using true_type = typename funtype::template out<stage_requirement_picker>;

				typename true_type::function_type funptr = f;
				return map_t::value_type{ typeid(typename true_type::type), std::make_tuple(&true_type::update_requirement, funptr) };
			}


			//1, 将后边两个函数进行类型还原的函数. 2，bool (*)(stage_context& , property_interface, void (*)(stage_context&, T&)). 3, bool (*)(stage_context&, T&);
			map_t mapping;


			template<typename ...F>
			stage_requirement(F&& ...f) : mapping({ make_mapping_pair(f)... }) {}
		};

		namespace Implement
		{

			template<typename T> class property_implement : public property_interface, public T 
			{ 
			public:
				property_implement() : property_interface(typeid(T), typeid(decltype(*this))) {}
				operator T& () { return *this; } 
				operator const T&() const { return *this; } 
			};

			template<typename T> class property_proxy_implement;

			class property_proxy_interface : public base_interface<property_proxy_implement>
			{
				const std::type_index associate_info;
				virtual void update(stage_context& sc) = 0;
				virtual property_interface& get_associate() = 0;
				virtual void push(creator& c) = 0;

				friend struct property_map;
				friend class property_proxy_map;
			public:
				const std::type_index& associate_id() const { return associate_info; }
				property_proxy_interface(const std::type_index& original, const std::type_index& real, const std::type_index& asso);
				virtual ~property_proxy_interface();
			};

			template<typename T> class property_proxy_implement : public property_proxy_interface
			{
				T direct_block;
				T swap_lock;
				property_implement<typename T::renderer_data> renderer_lock;
			public:
				property_proxy_implement() : property_proxy_interface(typeid(T), typeid(decltype(*this)), typeid(typename T::renderer_data)) {}
				virtual property_interface& get_associate() { return renderer_lock; }
				operator T& () { return direct_block; }
				operator const T& () const { return direct_block; }

				virtual void update(stage_context& sc)
				{
					swap_lock.update(renderer_lock, sc);
				}
				virtual void push(creator& c)
				{
					direct_block.push(swap_lock, c);
				}
			};

			struct property_map
			{
				bool allready_update = true;

				// proxy_id
				std::map<std::type_index, std::shared_ptr<property_proxy_interface>> proxy_mapping;

				// associate id
				std::map<std::type_index, std::shared_ptr<property_proxy_interface>> associate_mapping;
				friend struct element_renderer_storage;
			public:
				template<typename F> bool find_associate(const std::type_index& ti, F&& f)
				{
					auto ite = associate_mapping.find(ti);
					if (ite != associate_mapping.end())
						return (f(ite->second->get_associate()), true);
					return false;
				}
				void update(stage_context& sc);
			};
		}

		class property_proxy_map
		{
			// // proxy_id
			std::map<std::type_index, std::shared_ptr<Implement::property_proxy_interface>> mapping;

			template<typename T>
			struct check
			{
				using funtype = Tmp::pick_func<typename Tmp::degenerate_func<Tmp::extract_func_t<T>>::type>;
				static_assert(funtype::size == 1, "only receive one parameter");
				using true_type = std::decay_t<typename funtype::template out<Tmp::itself>::type>;
			};
			template<typename T> using check_t = typename check<T>::true_type;
			std::shared_ptr<Implement::property_map> push(creator& c, const std::set<std::type_index>& require);
			friend struct element_logic_storage;
		public:
			std::shared_ptr<Implement::property_map> inside_map;
			std::shared_ptr<Implement::property_map> push(creator& c);
			property_proxy_map() : inside_map(std::make_shared<Implement::property_map>()) {}

			template<typename F> property_proxy_map& operator<<(F&& f)
			{
				using type = check_t<F>;
				auto ite = mapping.find(typeid(type));
				if (ite != mapping.end())
					f(ite->second->cast<type>());
				else {
					auto ptr = std::make_shared<Implement::property_proxy_implement<type>>();
					mapping.insert({ typeid(type), ptr });
					f(*ptr);
				}
				return *this;
			}
			void clear() { mapping.clear(); }
			
		};

		namespace Implement
		{
			class stage_interface
			{
			public:
				virtual const stage_requirement& requirement() const;
			};
		}

		class geometry_interface : public Implement::stage_interface
		{
		protected:
			input_assember_stage input_stage;
		public:
			const input_assember_stage& input() { return input_stage; }
			void apply(stage_context& sc) { sc << input_stage; }
			virtual void execute(stage_context&) = 0;
		};

		class compute_interface : public Implement::stage_interface
		{
		protected:
			compute_shader cs;
		};



		namespace Implement
		{

			compute_shader stage_load_cs(const std::u16string& p, creator& c);
			vertex_shader stage_load_vs(const std::u16string& path, creator& c);
			pixel_shader stage_load_ps(const std::u16string& path, creator& c);

			class stage_ptr
			{
				bool update(stage_context& sc, const std::type_index& ti, Tool::stack_list<property_map>* sl = nullptr);
				virtual bool update_implement(stage_context&, property_interface& pi) = 0;
				friend struct element_draw_request;
			public:
				virtual const std::set<std::type_index>& requirement() const = 0;
				virtual void apply(stage_context&) = 0;
				bool update(stage_context& sc, Tool::stack_list<property_map>* sl = nullptr);
				template<typename ...PropertyMapping> auto update(stage_context& pi, PropertyMapping& ...pl) { 
					return Tool::make_stack_list([&, this](Tool::stack_list<property_map>* p) {
						update(sc, p);
					}, pl...);
				}
			};

			template<typename compute_t> class compute_implement;

			class compute_ptr : public base_interface<compute_implement>, public stage_ptr
			{
				friend struct element_draw_request;
			public:
				using base_interface<compute_implement>::base_interface;
			};

			template<typename compute_t>
			class compute_implement : public compute_ptr, public compute_t
			{
				compute_shader shader;
			public:
				virtual const std::set<std::type_index>& requirement() const { return compute_t::compute_requirement(); }
				virtual bool update_implement(stage_context& sc, property_interface& pi) override { return compute_t::compute_update(sc, pi); }
				virtual void apply(stage_context& sc) override { sc << shader; compute_t::compute_apply(sc); }
				compute_implement(creator& c) : compute_ptr(typeid(compute_t), typeid(decltype(*this))), compute_t(c) { shader = stage_load_cs(compute_t::compute_shader_patch_cs(), c); assert(shader); }
			};
			template<typename T> using compute_implement_t = compute_implement<std::decay_t<T>>;

			template<typename placement_t> class placement_implement;

			class placement_ptr : public base_interface<placement_implement>,  public stage_ptr
			{
				
				friend struct element_draw_request;
			public:
				using base_interface<placement_implement>::base_interface;
				virtual const vertex_shader& shader() const = 0;
			};

			template<typename placement_t>
			class placement_implement : public placement_ptr, public placement_t
			{
				vertex_shader vsshader;
			public:
				virtual const std::set<std::type_index>& requirement() const { return placement_t::placement_requirement(); }
				virtual const vertex_shader& shader() const { return vsshader; }
				virtual bool update_implement(stage_context& sc, property_interface& pi) override { return placement_t::placement_update(sc, pi); }
				virtual void apply(stage_context& sc) override { sc << vsshader; placement_t::placement_apply(sc); }
				placement_implement(creator& c) : placement_ptr(typeid(placement_t), typeid(decltype(*this))), placement_t(c) { vsshader = stage_load_vs(placement_t::placement_shader_patch_vs(), c); assert(vsshader); }
			};

			template<typename T> using placement_implement_t = placement_implement<std::decay_t<T>>;

			template<typename geometry_t> class geometry_implement;

			class geometry_ptr : public base_interface<geometry_implement>,  public stage_ptr
			{
				virtual void apply(stage_context& sc) override final{}
			public:
				std::map<std::type_index, input_layout> layout_map;
				virtual void apply_implement(stage_context& sc, const std::type_index& ti) = 0;
				using stage_ptr::stage_ptr;
				virtual void update_layout(placement_ptr&, creator& c) = 0;
				using base_interface<geometry_implement>::base_interface;
			};

			template<typename geometry_t>
			class geometry_implement : public geometry_ptr, public geometry_t
			{

			public:
				virtual const std::set<std::type_index>& requirement() const { return geometry_t::geometry_requirement(); }
				virtual void update_layout(placement_ptr& pi, creator& c) override
				{
					auto ite = layout_map.find(pi.id());
					if (ite == layout_map.end())
					{
						input_layout tem = c.create_layout(geometry_t::geometry_input(), pi.shader());
						layout_map.insert({ pi.id(), tem });
					}
				}

				virtual void apply_implement(stage_context& sc, const std::type_index& ti) override
				{
					auto ite = layout_map.find(ti);
					if (ite != layout_map.end())
						sc << ite->second;
					geometry_t::geometry_apply(sc);
				}

				virtual bool update_implement(stage_context& sc, property_interface& pi) override { return geometry_t::geometry_update(sc, pi); }
				geometry_implement(creator& c) : geometry_ptr(typeid(geometry_t), typeid(decltype(*this))), geometry_t(c) {}
			};

			template<typename material_t> class material_implement;

			class material_ptr : public base_interface<material_implement>, public stage_ptr
			{
				std::type_index pipeline_id;
			public:
				const std::type_index& pipeline() const { return pipeline_id; }
				material_ptr(const std::type_index& ori, const std::type_index& rea, const std::type_index& pipe = typeid(void)) : base_interface<material_implement>(ori, rea), pipeline_id(pipe) {}
			};

			template<typename material_t>
			class material_implement : public material_ptr, public material_t
			{
				pixel_shader shader;

				//template<typename T, typename = void> struct material_pipeline_detect : std::false_type {};
				//template<typename T> struct material_pipeline_detect<T, std::void_t<decltype(material_t::pipeline_id())>> : std::true_type {};
			public:
				virtual const std::set<std::type_index>& requirement() const { return material_t::material_requirement(); }
				virtual void apply(stage_context& sc) override { sc << shader; material_t::material_apply(sc); }
				virtual bool update_implement(stage_context& sc, property_interface& pi) override { return material_t::material_update(sc, pi); }
				material_implement(creator& c) : material_ptr(typeid(material_t), typeid(decltype(*this)), material_t::pipeline_id()), material_t(c) { shader = stage_load_ps(material_t::material_shader_patch_ps(), c);  assert(shader); }
			};
			template<typename T> using placement_implement_t = placement_implement<std::decay_t<T>>;
		}

		struct material_default
		{
			static const std::type_index& pipeline_id() { static std::type_index tem = typeid(void);  return tem; }
			static const std::set<std::type_index>& material_requirement() { return make_property_info_set<>{}; }
			static void material_apply(stage_context& sc) { }
			static bool material_update(stage_context& sc, property_interface& pi) { return false; }
		};


		class stage_instance
		{
			std::map<std::type_index, std::shared_ptr<Implement::compute_ptr>> compute_map;
			std::map<std::type_index, std::shared_ptr<Implement::material_ptr>> material_map;
			std::map<std::type_index, std::shared_ptr<Implement::placement_ptr>> placement_map;
			std::map<std::type_index, std::shared_ptr<Implement::geometry_ptr>> geometry_map;
			creator tool;


			template<typename T, template<typename ...> class Imp, typename map> std::shared_ptr<Imp<std::decay_t<T>>> create_implement(map& m)
			{
				using type = std::decay_t<T>;
				auto ite = m.find(typeid(type));
				if (ite == m.end())
				{
					auto ptr = std::make_shared <Imp<type>>(tool);
					m.insert({ typeid(type), ptr });
					return ptr;
				}
				else {
					return std::static_pointer_cast<Imp<type>>(ite->second);
				}
			}

		public:

			stage_instance(creator& c) : tool(c) {}

			template<typename T>  decltype(auto) create_compute()
			{
				return create_implement<T, Implement::compute_implement>(compute_map);
			}
			template<typename T>  decltype(auto) create_placement()
			{
				return create_implement<T, Implement::placement_implement>(placement_map);
			}
			template<typename T>  decltype(auto) create_geometry()
			{
				return create_implement<T, Implement::geometry_implement>(geometry_map);
			}
			template<typename T>  decltype(auto) create_material()
			{
				return create_implement<T, Implement::material_implement>(material_map);
			}
		};

		struct element_block {};

		namespace Implement
		{
			struct element_implement
			{
				element_block block;
				std::vector<std::shared_ptr<compute_ptr>> compute;
				std::shared_ptr<placement_ptr> placement;
				std::shared_ptr<geometry_ptr> geometry;
				std::map<std::type_index, std::shared_ptr<material_ptr>> material;
				property_proxy_map mapping;

				element_implement& operator<<(std::shared_ptr<compute_ptr> ptr) { if (ptr) compute.push_back(std::move(ptr)); return *this; }
				element_implement& operator<<(std::shared_ptr<placement_ptr> ptr) { placement = std::move(ptr); return *this; }
				element_implement& operator<<(std::shared_ptr<geometry_ptr> ptr) { geometry = std::move(ptr); return *this; }
				element_implement& operator<<(std::shared_ptr<material_ptr> ptr) {
					if (ptr) {
						auto id = ptr->pipeline();
						material.insert({ id, std::move(ptr) });
					}
					return *this;
				}
			};
		}

		struct element
		{
			std::shared_ptr<Implement::element_implement> ptr;
		public:
			element() : ptr(std::make_shared<Implement::element_implement>()) {}
			template<typename T> element operator<< (std::shared_ptr<T> t) { *ptr << std::move(t);  return *this; }
			template<typename T> element operator<< (T&& t) { ptr->mapping << t;  return *this; }
		};
		

		namespace Implement
		{

			struct element_dispatch_request
			{
				std::shared_ptr<compute_ptr> compute;
				std::shared_ptr<property_map> mapping;
				void update(stage_context& sc) { mapping->update(sc); }
				void dispatch(stage_context& sc, Tool::stack_list<property_map> * = nullptr);
			};

			struct element_draw_request
			{
				std::shared_ptr<placement_ptr> placemenet;
				std::shared_ptr<geometry_ptr> geometry;
				std::shared_ptr<material_ptr> material;
				std::shared_ptr<property_map> mapping;
				void update(stage_context& sc) { mapping->update(sc); }
				void draw(stage_context& sc, Tool::stack_list<property_map> * = nullptr);
			};
		}

		struct element_swap_block
		{
			std::mutex swap_lock;
			std::vector<Implement::element_dispatch_request> dispatch_request;
			std::unordered_map<std::type_index, std::vector<Implement::element_draw_request>> draw_request;
		};

		struct element_logic_storage
		{

			std::mutex swap_mutex;
			std::vector<std::shared_ptr<Implement::element_implement>> element_store;

			element_logic_storage& operator<< (const element& ele) { if (ele.ptr) element_store.push_back(ele.ptr); return *this; }
			void push (element_swap_block& esb, creator& c);
		};


		struct element_renderer_storage
		{
			std::vector<Implement::element_dispatch_request> dispatch_request;
			std::unordered_map<std::type_index, std::vector<Implement::element_draw_request>> draw_request;
			void get(element_swap_block& esb, stage_context& sc);
		};

		class pipeline_interface
		{
			const std::type_index type_info;
		public:
			property_proxy_map property_mapping;
			pipeline_interface(const std::type_index& ti);
			const std::type_index& id() const { return type_info; }
			virtual ~pipeline_interface();
			void push(creator& c);
			void execute(stage_context& sc, element_renderer_storage& ers, Tool::stack_list<Implement::property_map>* pml = nullptr);

			virtual void execute_implement(stage_context& sc, element_renderer_storage& ers, Tool::stack_list<Implement::property_map>* pml = nullptr) = 0;
		};

	}
}