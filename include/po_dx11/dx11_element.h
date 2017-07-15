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
			virtual auto acception() -> const acception_t& const = 0;
			virtual bool construct(property_storage&, creator&) = 0;
		};

		class property_mapping
		{

			std::map<std::type_index, std::shared_ptr<property_storage>> mapping;
			friend struct element_implement;

		public:

			bool insert(std::shared_ptr<property_storage> sp);

			property_storage& create(const typename property_interface::acception_t::value_type&);
			property_storage& re_create(const typename property_interface::acception_t::value_type&);

			bool create_and_construct(const typename property_interface::acception_t::value_type& vt, )

			void clear();

			template<typename T> bool have() const { return have(typeid(T)); }
			bool have(std::type_index) const;

			template<typename F> void for_each(F&& func)
			{
				for (auto& f : mapping) func(f);
			}
			
			void update(pipeline& p);
		};

		namespace Implement
		{
			class stage_interface : public base_interface
			{
			protected:
				property_mapping default_mapping;
			public:
				using Implement::base_interface::base_interface;
				using acceptance_t = std::set<std::type_index>;

				virtual bool update(property_interface&, pipeline&);
				virtual auto acceptance() -> const acceptance_t& const;
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
			using Implement::stage_interface::base_interface;
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
		public:
			using Implement::stage_interface::base_interface;

			virtual void apply(pipeline&);
			virtual const requirement_t& requirement() const = 0;
			virtual void draw(pipeline&) = 0;
		};

		class material_interface : public Implement::stage_interface
		{
			std::u16string path;
		protected:
			pixel_stage stage_ps;
			bool load_ps(std::u16string p, creator& c);
		public:
			using Implement::stage_interface::base_interface;
			virtual void apply(pipeline&);
		};

		class compute_interface : public Implement::stage_interface
		{
			std::u16string path;
		protected:
			compute_stage stage_cs;
			bool load_cs(std::u16string p, creator& c);
		public:
			using Implement::stage_interface::base_interface;
			virtual void apply(pipeline&);
			virtual void dispath(pipeline& p) = 0;
		};

		namespace Implement
		{
			struct element_implmenet
			{
				std::vector<std::shared_ptr<compute_interface>> compute_vector;
				std::shared_ptr<geometry_interface> geometry_ptr;
				std::shared_ptr<material_interface> material_interface;
				std::shared_ptr<placement_interface> placement_interface;
				property_mapping mapping;

				bool construct_imp(property_interface&, creator& c);
				bool re_construct_imp(property_interface&, creator& c);

			public:

				template<typename F> bool construct(creator& c, F&& f) {

					using funtype = Tmp::pick_func<typename Tmp::degenerate_func<Tmp::extract_func_t<F>>::type>;
					static_assert(funtype::size == 1, "only receive one parameter");
					using true_type = std::decay_t<typename funtype::template out<Tmp::itself>::type>;
					static_assert(std::is_base_of<property_interface, true_type>::value, "property need derived form property_interface.");

					true_type temporary;
					f(temporary);

					return construct_imp(temporary, c);
				}

				template<typename T, typename ...AT> bool re_construct(AT&& ...at)
				{
					using funtype = Tmp::pick_func<typename Tmp::degenerate_func<Tmp::extract_func_t<F>>::type>;
					static_assert(funtype::size == 1, "only receive one parameter");
					using true_type = std::decay_t<typename funtype::template out<Tmp::itself>::type>;
					static_assert(std::is_base_of<property_interface, true_type>::value, "property need derived form property_interface.");

					true_type temporary;
					f(temporary);

					return re_construct_imp(temporary, c);
				}

			};
		}

		class element
		{
			std::shared_ptr<Implement::element_implmenet> element_ptr;
		public:

		};

	}
}