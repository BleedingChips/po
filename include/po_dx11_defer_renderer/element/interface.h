#pragma once
#include "../../po_dx11/dx11_frame.h"
#include "../../po_dx/dx_math.h"
#include "../../po/tool/scene.h"
#include <set>
#include <typeindex>
#include <memory>
namespace PO
{
	namespace Dx11
	{
		using namespace Dx;

		class property_interface
		{
			std::type_index id_info;
		public:
			template<typename T> bool is() const { return id_info == typeid(T); }
			std::type_index id() const { return id_info; }
			virtual void init(creator& c);
			virtual void update(pipeline& p);
			template<typename T> T& cast() { return static_cast<T&>(*this); }
			property_interface(std::type_index ti);
			virtual ~property_interface();
		};

		class placement_interface
		{
			std::type_index id_info;
		protected:
			vertex_stage vs;
			friend class geometry_interface;
		public:
			template<typename T> bool is() const { return id_info == typeid(T); }
			std::type_index id() const { return id_info; }

			virtual bool update(property_interface&, pipeline&, creator&);
			virtual void apply(pipeline& p);
			virtual const std::set<std::type_index>& acceptance() const;
			virtual ~placement_interface();
			placement_interface(std::type_index ti);

			virtual void init(creator&, raw_scene&) = 0;

		};

		class interface_storage;

		class geometry_interface
		{
			std::type_index id_info;
		protected:
			input_assember_stage ia;
			raterizer_state ra;
			std::shared_ptr<placement_interface> placement_interface;
			void update_layout(creator& c);
		public:
			template<typename T> bool is() const { return id_info == typeid(T); }
			std::type_index id() const { return id_info; }
			operator bool() const { return placement_interface.operator bool(); }

			void apply(pipeline& p) { placement_interface->apply(p); }
			bool update_placement(property_interface& pi, pipeline& p, creator& c) { return placement_interface->update(pi, p, c); }
			const std::set<std::type_index>& acceptance_placement() const { return placement_interface->acceptance(); }
			virtual bool update(property_interface&, pipeline&, creator&);
			virtual const std::set<std::type_index>& acceptance() const;
			template<typename T> bool set_placement(interface_storage& eis, creator& c);
			virtual ~geometry_interface();
			geometry_interface(std::type_index ti);

			virtual void init(creator& c, raw_scene& rs, interface_storage&) = 0;
			virtual void draw(pipeline& p) = 0;
		};

		enum class renderer_order
		{
			Defer = 0,
			Forward,
			Post,


			Max
		};

		class material_interface
		{
			std::type_index id_info;
		protected:
			pixel_stage ps;
			blend_state bs;
			renderer_order order = renderer_order::Defer;
			std::u16string path;
		public:
			template<typename T> bool is() const { return id_info == typeid(T); }
			std::type_index id() const { return id_info; }

			virtual void apply(pipeline& c);
			virtual bool update(property_interface&, pipeline&, creator&);
			virtual const std::set<std::type_index>& acceptance() const;
			virtual ~material_interface();
			material_interface(std::type_index ti, renderer_order o = renderer_order::Defer);

			virtual void init(creator&, raw_scene&) = 0;
		};

		template<typename T> struct geometry {};
		template<typename T> struct placement {};
		template<typename T> struct material {};

		class interface_storage
		{
			std::map<std::type_index, std::weak_ptr<placement_interface>> placement_ptr;
			std::map<std::type_index, std::weak_ptr<geometry_interface>> geometry_ptr;
			std::map<std::type_index, std::weak_ptr<material_interface>> material_ptr;
			raw_scene rs;
		public:
			template<typename T> std::shared_ptr<T> find(placement<T>, creator& c)
			{
				static_assert(std::is_base_of<placement_interface, T>::value, "");
				auto find = placement_ptr.find(typeid(T));
				if (find != placement_ptr.end() && !find->second.expired())
					return std::static_pointer_cast<T>(find->second.lock());
				auto po = std::make_shared<T>();
				po->init(c, rs);
				std::weak_ptr<placement_interface> ptr = std::static_pointer_cast<placement_interface>(po);
				placement_ptr[typeid(T)] = std::move(ptr);
				return po;
			}
			template<typename T> std::shared_ptr<T> find(geometry<T>, creator& c)
			{
				static_assert(std::is_base_of<geometry_interface, T>::value, "");
				auto find = geometry_ptr.find(typeid(T));
				if (find != geometry_ptr.end() && !find->second.expired())
					return std::static_pointer_cast<T>(find->second.lock());
				auto po = std::make_shared<T>();
				po->init(c, rs, *this);
				std::weak_ptr<geometry_interface> ptr = std::static_pointer_cast<geometry_interface>(po);
				geometry_ptr[typeid(T)] = std::move(ptr);
				return po;
			}
			template<typename T> std::shared_ptr<T> find(material<T>, creator& c)
			{
				static_assert(std::is_base_of<material_interface, T>::value, "");
				auto find = material_ptr.find(typeid(T));
				if (find != material_ptr.end() && !find->second.expired())
					return std::static_pointer_cast<T>(find->second.lock());
				auto po = std::make_shared<T>();
				po->init(c, rs);
				std::weak_ptr<material_interface> ptr = std::static_pointer_cast<material_interface>(po);
				material_ptr[typeid(T)] = std::move(ptr);
				return po;
			}
		};

		template<typename T> bool geometry_interface::set_placement(interface_storage& eis, creator& c)
		{
			placement_interface = eis.find(placement<T>{}, c);
			if (placement_interface)
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


				void push(std::shared_ptr<property_interface> p);
				std::shared_ptr<property_interface> find(std::type_index) const;


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
				void update(pipeline& p);
				void init(creator& c);
				

				template<typename F> bool find(F&& f) {
					using funtype = Tmp::pick_func<typename Tmp::degenerate_func<Tmp::extract_func_t<F>>::type>;
					static_assert(funtype::size, "only receive one parameter");
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
					this->push(p);
					return *p;
				}
			};


			struct element_implement
			{
				renderer_order order = renderer_order::Defer;
				std::shared_ptr<geometry_interface> geometry;
				std::shared_ptr<material_interface> material;
				property_storage property_map;

				element_implement& operator=(std::shared_ptr<geometry_interface> s) { geometry = std::move(s); return *this; }
				element_implement& operator=(std::shared_ptr<material_interface> s) { material = std::move(s); return *this; }

				element_implement() = default;
				element_implement(const element_implement&) = default;
				element_implement(element_implement&&) = default;
				element_implement& operator=(const element_implement&) = default;
				element_implement& operator=(element_implement&&) = default;
				bool call(pipeline& p, creator& c, const property_storage& out_mapping);
				bool direct_call(pipeline& p, creator& c);
			};

			struct element_implement_storage
			{
				std::map<renderer_order, std::vector<std::shared_ptr<element_implement>>> element_ptr;
				property_storage property_map;
				bool push_back(std::shared_ptr<element_implement> p);
				bool call(renderer_order or, pipeline& p, creator& c);
				void update(pipeline& p) { property_map.update(p); }
				void init(creator& c) { property_map.init(c); }
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
			element() {}
			element(const element&) = default;
			element(element&&) = default;
			element& operator=(const element&) = default;
			element& operator=(element&&) = default;

			element& operator=(std::shared_ptr<geometry_interface> s) { 
				if (!ptr) ptr = std::make_shared<Implement::element_implement>();
				*ptr = std::move(s);
				return *this;
			}

			element& operator=(std::shared_ptr<material_interface> s) {
				if (!ptr) ptr = std::make_shared<Implement::element_implement>();
				*ptr = std::move(s);
				return *this;
			}

			void init(creator& c);
			bool direct_call(pipeline& p, creator& c);

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
			template<typename T, typename ...AT> decltype(auto) create(AT&& ...at) { 
				if (!ptr) ptr = std::make_shared<Implement::element_implement>();
				return ptr->property_map.template create<T>(std::forward<AT>(at)...);
			}

			void clear();
			void clear_property();

			
		};
		
	}
}
