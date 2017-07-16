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
				defer_element_implement& operator=(std::shared_ptr<defer_material_interface> di);
				element_implement& operator=(std::shared_ptr<material_interface> di) = delete;
				void clear_all();
			};
		}

		class defer_element
		{
			std::shared_ptr<Implement::defer_element_implement> element_ptr;
			friend class interface_storage;
			friend class defer_element_implement_storage;
			void check_ptr();
			template<typename T>
			defer_element& operator=(std::shared_ptr<T> p)
			{
				check_ptr();
				*element_ptr = std::move(p);
				return *this;
			}
		public:
			void clear_all();
			bool check(const property_mapping& pm) const { if (element_ptr) return element_ptr->check(pm); return true; }
		};

		class interface_storage
		{
			std::unordered_map<std::type_index, std::weak_ptr<placement_interface>> placement_ptr;
			std::unordered_map<std::type_index, std::weak_ptr<geometry_interface>> geometry_ptr;
			std::unordered_map<std::type_index, std::weak_ptr<defer_material_interface>> material_ptr;
			std::unordered_map<std::type_index, std::weak_ptr<compute_interface>> compute_ptr;
		public:
			template<typename F>
			decltype(auto) make_geometry(defer_element& de, F&& f)
			{
				using funtype = Tmp::pick_func<typename Tmp::degenerate_func<Tmp::extract_func_t<F>>::type>;
				static_assert(funtype::size == 1, "only receive one parameter");
				using true_type = std::decay_t<typename funtype::template out<Tmp::itself>::type>;
				static_assert(std::is_base_of<geometry_interface, true_type>::value, "geometry need derived form geometry_interface.");

				std::shared_ptr<true_type> temporary;
				auto ite = geometry_ptr.find(typeid(true_type));
				if (ite != geometry_ptr.end())
					temporary = std::static_pointer_cast<true_type>(ite->second);
				else {
					temporary = std::make_shared<true_type>();
					geometry_ptr.insert({ temporary->id(), temporary }); 
					auto& require = geometry_ptr->requirement();
					auto ite2 = placement_ptr.find(std::get<0>(require));
					if (ite2 != placement_ptr.end())
						geometry_ptr->set_placemenet(ite2->second);
					else {
						std::shared_ptr<placement_interface> tem = std::get<1>(require)();
						auto id = tem->id();
						placement_ptr.insert({id, tem});
						geometry_ptr->set_placement(std::move(tem));
					}
				}
				de = temporary;
				return f(*temporary);
			}

			template<typename F>
			decltype(auto) make_material(defer_element& de, F&& f)
			{
				using funtype = Tmp::pick_func<typename Tmp::degenerate_func<Tmp::extract_func_t<F>>::type>;
				static_assert(funtype::size == 1, "only receive one parameter");
				using true_type = std::decay_t<typename funtype::template out<Tmp::itself>::type>;
				static_assert(std::is_base_of<defer_material_interface, true_type>::value, "defer_material need derived form defer_material_interface.");

				std::shared_ptr<true_type> temporary;
				auto ite = material_ptr.find(typeid(true_type));
				if (ite != material_ptr.end())
					temporary = std::static_pointer_cast<true_type>(ite->second);
				else {
					temporary = std::make_shared<true_type>();
					material_ptr.insert({ temporary->id(), temporary });
				}
				de = temporary;
				return f(*temporary);
			}

			template<typename F>
			decltype(auto) make_compute(defer_element& de, F&& f)
			{
				using funtype = Tmp::pick_func<typename Tmp::degenerate_func<Tmp::extract_func_t<F>>::type>;
				static_assert(funtype::size == 1, "only receive one parameter");
				using true_type = std::decay_t<typename funtype::template out<Tmp::itself>::type>;
				static_assert(std::is_base_of<compute_interface, true_type>::value, "compute need derived form compute_interface.");

				std::shared_ptr<true_type> temporary;
				auto ite = material_ptr.find(typeid(true_type));
				if (ite != material_ptr.end())
					temporary = std::static_pointer_cast<true_type>(ite->second);
				else {
					temporary = std::make_shared<true_type>();
					material_ptr.insert({ temporary->id(), temporary });
				}
				de = temporary;
				return f(*temporary);
			}
		};

		class defer_element_implement_storage
		{
			std::map<render_order, std::vector<std::shared_ptr<Implement::defer_element_implement>>> element_ptr;
		public:
			property_mapping mapping;
			bool check(const defer_element& p) const { return p.check(mapping); }
			void draw(render_order or , pipeline& p);
			void draw(defer_element&, pipeline&);
			bool insert(defer_element&);
		};

		/*
		template<typename T> bool geometry_interface::set_placement(creator& c, interface_storage& eis)
		{
			placement_ptr = eis.find(placement<T>{}, c);
			if (placement_ptr)
			{
				update_layout(c);
				return true;
			}
			return false;
		}

		namespace Implement
		{
			class property_storage
			{

				std::map<std::type_index, std::shared_ptr<property_interface>> mapping;
				friend struct element_implement;


				void insert(std::shared_ptr<property_interface> p);

			public:

				property_storage() = default;
				property_storage(const property_storage&) = default;
				property_storage& operator=(const property_storage&) = default;
				property_storage(property_storage&&) = default;
				property_storage& operator=(property_storage&&) = default;
				~property_storage();

				void clear();
				template<typename T> bool have() const { return have(typeid(T)); }
				bool have(std::type_index) const;
				//void update(pipeline& p, uint64_t u);
				void push(creator& c);
				void pre_push(geometry_interface& gi);

				template<typename F> bool find(F&& f) {
					using funtype = Tmp::pick_func<typename Tmp::degenerate_func<Tmp::extract_func_t<F>>::type>;
					static_assert(funtype::size == 1, "only receive one parameter");
					using true_type = std::decay_t<typename funtype::template out<Tmp::itself>::type>;
					static_assert(std::is_base_of<property_interface, true_type>::value, "property need derived form property_interface.");
					auto tem = mapping.find(typeid(true_type));
					if(tem != mapping.end())
						return (f(static_cast<true_type&>(*(tem->second))), true);
					return false;
				}

				template<typename T, typename ...AT> T& create(AT&& ...at)
				{
					static_assert(std::is_base_of<property_interface, T>::value, "property need derived form property_interface.");
					auto p = std::make_shared<T>(std::forward<AT>(at)...);
					mapping[p->id()] = p;
					return *p;
				}
			};


			struct element_implement
			{
				render_order order = render_order::NotSet;
				render_state state = render_state::Null;
				std::shared_ptr<geometry_interface> geometry;
				std::shared_ptr<material_interface> material;
				std::vector<std::shared_ptr<compute_interface>> compute;
				property_storage property_map;

				bool dispatch(pipeline& p, property_storage& out_mapping, uint64_t vision);
				bool draw(pipeline& p, property_storage& out_mapping, uint64_t vision);

				element_implement& operator=(std::shared_ptr<geometry_interface> s);
				element_implement& operator=(std::shared_ptr<material_interface> s);
				element_implement& operator=(std::shared_ptr<compute_interface> s);

				element_implement() = default;
				element_implement(const element_implement&) = default;
				element_implement(element_implement&&) = default;

				element_implement& operator=(const element_implement&) = default;
				element_implement& operator=(element_implement&&) = default;
				
				void dispatch_imp(pipeline& p, property_storage& out_mapping, uint64_t vision);
				void draw_imp(pipeline& p, property_storage& out_mapping, uint64_t vision);
				std::set<std::type_index> ckeck(const property_storage & p) const;
				void clear_compute();
				void push(creator& c);
			};

			class element_implement_storage
			{
				std::map<render_order, std::vector<std::shared_ptr<element_implement>>> element_ptr;
				property_storage property_map;
				uint64_t vision_for_update;
			public:
				uint64_t vision() const { return vision_for_update; }
				std::set<std::type_index> check(const std::shared_ptr<element_implement>& p) const;
				element_implement_storage();
				bool push_back(std::shared_ptr<element_implement> p, creator& c);
				bool dispatch(pipeline& p);
				bool draw(render_order or, pipeline& p);
				bool direct_draw(element_implement&, pipeline&);
				void update();
				void push(creator& c) { property_map.push(c); }
				void clear_element();
				void clear_property() { property_map.clear(); }
				template<typename K> decltype(auto) find(K&& k) { return property_map.find(std::forward<K>(k)); }
				template<typename T, typename ...AT> decltype(auto) create(AT&& ...at) { return property_map.template create<T>(std::forward<AT>(at)...); }
			};
		}


		class element
		{
			std::shared_ptr<Implement::element_implement> ptr;

		public:

			operator std::shared_ptr<Implement::element_implement>() const { return ptr; }
			operator Implement::element_implement&();

			element() {}
			element(const element&) = default;
			element(element&&) = default;
			element& operator=(const element&) = default;
			element& operator=(element&&) = default;

			template<typename T>
			element& operator=(std::shared_ptr<T> s) { 
				if (!ptr) ptr = std::make_shared<Implement::element_implement>();
				*ptr = std::move(s);
				return *this;
			}

			//void push(creator& c);
			//bool draw(pipeline& p, creator& c);

			operator bool() const { return ptr.operator bool(); }
			template<typename T> void get(T t, interface_storage& is, creator& c) 
			{
				if (!ptr) ptr = std::make_shared<Implement::element_implement>();
				*ptr = is.find(t, c);
			}

			template<typename K> bool find(K&& k) {
				if (!ptr) ptr = std::make_shared<Implement::element_implement>();
				return ptr->property_map.find(std::forward<K>(k));
			}
			template<typename T, typename ...AT> T& create(AT&& ...at) { 
				if (!ptr) ptr = std::make_shared<Implement::element_implement>();
				return ptr->property_map.template create<T>(std::forward<AT>(at)...);
			}
			void clear();
			void clear_property();
		};
		*/
	}
}
