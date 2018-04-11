#pragma once
#include "../platform/asynchronous_affairs.h"
#include "../platform/platform.h"
#include "../tool/tool.h"
#include "object_pool.h"
#include <deque>
#include <set>
#include <map>
#include <array>
#include <typeindex>
namespace PO::ECSFramework
{
	// pre define **************************************************
	namespace Implement
	{
		template<typename T> struct default_deleter { void operator ()(T* da) const noexcept { if (da != nullptr) { da->~T(); } } };
		template<typename T> using ecs_intrusive_ptr = PO::Tool::intrusive_ptr<T, default_deleter<T>>;

		struct type_index_view
		{
			const std::type_index* view = nullptr;
			size_t view_count = 0;
			const std::type_index& operator[](size_t index) const noexcept { return view[index]; }
			operator bool() const { return view_count != 0; }
		};

		template<typename ...type>
		struct type_array
		{
			std::array<std::type_index, sizeof...(type)> array_buffer = { typeid(type)... };
			type_array() { std::sort(array_buffer.begin(), array_buffer.end()); }
			type_index_view view() const noexcept { return type_index_view{ array_buffer.data(), array_buffer.size() }; }
		};

		template<>
		struct type_array<>
		{
			type_array() { }
			type_index_view view() const noexcept { return type_index_view{ nullptr, 0 }; }
		};

		template<typename T> struct is_const;
		template<typename T> struct is_const<T&> : std::false_type {};
		template<typename T> struct is_const<T&&> : std::false_type {};
		template<typename T> struct is_const<const T&> : std::true_type {};

		template<typename ...component_t> struct ctl
		{
			template<template<typename ...> class out> using outpacket = out<component_t...>;
			constexpr static const size_t size = sizeof...(component_t);
		};
	}

	// component **************************************************
	namespace Implement
	{
		struct singleton_component_ref
		{
			mutable PO::Tool::atomic_reference_count ref;
			bool avalible = true;
			std::type_index defined_id;
			void* data;
			void(*deleter)(void*) noexcept;
			void add_ref() const noexcept { ref.add_ref(); }
			bool sub_ref() const noexcept { return ref.sub_ref(); }
			void destory() noexcept;
			operator bool() const noexcept { return avalible; }
			std::type_index id() const noexcept { return defined_id; }
			template<typename T> T& cast() noexcept { return *(static_cast<T*>(data)); }
			template<typename T> const T& cast() const noexcept { return *(static_cast<const T*>(data)); }
			~singleton_component_ref();
			singleton_component_ref(std::type_index ti, void(*deleter)(void*) noexcept);
		};

		using singleton_component_ptr = Tool::intrusive_ptr<singleton_component_ref>;

		struct component_ref :
			object_pool_base_deleter<component_ref>, singleton_component_ref
		{
			using singleton_component_ref::singleton_component_ref;
		};

		using component_ptr = ecs_intrusive_ptr<component_ref>;
	}

	// entity **************************************************
	class context_implement;
	namespace Implement
	{
		class entity_implement
		{
			friend class context_implement;
			std::unordered_map<std::type_index, component_ptr> component_set;
		public:
			bool reflesh() noexcept;
			bool insert(component_ptr) noexcept;
			bool check_exist(std::type_index) const noexcept;
			component_ptr get_component(std::type_index ti) const noexcept;
			bool destory_component(std::type_index id) noexcept;
			void destory_all_component(std::set<std::type_index>& s);
			//void destory() noexcept;
			~entity_implement();
		};

		class entity_ref :
			object_pool_base_deleter<entity_ref>
		{
			mutable PO::Tool::atomic_reference_count ref;
			bool avalible = true;
			entity_implement* shift() noexcept { return reinterpret_cast<entity_implement*>(this + 1); }
			const entity_implement* shift() const noexcept { return reinterpret_cast<const entity_implement*>(this + 1); }
		public:
			void add_ref() const noexcept { ref.add_ref(); }
			bool sub_ref() const noexcept { return ref.sub_ref(); }
			bool check_exist(std::type_index ti) const noexcept { return shift()->check_exist(ti); }
			bool insert(component_ptr chp) noexcept { return shift()->insert(std::move(chp)); }
			component_ptr get_component(std::type_index ti) const noexcept { return shift()->get_component(ti); }
			bool destory_component(std::type_index id) noexcept { return shift()->destory_component(id); }
			bool reflesh() noexcept { return shift()->reflesh(); }
			void destory() noexcept;
			operator bool() const noexcept { return avalible; }
			void destory_all_component(std::set<std::type_index>& s) { return shift()->destory_all_component(s); }
			~entity_ref();
		};

		using entity_ptr = Implement::ecs_intrusive_ptr<entity_ref>;
	}

	template<typename ...implement> struct filter;

	class entity
	{
		Implement::entity_ptr ptr;
		friend class context_implement;
		friend class context;
		template<typename ...implement> friend struct filter;
	public:
		entity(const entity&) = default;
		entity(Implement::entity_ptr p) : ptr(std::move(p)) {}
		entity() = default;
		entity(entity&&) = default;
		entity& operator=(entity e) { ptr = std::move(e.ptr); return *this; }
		operator bool() const noexcept { return ptr && *ptr; }
		bool check_exist(std::type_index ti) const noexcept { return ptr->check_exist(ti); }
	};

	// system_requirement_storage **************************************************
	namespace Implement
	{
		template<typename T> struct system_requirement_storage
		{
			component_ptr ptr;
			operator bool() const noexcept { assert(ptr); return *ptr; }
			bool able_extract_component(entity_ptr e) noexcept { ptr = e->get_component(typeid(T)); return ptr; }
			T get() noexcept { return ptr->cast<std::remove_reference_t<T>>(); }
			system_requirement_storage& operator=(const system_requirement_storage&) = default;
		};
		/*
		template<typename T> struct system_requirement_storage<const T>
		{
			component_ptr ptr;
			operator bool() const noexcept { assert(ptr); return *ptr; }
			bool able_extract_component(entity_ptr e) noexcept { ptr = e->get_component(typeid(T)); return ptr; }
			const T& get() const noexcept { return ptr->cast<std::remove_reference_t<T>>(); }
			system_requirement_storage& operator=(const system_requirement_storage&) = default;
		};
		*/
	}

	// filter **************************************************
	namespace Implement
	{
		template<typename this_type, typename ...type> bool is_all_true(this_type tt, type... t)
		{
			return tt;
		}

		template<typename this_type, typename other_type, typename ...type> bool is_all_true(this_type tt, other_type ot, type... t)
		{
			return is_all_true(tt && ot, t...);
		}
		struct context_interface;

		struct pre_filter_storage_interface
		{
			virtual void insert(entity_ptr) = 0;
			virtual void reflesh(size_t vision) = 0;
			virtual ~pre_filter_storage_interface() = default;
			virtual type_index_view needed() = 0;
		};


		template<typename index, typename ...component> struct pre_filter_storage_implement;
		template<size_t ...index, typename ...component>
		class pre_filter_storage_implement<std::integer_sequence<size_t, index...>, component...>
		{
			std::tuple<entity_ptr, system_requirement_storage<component>...> storage;
		public:
			pre_filter_storage_implement() = default;
			pre_filter_storage_implement(const pre_filter_storage_implement&) = default;
			pre_filter_storage_implement(pre_filter_storage_implement&&) = default;
			pre_filter_storage_implement& operator=(const pre_filter_storage_implement&) = default;
			operator bool() const noexcept { 
				return is_all_true(std::get<0>(storage)->operator bool(), std::get<index + 1>(storage)...);
			}
			bool able_extract_component(entity_ptr e) noexcept {
				assert(e && *e);
				std::get<0>(storage) = e;
				return is_all_true(true, std::get<index + 1>(storage).able_extract_component(e)...);
			}
			template<typename callable> decltype(auto) operator()(callable&& f)
			{
				assert(*this);
				return std::forward<callable>(f)(std::get<0>(storage), std::get<index + 1>(storage).get()...);
			}
		};

		template<typename ...component>
		class pre_filter_storage : public pre_filter_storage_interface
		{
			using storage_type = pre_filter_storage_implement<std::make_integer_sequence<size_t, sizeof...(component)>, component...>;
			size_t current_vision = 0;
			std::vector<storage_type> storage;

		public:

			virtual type_index_view needed() override
			{
				static Tmp::replace_t<Tmp::remove_repeat_t<ctl<component...>>, type_array> info;
				return info.view();
			}

			void reflesh(size_t vision) override
			{
				if (current_vision != vision)
				{
					current_vision = vision;
					storage.erase(std::remove_if(storage.begin(), storage.end(), [](auto& i) { return !i; }), storage.end());
				}
			}

			virtual void insert(entity_ptr e) override
			{
				assert(e && *e);
				storage_type temporary;
				if (temporary.able_extract_component(std::move(e)))
					storage.push_back(std::move(temporary));
			}

			template<typename callable> void operator()(callable&& c)
			{
				storage.erase(std::remove_if(storage.begin(), storage.end(), [&](storage_type& storage) {
					if (storage)
					{
						storage(std::forward<callable>(c));
						return false;
					}
					return true;
				}), storage.end());
			}

		};

		template<typename ...component> 
		class filter_with_entity
		{
			template<typename ...component> friend struct filter;
			bool avalible = false;
			pre_filter_storage_implement<std::make_index_sequence<sizeof...(component)>, component...> storage;
			filter_with_entity(entity_ptr e)
			{
				avalible = storage.able_extract_component(e);
			}
			filter_with_entity(filter_with_entity&& e) = default;
		public:
			
			template<typename callable> filter_with_entity& operator<<(callable&& c)
			{
				if (avalible)
				{
					storage(std::forward<callable>(c));
					return true;
				}
				return false;
			}
		};

	}

	template<typename ...component> class pre_filter
	{
		Implement::pre_filter_storage<component...>& storage;
	public:
		pre_filter(Implement::pre_filter_storage<component...>& s) : storage(s) {}
		pre_filter(const pre_filter&) = default;
		template<typename callable> pre_filter& operator << (callable&& c) { storage(std::forward<callable>(c)); return *this; }
	};

	template<typename ...component> struct filter
	{
		template<typename ...other_component>
		filter<component..., other_component...> operator*(filter<other_component...>) const noexcept { return filter<component..., other_component...>{}; }
		Implement::filter_with_entity<component...> operator<<(entity e) const noexcept { return Implement::filter_with_entity<component...>{std::move(e)}; }
	};

	// event **************************************************
	namespace Implement
	{
		struct event_pool_interface : Tool::atomic_reference_count
		{
			bool avalible;
			void* data;
			size_t element_count;
			std::type_index id;
			const size_t elemnt_size;
			const size_t element_align;
			operator bool() const noexcept { return avalible; }
			event_pool_interface(std::type_index i, size_t size, size_t align) : avalible(true), element_count(0), data(nullptr), id(i), elemnt_size(size), element_align(align) {}
		};

		template<typename component>
		Tool::intrusive_ptr<event_pool_interface> make_event_pool_interface() {
			return new event_pool_interface{typeid(component), sizeof(component), alignof(component)};
		}

		template<typename event_t> struct event_pool
		{
			std::vector<event_t> pool;
			void clear() { pool.clear(); }
			void push_back(event_t e) { pool.push_back(std::move(e)); }
			void update(event_pool_interface& p) noexcept { p.data = pool.data(); p.element_count = pool.size(); }
		};
	}

	template<typename componenet> class provider 
	{
		Implement::event_pool<std::decay_t<componenet>> & pool;
	public:
		provider(Implement::event_pool<std::decay_t<componenet>> &p) : pool(p) {}
		void clear() { pool.clear(); }
		void push_back(std::decay_t<componenet> e) { pool.push_back(std::move(e)); }
	};

	template<typename componenet> class receiver 
	{
		const Tool::intrusive_ptr<Implement::event_pool_interface>* all_interface;
		size_t interface_size;
	public:
		receiver(const Tool::intrusive_ptr<Implement::event_pool_interface>* a, size_t count) :all_interface(a), interface_size(count) {}
		template<typename callable> receiver& operator<<(callable&& c)
		{
			if (all_interface != nullptr)
			{
				for (size_t i = 0; i < interface_size; ++i)
				{
					assert(all_interface[i]->elemnt_size == sizeof(componenet));
					assert(all_interface[i]->element_align == alignof(componenet));
					assert(all_interface[i]->id == typeid(componenet));
					if (all_interface[i]->avalible)
					{
						const componenet* ptr = reinterpret_cast<const componenet*>(all_interface[i]->data);
						for (size_t k = 0; k < all_interface[i]->element_count; ++k)
						{
							std::forward<callable>(c)(ptr[k]);
						}
					}
				}
			}
			return *this;
		}
	};


	// system detected *************************************************
	namespace Implement
	{

		template<typename write_t, typename read_t>
		struct rw_type
		{
			using write = write_t;
			using read = read_t;
		};

		template<typename write_type, typename read_output, typename ...requirement> struct system_requirement_detect_step2;
		template<typename ...write_type, typename ...read_output> struct system_requirement_detect_step2<ctl<write_type...>, ctl<read_output...>>
		{
			using type = ctl<read_output...>;
		};
		template<typename ...write_type, typename ...read_output, typename this_requirement, typename ...other_requirement> 
		struct system_requirement_detect_step2<ctl<write_type...>, ctl<read_output...>, this_requirement, other_requirement...>
		{
			using type = typename system_requirement_detect_step2<ctl<write_type...>,
				std::conditional_t<Tmp::is_one_of<this_requirement, write_type...>::value, ctl<read_output...>, ctl<read_output..., this_requirement>>,
				other_requirement...
			>::type;
		};

		template<typename write_output, typename read_output, typename ...requirement> struct system_requirement_detect_step1;

		template<typename ...write_output, typename ...read_output> struct system_requirement_detect_step1<ctl<write_output...>, ctl<read_output...>>
		{
			using type = rw_type<ctl<write_output...>, typename system_requirement_detect_step2<ctl<write_output...>, ctl<>, read_output...>::type>;
		};

		template<typename ...write_output, typename ...read_output, typename this_requirement, typename ...other_requirement> 
		struct system_requirement_detect_step1<ctl<write_output...>, ctl<read_output...>, this_requirement, other_requirement...>
		{
			using type = typename system_requirement_detect_step1<
				std::conditional_t<!is_const<this_requirement>::value && !Tmp::is_one_of<std::decay_t<this_requirement>, write_output...>::value, ctl<write_output..., std::decay_t<this_requirement>>, ctl<write_output...>>,
				std::conditional_t<is_const<this_requirement>::value && !Tmp::is_one_of<std::decay_t<this_requirement>, read_output...>::value, ctl<read_output..., std::decay_t<this_requirement>>, ctl<read_output...>>,
				other_requirement...
			>::type;
		};

		template<typename ...write_output, typename ...read_output, typename ...filter_requirement, typename ...other_requirement>
		struct system_requirement_detect_step1<ctl<write_output...>, ctl<read_output...>, filter<filter_requirement...>, other_requirement...>
		{
			using type = typename system_requirement_detect_step1<ctl<write_output...>, ctl<read_output...>, filter_requirement..., other_requirement...>::type;
		};

		template<typename ...write_output, typename ...read_output, typename ...filter_requirement, typename ...other_requirement>
		struct system_requirement_detect_step1<ctl<write_output...>, ctl<read_output...>, pre_filter<filter_requirement...>, other_requirement...>
		{
			using type = typename system_requirement_detect_step1<ctl<write_output...>, ctl<read_output...>, filter_requirement..., other_requirement...>::type;
		};

		template<typename ...write_output, typename ...read_output, typename event_t, typename ...other_requirement>
		struct system_requirement_detect_step1<ctl<write_output...>, ctl<read_output...>, provider<event_t>, other_requirement...>
		{
			using type = typename system_requirement_detect_step1<ctl<write_output..., std::decay_t<event_t>>, ctl<read_output...>, other_requirement...>::type;
		};

		template<typename ...write_output, typename ...read_output, typename event_t, typename ...other_requirement>
		struct system_requirement_detect_step1<ctl<write_output...>, ctl<read_output...>, receiver<event_t>, other_requirement...>
		{
			using type = typename system_requirement_detect_step1<ctl<write_output...>, ctl<read_output..., std::decay_t<event_t>>, other_requirement...>::type;
		};

		template<typename ...requirement> struct system_requirement_detect;
		template<typename ...requirement> struct system_requirement_detect<context&, requirement...> {
			using type = typename system_requirement_detect_step1<ctl<>, ctl<>, requirement...>::type;
		};
		template<typename ...requirement> struct system_requirement_detect<const context&, requirement...> {
			using type = typename system_requirement_detect_step1<ctl<>, ctl<>, requirement...>::type;
		};
	}

	// system predefine **************************************************
	namespace Implement
	{

		struct system_requirement_info
		{
			const type_index_view write;
			const type_index_view read;
			//const type_index_view singleton_read;
			//const type_index_view singleton_write;
		};

		template<typename write_t, typename read_t> struct system_requirement_info_initializer;
		template<typename ...write_t, typename ...read_t>
		struct system_requirement_info_initializer<ctl<write_t...>, ctl<read_t...>>
		{
			type_array<write_t...> write;
			type_array<read_t...> read;
			system_requirement_info view() const noexcept { return  system_requirement_info{ write.view(), read.view() }; }
		};

		template<typename write_t, typename read_t>
		const system_requirement_info& system_requirement_info_instance()
		{
			static system_requirement_info_initializer<write_t, read_t> init;
			static system_requirement_info info(init.view());
			return info;
		}

	}



	// system **************************************************
	enum class SystemLayout
	{
		PRE_UPDATE = 0,
		UPDATE = 1,
		POS_UPDATE = 2
	};

	inline bool operator<(SystemLayout SL, SystemLayout SL2) { return static_cast<size_t>(SL) < static_cast<size_t>(SL2); }
	inline bool operator==(SystemLayout SL, SystemLayout SL2) { return static_cast<size_t>(SL) == static_cast<size_t>(SL2); }

	enum class SystemSequence
	{
		UNDEFINE,
		BEFORE,
		AFTER,
		NOT_CARE
	};

	struct system_default_define 
	{
		SystemSequence check_sequence(std::type_index ti) noexcept { return SystemSequence::UNDEFINE; }
		static SystemLayout layout() noexcept { return SystemLayout::UPDATE; }
	};

	namespace Implement
	{

		struct system_interface
		{
			virtual void call(context&) = 0;
			virtual SystemSequence check_sequence(std::type_index ti) noexcept = 0;
			virtual system_requirement_info info() noexcept = 0;
			virtual std::type_index id() const noexcept = 0;
			virtual void init(context_interface& ci) = 0;
			virtual ~system_interface() = default;
		};

		struct system_ref : Tool::atomic_reference_count
		{
			SystemLayout layout;
			system_interface* ptr;
			void call(context& c) { ptr->call(c); }
			void destory() noexcept;
			operator bool() const noexcept { return ptr != nullptr; }
			std::type_index id() const noexcept { return ptr->id(); }
			system_requirement_info info() noexcept { return ptr->info(); }
			void init(context_interface& ci) { ptr->init(ci); }
			SystemSequence check_sequence(std::type_index ti) { return ptr->check_sequence(ti); }
			~system_ref();
		};

		using system_ptr = Tool::intrusive_ptr<system_ref>;

		template<typename component> struct system_storage_element
		{
			singleton_component_ptr ptr;
			bool update(context_interface& ci)
			{
				ptr = ci.get_singleton_component(typeid(component));
				return ptr && *ptr;
			}
			bool init(context_interface& ci) { return true; }
			bool pos_update(context_interface& ci) { return true; }
			component get() { return ptr->cast<std::decay_t<component>>(); }
		};

		template<typename ...component> struct system_storage_element<pre_filter<component...>>
		{
			std::shared_ptr<pre_filter_storage<component...>> filter;
			bool update(context_interface& ci) { return true; }
			system_storage_element() : filter(std::make_shared<pre_filter_storage<component...>>()){}
			bool init(context_interface& ci) {
				ci.set_filter(filter);
				return true;
			}
			bool pos_update(context_interface& ci) { return true; }
			pre_filter_storage<component...>& get() { return *filter; }
		};

		template<typename ...component> struct system_storage_element<filter<component...>>
		{
			system_storage_element(){}
			bool init(context_interface& ci) { return true; }
			bool update(context_interface& ci) { return true; }
			bool pos_update(context_interface& ci) { return true; }
			filter<component...> get() { return filter<component...>{}; }
		};

		template<typename component> struct system_storage_element<provider<component>>
		{
			Tool::intrusive_ptr<Implement::event_pool_interface> pool_interface;
			Implement::event_pool<std::decay_t<component>> pool;
			system_storage_element() : pool_interface(make_event_pool_interface<component>()) {}
			bool update(context_interface& ci) { ci.set_event_pool(pool_interface); return true; }
			bool init(context_interface& ci) { return true; }
			bool pos_update(context_interface& ci) { pool.update(*pool_interface);  return true; }
			provider<component> get() { return provider<component>{pool}; }
			~system_storage_element() { pool_interface->avalible = false; }
		};

		template<typename component> struct system_storage_element<receiver<component>>
		{
			const Tool::intrusive_ptr<event_pool_interface>* array_ptr;
			size_t count;
			system_storage_element() {}
			bool update(context_interface& ci) { array_ptr = ci.get_event_pool(typeid(component), count); return true; }
			bool init(context_interface& ci) {  return true; }
			bool pos_update(context_interface& ci) { return true; }
			receiver<component> get() { return receiver<component>{array_ptr, count}; }
		};

		template<typename index, typename ...other_type>
		struct system_storage_implement;
		template<size_t ...index, typename ...other_type>
		struct system_storage_implement<std::integer_sequence<size_t, index...>, other_type...>
		{
			std::tuple<system_storage_element<other_type>...> storage;
			bool update(context_interface& ci) { return is_all_true(std::get<index>(storage).update(ci)..., true); }
			void pos_update(context_interface& ci) { is_all_true(std::get<index>(storage).pos_update(ci)..., true);}
			void init(context_interface& ci) { is_all_true(true, std::get<index>(storage).init(ci)...); }
			template<typename callable> void call(context& c, callable&& cb)
			{
				std::forward<callable>(cb)(c, std::get<index>(storage).get()...);
			}
		};
		template<>
		struct system_storage_implement<std::integer_sequence<size_t>>
		{
			bool update(context_interface& ci) { return true; }
			void init(context_interface& ci) { }
			void pos_update(context_interface& ci) {  }
			template<typename callable> void call(context& c, callable&& cb)
			{
				std::forward<callable>(cb)(c);
			}
		};


		template<typename context_t, typename ...other_type>
		struct system_storage : system_storage_implement<std::make_index_sequence<sizeof...(other_type)>, other_type...> {};

		template<typename implement> class system_implement : public system_interface, implement
		{
			using function_type = Tmp::pick_func<typename Tmp::degenerate_func<Tmp::extract_func_t<implement>>::type>;
			using true_type = typename std::decay_t<typename function_type::template out<system_requirement_detect>>::type;
			using storage_type = typename std::decay_t<typename function_type::template out<system_storage>>;
			storage_type storage;
		public:
			virtual void init(context_interface& ci) override
			{
				storage.init(ci);
			}
			virtual std::type_index id() const noexcept { return typeid(implement); }
			virtual void call(context& c) override {
				
				if (storage.update(c.ref))
				{
					storage.call(c, [this](auto&& ...at) {
						(*static_cast<implement*>(this))(std::forward<decltype(at) && >(at)...);
						//implment::operator()(c);
					});
					storage.pos_update(c.ref);
				}
			}
			virtual SystemSequence check_sequence(std::type_index ti) noexcept override { return implement::check_sequence(ti); }
			virtual system_requirement_info info() noexcept override 
			{ 
				return system_requirement_info_instance<
					typename true_type::write,
					typename true_type::read
				>();
			}
			template<typename ...CP>
			system_implement(CP&& ...cp) : implement(std::forward<CP>(cp)...) {}
		};
	}

	// context **************************************************
	namespace Implement
	{
		struct context_interface
		{
			virtual component_ptr allocate_component(std::type_index, size_t type, size_t aligna, void(*deleter)(void*) noexcept) = 0;
			virtual singleton_component_ptr allocate_singleton_component(std::type_index, size_t type, size_t aligna, void(*deleter)(void*) noexcept) = 0;
			virtual system_ptr allocate_system(std::type_index, size_t type, size_t aligna) = 0;
			virtual entity create_entity() = 0;
			virtual void close_context() noexcept = 0;
			virtual void set_filter(std::shared_ptr<pre_filter_storage_interface>) = 0;
			virtual singleton_component_ptr get_singleton_component(std::type_index) noexcept = 0;
			virtual void set_event_pool(Tool::intrusive_ptr<event_pool_interface> in) = 0;
			virtual const Tool::intrusive_ptr<event_pool_interface>* get_event_pool(std::type_index id, size_t& size) noexcept = 0;
		};
	}

	class context
	{
		Implement::context_interface& ref;

		friend class context_implement;
		template<typename implement> friend class Implement::system_implement;
		virtual void insert_component(Implement::component_ptr, Implement::entity_ptr) = 0;
		virtual void insert_singleton_component(Implement::singleton_component_ptr) = 0;
		virtual void insert_system(Implement::system_ptr) = 0;
		virtual void destory_entity(Implement::entity_ptr) = 0;
		virtual void destory_component(Implement::entity_ptr, std::type_index) = 0;

	public:

		virtual void destory_singleton_component(std::type_index) = 0;
		virtual void destory_system(std::type_index) = 0;

		void close_context() noexcept { ref.close_context(); }
		entity create_entity() { return ref.create_entity(); }
		context(Implement::context_interface& ci) : ref(ci) {}
		context(const context&) = delete;

		void destory_entity(entity e) { destory_entity(std::move(e.ptr)); }
		void destory_component(entity e, std::type_index ti) { destory_component(std::move(e.ptr), ti); }
		
		
		template<typename componenet_t>
		void destory_component(entity e) { destory_component(std::move(e), typeid(componenet_t)); }
		template<typename componenet_t>
		void destory_singleton_component() { destory_singleton_component(typeid(componenet_t)); }
		template<typename system_t>
		void destory_system() { destory_system(typeid(system_t)); }

		template<typename component_type, typename ...construction_para> void create_component(const entity& ep, construction_para&& ...cp)
		{
			if (ep)
			{
				auto holder = ref.allocate_component(typeid(component_type), sizeof(component_type), alignof(component_type), [](void* da) noexcept  { static_cast<component_type*>(da)->~component_type(); });
				new(holder->data) component_type{std::forward<construction_para>(cp)...};
				insert_component(std::move(holder), ep.ptr);
			}
		}

		template<typename component_type, typename ...construction_para> void create_singleton_component(construction_para&& ...cp)
		{
			auto holder = ref.allocate_singleton_component(typeid(component_type), sizeof(component_type), alignof(component_type), [](void* da) noexcept { static_cast<component_type*>(da)->~component_type(); });
			new(holder->data) component_type{ std::forward<construction_para>(cp)... };
			insert_singleton_component(std::move(holder));
		}

		template<typename system_type, typename ...construction_para> void create_system(construction_para&& ...cp)
		{
			using final_type = Implement::system_implement<std::decay_t<system_type>>;
			auto holder = ref.allocate_system(typeid(final_type), sizeof(final_type), alignof(final_type));
			final_type* ptr = new(holder->ptr) final_type{ std::forward<construction_para>(cp)... };
			holder->ptr = ptr;
			holder->layout = system_type::layout();
			insert_system(std::move(holder));
			//temporary_system_context_holder.emplace_back(Implement::context_system_holder{ std::move(holder) });
		}
	};


























	/*

	namespace Implement
	{
		struct context_interface
		{
			virtual Implement::component_holder_ptr allocate_component(std::type_index, size_t type, size_t aligna, void*& component_out) = 0;
			virtual Implement::system_holder_ptr allocate_system(std::type_index, size_t type, size_t aligna, void*& system_out) = 0;
			virtual entity create_entity() = 0;
			virtual void close_context() noexcept = 0;
		};

	}
	

	//only call in thread;
	class context
	{
		Implement::context_interface& ref;
		
		friend class context_implement;
		virtual void insert(Implement::component_holder_ptr, Implement::entity_implement_ptr) = 0;
		virtual void insert(Implement::system_holder_ptr) = 0;
	public:
		void close_context() noexcept { ref.close_context(); }
		entity create_entity() { return ref.create_entity(); }
		context(Implement::context_interface& ci) : ref(ci) {}
		context(const context&) = delete;
		
		template<typename component_type, typename ...construction_para> void create_component(const entity& ep, construction_para&& ...cp)
		{
			if (ep)
			{
				using final_type = Implement::component_implement<std::decay_t<component_type>>;
				void* component_out;
				auto holder = ref.allocate_component(typeid(component_type), sizeof(final_type), alignof(final_type), component_out);
				holder->ptr = Implement::ecs_unique_ptr<Implement::component_inside_ptr>{ new(component_out) Implement::component_implement<std::decay_t<component_type>>{std::forward<construction_para>(cp)...} };
				insert(std::move(holder), ep.ptr);
				//temporary_component_context_holder.emplace_back(std::move(holder), std::move(ep));
			}
		}

		template<typename component_type, typename ...construction_para> void create_singleton_component(construction_para&& ...cp)
		{
			using final_type = Implement::component_implement<std::decay_t<component_type>>;
			void* component_out;
			auto holder = ref.allocate_component(typeid(component_type), sizeof(final_type), alignof(final_type), component_out);
			holder->ptr = Implement::ecs_unique_ptr<Implement::component_inside_ptr>{ new(component_out) Implement::component_implement<std::decay_t<component_type>>{std::forward<construction_para>(cp)...} };
			insert(std::move(holder), nullptr);
			//temporary_component_context_holder.emplace_back(std::move(holder), nullptr);
		}

		template<typename system_type, typename ...construction_para> void create_system(construction_para&& ...cp)
		{
			using final_type = Implement::system_implement<std::decay_t<system_type>>;
			void* system_out;
			auto holder = ref.allocate_system(typeid(final_type), sizeof(final_type), alignof(final_type), system_out);
			holder->ptr = Implement::ecs_unique_ptr<Implement::system_inside_ptr>{ new(system_out) final_type{std::forward<construction_para>(cp)...} };
			insert(std::move(holder));
			//temporary_system_context_holder.emplace_back(Implement::context_system_holder{ std::move(holder) });
		}
	};
	*/
}