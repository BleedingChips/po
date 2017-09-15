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


		class property_resource
		{
			bool m_need_update = true;
		protected:
			void need_update() { m_need_update = true; }
		public:
			bool is_need_update() const { return m_need_update; }
			void finished_update() { m_need_update = false; }
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
				virtual property_interface& get_associate() = 0;

				virtual void logic_to_swap(creator& c) = 0;
				virtual void swap_to_renderer() = 0;
				virtual void logic_to_renderer(creator& c) = 0;

				friend struct property_map;
				friend class property_proxy_map;
			public:
				const std::type_index& associate_id() const { return associate_info; }
				property_proxy_interface(const std::type_index& original, const std::type_index& real, const std::type_index& asso);
				virtual ~property_proxy_interface();
			};

			template<typename T, typename = void> struct property_have_is_need_update :std::false_type{};
			template<typename T> struct property_have_is_need_update<T, std::void_t<decltype(std::declval<T>().is_need_update())>> : std::true_type {};


			template<typename T> class property_proxy_implement : public property_proxy_interface
			{
				T direct_block;
				typename T::renderer_data swap_lock;
				property_implement<typename T::renderer_data> renderer_lock;
				bool need_update = false;
			public:
				property_proxy_implement() : property_proxy_interface(typeid(T), typeid(decltype(*this)), typeid(typename T::renderer_data)) {}
				virtual property_interface& get_associate() { return renderer_lock; }
				operator T& () { return direct_block; }
				operator const T& () const { return direct_block; }

				virtual void logic_to_swap(creator& c)
				{
					need_update = direct_block.is_need_update();
					if (need_update)
					{
						direct_block.update(c, swap_lock);
						direct_block.finished_update();
					}
				}
				virtual void swap_to_renderer()
				{
					if (need_update)
					{
						static_cast<T::renderer_data&>(renderer_lock) = swap_lock;
						need_update = false;
					}
				}
				virtual void logic_to_renderer(creator& c)
				{
					need_update = false;
					if (direct_block.is_need_update())
					{
						direct_block.update(c, renderer_lock);
						direct_block.finished_update();
					}
				}
			};

			struct property_map
			{
				bool allready_update = true;

				// proxy_id
				std::map<std::type_index, std::shared_ptr<property_proxy_interface>> swap_mapping;

				// associate id
				std::map<std::type_index, std::shared_ptr<property_proxy_interface>> renderer_mapping;
				friend struct element_renderer_storage;
			public:
				template<typename F> bool find_associate(const std::type_index& ti, F&& f)
				{
					auto ite = renderer_mapping.find(ti);
					if (ite != renderer_mapping.end())
						return (f(ite->second->get_associate()), true);
					return false;
				}
				void swap_to_renderer();
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

			friend struct element_logic_storage;

			std::shared_ptr<Implement::property_map> inside_map;

		public:
			
			std::shared_ptr<Implement::property_map> map() const { return inside_map; }

			void logic_to_swap(creator& c);
			void logic_to_renderer(creator& c);

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

		class element_requirement;

		namespace Implement
		{
			class stage_resource
			{
			public:
				virtual bool apply_property(stage_context& sc, property_interface& pi);
				const element_requirement& requirement() const;
				virtual ~stage_resource();
			};

			shader_compute stage_load_cs(const std::u16string& p, creator& c);
			shader_vertex stage_load_vs(const std::u16string& path, creator& c);
			shader_pixel stage_load_ps(const std::u16string& path, creator& c);
		}

		class geometry_resource : public Implement::stage_resource
		{
		protected:
			primitive_topology primitive;
			layout_view view;
			raterizer_state state;
		public:
			layout_view ia_view() const { return view; }
			geometry_resource(creator& c, layout_view view = layout_view{}, std::optional<raterizer_state::description> state_description = {}, primitive_topology primitive = primitive_topology::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			void apply(stage_context& sc);
		};

		class compute_resource : public Implement::stage_resource
		{
		protected:
			std::u16string patch;
			shader_compute shader;
		public:
			void apply(stage_context& sc);
			compute_resource(creator& c, std::u16string cs_pacth);
		};

		class material_resource : public Implement::stage_resource
		{
		protected:
			std::u16string patch;
			shader_pixel shader;
			blend_state bs;
			std::type_index id_for_pipeline;
		public:
			void apply(stage_context& sc);
			const std::type_index& pipeline_id() const { return id_for_pipeline; }
			material_resource(creator& c, std::u16string ps_patch, std::optional<blend_state::description> description = {}, const std::type_index& pipeline_id = typeid(void));
		};

		class placement_resource : public Implement::stage_resource
		{
		protected:
			std::u16string patch;
			shader_vertex shader;
		public:
			void apply(stage_context& sc);
			placement_resource(creator& c, std::u16string ps_patch);
			const shader_vertex& get_vs_shader() const { return shader; }
		};

		class element_requirement
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
					if constexpr(std::is_same_v<stage_context, stage_context>)
						return pi.cast([&](type& de) {
						(*fun_ptr)(sc, de);
					});
					else
						return pi.cast([&](type& de) {
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

			static bool apply_property_implement(Implement::stage_resource&, stage_context&, const map_t::value_type& map_ite, Tool::stack_list<Implement::property_map>* sl);

		public:

			bool apply_property(Implement::stage_resource&, stage_context&, Tool::stack_list<Implement::property_map>* sl = nullptr) const;

			template<typename ...F>
			element_requirement(F&& ...f) : mapping({ make_mapping_pair(f)... }) {}
		};
		
		template<typename ...T> const element_requirement& make_element_requirement(T&&... t)
		{
			static element_requirement require{ t... };
			return require;
		}


		namespace Implement
		{

			class stage_ptr
			{
			public:
				virtual bool apply_property(stage_context& sc, Tool::stack_list<property_map>* sl = nullptr) = 0;
				virtual void apply_stage(stage_context& sc) = 0;
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
			public:
				virtual bool apply_property(stage_context& sc, Tool::stack_list<property_map>* sl) override final
				{
					const auto& ref = compute_t::requirement();
					return ref.apply_property(static_cast<stage_resource&>(*this), sc, sl);
				}
				void apply_stage(stage_context& sc) override final
				{
					compute_t::apply(sc);
				}
				compute_implement(creator& c) : compute_ptr(typeid(compute_t), typeid(decltype(*this))), compute_t(c) {}
			};
			template<typename T> using compute_implement_t = compute_implement<std::decay_t<T>>;

			template<typename placement_t> class placement_implement;

			class placement_ptr : public base_interface<placement_implement>,  public stage_ptr
			{
				friend struct element_draw_request;
			public:
				using base_interface<placement_implement>::base_interface;
				virtual const shader_vertex& get_vs_shader() const = 0;
			};

			template<typename placement_t>
			class placement_implement : public placement_ptr, public placement_t
			{
			public:
				virtual const shader_vertex& shader() const { return placement_t::get_vs_shader(); }
				virtual bool apply_property(stage_context& sc, Tool::stack_list<property_map>* sl) override final
				{
					const auto& ref = placement_t::requirement();
					return ref.apply_property(static_cast<stage_resource&>(*this), sc, sl);
				}
				void apply_stage(stage_context& sc) override final
				{
					placement_t::apply(sc);
				}
				virtual const shader_vertex& get_vs_shader() const override
				{
					return placement_t::get_vs_shader();
				}
				placement_implement(creator& c) : placement_ptr(typeid(placement_t), typeid(decltype(*this))), placement_t(c) { }
			};

			template<typename T> using placement_implement_t = placement_implement<std::decay_t<T>>;

			template<typename geometry_t> class geometry_implement;

			class geometry_ptr : public base_interface<geometry_implement>,  public stage_ptr
			{
				//virtual void apply(stage_context& sc) override final{}
			protected:
				std::map<std::type_index, input_layout> layout_map;
			public:
				virtual void apply_layout(stage_context& sc, placement_ptr&) = 0;
				using base_interface<geometry_implement>::base_interface;
			};

			template<typename geometry_t>
			class geometry_implement : public geometry_ptr, public geometry_t
			{
			public:

				virtual bool apply_property(stage_context& sc, Tool::stack_list<property_map>* sl) override final
				{
					const auto& ref = geometry_t::requirement();
					return ref.apply_property(static_cast<stage_resource&>(*this), sc, sl);
				}
				void apply_stage(stage_context& sc) override final
				{
					geometry_t::apply(sc);
				}

				virtual void apply_layout(stage_context& sc, placement_ptr& pp) override
				{
					auto ite = layout_map.find(pp.id());
					if (ite == layout_map.end())
					{
						input_layout tem;
						tem.create(sc, geometry_t::ia_view(), pp.get_vs_shader());
						layout_map.insert({ pp.id(), tem });
						sc << tem;
					}
					else
						sc << ite->second;
				}
				geometry_implement(creator& c) : geometry_ptr(typeid(geometry_t), typeid(decltype(*this))), geometry_t(c) {}
			};

			template<typename material_t> class material_implement;

			class material_ptr : public base_interface<material_implement>, public stage_ptr
			{
			public:
				virtual const std::type_index& pipeline() const = 0;
				using base_interface<material_implement>::base_interface;
			};

			template<typename material_t>
			class material_implement : public material_ptr, public material_t
			{
			public:

				const std::type_index& pipeline() const override final { return material_t::pipeline_id(); }
				virtual bool apply_property(stage_context& sc, Tool::stack_list<property_map>* sl) override final
				{
					const auto& ref = material_t::requirement();
					return ref.apply_property(static_cast<stage_resource&>(*this), sc, sl);
				}
				void apply_stage(stage_context& sc) override final
				{
					material_t::apply(sc);
				}

				material_implement(creator& c) : material_ptr(typeid(material_t), typeid(decltype(*this))), material_t(c) {}
			};
			template<typename T> using placement_implement_t = placement_implement<std::decay_t<T>>;
		}


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
				void swap_to_renderer() { mapping->swap_to_renderer(); }
				void dispatch(stage_context& sc, Tool::stack_list<property_map> * = nullptr);
			};

			struct element_draw_request
			{
				std::shared_ptr<placement_ptr> placemenet;
				std::shared_ptr<geometry_ptr> geometry;
				std::shared_ptr<material_ptr> material;
				std::shared_ptr<property_map> mapping;
				void swap_to_renderer() { mapping->swap_to_renderer(); }
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
			void logic_to_swap (element_swap_block& esb, creator& c);
		};


		struct element_renderer_storage
		{
			std::vector<Implement::element_dispatch_request> dispatch_request;
			std::unordered_map<std::type_index, std::vector<Implement::element_draw_request>> draw_request;
			void swap_to_renderer(element_swap_block& esb, stage_context& sc);
		};

		class pipeline_interface
		{
			const std::type_index type_info;
		public:
			property_proxy_map property_mapping;
			pipeline_interface(const std::type_index& ti);
			const std::type_index& id() const { return type_info; }
			virtual ~pipeline_interface();
			void execute(stage_context& sc, element_renderer_storage& ers, Tool::stack_list<Implement::property_map>* pml = nullptr);
			virtual void execute_implement(stage_context& sc, element_renderer_storage& ers, Tool::stack_list<Implement::property_map>* pml = nullptr) = 0;
		};

	}
}