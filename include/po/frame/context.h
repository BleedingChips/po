#pragma once
#include "../platform/asynchronous_affairs.h"
#include "../platform/platform.h"
#include "../tool/tool.h"
#include "object_pool.h"
#include <deque>
#include <set>
#include <map>
#include <array>
namespace PO::ECSFramework
{
	// pre define **************************************************
	namespace Implement
	{
		template<typename T> struct default_deleter { void operator ()(T* da) const noexcept { if (da != nullptr) { da->~T(); } } };
		template<typename T> using ecs_intrusive_ptr = PO::Tool::intrusive_ptr<T, default_deleter<T>>;
	}

	// component **************************************************
	namespace Implement
	{
		class component_ref :
			object_pool_base_deleter<component_ref>
		{
			mutable PO::Tool::atomic_reference_count ref;
			bool avalible = true;
			std::type_index defined_id;
			void* data;
			void(*deleter)(void*) noexcept;
		public:
			void add_ref() const noexcept { ref.add_ref(); }
			bool sub_ref() const noexcept { return ref.sub_ref(); }
			void destory() noexcept;
			operator bool() const noexcept { return avalible; }
			std::type_index id() const noexcept { return defined_id; }
			template<typename T> T& cast() noexcept { return *static_cast<T*>(data); }
			template<typename T> const T& cast() const noexcept { return *static_cast<T*>(data); }
			~component_ref();
		};

		using component_ptr = ecs_intrusive_ptr<component_ref>;
	}

	// entity **************************************************
	namespace Implement
	{
		class entity_implement
		{
			std::unordered_map<std::type_index, component_ptr> component_set;
		public:
			void insert(component_ptr) noexcept;
			bool check_exist(std::type_index) const noexcept;
			component_ptr get_component(std::type_index ti) noexcept;
			component_ptr get_component(std::type_index ti) const noexcept;
		};

		class entity_ref :
			object_pool_base_deleter<entity_implement>
		{
			mutable PO::Tool::atomic_reference_count ref;
			bool avalible = true;
			entity_implement* shift() noexcept { return reinterpret_cast<entity_implement*>(this + 1); }
			const entity_implement* shift() const noexcept { return reinterpret_cast<const entity_implement*>(this + 1); }
		public:
			void add_ref() const noexcept { ref.add_ref(); }
			bool sub_ref() const noexcept { return ref.sub_ref(); }
			bool check_exist(std::type_index ti) const noexcept { return shift()->check_exist(ti); }
			void insert(component_ptr chp) noexcept { return shift()->insert(std::move(chp)); }
			component_ptr get_component(std::type_index ti) noexcept { return shift()->get_component(ti); }
			component_ptr get_component(std::type_index ti) const noexcept { return shift()->get_component(ti); }
			void destory() noexcept;
			operator bool() const noexcept { return avalible; }
			~entity_ref();
		};

		using entity_ptr = Implement::ecs_intrusive_ptr<entity_ref>;
	}

	template<typename ...implement> class filter;

	class entity
	{
		Implement::entity_ptr ptr;
		friend class context_implement;
		friend class context;
		template<typename ...implement> friend class filter;
		entity(Implement::entity_ptr eip) : ptr(std::move(eip)) {}
	public:
		entity(const entity&) = default;
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
			T& get() noexcept { return ptr->cast<T>(); }
		};

		template<typename T> struct system_requirement_storage<const T>
		{
			component_ptr ptr;
			operator bool() const noexcept { assert(ptr); return *ptr; }
			bool able_extract_component(entity_ptr e) noexcept { ptr = e->get_component(typeid(T)); return ptr; }
			const T& get() const noexcept { return ptr->cast<T>(); }
		};
	}

	// component_map **************************************************
	namespace Implement
	{
		struct filter_storage_interface
		{
			virtual void insert(entity_ptr) = 0;
			virtual void reflesh(size_t vision) = 0;
			virtual ~filter_storage_interface() = default;
		};

		struct component_holder
		{
			component_ptr componenet;
			entity_ptr entity;
		};

		struct componenet_map
		{
			struct element
			{
				std::vector<component_holder> ptr;
				std::vector<std::weak_ptr<filter_storage_interface>> accosiate_filter;
			};

			std::unordered_map<std::type_index, element> all_component_map;
			void insert(component_holder c);
			void reflesh(std::type_index ti, size_t vision);
		};
	}


	// filter **************************************************
	namespace Implement
	{

		struct context_interface
		{
			virtual component_ptr allocate_component(std::type_index, size_t type, size_t aligna) = 0;
			virtual system_ptr allocate_system(std::type_index, size_t type, size_t aligna, void*& system_out) = 0;
			virtual entity create_entity() = 0;
			virtual void close_context() noexcept = 0;
			virtual void set_filter(std::type_index, std::weak_ptr<filter_storage_interface>) = 0;
			virtual component_ptr find_component_ref(void* da) = 0;
		};


		template<typename index, typename ...component> struct filter_storage_implement;
		template<size_t ...index, typename ...component>
		class filter_storage_implement<std::integer_sequence<size_t, index...>, component...>
		{
			std::tuple<entity_ptr, system_requirement_storage<component>...> storage;
		public:
			filter_storage_implement() = default;
			filter_storage_implement(const filter_storage_implement&) = default;
			filter_storage_implement(filter_storage_implement&&) = default;
			operator bool() const noexcept { return *std::get<0>(storage) && std::get<index + 1>(storage) && ... && true; }
			bool able_extract_component(entity_ptr e) noexcept {
				assert(e && *e);
				std::get<0>(storage) = e;
				return std::get<index + 1>(storage).able_extract_component(e) && ... && true;
			}
			template<typename callable> decltype(auto) operator()(callable&& f)
			{
				assert(*this);
				return std::forward<callable>(f)(std::get<0>(storage), std::get<index + 1>(storage).get()...);
			}
		};

		template<typename ...component>
		class filter_storage : filter_storage_interface
		{
			using storage_type = filter_storage_implement<std::make_integer_sequence<size_t, sizeof...(component)>, component...>;
			size_t current_vision = 0;
			std::vector<storage_type> storage;

		public:

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

			template<typename callable> void operator()(callable&& c, size_t vision)
			{
				for (auto& ite : storage)
				{
					assert(ite && *ite);
					ite(c);
				}
			}

		};

	}

	template<typename ...component> class filter
	{
		Implement::filter_storage<component...>& storage;
	public:
		filter(Implement::filter_storage<component...>& s) : storage(s) {}
		filter(const filter&) = default;
		template<typename callable> void operator()(callable&& c) { storage(std::forwrd<callable>(c)); }
	};

	// event **************************************************
	namespace Implement
	{
		struct event_manager {};
	}

	template<typename componenet> struct event_provider {};
	template<typename componenet> struct event_receiver {};


	// system detected *************************************************
	namespace Implement
	{
		template<typename T> struct is_const;
		template<typename T> struct is_const<T&> : std::false_type {};
		template<typename T> struct is_const<T&&> : std::false_type {};
		template<typename T> struct is_const<const T&> : std::true_type {};

		template<typename ...component_t> struct ctl
		{
			template<template<typename ...> class out> using outpacket = out<component_t...>;
			constexpr static const size_t size = sizeof...(component_t);
		};

		template<typename read_t, typename write_t>
		struct rw_type
		{
			using read = read_t;
			using write = write_t;
		};

		template<typename read_out, typename write_out, typename ...component> struct system_requirement_detect_step3;
		template<typename ...read_out, typename ...write_out> struct system_requirement_detect_step3<ctl<read_out...>, ctl<write_out...>>
		{
			using type = rw_type<ctl<read_out...>, ctl<write_out...>>;
		};

		template<typename ...read_out, typename ...write_out, typename this_requirement, typename ...other_requirement> 
		struct system_requirement_detect_step3<ctl<read_out...>, ctl<write_out...>, this_requirement, other_requirement...>
		{
			using type = typename system_requirement_detect_step3<
				std::conditional_t<Tmp::is_one_of<this_requirement, read_out...>::value || Tmp::is_one_of<this_requirement, write_out...>::value,
					ctl<read_out...>,
					ctl<read_out..., this_requirement>
				>,
				ctl<write_out...>,
				other_requirement...
			>::type;
		};

		template<typename read_out, typename write_out, typename ...component>
		struct system_requirement_detect_step2;

		template<typename ...read_out, typename ...write_out, typename this_component, typename ...other_component> 
		struct system_requirement_detect_step2<ctl<read_out...>, ctl<write_out...>, this_component, other_component...>
		{
			using type = typename system_requirement_detect_step2<
				std::conditional_t<is_const<this_component>::value, ctl<read_out..., std::decay_t<this_component>>, ctl<read_out...>>,
				std::conditional_t<is_const<this_component>::value, ctl<write_out...>, ctl<write_out..., std::decay_t<this_component>>>,
				other_component...
			>::type;
		};

		template<typename ...read_out, typename ...write_out>
		struct system_requirement_detect_step2<ctl<read_out...>, ctl<write_out...>>
		{
			using type = typename system_requirement_detect_step3<ctl<>, Tmp::remove_repeat_t<ctl<write_out...>>, read_out...>::type;
		};

		template<typename target, typename output> struct insert_filter_type;
		template<typename target, typename ...output, template<typename ...> class ft> struct insert_filter_type<target, ft<output...>> { using type = ft<output...>; };
		template<typename ...target, typename ...output, template<typename ...> class ft> struct insert_filter_type<filter<target...>, ft<output...>>
		{
			using type = ft<output..., target...>;
		};

		template<typename target, typename output> struct insert_singleton_type;
		template<typename target, typename ...output, template<typename ...> class ft> struct insert_singleton_type<target, ft<output...>> { using type = ft<output..., target>; };
		template<typename ...target, typename ...output, template<typename ...> class ft> struct insert_singleton_type<filter<target...>, ft<output...>>
		{
			using type = ft<output...>;
		};

		template<typename filter_type, typename singleton_type> struct system_requirement;
		template<typename ...filter_type, typename ...singleton_type> struct system_requirement<ctl<filter_type...>, ctl<singleton_type...>>
		{
			using filter = typename system_requirement_detect_step2<ctl<>, ctl<>, filter_type...>::type;
			using singleton = typename system_requirement_detect_step2<ctl<>, ctl<>, singleton_type...>::type;
		};

		template<typename filter_type, typename singleton_type, typename ...requirement> struct system_requirement_detect_step1
		{
			using type = system_requirement<filter_type, singleton_type>;
		};
		template<typename filter_type, typename singleton_type, typename this_requirement, typename ...requirement>
		struct system_requirement_detect_step1<filter_type, singleton_type, this_requirement, requirement...>
		{
			using type = typename system_requirement_detect_step1<
				typename insert_filter_type<this_requirement, filter_type>::type,
				typename insert_singleton_type<this_requirement, singleton_type>::type,
				requirement...
			>::type;
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

		struct system_requirement_info
		{
			const type_index_view read;
			const type_index_view write;
			const type_index_view singleton_read;
			const type_index_view singleton_write;
		};

		template<typename er, typename ew, typename sr, typename sw> struct system_requirement_info_initializer;
		template<typename ...er, typename ...ew, typename ...sr, typename ...sw>
		struct system_requirement_info_initializer<ctl<er...>, ctl<ew...>, ctl<sr...>, ctl<sw...>>
		{
			type_array<er...> read;
			type_array<ew...> write;
			type_array<sr...> single_read;
			type_array<sw...> single_write;
			system_requirement_info view() const noexcept { return  system_requirement_info{ read.view(), write.view(), single_read.view(), single_write.view() }; }
		};

		template<typename er, typename ew, typename sr, typename sw>
		const system_requirement_info& system_requirement_info_instance()
		{
			static system_requirement_info_initializer<er, ew, sr, sw> init;
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
			virtual void operator()(context&) = 0;
			virtual SystemSequence check_sequence(std::type_index ti) noexcept = 0;
			virtual system_requirement_info info() noexcept = 0;
			virtual std::type_index id() const noexcept = 0;
		};

		struct system_ref : Tool::atomic_reference_count
		{
			SystemLayout layout;
			std::unique_ptr<system_interface> ptr;
			void operator()(context& c) { (*ptr)(c); }
			void destory();
		};

		using system_ptr = Tool::intrusive_ptr<system_ref>;

		template<typename component> struct system_storage_element
		{
			component_ptr ptr;
			void update();
		};

		template<typename ...component> struct system_storage_element<filter<component...>>
		{
			std::shared_ptr<filter_storage<component...>> filter;

		};

		template<typename context_t, typename ...other_type>
		struct system_storage
		{
			std::tuple<other_type...> 
		};

		template<typename implement> class system_implement : public system_interface, implement
		{
			using function_type = Tmp::pick_func<typename Tmp::degenerate_func<Tmp::extract_func_t<implement>>::type>;
			using true_type = typename std::decay_t<typename function_type::template out<system_requirement_detect>>::type;
		public:
			virtual std::type_index id() const noexcept { return typeid(implement); }
			virtual void operator()(context&) override {}
			virtual SystemSequence check_sequence(std::type_index ti) noexcept override { return implement::check_sequence(ti); }
			virtual system_requirement_info info() noexcept override 
			{ 
				return system_requirement_info_instance<
					typename true_type::filter::read,
					typename true_type::filter::write,
					typename true_type::singleton::read,
					typename true_type::singleton::write
				>();
			}
			template<typename ...CP>
			system_implement(CP&& ...cp) : implement(std::forward<CP>(cp)...) {}
		};
	}

	// context **************************************************
	class context
	{
		Implement::context_interface& ref;

		friend class context_implement;
		virtual void insert(Implement::component_ptr, Implement::entity_ptr) = 0;
		virtual void insert(Implement::system_ptr) = 0;
		virtual void destory(Implement::entity_ptr) = 0;
		virtual void destory(Implement::component_ptr) = 0;

	public:

		void close_context() noexcept { ref.close_context(); }
		entity create_entity() { return ref.create_entity(); }
		context(Implement::context_interface& ci) : ref(ci) {}
		context(const context&) = delete;

		void destory(entity e) { destory(std::move(e.ptr)); }
		virtual void destory_system(std::type_index) = 0;
		
		template<typename T>
		bool find_component_ref(T& t)
		{
			auto ptr = ref.find_component_ref(&t);
			if (ptr)
				return (destory(ptr), true);
			return false;
		}

		template<typename component_type, typename ...construction_para> void create_component(const entity& ep, construction_para&& ...cp)
		{
			if (ep)
			{
				auto holder = ref.allocate_component(typeid(component_type), sizeof(component_type), alignof(component_type));
				new(holder->data) component_type{std::forward<construction_para>(cp)...};
				holder->deleter = [](void* d) { static_cast<component_type*>(d)->~component_type();  };
				insert(std::move(holder), ep.ptr);
			}
		}

		template<typename component_type, typename ...construction_para> void create_singleton_component(construction_para&& ...cp)
		{
			auto holder = ref.allocate_component(typeid(component_type), sizeof(component_type), alignof(component_type));
			new(holder->data) component_type{ std::forward<construction_para>(cp)... };
			holder->deleter = [](void* d) { static_cast<component_type*>(d)->~component_type();  };
			insert(std::move(holder), nullptr);
		}

		template<typename system_type, typename ...construction_para> void create_system(construction_para&& ...cp)
		{
			using final_type = Implement::system_implement<std::decay_t<system_type>>;
			void* system_out;
			auto holder = ref.allocate_system(typeid(final_type), sizeof(final_type), alignof(final_type), system_out);
			holder->ptr = std::unique_ptr<Implement::system_interface>{ new(system_out) final_type{ std::forward<construction_para>(cp)... } };
			holder->layout = system_type::layout();
			insert(std::move(holder));
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