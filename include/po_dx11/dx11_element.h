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

		namespace Implement
		{
			class base_interface
			{
				std::type_index id_info;
			public:
				base_interface(std::type_index ti);
				virtual ~base_interface();
				std::type_index id() const { return id_info; }
				template<typename T> bool is() const { return id_info == typeid(T); }
				template<typename T> T& cast() { return static_cast<T&>(*this); }
				template<typename Function> bool cast(Function&& f)
				{
					using funtype = Tmp::pick_func<typename Tmp::degenerate_func<Tmp::extract_func_t<Function>>::type>;
					static_assert(funtype::size == 1, "only receive one parameter");
					using true_type = std::decay_t<typename funtype::template out<Tmp::itself>::type>;
					static_assert(std::is_base_of<base_interface, true_type>::value, "need derived form base_interface.");
					if (is<true_type>())
						return (f(cast<true_type>()), true);
					return false;
				}
			};
		}

		class property_interface : public Implement::base_interface
		{
		protected:
			std::function<void(pipeline& p)> update_function;
		public:
			using Implement::base_interface::base_interface;
			void update(pipeline& p);
		};

		class property_constructor : public Implement::base_interface
		{
		public:
			using construction_t = std::map<std::type_index, std::function<std::shared_ptr<property_interface>()>>;
		protected:
			template<typename T> static auto make_construction_t() -> construction_t::value_type {
				using type = std::decay_t<T>;
				static_assert(std::is_base_of<property_interface, type>::value, "acception need derived form property_interface");
				return typename construction_t::value_type{ typeid(type), []()->std::shared_ptr<property_interface> {return std::make_shared<type>()}; };
			}
		public:
			using Implement::base_interface::base_interface;
			virtual auto construction()const -> const construction_t&  = 0;
			virtual bool construct(property_interface&, creator&) = 0;
		};

		class property_mapping
		{

			std::map<std::type_index, std::shared_ptr<property_interface>> mapping;
			friend struct element_implement;

		public:

			bool insert(std::shared_ptr<property_interface> sp);

			property_interface& create(const typename property_constructor::construction_t::value_type&);
			property_interface& recreate(const typename property_constructor::construction_t::value_type&);
			bool create_and_construct(const typename property_constructor::construction_t::value_type&, property_constructor&, creator& c);
			bool recreate_and_construct(const typename property_constructor::construction_t::value_type&, property_constructor&, creator& c);
			void clear();

			template<typename F, typename F2> bool make_constructor(creator& cr, F&& f, F2&& f2)
			{
				using funtype = Tmp::pick_func<typename Tmp::degenerate_func<Tmp::extract_func_t<F>>::type>;
				static_assert(funtype::size == 1, "only receive one parameter");
				using true_type = std::decay_t<typename funtype::template out<Tmp::itself>::type>;
				static_assert(std::is_base_of<property_constructor, true_type>::value, "need derived form property_constructor.");
				bool final_state = false;
				true_type tt;
				f(tt);
				for (auto& con : tt.construct())
				{
					bool state = f2(con.first);
					final_state = final_state || state;
					if(state)
						create_and_construct(con, tt, cr);
				}
				return state;
			}

			template<typename F> void make_interface(F&& f)
			{
				using funtype = Tmp::pick_func<typename Tmp::degenerate_func<Tmp::extract_func_t<F>>::type>;
				static_assert(funtype::size == 1, "only receive one parameter");
				using true_type = std::decay_t<typename funtype::template out<Tmp::itself>::type>;
				static_assert(std::is_base_of<property_interface, true_type>::value, "need derived form property_interface.");

				auto ite = mapping.find(typeid(true_type));
				if (ite != mapping.end())
				{
					f(ite->second->cast<true_type>());
				}
				else {
					auto ptr = std::make_shared<true_type>();
					f(*ptr);
					auto id = ptr->id();
					mapping.insert({ id, std::move(ptr) });
				}
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
			
			void update(pipeline& p);
		};

		namespace Implement
		{
			class stage_interface : public base_interface
			{
			public:
				using Implement::base_interface::base_interface;
				bool update_implement(pipeline&, property_mapping&, property_mapping& pm);
				bool update_implement(pipeline&, property_mapping&);
				using acceptance_t = std::set<std::type_index>;
			protected:
				property_mapping default_mapping;
				virtual bool update(property_interface&, pipeline&);

				template<typename ...AT> struct make_acceptance
				{
					operator const acceptance_t&() const { 
						static const acceptance_t acceptance{ typeid(AT)... };
						return acceptance; 
					}
				};
			public:

				bool check(const property_mapping& pm) const;
				bool check(const property_mapping& pm, const property_mapping& pm2) const;

				acceptance_t lack_acceptance(const property_mapping& pm) const;
				acceptance_t lack_acceptance(const property_mapping& pm, const property_mapping& pm2) const;

				virtual auto acceptance() const -> const acceptance_t&;
				virtual void init(creator&) = 0;
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
			virtual void apply(pipeline&);
			const vertex_stage& vs()const { return stage_vs; }
		};

		class geometry_interface : public Implement::stage_interface
		{
		protected:
			input_assember_stage stage_ia;
			raterizer_state stage_rs;
		public:
			using Implement::stage_interface::stage_interface;
			virtual void apply(pipeline&);
			virtual void draw(pipeline&) = 0;
			const input_assember_stage& ia()const { return stage_ia; }
		};

		class material_interface : public Implement::stage_interface
		{
			std::u16string path;
		protected:
			pixel_stage stage_ps;
			blend_state stage_bs;
			bool load_ps(std::u16string p, creator& c);
		public:
			using Implement::stage_interface::stage_interface;
			virtual void apply(pipeline&);
		};

		class compute_interface : public Implement::stage_interface
		{
			std::u16string path;
		protected:
			compute_stage stage_cs;
			bool load_cs(std::u16string p, creator& c);
		public:
			using Implement::stage_interface::stage_interface;
			virtual void apply(pipeline&);
			virtual void dispath(pipeline& p) = 0;
		};

		namespace Implement
		{

			template<typename T, typename K = void> struct element_implement_type_holder
			{
				using type_1 = std::decay_t<T>;
				using type_2 = std::decay_t<K>;
			};

			struct element_implement
			{
				std::vector<std::shared_ptr<compute_interface>> compute_vector;
				std::shared_ptr<placement_interface> placemenet_ptr;
				std::shared_ptr<geometry_interface> geometry_ptr;
				std::shared_ptr<material_interface> material_ptr;
				input_layout layout;
				property_mapping mapping;

				bool construct_imp(property_constructor&, creator& c);
				bool reconstruct_imp(property_constructor&, creator& c);

				void clear_property() { mapping.clear(); }
				void clear_unuesd_property();
				void clear_compute() { compute_vector.clear(); }
				void clear_all();

				void set(std::shared_ptr<geometry_interface> gp, std::shared_ptr<placement_interface> pp, input_layout lay);
				void set(std::shared_ptr<material_interface> mp);
				void set(std::shared_ptr<compute_interface> cp);

				bool check() const;
				bool check(const property_mapping& pm) const;

				Implement::stage_interface::acceptance_t lack_acceptance(const property_mapping& pm) const;
				Implement::stage_interface::acceptance_t lack_acceptance() const;

				bool insert(std::shared_ptr<property_interface> sp) { return mapping.insert(std::move(sp)); }

				template<typename F> bool make_constructor(creator& c, F&& f) {
					return mapping.make_constructor(c, f, [this](const std::type_index& ti) {
						for (auto& com : compute_vector)
							if (com->acceptance().find(ti) != com->acceptance().end())
								return true;
						if (material_ptr)
							if (material_ptr->acceptance().find(ti) != material_ptr->acceptance().end())
								return true;
						if (geometry_ptr)
						{
							if (geometry_ptr->acceptance().find(ti) != geometry_ptr->acceptance().end())
								return true;
							if (geometry_ptr->acceptance_placement().find(ti) != geometry_ptr->acceptance_placement().end())
								return true;
						}
						return false;
					});
				}

				template<typename F> void make_interface(F&& f) { mapping.make_interface(f); }

				template<typename F> decltype(auto) make_geometry_and_placement(creator& c, F&& f)
				{
					using funtype = Tmp::pick_func<typename Tmp::degenerate_func<Tmp::extract_func_t<F>>::type>;
					static_assert(funtype::size == 2, "only receive two parameter");
					using true_type = typename funtype::template out<element_implement_type_holder>;
					using t1 = typename true_type::type_1;
					using t2 = typename true_type::type_2;
					static_assert(
						std::is_base_of<geometry_interface, typename true_type::type_1>::value && std::is_base_of<placement_interface, typename true_type::type_2>::value,
						"first parameter should derived form geometry_interface and the other should derived form placement_interface"
						);

					bool reflesh_layout = false;
					if (!geometry_ptr || geometry_ptr->id() != typeid(t1))
					{
						reflesh_layout = true;
						geometry_ptr = std::make_shared<t1>();
						geometry_ptr->init(c);
					}
					if (!placemenet_ptr || placemenet_ptr->id() != typeid(t2))
					{
						reflesh_layout = true;
						placemenet_ptr = std::make_shared<t2>();
						placemenet_ptr->init(c);
					}
					if (reflesh_layout)
						layout = c.create_layout(geometry_ptr->ia(), placemenet_ptr->vs());
					return f(geometry_ptr->cast<t1>(), placemenet_ptr->cast<t2>());
				}

				template<typename F> decltype(auto) make_material(creator& c, F&& f)
				{
					using funtype = Tmp::pick_func<typename Tmp::degenerate_func<Tmp::extract_func_t<F>>::type>;
					static_assert(funtype::size == 1, "only receive one parameter");
					using true_type = typename funtype::template out<element_implement_type_holder>::t1;
					static_assert(std::is_base_of<material_interface, true_type>::value, "material need derived form material_interface.");

					if (!material_ptr || (material_ptr && material_ptr->id() != typeid(true_type))){
						material_ptr = std::make_shared<true_type>();
						material_ptr->init(c);
					}
					return f(material_ptr->cast<true_type>());
				}

				template<typename F> decltype(auto) push_compute(creator& c, F&& f)
				{
					using funtype = Tmp::pick_func<typename Tmp::degenerate_func<Tmp::extract_func_t<F>>::type>;
					static_assert(funtype::size == 1, "only receive one parameter");
					using true_type = typename funtype::template out<element_implement_type_holder>::t1;
					static_assert(std::is_base_of<compute_interface, true_type>::value, "compute need derived form compute_interface.");

					std::shared_ptr<compute_interface> ite;
					for(auto& ptr : compute_vector)
						if (ptr->is<true_type>())
						{
							ite = ptr;
							break;
						}
					if (!ite)
						ite = std::static_pointer_cast<compute_interface>(std::make_shared<true_type>());
					ite->init(c);
					compute_vector.push_back(ite);
					return f(ite->cast<true_type>());
				}

				void draw(pipeline& p);
				void draw(pipeline& p, property_mapping& mapping);

			};
		}

		class element
		{
			std::shared_ptr<Implement::element_implement> element_ptr;
			void check_ptr() {
				if (!element_ptr)
					element_ptr = std::make_shared<Implement::element_implement>();
			}
		public:
			void draw(pipeline& p);
			void draw(pipeline& p, property_mapping& mapping);

			bool check() const;
			bool check(const property_mapping& pm) const;

			Implement::stage_interface::acceptance_t lack_acceptance() const;
			Implement::stage_interface::acceptance_t lack_acceptance(const property_mapping& pm) const;

			template<typename T> void set(std::shared_ptr<T> t) {
				check_ptr();
				element_ptr->set(std::move(t));
			}
			
			template<typename F> void make_interface(F&& f) { 
				check_ptr();
				element_ptr->make_interface(f);
			}
			template<typename F> void make_constructor(creator& c, F&& f) { 
				check_ptr();
				element_ptr->make_interface(c, f);
			}
			template<typename F> decltype(auto) make_geometry_and_placement(creator& c, F&& f) {
				check_ptr();
				return element_ptr->make_geometry_and_placement(c, std::forward<F>(f));
			}
			template<typename F> decltype(auto) make_material(creator& c, F&& f) {
				check_ptr();
				return element_ptr->make_material(c, std::forward<F>(f));
			}
			template<typename F> decltype(auto) push_compute(creator& c, F&& f) {
				check_ptr();
				return element_ptr->push_compute(c, std::forward<F>(f));
			}
		};

	}
}