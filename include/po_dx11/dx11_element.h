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

		class property_storage : public Implement::base_interface
		{
			bool need_to_be_update = false;
			virtual void update_imp(pipeline& p);
		protected:
			void need_update() { need_to_be_update = true; }
		public:
			using Implement::base_interface::base_interface;
			void update(pipeline& p);
		};

		class property_interface : public Implement::base_interface
		{
		public:
			using acception_t = std::map<std::type_index, std::function<std::shared_ptr<property_storage>()>>;
		protected:
			template<typename T> static auto make_acception_t() -> acception_t::value_type {
				using type = std::decay_t<T>;
				static_assert(std::is_base_of<property_storage, type>::value, "acception need derived form property_storage");
				return typename acception_t::value_type{ typeid(type), []()->std::shared_ptr<property_storage> {return std::make_shared<type>()}; };
			}
		public:
			using Implement::base_interface::base_interface;
			virtual auto acception()const -> const acception_t&  = 0;
			virtual bool construct(property_storage&, creator&) = 0;
		};

		class property_mapping
		{

			std::map<std::type_index, std::shared_ptr<property_storage>> mapping;
			friend struct element_implement;

		public:

			bool insert(std::shared_ptr<property_storage> sp);

			property_storage& create(const typename property_interface::acception_t::value_type&);
			property_storage& recreate(const typename property_interface::acception_t::value_type&);
			bool create_and_construct(const typename property_interface::acception_t::value_type&, property_interface&, creator& c);
			bool recreate_and_construct(const typename property_interface::acception_t::value_type&, property_interface&, creator& c);
			void clear();

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
					return f(*(ite->second));
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
				bool update(pipeline&, property_mapping&, property_mapping& pm);
				bool update(pipeline&, property_mapping&);
			protected:
				property_mapping default_mapping;
				virtual bool update_imp(property_storage&, pipeline&);
			public:
				using acceptance_t = std::set<std::type_index>;

				virtual bool check(const property_mapping& pm) const;
				virtual bool check(const property_mapping& pm, const property_mapping& pm2) const;

				virtual auto acceptance() const -> const acceptance_t&;
				virtual void init(creator&) = 0;
			};
		}

		class placement_interface : public Implement::stage_interface
		{
			std::u16string path;
		protected:
			vertex_stage stage_vs;
			bool load_vs(std::u16string path, creator& c);
		public:
			using Implement::stage_interface::stage_interface;
			virtual void apply(pipeline&);
		};

		class geometry_interface : public Implement::stage_interface
		{
		protected:
			input_assember_stage stage_ia;
			raterizer_state stage_rs;
			using requirement_t = std::tuple<std::type_index, std::function<std::shared_ptr<placement_interface>()>>;
			template<typename T> static requirement_t make_requirement() {
				using type = std::decay_t<T>;
				static_assert(std::is_base_of<placement_interface, type>::value, "requirement need derived form placement_interface");
				return requirement_t{ typeid(type), []()->std::shared_ptr<placement_interface> {return std::make_shared<type>()}; };
			}
			std::shared_ptr<placement_interface> placement_ptr;
		public:
			using Implement::stage_interface::stage_interface;

			bool update_placement(pipeline& p, property_mapping& pm);
			bool update_placement(pipeline& p, property_mapping& pm, property_mapping& pm2);
			auto acceptance_placement() const -> const acceptance_t&;
			void apply_placement(pipeline& p);
			bool check(const property_mapping& pm) const override;
			bool check(const property_mapping& pm, const property_mapping& pm2) const override;

			virtual bool set_placement(std::shared_ptr<placement_interface> ptr);
			virtual void apply(pipeline&);
			virtual auto requirement() const -> const requirement_t & = 0;
			virtual void draw(pipeline&) = 0;
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
			struct element_implement
			{
				std::vector<std::shared_ptr<compute_interface>> compute_vector;
				std::shared_ptr<geometry_interface> geometry_ptr;
				std::shared_ptr<material_interface> material_ptr;
				property_mapping mapping;

				bool construct_imp(property_interface&, creator& c);
				bool reconstruct_imp(property_interface&, creator& c);

				void clear_property() { mapping.clear(); }
				void clear_unuesd_property();
				void clear_compute() { compute_vector.clear(); }
				void clear_all();

				element_implement& operator=(std::shared_ptr<geometry_interface> p);
				element_implement& operator=(std::shared_ptr<material_interface> p);
				element_implement& operator=(std::shared_ptr<compute_interface> p);

				bool check() const;
				bool check(const property_mapping& pm) const;

				bool insert(std::shared_ptr<property_storage> sp) { return mapping.insert(std::move(sp)); }

				template<typename F> bool construct(creator& c, F&& f) {

					using funtype = Tmp::pick_func<typename Tmp::degenerate_func<Tmp::extract_func_t<F>>::type>;
					static_assert(funtype::size == 1, "only receive one parameter");
					using true_type = std::decay_t<typename funtype::template out<Tmp::itself>::type>;
					static_assert(std::is_base_of<property_interface, true_type>::value, "property need derived form property_interface.");

					true_type temporary;
					f(temporary);

					return construct_imp(temporary, c);
				}

				template<typename T, typename ...AT> bool reconstruct(AT&& ...at)
				{
					using funtype = Tmp::pick_func<typename Tmp::degenerate_func<Tmp::extract_func_t<F>>::type>;
					static_assert(funtype::size == 1, "only receive one parameter");
					using true_type = std::decay_t<typename funtype::template out<Tmp::itself>::type>;
					static_assert(std::is_base_of<property_interface, true_type>::value, "property need derived form property_interface.");

					true_type temporary;
					f(temporary);

					return re_construct_imp(temporary, c);
				}

				template<typename F> decltype(auto) make_geometry(F&& f)
				{
					using funtype = Tmp::pick_func<typename Tmp::degenerate_func<Tmp::extract_func_t<F>>::type>;
					static_assert(funtype::size == 1, "only receive one parameter");
					using true_type = std::decay_t<typename funtype::template out<Tmp::itself>::type>;
					static_assert(std::is_base_of<geometry_interface, true_type>::value, "geometry need derived form geometry_interface.");

					if (!geometry_ptr || geometry_ptr->id() != typeid(true_type)) {
						geometry_ptr = std::make_shared<true_type>();
						auto placement_ptr = std::get<1>(geometry_ptr->requirement())();
						geometry_ptr->set_placement(std::move(placement_ptr));
					}
					return f(geometry_ptr->cast<true_type>());
				}

				template<typename F> decltype(auto) make_material(F&& f)
				{
					using funtype = Tmp::pick_func<typename Tmp::degenerate_func<Tmp::extract_func_t<F>>::type>;
					static_assert(funtype::size == 1, "only receive one parameter");
					using true_type = std::decay_t<typename funtype::template out<Tmp::itself>::type>;
					static_assert(std::is_base_of<material_interface, true_type>::value, "material need derived form material_interface.");

					if (!material_ptr || material_ptr->id() != typeid(true_type)) {
						material_ptr = std::make_shared<true_type>();
					}
					return f(material_ptr->cast<true_type>());
				}

				template<typename F> decltype(auto) push_compute(F&& f)
				{
					using funtype = Tmp::pick_func<typename Tmp::degenerate_func<Tmp::extract_func_t<F>>::type>;
					static_assert(funtype::size == 1, "only receive one parameter");
					using true_type = std::decay_t<typename funtype::template out<Tmp::itself>::type>;
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
		public:
			void draw(pipeline& p);
			void draw(pipeline& p, property_mapping& mapping);
			template<typename F> decltype(auto) make_geometry(F&& f) {
				if (!element_ptr)
					element_ptr = std::shared_ptr<Implement::element_implmenet>();
				return element_ptr->make_geometry(std::forward<F>(f));
			}
			template<typename F> decltype(auto) make_material(F&& f) {
				if (!element_ptr)
					element_ptr = std::shared_ptr<Implement::element_implmenet>();
				return element_ptr->make_material(std::forward<F>(f));
			}
			template<typename F> decltype(auto) push_compute(F&& f) {
				if (!element_ptr)
					element_ptr = std::shared_ptr<Implement::element_implmenet>();
				return element_ptr->push_compute(std::forward<F>(f));
			}
		};

	}
}