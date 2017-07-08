#pragma once
#include "../../po_dx11/dx11_frame.h"
#include "../../po_dx/dx_math.h"
#include <set>
#include <typeindex>
#include <memory>
namespace PO
{
	namespace Dx11
	{
		using namespace Dx;

		bool add_shader_path(std::type_index ti, const std::u16string& path);
		template<typename T> bool add_shader_path(const std::u16string& s) { return add_shader_path(typeid(T), s); }

		class property_interface
		{
			std::type_index id_info;
			uint64_t vision_for_update;
			bool is_need_to_push;
		protected:
			void need_push() { is_need_to_push = true; }

			virtual void push(creator& c);
			virtual void update(pipeline& p);

		public:

			template<typename T> bool is() const { return id_info == typeid(T); }
			std::type_index id() const { return id_info; }

			bool push_implmenet(creator&);
			bool update_vision(pipeline&, uint64_t);
			void force_update(pipeline& p);

			template<typename T> T& cast() { return static_cast<T&>(*this); }

			template<typename Function> bool cast(Function&& f)
			{
				using funtype = Tmp::pick_func<typename Tmp::degenerate_func<Tmp::extract_func_t<Function>>::type>;
				static_assert(funtype::size == 1, "only receive one parameter");
				using true_type = std::decay_t<typename funtype::template out<Tmp::itself>::type>;
				static_assert(std::is_base_of<property_interface, true_type>::value, "property need derived form property_interface.");
				if (is<true_type>())
					return (f(cast<true_type>()), true);
				return false;
			}

			property_interface(std::type_index ti);
			virtual ~property_interface();
			
		};

		class placement_interface
		{
			std::type_index id_info;
		protected:
			vertex_stage vs;
			friend class geometry_interface;
			bool load_vs(const std::u16string& s, creator& c);
		public:
			template<typename T> bool is() const { return id_info == typeid(T); }
			std::type_index id() const { return id_info; }
			

			virtual bool update(property_interface&, pipeline&);
			virtual void apply(pipeline& p);
			virtual const std::set<std::type_index>& acceptance() const;
			virtual ~placement_interface();
			placement_interface(std::type_index ti);
			virtual void init(creator& c) = 0;
		};

		class interface_storage;

		class geometry_interface
		{
			std::type_index id_info;
		protected:

			input_assember_stage ia;
			raterizer_state ra;
			std::shared_ptr<placement_interface> placement_ptr;
			void update_layout(creator& c);

			template<typename T> bool set_placement(creator& c, interface_storage& eis);
		public:

			template<typename T> bool is() const { return id_info == typeid(T); }
			std::type_index id() const { return id_info; }
			operator bool() const { return placement_ptr.operator bool(); }

			void apply(pipeline& p) { placement_ptr->apply(p); }

			bool update_placement(property_interface& pi, pipeline& p) { return placement_ptr->update(pi, p); }
			const std::set<std::type_index>& acceptance_placement() const { return placement_ptr->acceptance(); }

			virtual bool update(property_interface&, pipeline&);
			
			virtual const std::set<std::type_index>& acceptance() const;
			
			virtual ~geometry_interface();
			geometry_interface(std::type_index ti);

			virtual void init(creator& c, interface_storage&) = 0;
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
			bool load_ps(std::u16string path, creator& c);
		public:
			template<typename T> bool is() const { return id_info == typeid(T); }
			std::type_index id() const { return id_info; }
			
			renderer_order get_order() const { return order; }

			virtual ~material_interface();
			material_interface(std::type_index ti, renderer_order o = renderer_order::Defer);

			virtual void apply(pipeline& c);

			virtual bool update(property_interface&, pipeline&);
			virtual const std::set<std::type_index>& acceptance() const;
			virtual void init(creator&) = 0;
			
		};

		class compute_interface
		{
			std::type_index id_info;
		protected:
			compute_stage cs;
		public:
			template<typename T> bool is() const { return id_info == typeid(T); }
			std::type_index id() const { return id_info; }
			bool load_cs(const std::u16string& path, creator& c);

			virtual bool update(property_interface&, pipeline&);
			virtual const std::set<std::type_index>& acceptance() const;
			virtual ~compute_interface();
			compute_interface(std::type_index ti);

			virtual bool draw(pipeline& c) = 0;
			virtual void init(creator&) = 0;
		};

		template<typename T> struct geometry {};
		template<typename T> struct placement {};
		template<typename T> struct material {};
		template<typename T> struct compute {};

		class interface_storage
		{
			std::unordered_map<std::type_index, std::weak_ptr<placement_interface>> placement_ptr;
			std::unordered_map<std::type_index, std::weak_ptr<geometry_interface>> geometry_ptr;
			std::unordered_map<std::type_index, std::weak_ptr<material_interface>> material_ptr;
			std::unordered_map<std::type_index, std::weak_ptr<compute_interface>> compute_ptr;
		public:
			template<typename T> std::shared_ptr<T> find(placement<T>, creator& c)
			{
				static_assert(std::is_base_of<placement_interface, T>::value, "");
				auto find = placement_ptr.find(typeid(T));
				if (find != placement_ptr.end() && !find->second.expired())
					return std::static_pointer_cast<T>(find->second.lock());
				auto po = std::make_shared<T>();
				po->init(c);
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
				po->init(c, *this);
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
				po->init(c);
				std::weak_ptr<material_interface> ptr = std::static_pointer_cast<material_interface>(po);
				material_ptr[typeid(T)] = std::move(ptr);
				return po;
			}
			template<typename T> std::shared_ptr<T> find(compute<T>, creator& c)
			{
				static_assert(std::is_base_of<compute_interface, T>::value, "");
				auto find = compute_ptr.find(typeid(T));
				if (find != compute_ptr.end() && !find->second.expired())
					return std::static_pointer_cast<T>(find->second.lock());
				auto po = std::make_shared<T>();
				po->init(c);
				std::weak_ptr<compute_interface> ptr = std::static_pointer_cast<compute_interface>(po);
				compute_ptr[typeid(T)] = std::move(ptr);
				return po;
			}
		};

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


				void push(std::shared_ptr<property_interface> p);

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
					this->push(p);
					return *p;
				}
			};


			struct element_implement
			{
				renderer_order order = renderer_order::Defer;
				std::shared_ptr<geometry_interface> geometry;
				std::shared_ptr<material_interface> material;
				std::vector<std::shared_ptr<compute_interface>> compute;
				property_storage property_map;

				element_implement& operator=(std::shared_ptr<geometry_interface> s);
				element_implement& operator=(std::shared_ptr<material_interface> s);
				element_implement& operator=(std::shared_ptr<compute_interface> s);

				element_implement() = default;
				element_implement(const element_implement&) = default;
				element_implement(element_implement&&) = default;
				element_implement& operator=(const element_implement&) = default;
				element_implement& operator=(element_implement&&) = default;
				bool dispatch(pipeline& p, property_storage& out_mapping, uint64_t vision);
				bool draw(pipeline& p, property_storage& out_mapping, uint64_t vision);
				void clear_compute();
				void push(creator& c) { property_map.push(c); }
			};

			class element_implement_storage
			{
				std::map<renderer_order, std::vector<std::shared_ptr<element_implement>>> element_ptr;
				property_storage property_map;
				uint64_t vision_for_update;
			public:
				uint64_t vision() const { return vision_for_update; }
				element_implement_storage();
				bool push_back(std::shared_ptr<element_implement> p, creator& c);
				bool dispatch(pipeline& p);
				bool draw(renderer_order or, pipeline& p);
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

			void push(creator& c);
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
		
	}
}
