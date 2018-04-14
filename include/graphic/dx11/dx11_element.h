#pragma once
#include "dx11_frame.h"
#include <typeindex>
#include <set>
#include <memory>
#include "../../po/po.h"
/*
修复建议：
1，xxx_resource 的继承限制应该被去除。
2，render_data 和update()的实现的限制也应该被去除，应该设置成可选的。
3，尝试使用ECS来实现element模型，既Element_Draw里边不再储存shader或者property的指针，转而储存ID。
4,element_requirement的stage_context&的参数也应该去除，如果可能的话，转而将资源打包返回。
*/




namespace PO
{
	namespace Dx11
	{

		void add_shader_path(std::u16string path);

		class property_proxy_map;

		template<typename T, size_t i> struct indexed_property : T{};

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

		namespace Implement
		{
			class property_interface : public Implement::base_interface<Implement::property_implement>
			{
			public:
				using Implement::base_interface<Implement::property_implement>::base_interface;
				virtual ~property_interface();
			};

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

				bool dirty = true;

			protected:

				bool is_dirty() const { return dirty; }
				void finish_update() { dirty = false; }

			public:

				void need_update() { dirty = true; }

				const std::type_index& associate_id() const { return associate_info; }
				property_proxy_interface(const std::type_index& original, const std::type_index& real, const std::type_index& asso);
				virtual ~property_proxy_interface();
			};

			template<typename T> using find_renderer_data = typename T::renderer_data;
			template<typename T> using find_renderer_data_append = typename T::renderer_data_append;

			template<typename T> struct property_with_renderer_data : public T::renderer_data
			{
			private:
				friend class property_proxy_implement<T>;
				void update_from(creator& c, T& t) { t.update(c, static_cast<typename T::renderer_data&>(*this)); }
			};

			template<typename T> struct property_with_renderer_data_append : public T, public T::renderer_data_append
			{
			private:
				friend class property_proxy_implement<T>;
				void update_from(creator& c, T& t) {
					static_cast<T&>(*this) = t;
					t.update(c, static_cast<typename T::renderer_data_append&>(*this));
				}
			};

			template<typename T> struct property_with_nothing : public T
			{
			private:
				friend class property_proxy_implement<T>;
				void update_from(creator& c, T& t) {
					static_cast<T&>(*this) = t;
				}
			};

			template<typename T, bool rd, bool rda> struct property_wrapper_implement;
			template<typename T, bool rda> struct property_wrapper_implement<T, true, rda> { using type = property_with_renderer_data<T>; };
			template<typename T> struct property_wrapper_implement<T, false, true> { using type = property_with_renderer_data_append<T>; };
			template<typename T> struct property_wrapper_implement<T, false, false> { using type = property_with_nothing<T>; };
		}

		template<typename T> using property_wrapper_t = typename Implement::property_wrapper_implement<T, 
			Tmp::able_instance<Implement::find_renderer_data, T>::value, 
			Tmp::able_instance<Implement::find_renderer_data_append, T>::value
		>::type;

		namespace Implement
		{

			template<typename T> struct property_proxy_wrapper_implement {}; 
			template<typename T> struct property_proxy_wrapper {};

			template<typename T> class property_proxy_implement : public property_proxy_interface
			{
				T direct_block;
				property_wrapper_t<T> swap_lock;
				property_implement<property_wrapper_t<T>> renderer_lock;
				bool swap_ready = true;
			public:
				property_proxy_implement() : property_proxy_interface(typeid(T), typeid(decltype(*this)), typeid(property_wrapper_t<T>)) {}
				virtual property_interface& get_associate() { return renderer_lock; }
				operator T& () { return direct_block; }
				operator const T& () const { return direct_block; }

				virtual void logic_to_swap(creator& c)
				{
					if (property_proxy_interface::is_dirty())
					{
						swap_ready = true;
						swap_lock.update_from(c, direct_block);
						property_proxy_interface::finish_update();
					}
				}
				virtual void swap_to_renderer()
				{
					if (swap_ready)
					{
						static_cast<property_wrapper_t<T>&>(renderer_lock) = swap_lock;
						swap_ready = false;
					}
				}
				virtual void logic_to_renderer(creator& c)
				{
					swap_ready = false;
					if (property_proxy_interface::is_dirty())
					{
						renderer_lock.update_from(c, direct_block);
						property_proxy_interface::finish_update();
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
				using parameter_type = typename funtype::template out<Tmp::itself>::type;
				static_assert(std::is_reference_v<parameter_type>, "parameter should be a reference");
				using true_type = std::decay_t<parameter_type>;
				static constexpr bool is_const = std::is_const_v<parameter_type>;
			};
			template<typename T> using check_t = typename check<T>::true_type;

			friend struct element_compute_logic_storage;
			friend struct element_draw_logic_storage;

			std::shared_ptr<Implement::property_map> inside_map;

		public:
			
			template<typename T>
			bool shared_property_to(property_proxy_map& ppm) { return shared_property_to(typeid(T), ppm); }

			bool shared_property_to(const std::type_index& id, property_proxy_map& ppm) const;

			std::shared_ptr<Implement::property_map> map() const { return inside_map; }

			void logic_to_swap(creator& c);
			void logic_to_renderer(creator& c);

			property_proxy_map() : inside_map(std::make_shared<Implement::property_map>()) {}

			template<typename F> property_proxy_map& operator<<(F&& f)
			{
				using type = check_t<F>;
				auto ite = mapping.find(typeid(type));
				if (ite != mapping.end())
				{
					if constexpr(check<F>::is_const)
					{
						f(ite->second->cast<type>());
					}
					else {
						ite->second->need_update();
						f(ite->second->cast<type>());
					}
				}
				else {
					auto ptr = std::make_shared<Implement::property_proxy_implement<type>>();
					ptr->need_update();
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
		public:
			const depth_stencil_state& replace_depth_stencil_state(const depth_stencil_state& dss) const { return dss; }
			void apply(stage_context& sc);
			material_resource(creator& c, std::u16string ps_patch, std::optional<blend_state::description> description = {});
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
				static bool update_requirement(stage_context& sc, Implement::property_interface& pi, void* data)
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

			using func_type = std::tuple<bool(*)(stage_context&, Implement::property_interface&, void*), void*>;
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
						tem.create(sc.dev, geometry_t::ia_view(), pp.get_vs_shader());
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
				virtual void apply_depth_stencil_state(stage_context& sc, const depth_stencil_state& dss) = 0;
				using base_interface<material_implement>::base_interface;
			};

			template<typename material_t>
			class material_implement : public material_ptr, public material_t
			{
			public:
				virtual void apply_depth_stencil_state(stage_context& sc, const depth_stencil_state& dss) override
				{
					sc << material_t::replace_depth_stencil_state(dss);
				}
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
			creator tool;

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

		namespace Implement
		{
			struct element_draw_implement
			{
				std::shared_ptr<placement_ptr> placement;
				std::shared_ptr<geometry_ptr> geometry;
				std::shared_ptr<material_ptr> material;
				property_proxy_map mapping;
				element_draw_implement& operator<<(std::shared_ptr<placement_ptr> ptr) { placement = std::move(ptr); return *this; }
				element_draw_implement& operator<<(std::shared_ptr<geometry_ptr> ptr) { geometry = std::move(ptr); return *this; }
				element_draw_implement& operator<<(std::shared_ptr<material_ptr> ptr) { material = std::move(ptr); return *this; }
			};

			struct element_compute_implment
			{
				std::shared_ptr<compute_ptr> compute;
				std::function<void(stage_context&)> back_task;
				std::function<bool(property_proxy_map&)> need_continue;
				property_proxy_map mapping;
				element_compute_implment& operator<<(std::shared_ptr<compute_ptr> ptr) { compute = std::move(ptr); return *this; }
				void set_task(std::function<void(stage_context&)> f) { back_task = std::move(f); }
				void set_continue(std::function<bool(property_proxy_map&)> f) { need_continue = std::move(f); }
			};

			struct element_dispatch_request
			{
				std::shared_ptr<compute_ptr> compute;
				std::function<void(stage_context&)> back_task;
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
				//void draw(stage_context& sc, Tool::stack_list<property_map> * = nullptr);
				void draw(stage_context& sc, const depth_stencil_state& ss = {}, Tool::stack_list<property_map> * = nullptr);
			};

		}

		struct element_draw
		{
			std::shared_ptr<Implement::element_draw_implement> ptr;
		public:
			element_draw() : ptr(std::make_shared<Implement::element_draw_implement>()) {}
			template<typename T> element_draw& operator<< (std::shared_ptr<T> t) { *ptr << std::move(t);  return *this; }
			template<typename T> element_draw& operator<< (T&& t) { ptr->mapping << t;  return *this; }
		};

		struct element_compute
		{
			std::shared_ptr<Implement::element_compute_implment> ptr;
		public:
			element_compute() : ptr(std::make_shared<Implement::element_compute_implment>()) {}
			void set_task(std::function<void(stage_context&)> f) { ptr->set_task(std::move(f)); }
			void set_continue(std::function<bool(property_proxy_map&)> f) { ptr->set_continue(std::move(f)); }
			template<typename T> element_compute& operator<< (std::shared_ptr<T> t) { *ptr << std::move(t);  return *this; }
			template<typename T> element_compute& operator<< (T&& t) { ptr->mapping << t;  return *this; }
		};

		struct element_compute_swap_block
		{
			std::mutex swap_lock;
			std::vector<Implement::element_dispatch_request> dispatch_request;
		};

		struct element_draw_swap_block
		{
			std::mutex swap_lock;
			std::vector<Implement::element_draw_request> draw_request;
		};

		struct element_compute_logic_storage
		{

			std::mutex swap_mutex;
			
			std::vector<std::shared_ptr<Implement::element_compute_implment>> element_compute_store;
			
			element_compute_logic_storage& operator<< (const element_compute& ele) { if (ele.ptr) element_compute_store.push_back(ele.ptr); return *this; }
			void logic_to_swap (element_compute_swap_block& esb, creator& c);
		};

		struct element_draw_logic_storage
		{
			std::mutex swap_mutex;
			std::vector<std::shared_ptr<Implement::element_draw_implement>> element_draw_store;
			element_draw_logic_storage& operator<< (const element_draw& ele) { if (ele.ptr) element_draw_store.push_back(ele.ptr); return *this; }
			void logic_to_swap(element_draw_swap_block& esb, creator& c);
		};

		struct element_compute_renderer_storage
		{
			std::vector<Implement::element_dispatch_request> dispatch_request;
			void swap_to_renderer(element_compute_swap_block& esb, stage_context& sc);
		};

		struct element_draw_renderer_storage
		{
			std::vector<Implement::element_draw_request> draw_request;
			void swap_to_renderer(element_draw_swap_block& esb, stage_context& sc);
		};

		struct element_draw_storage
		{
			element_draw_swap_block swap;
			element_draw_logic_storage logic;
			element_draw_renderer_storage renderer;
			void logic_to_swap(creator& c) {  logic.logic_to_swap(swap, c);  }
			void swap_to_renderer(stage_context& sc) { renderer.swap_to_renderer(swap, sc); }
		};

		struct element_compute_storage
		{
			element_compute_swap_block swap;
			element_compute_logic_storage logic;
			element_compute_renderer_storage renderer;
			void logic_to_swap(creator& c) { logic.logic_to_swap(swap, c); }
			void swap_to_renderer(stage_context& sc) { renderer.swap_to_renderer(swap, sc); }
		};

	}

	
}