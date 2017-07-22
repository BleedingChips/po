#pragma once
#include "../po_dx11/dx11_element.h"
#include <set>
#include <typeindex>
#include <memory>
namespace PO
{
	namespace Dx11
	{

		enum class render_order
		{
			Defer = 0,
			Forward,
			Post,
			Transparent,
			Max,
			NotSet,
		};

		class defer_material_interface : public material_interface
		{
			render_order order_;
		protected:
			
		public:
			defer_material_interface(std::type_index ti, render_order ro);
			render_order order() const { return order_; }
		};

		namespace Implement
		{
			struct defer_element_implement : public element_implement
			{
				render_order order_ = render_order::NotSet;
				render_order order() const { return order_; }
				void set(std::shared_ptr<defer_material_interface> p);
				void clear_all();
			};
		}

		class defer_element
		{
			std::shared_ptr<Implement::defer_element_implement> element_ptr;
			friend class interface_storage;
			friend class defer_element_implement_storage;
			void check_ptr();
		public:
			void clear_all();
			bool check(const property_mapping& pm) const;
			Implement::stage_interface::acceptance_t lack_acceptance() const;
			Implement::stage_interface::acceptance_t lack_acceptance(const property_mapping& pm) const;
			template<typename F> void make_interface(F&& f) {
				check_ptr();
				element_ptr->make_interface(f);
			}
			template<typename F> void make_constructor(creator& c, F&& f) {
				check_ptr();
				element_ptr->make_interface(c, f);
			}
		};

		class interface_storage
		{
			std::unordered_map<std::type_index, std::weak_ptr<placement_interface>> placement_ptr;
			std::unordered_map<std::type_index, std::weak_ptr<geometry_interface>> geometry_ptr;
			std::unordered_map<std::type_index, std::weak_ptr<defer_material_interface>> material_ptr;
			std::unordered_map<std::type_index, std::weak_ptr<compute_interface>> compute_ptr;
			void make_geometry_placement(creator&, defer_element&, 
				const std::type_index&, const std::function<std::shared_ptr<geometry_interface>()>&, 
				const std::type_index&, const std::function<std::shared_ptr<placement_interface>()>&
				);
			void make_material(creator&, defer_element&, const std::type_index&, const std::function<std::shared_ptr<defer_material_interface>()>&);
			std::shared_ptr<compute_interface> make_compute(creator&, defer_element&, const std::type_index&, const std::function<std::shared_ptr<compute_interface>()>&);
		public:
			template<typename F>
			decltype(auto) make_geometry_and_placement(creator& c, defer_element& de, F&& f)
			{
				using funtype = Tmp::pick_func<typename Tmp::degenerate_func<Tmp::extract_func_t<F>>::type>;
				static_assert(funtype::size == 2, "only receive two parameter");
				using true_type = typename funtype::template out<Implement::element_implement_type_holder>;
				using t1 = typename true_type::type_1;
				using t2 = typename true_type::type_2;
				static_assert(
					std::is_base_of<geometry_interface, typename true_type::type_1>::value && std::is_base_of<placement_interface, typename true_type::type_2>::value,
					"first parameter should derived form geometry_interface and the other should derived form placement_interface"
					);

				make_geometry_placement(
					c, de,
					typeid(t1), []()->std::shared_ptr<geometry_interface> { return std::make_shared<t1>(); },
					typeid(t2), []()->std::shared_ptr<placement_interface> { return std::make_shared<t2>(); }
					);
				return f((de.element_ptr)->geometry_ptr->cast<t1>(), (de.element_ptr)->placemenet_ptr->cast<t2>());
			}

			template<typename F>
			decltype(auto) make_material(creator& c, defer_element& de, F&& f)
			{
				using funtype = Tmp::pick_func<typename Tmp::degenerate_func<Tmp::extract_func_t<F>>::type>;
				static_assert(funtype::size == 1, "only receive one parameter");
				using true_type = typename funtype::template out<Implement::element_implement_type_holder>::type_1;
				static_assert(std::is_base_of<material_interface, true_type>::value, "material need derived form material_interface.");
				make_material(c, de, typeid(true_type), []()->std::shared_ptr<defer_material_interface> { return std::make_shared<true_type>(); });
				return f((de.element_ptr)->material_ptr->cast<true_type>());
			}

			template<typename F>
			decltype(auto) make_compute(creator& c, defer_element& de, F&& f)
			{
				using funtype = Tmp::pick_func<typename Tmp::degenerate_func<Tmp::extract_func_t<F>>::type>;
				static_assert(funtype::size == 1, "only receive one parameter");
				using true_type = typename funtype::template out<Implement::element_implement_type_holder>::type_1;
				static_assert(std::is_base_of<compute_interface, true_type>::value, "compute need derived form compute_interface.");
				return f(make_compute(c, de, typeid(true_type), []()->std::shared_ptr<compute_interface> { return std::make_shared<true_type>(); })->cast<true_type>());
			}

		};

		class defer_element_implement_storage
		{
			std::map<render_order, std::vector<std::shared_ptr<Implement::defer_element_implement>>> element_ptr;
		public:
			property_mapping mapping;
			bool check(const defer_element& p) const { return p.check(mapping); }
			Implement::stage_interface::acceptance_t check_acceptance(const defer_element& p) const;
			void draw(render_order or , pipeline& p);
			void draw(defer_element&, pipeline&);
			bool insert(const defer_element&);
			void clear();
		};

		extern blend_state::description one_to_zero;
		extern blend_state::description s_alpha_to_inv_s_alpha;
	}
}
