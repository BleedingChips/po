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

		template<typename ...T> struct make_property_info_set
		{
			operator const std::set<std::type_index>& () { 
				static std::set<std::type_index> info{ typeid(T)... };
				return info; 
			}
		};

		class property_proxy_map;

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
					return typeid(type) == original_info;
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
					static_assert(std::is_base_of<base_interface, true_type>::value, "need derived form base_interface.");
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

		namespace Implement
		{

			template<typename T> class property_implement : public property_interface, public T 
			{ 
			public:
				property_implement() : property_interface()
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
						return (ite->second->get_associate(), true);
					return false;
				}
				void update(stage_context& sc);
			};
		}

		class property_proxy_map
		{
			// // proxy_id
			std::map<std::type_index, std::shared_ptr<Implement::property_proxy_interface>> mapping;
			std::shared_ptr<Implement::property_map> inside_map;

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

		/*
		class pipeline_interface
		{
			std::type_index type_info;
		public:
			pipeline_interface(const std::type_index& ti);
			const std::type_index& id() const { return type_info; }
			virtual ~pipeline_interface();

			virtual void execute(stage_context&, Tool::stack_list<property_map>* pml = nullptr) = 0;
			virtual bool check_acceptance(const std::set<std::type_index>&, Tool::stack_list<const property_map>*) = 0;
			virtual std::set<std::type_index> lack_acceptance(const customer_info&, Tool::stack_list<const property_map>*) = 0;
		};
		*/

		namespace Implement
		{

			compute_shader stage_load_cs(const std::u16string& p, creator& c);
			vertex_shader stage_load_vs(const std::u16string& path, creator& c);
			pixel_shader stage_load_ps(const std::u16string& path, creator& c);

			class stage_interface
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

			class compute_interface : public base_interface<compute_implement>, public stage_interface
			{
				friend struct element_draw_request;
			public:
				using base_interface<compute_implement>::base_interface;
			};

			template<typename compute_t>
			class compute_implement : public compute_interface, public compute_t
			{
				compute_shader shader;
			public:
				virtual const std::set<std::type_index>& requirement() const { return compute_t::compute_requirement(); }
				virtual bool update_implement(stage_context& sc, property_interface& pi) override { return compute_t::compute_update(sc, pi); }
				virtual void apply(stage_context& sc) override { sc << shader; compute_t::compute_apply(sc); }
				virtual const std::set<std::type_index>& require() const override { return compute_t::compute_require(); }
				compute_implement(creator& c) : compute_interface(typeid(compute_t), decltype(*this)), compute_t(c){ shader = stage_load_cs(compute_t::compute_shader_patch_cs(), c); }
			};
			template<typename T> using compute_implement_t = compute_implement<std::decay_t<T>>;

			template<typename placement_t> class placement_implement;

			class placement_interface : public base_interface<placement_implement>,  public stage_interface
			{
				
				friend struct element_draw_request;
			public:
				using base_interface<placement_implement>::base_interface;
				virtual const vertex_shader& shader() const = 0;
			};

			template<typename placement_t>
			class placement_implement : public placement_interface, public placement_t
			{
				vertex_shader vsshader;
			public:
				virtual const std::set<std::type_index>& requirement() const { return placement_t::placement_requirement(); }
				virtual const vertex_shader& shader() const { return vsshader; }
				virtual bool update_implement(stage_context& sc, property_interface& pi) override { return placement_t::placement_update(sc, pi); }
				virtual void apply(stage_context& sc) override { sc << vsshader; placement_t::placement_apply(sc); }
				placement_implement(creator& c) : placement_interface(typeid(placement_t), typeid(decltype(*this))), placement_t(c) { vsshader = stage_load_vs(placement_t::placement_shader_patch_vs(), c); }
			};

			template<typename T> using placement_implement_t = placement_implement<std::decay_t<T>>;

			template<typename geometry_t> class geometry_implement;

			class geometry_interface : public base_interface<geometry_implement>,  public stage_interface
			{
				
				virtual void apply(stage_context& sc) override final{}
			public:
				std::map<std::type_index, input_layout> layout_map;
				virtual void apply_implement(stage_context& sc, const std::type_index& ti) = 0;
				using stage_interface::stage_interface;
				virtual void update_layout(placement_interface&, creator& c) = 0;
				using base_interface<geometry_implement>::base_interface;
			};

			template<typename geometry_t>
			class geometry_implement : public geometry_interface, public geometry_t
			{

			public:
				virtual const std::set<std::type_index>& requirement() const { return geometry_t::geometry_requirement(); }
				virtual void update_layout(placement_interface& pi, creator& c) override
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
				geometry_implement(creator& c) : geometry_interface(typeid(geometry_t), typeid(decltype(*this))), geometry_t(c) {}
			};

			template<typename material_t> class material_implement;

			class material_interface : public base_interface<material_implement>, public stage_interface
			{
				std::type_index pipeline_id;
			public:
				const std::type_index& pipeline() const { return pipeline_id; }
				material_interface(const std::type_index& ori, const std::type_index& rea, const std::type_index& pipe = typeid(void)) : base_interface<material_implement>(ori, rea), pipeline_id(pipe) {}
			};

			template<typename material_t>
			class material_implement : public material_interface, public material_t
			{
				pixel_shader shader;
			public:
				virtual const std::set<std::type_index>& requirement() const { return material_t::material_requirement(); }
				virtual void apply(stage_context& sc) override { sc << shader; material_t::material_apply(sc); }
				virtual bool update_implement(stage_context& sc, property_interface& pi) override { return material_t::material_update(sc, pi); }
				material_implement(creator& c) : material_interface(typeid(material_t), typeid(decltype(*this))), material_t(c){ shader = stage_load_ps(material_t::material_shader_patch_ps(), c); }
			};
			template<typename T> using placement_implement_t = placement_implement<std::decay_t<T>>;
		}

		class stage_instance
		{
			std::map<std::type_index, std::shared_ptr<Implement::compute_interface>> compute_map;
			std::map<std::type_index, std::shared_ptr<Implement::material_interface>> material_map;
			std::map<std::type_index, std::shared_ptr<Implement::placement_interface>> placement_map;
			std::map<std::type_index, std::shared_ptr<Implement::geometry_interface>> geometry_map;
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
				std::vector<std::shared_ptr<compute_interface>> compute;
				std::shared_ptr<placement_interface> placement;
				std::shared_ptr<geometry_interface> geometry;
				std::map<std::type_index, std::shared_ptr<material_interface>> material;
				property_proxy_map mapping;

				element_implement& operator<<(std::shared_ptr<compute_interface> ptr) { if (ptr) compute.push_back(std::move(ptr)); return *this; }
				element_implement& operator<<(std::shared_ptr<placement_interface> ptr) { placement = std::move(ptr); return *this; }
				element_implement& operator<<(std::shared_ptr<geometry_interface> ptr) { geometry = std::move(ptr); return *this; }
				element_implement& operator<<(std::shared_ptr<material_interface> ptr) {
					if (ptr) {
						auto id = ptr->pipeline();
						material.insert({ id, std::move(ptr) });
					}
					return *this;
				}
				template<typename T>
				 auto operator<<(T&& f) -> std::enable_if_t<std::is_function_v<T>, element_implement&> {
					mapping << f;
					return *this;
				}
			};
		}

		struct element
		{
			std::shared_ptr<Implement::element_implement> ptr;
		public:
			element() : ptr(std::make_shared<Implement::element_implement>()) {}
			template<typename T> element operator<< (T&& t) { *ptr << t;  return *this; }
		};
		

		namespace Implement
		{

			struct element_dispatch_request
			{
				std::shared_ptr<compute_interface> compute;
				std::shared_ptr<property_map> mapping;
				void update(stage_context& sc) { mapping->update(sc); }
				void dispatch(stage_context& sc, Tool::stack_list<property_map> * = nullptr);
			};

			struct element_draw_request
			{
				std::shared_ptr<placement_interface> placemenet;
				std::shared_ptr<geometry_interface> geometry;
				std::shared_ptr<material_interface> material;
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

		/*

		namespace Implement
		{
			class property_map_implement
			{
				std::map<std::type_index, std::shared_ptr<property_interface>> mapping;

				template<typename T>
				struct check
				{
					using funtype = Tmp::pick_func<typename Tmp::degenerate_func<Tmp::extract_func_t<T>>::type>;
					static_assert(funtype::size == 1, "only receive one parameter");
					using true_type = std::decay_t<typename funtype::template out<Tmp::itself>::type>;
					static_assert(std::is_base_of<property_interface, true_type>::value, "need derived form property_interface.");
				};

				template<typename T> using check_t = typename check<T>::true_type;

			public:

				bool insert(std::shared_ptr<property_interface> sp);

				void clear();

				template<typename F> bool find_property(F&& f)
				{
					auto ite = mapping.find(typeid(check_t<F>));
					if (ite != mapping.end())
					{
						auto ptr = ite->second;
						return (f(ptr->cast<check_t<F>>()), true);
					}
					return false;
				}

				template<typename F, typename ...AT> void create_property(F&& f, AT&& ... at)
				{
					auto ptr = std::make_shared<check_t<F>>(std::forward<AT>(at)...);
					mapping.insert({ ptr->id(), ptr });
					f(*ptr);
				}

				template<typename F, typename ...AT> bool create_property_if_not_existing(F&& f, AT&& ... at)
				{
					auto ite = mapping.find(typeid(check_t<F>));
					if (ite != mapping.end())
					{
						auto ptr = ite->second;
						f(ptr->cast<check_t<F>>());
						return false;
					}
					else {
						auto ptr = std::make_shared<check_t<F>>(std::forward<AT>(at)...);
						mapping.insert({ ptr->id(), ptr });
						f(*ptr);
						return true;
					}
				}

				template<typename F> property_map_implement& operator<<(F&& f)
				{
					using type = check_t<F>;
					static constexpr bool value = std::is_constructible_v<type>;
					if constexpr (value)
						create_property_if_not_existing(f);
					else
						find_property(f);
					return *this;
				}

				template<typename T> bool have() const { return have(typeid(T)); }
				bool have(std::type_index) const;

				template<typename F> void for_each(F&& func)
				{
					for (auto& f : mapping) func(f);
				}

				template<typename F> void remove_if(F&& func)
				{
					for (auto ite = mapping.begin(); ite != mapping.end();)
					{
						if (func(*ite->second))
							mapping.erase(ite++);
						else
							++ite;
					}
				}

				template<typename F> bool find(std::type_index ti, F&& f)
				{
					auto ite = mapping.find(ti);
					if (ite != mapping.end())
					{
						f(*(ite->second));
						return true;
					}
					return false;
				}

			};
		}


		using property_map = Tool::scope_lock<Implement::property_map_implement>;

		class acceptance_t
		{
			template<typename T, typename K = void> struct check_picker { 
				static_assert(std::is_same_v<stage_context&, K> || std::is_same_v<void, K>, "");
				static constexpr bool  value = std::is_same_v<void, K>;
				using type = T; 
			};

			template<typename T>
			struct check
			{
				using funtype = Tmp::pick_func<typename Tmp::degenerate_func<Tmp::extract_func_t<T>>::type>;
				static_assert(funtype::size == 2 || funtype::size == 1, "only receive class& that derive form property_interface or an optional stage_context&");
				using true_type = std::decay_t<typename funtype::template out<check_picker>::type>;
				static_assert(std::is_base_of<property_interface, true_type>::value, "need derived form property_interface.");
			};

			using map_t = std::map<std::type_index, std::function<bool(stage_context&, Implement::storage_renderer_interface& sri)>>;

			map_t acceptance;

			static bool update_implement(stage_context&, typename map_t::value_type&, Tool::stack_list<property_map>* sl = nullptr);

			template<typename F> map_t::value_type make_pair(F&& f)
			{
				using type = typename check<F>::true_type;
				static constexpr bool value = check<F>::value;
				if constexpr(value)
					return map_t::value_type{ typeid(type), [f](property_interface& pi, stage_context& sc) { return pi.update_implement(sc); pi.cast(f); } };
				else
					return map_t::value_type{ typeid(type), [f](property_interface& pi, stage_context& sc) { return pi.update_implement(sc); pi.cast(f, sc); } };
				return std::move(*this);
			}

		public:

			template<typename ...F>
			acceptance_t(F&& ...f) : acceptance({make_pair(std::forward<F>(f))...}) {}
			//acceptance_t(const acceptance_t&) = default;
			acceptance_t(acceptance_t&&) = default;

			bool check_acceptance(Tool::stack_list<const property_map>* sl = nullptr) const;
			std::set<std::type_index> lack_acceptance(Tool::stack_list<const property_map>* sl = nullptr) const;
			bool update(stage_context&, Tool::stack_list<property_map>* sl = nullptr);

			template<typename ...property_map_t> decltype(auto) check_acceptance(const property_map_t& ...pm)
			{
				return Tool::make_stack_list<const property_map>([this](Tool::stack_list<const property_map>* sl) {
					return check_acceptance(sl);
				});
			}

			template<typename ...property_map_t> decltype(auto) lack_acceptance(const property_map_t& ...pm)
			{
				return Tool::make_stack_list<const property_map>([this](Tool::stack_list<const property_map>* sl) {
					return lack_acceptance(sl);
				});
			}

			template<typename ...property_map_t> decltype(auto) update(stage_context& sc, property_map_t& ...pm)
			{
				return Tool::make_stack_list<property_map>([&, this](Tool::stack_list<property_map>* sl) {
					return update(sc, sl);
				});
			}

		};



		namespace Implement
		{

			class stage_interface : public base_interface
			{
				acceptance_t acceptance;
			protected:
				property_map default_mapping;
			public:

				template<typename ...F>
				stage_interface(const std::type_index& ti, F&& ...f) : base_interface(ti), acceptance(std::forward<F>(f)...) {}

				operator const property_map& () const { return default_mapping; }

				bool update(stage_context& sc, Tool::stack_list<property_map>* sl = nullptr) { Tool::stack_list<property_map> temporary{ default_mapping , sl }; return acceptance.update(sc, &temporary); }
				template<typename ...property_map_t> bool update(stage_context& sc, property_map_t& ...pm) { return acceptance.update(sc, pm..., default_mapping); }

				template<typename ...PropertyMapping> bool update_implement(stage_context& pi, PropertyMapping& ...pl)
				{
					return make_property_mapping_list([&, this](property_mapping_list* pml) {
						update_implement(pi, pml);
					}, pl...);
				}


				bool check_acceptance(Tool::stack_list<const property_map>* pml = nullptr) const {
					Tool::stack_list<const property_map> temporary{ default_mapping , pml };
					return acceptance.check_acceptance(&temporary);
				}

				template<typename ...PropertyMapping> bool check_acceptance(const PropertyMapping& ...pl)
				{
					return acceptance.check_acceptance(pl..., default_mapping);
				}

				std::set<std::type_index> lack_acceptance(Tool::stack_list<const property_map>* pml = nullptr) const {
					Tool::stack_list<const property_map> temporary{ default_mapping , pml };
					return acceptance.lack_acceptance(&temporary);
				}

				template<typename ...PropertyMapping> std::set<std::type_index> lack_acceptance(const PropertyMapping&... PM)
				{
					return acceptance.lack_acceptance(pl..., default_mapping);
				}

				virtual void init(creator&) = 0;
			};

			class pipeline_interface
			{
				std::type_index type_info;
			public:
				pipeline_interface(const std::type_index& ti);
				const std::type_index& id() const { return type_info; }
				virtual ~pipeline_interface();
				virtual void execute(stage_context&, Tool::stack_list<property_map>* pml = nullptr) = 0;
				virtual bool check_acceptance(const acceptance_t&, Tool::stack_list<const property_map>*) = 0;
				virtual std::set<std::type_index> lack_acceptance(const acceptance_t&, Tool::stack_list<const property_map>*) = 0;
			};

		}

		class placement_interface : public Implement::stage_interface
		{
			std::u16string path;
		protected:
			friend class geometry_interface;
			vertex_stage stage_vs;
			bool load_vs(std::u16string path, creator& c);
		public:
			using Implement::stage_interface::stage_interface;
			virtual void apply(stage_context&);
			const vertex_stage& vs()const { return stage_vs; }
		};

		class geometry_interface : public Implement::stage_interface
		{
		protected:
			input_assember_stage stage_ia;
			raterizer_state stage_rs;
		public:
			using Implement::stage_interface::stage_interface;
			virtual void apply(stage_context&);
			virtual void draw(stage_context&) = 0;
			const input_assember_stage& ia()const { return stage_ia; }
		};

		class material_interface : public Implement::stage_interface
		{
			std::type_index pipeline;
			std::u16string path;
		protected:
			pixel_stage stage_ps;
			blend_state stage_bs;
			bool load_ps(std::u16string p, creator& c);
		public:
			template<typename ...F>
			material_interface(const std::type_index& material_type, const std::type_index& pipeline_type = typeid(void), F&& ...f) :
				Implement::stage_interface(material_type, std::forward<F>(f)...), pipeline(pipeline_type) {}
			const std::type_index& pipeline_id() const { return pipeline; }
			virtual void apply(stage_context&);
		};

		class compute_interface : public Implement::stage_interface
		{
			std::u16string path;
		protected:
			compute_stage stage_cs;
			bool load_cs(std::u16string p, creator& c);
		public:
			using Implement::stage_interface::stage_interface;
			virtual void apply(stage_context&);
			virtual void dispath(stage_context& p) = 0;
		};

		struct element_block
		{

		};

		namespace Implement
		{

#define DX11_ELEMENT_DEFINE_PTR(X) using X##_ptr = std::shared_ptr<X##_interface>; using X##_ptr_weak = std::weak_ptr<X##_interface>;
			DX11_ELEMENT_DEFINE_PTR(compute);
			DX11_ELEMENT_DEFINE_PTR(placement);
			DX11_ELEMENT_DEFINE_PTR(geometry);
			DX11_ELEMENT_DEFINE_PTR(material);
#undef DX11_ELEMENT_DEFINE_PTR

			struct element_store
			{
				element_block block;
				std::vector<compute_ptr> compute_list;
				placement_ptr placement;
				geometry_ptr geometry;
				std::map<std::type_index, material_ptr> material_map;
				
				property_map mapping;
				input_layout layout;
			};

		}

		using element_ts = Tool::scope_lock<Implement::element_store>;

		struct element
		{
			Tool::completeness<element_ts> element_data;

			template<typename T>
			element& operator = (T cp)
			{
				element_data.lock([&](typename element_ts::type& t) {
					using type = std::decay_t<T>;
					static constexpr bool is_material = std::is_base_of_v<material_interface, type>;
					if constexpr(is_material)
						t.material_map.insert({ cp->pipeline_id(), cp });
					else if constexpr (static constexpr bool is_geometry = std::is_base_of_v<geometry_interface, type>; is_geometry)
						t.geometry = std::move(cp);
					else if constexpr (static constexpr bool is_placement = std::is_base_of_v<placement_interface, type>; is_placement)
						t.placement = std::move(cp);
					else if constexpr (static constexpr bool is_compute = std::is_base_of_v<compute_interface, type>; is_compute)
						t.compute_list.push_back(std::move(cp));
					else
						static_cast(false, "");
				});
				return *this;
			}

		};

		using element_ptr = std::pair<Tool::completeness_ref, element_ts*>;

		inline element_ptr make_element_ptr(element& el) {
			return element_ptr{el.element_data, &el.element_data};
		}

		class element_instance
		{
			std::unordered_map<std::type_index, Implement::compute_ptr_weak> compute_map;
			std::unordered_map<std::type_index, Implement::placement_ptr_weak> placement_map;
			std::unordered_map<std::type_index, Implement::geometry_ptr_weak> geometry_map;
			std::unordered_map<std::type_index, Implement::material_ptr_weak> material_map;
			creator creat_tool;
		public:
			element_instance(creator c) : creat_tool(c) {}

			template<typename T, typename K, typename ...AK> static std::shared_ptr<T> create_if_no_exist_implement(std::unordered_map<std::type_index, std::weak_ptr<K>>& ptr_map, AK&& ...at)
			{
				using type = std::decay_t<T>;
				static_assert(std::is_constructible_v<type, AK...>, "");
				static_assert(std::is_base_of_v<K, type>, "");
				auto ite = ptr_map.find(typeid(type));
				if (ite == ptr_map.end() || ite->second.expired())
				{
					auto ptr = std::make_shared<type>(std::forward<AT>(at)...);
					ptr->init(creat_tool);
					ptr_map.insert({ ptr->id, std::static_pointer_cast<K>(std::weak_ptr<type>{ ptr }) });
					return ptr;
				}
				else {
					return std::static_pointer_cast<type>(ite->second.lock());
				}
			}

			template<typename T, typename ...AT> std::shared_ptr<T> create_if_no_exist(AT&& ...at)
			{
				using type = std::decay_t<T>;
				static_assert(std::is_constructible_v<type, AT...>, "");
				static constexpr bool is_material = std::is_base_of_v<material_interface, type>;
				if constexpr(is_material)
					return create_if_no_exist_implement<type>(material_map, std::forward<AT>(at)...);
				else if constexpr (static constexpr bool is_geometry = std::is_base_of_v<geometry_interface, type>; is_geometry)
					return create_if_no_exist_implement<type>(geometry_map, std::forward<AT>(at)...);
				else if constexpr (static constexpr bool is_placement = std::is_base_of_v<placement_interface, type>; is_placement)
					return create_if_no_exist_implement<type>(placement_map, std::forward<AT>(at)...);
				else if constexpr (static constexpr bool is_compute = std::is_base_of_v<compute_interface, type>; is_compute)
					return create_if_no_exist_implement<type>(compute_map, std::forward<AT>(at)...);
				else
					static_cast(false, "");
			}
		};

		/*
		namespace Implement
		{

			struct element_compute
			{
				std::shared_ptr<compute_interface> compute_ptr;
				

				operator bool() const { return static_cast<bool>(compute_ptr); }

				void clear_unused_property();
				void clear_property() { mapping.clear(); }
				void clear() { mapping.clear(); compute_ptr.reset(); }

				bool dispatch(stage_context& p, property_mapping_list* pml = nullptr);

				template<typename ...PropertyMapping> bool dispatch(stage_context& p, PropertyMapping&... property_mapping)
				{
					if (compute_ptr)
						if (compute_ptr->update_implement(p, property_mapping..., mapping))
							return (compute_ptr->apply(p), compute_ptr->dispath(p), true);
					return false;
				}

				bool check_acceptance() const { return compute_ptr ? compute_ptr->check_acceptance(mapping) : true; }
				stage_interface::acceptance_t lack_acceptance() const { return compute_ptr ? compute_ptr->lack_acceptance(mapping) : stage_interface::acceptance_t{}; }

				element_compute& operator= (std::shared_ptr<compute_interface> gp) { compute_ptr = std::move(gp); return *this; }
			};

			class element_entity
			{
				std::shared_ptr<placement_interface> placemenet_ptr;
				std::shared_ptr<geometry_interface> geometry_ptr;
				input_layout layout;
			public:
				void clear() { placemenet_ptr.reset(); geometry_ptr.reset(); }
				void update_layout(creator& c);
				bool apply(stage_context&);
			};

			class element_material
			{
				std::set<std::type_index, std::shared_ptr<material_interface>> material_set;
			public:
				bool apply(std::type_index, stage_context&);
			};

			class element_block
			{
			public:
			};

		}
		*/
		/*
			struct element_implement
			{
				std::shared_ptr<placement_interface> placemenet_ptr;
				std::shared_ptr<geometry_interface> geometry_ptr;
				std::shared_ptr<material_interface> material_ptr;

				input_layout layout;
				property_mapping mapping;

				void clear_unused_property();
				void clear_property() { mapping.clear(); }
				void clear() { mapping.clear(); placemenet_ptr.reset(); geometry_ptr.reset(); material_ptr.reset(); }

				element_implement& operator= (std::shared_ptr<geometry_interface> gp) { geometry_ptr = std::move(gp); return *this; }
				element_implement& operator= (std::shared_ptr<placement_interface> gp) { placemenet_ptr = std::move(gp); return *this; }
				element_implement& operator= (std::shared_ptr<material_interface> gp) { material_ptr = std::move(gp); return *this; }

				void update_layout(creator& c);

				const std::type_index& pipeline_id() const { material_ptr ? material_ptr->pipeline_id() : typeid(void); }

				bool draw(stage_context& p, property_mapping_list* pml);

				template<typename ...PropertyMapping> bool draw(stage_context& p,  PropertyMapping&& ...property_mapping)
				{
					return make_property_mapping_list([&, this](property_mapping_list* pml) {
						draw(p, pml);
					}, property_mapping...);
				}

				bool check_acceptance() const;
				stage_interface::acceptance_t lack_acceptance() const;
			};
			*/

		/*
		class element_implement



		using element_entity_ptr = std::shared_ptr<Implement::element_entity>;
		using element_material_ptr = std::shared_ptr<Implement::element_material>;
		using element_compute_ptr = std::shared_ptr<Implement::ele>

		struct element
		{
			element_entity_ptr entity_ptr;
			element_material_ptr material_ptr;
			property_mapping mapping;
		public:

		};

		class element
		{
			std::shared_ptr<Implement::element_implement> element_ptr;
			void check_ptr() {
				if (!element_ptr)
					element_ptr = std::make_shared<Implement::element_implement>();
			}
		public:
			template<typename ...PropertyMapping> bool draw(pipeline& p, PropertyMapping&& ...property_mapping) const { return element_ptr && element_ptr->draw(p, property_mapping...); }

			bool check_acceptance() const { return element_ptr ? element_ptr->check_acceptance() : true; }
			template<typename ...PropertyMapping> Implement::stage_interface::acceptance lack_acceptance(const PropertyMapping& ... property_mapping) const { return element_ptr ? element_ptr->lack_acceptance(property_mapping) : Implement::stage_interface::acceptance{}; }

			template<typename T> element& operator= (std::shared_ptr<T> sp) { check_ptr(); *element_ptr = std::move(sp); return *this; }
			template<typename F> element& operator<< (F&& f) { check_ptr(); *element_ptr << f; return *this; }

			const std::type_index& pipeline_id() const { element_ptr ? element_ptr->pipeline_id() : typeid(void); }
		};

		class element_compute
		{
			std::shared_ptr<Implement::element_compute_implement> element_ptr;
			void check_ptr() {
				if (!element_ptr)
					element_ptr = std::make_shared<Implement::element_compute_implement>();
			}
		public:
			template<typename ...PropertyMapping> bool draw(pipeline& p, PropertyMapping&& ...property_mapping) const { return element_ptr && element_ptr->draw(p, property_mapping...); }

			template<typename ...PropertyMapping> bool check_acceptance(const PropertyMapping& ... property_mapping) const { return element_ptr ? element_ptr->check_acceptance(property_mapping) : true; }
			template<typename ...PropertyMapping> Implement::stage_interface::acceptance lack_acceptance(const PropertyMapping& ... property_mapping) const { return element_ptr ? element_ptr->lack_acceptance(property_mapping) : Implement::stage_interface::acceptance{}; }

			template<typename T> element& operator= (std::shared_ptr<T> sp) { check_ptr(); *element_ptr = std::move(sp); return *this; }
			template<typename F> element& operator<< (F&& f) { check_ptr(); *element_ptr << f; return *this; }
		};

		namespace Implement
		{
			class element_instance_store
			{
				std::unordered_map<std::type_index, std::weak_ptr<placement_interface>> placement_ptr;
				std::unordered_map<std::type_index, std::weak_ptr<geometry_interface>> geometry_ptr;
				std::unordered_map<std::type_index, std::weak_ptr<material_interface>> material_ptr;
				std::unordered_map<std::type_index, std::weak_ptr<compute_interface>> compute_ptr;
			public:

				template<typename T, typename K, typename ...AK> static std::shared_ptr<T> create_if_no_exist_implement(std::unordered_map<std::type_index, std::weak_ptr<K>>& ptr_map, AK&& ...at)
				{
					using type = std::decay_t<T>;
					static_assert(std::is_constructible_v<type, AK...>, "");
					static_assert(std::is_base_of_v<K, type>, "");
					auto ite = ptr_map.find(typeid(type));
					if (ite == ptr_map.end() || ite->second.expired())
					{
						auto ptr = std::make_shared<type>(std::forward<AT>(at)...);
						ptr_map.insert({ ptr->id, std::static_pointer_cast<K>(std::weak_ptr<type>{ ptr }) });
						return ptr;
					}
					else {
						return std::static_pointer_cast<type>(ite->second.lock());
					}
				}

				template<typename T, typename ...AT> std::shared_ptr<T> create_if_no_exist(AT&& ...at)
				{
					using type = std::decay_t<T>;
					static_assert(std::is_constructible_v<type, AT...>, "");
					static constexpr bool is_material = std::is_base_of_v<material_interface, type>;
					if constexpr(is_material)
						return create_if_no_exist_implement<type>(material_ptr, std::forward<AT>(at)...);
					else if constexpr (static constexpr bool is_geometry = std::is_base_of_v<geometry_interface, type>; is_geometry)
						return create_if_no_exist_implement<type>(geometry_ptr, std::forward<AT>(at)...);
					else if constexpr (static constexpr bool is_placement = std::is_base_of_v<placement_interface, type>; is_placement)
						return create_if_no_exist_implement<type>(geometry_ptr, std::forward<AT>(at)...);
					else if constexpr (static constexpr bool is_compute = std::is_base_of_v<compute_interface, type>; is_compute)
						return create_if_no_exist_implement<type>(compute_interface, std::forward<AT>(at)...);
					else
						static_cast(false, "");
				}

			};
		}
		*/


	}
}