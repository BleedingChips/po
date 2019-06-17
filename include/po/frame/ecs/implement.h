#pragma once
#include "component_pool.h"
#include "gobal_component_pool.h"
#include "event_pool.h"
#include "system_pool.h"

namespace PO::ECS
{
	using duration_ms = std::chrono::milliseconds;

	struct ContextImplement : Context
	{
		void loop();
		virtual void exit() noexcept override;
		void set_duration(std::chrono::milliseconds ds) noexcept { m_target_duration = ds; }
		void set_thread_reserved(size_t tr) noexcept { m_thread_reserved = tr; }
		ContextImplement() noexcept;
	private:
		virtual operator Implement::ComponentPoolInterface* () override;
		virtual operator Implement::GobalComponentPoolInterface* () override;
		virtual operator Implement::EventPoolInterface* () override;
		virtual operator Implement::SystemPoolInterface* () override;
		virtual Implement::EntityInterfacePtr create_entity_imp() override;
		virtual float duration_s() const noexcept override;
		static void append_execute_function(ContextImplement*) noexcept;

		std::atomic_bool m_available;
		size_t m_thread_reserved = 0;
		std::chrono::milliseconds m_target_duration;
		std::atomic<std::chrono::milliseconds> m_last_duration;
		Implement::MemoryPageAllocator allocator;
		Implement::ComponentPool component_pool;
		Implement::GobalComponentPool gobal_component_pool;
		Implement::EventPool event_pool;
		Implement::SystemPool system_pool;
	};
}





























/*
namespace PO::ECSFramework
{
	// error *********************************************************************

	namespace Error
	{
		struct ClassLayoutConflict
		{
			std::type_index m_old;
			std::type_index m_new;
		};
	}

	// entity ********************************************************************
	
	namespace Implement
	{

		struct ComponentMemoryPageDesc;
		struct ContextState : Tool::intrusive_object<ContextState>
		{
			std::atomic_bool m_available;
		};

		struct EntityImp : Tool::intrusive_object<EntityImp>
		{
			operator bool() const { return m_context_ref; }
			bool remove(std::type_index id, ComponentMemoryPageDesc* desc, size_t index);
			std::tuple<ComponentMemoryPageDesc*, size_t> insert(std::type_index id, ComponentMemoryPageDesc* desc, size_t index);
			std::tuple<ComponentMemoryPageDesc*, size_t> read(std::type_index id) const;
			template<typename ...CompT> std::array<std::tuple<ComponentMemoryPageDesc*, size_t>, sizeof...(CompT)> read_tuple() const
			{
				std::unique_lock ul(m_comps_mutex);
				return { read_implement(typeid(CompT))... };
			}
		private:
			std::tuple<ComponentMemoryPageDesc*, size_t> read_implement(std::type_index id) const;
			mutable std::shared_mutex m_comps_mutex;
			std::map<std::type_index, std::tuple<ComponentMemoryPageDesc*, size_t>> m_components;
			Tool::intrusive_ptr<ContextState> m_context_ref;
		};

		using EntityImpPtr = Tool::intrusive_ptr<EntityImp>;

		struct MemoryPageAllocator
		{
			static std::tuple<size_t, size_t> calculate_space(size_t align, size_t size);
			void* allocate(size_t mulity);
			void release(void* input, size_t mulity);
			~MemoryPageAllocator();
		private:
			struct RawPageHead
			{
				RawPageHead* m_next_page = nullptr;
			};
			std::mutex m_page_mutex;
			std::vector<RawPageHead*> m_pages;
		};

		struct SimilerComponentPool;

		struct ComponentMemoryPageDesc
		{
			struct Control
			{
				void (*m_deconstructor)(void*) = nullptr;
				EntityImpPtr m_owner;
				operator bool() const { return m_deconstructor != nullptr && m_owner; }
			};
			const SimilerComponentPool* m_owner = nullptr;
			ComponentMemoryPageDesc* m_front_page = nullptr;
			ComponentMemoryPageDesc* m_next_page = nullptr;

			ComponentMemoryPageDesc* m_next_available_page = nullptr;

			size_t m_available_count = 0;

			size_t m_poll_count = 0;

			Control* m_control = nullptr;
			EntityImpPtr* m_poll_owner = nullptr;
			std::byte* m_components = nullptr;
			ComponentMemoryPageDesc* m_poll_next_page;
		};

		struct SimilerComponentPool
		{
			struct ReadWrapper;
			SimilerComponentPool(Tool::type_layout layout);
			SimilerComponentPool(const SimilerComponentPool&) = delete;

			bool is_same_layout(Tool::type_layout input) const noexcept {
				return input == m_layout;
			}

			template<typename Type, typename ...Parameters> Type& construction(MemoryPageAllocator& allocator, EntityImpPtr owner, Parameters&& ...para);
			void deconstruction(ComponentMemoryPageDesc*, size_t index);

			~SimilerComponentPool();
			void update(MemoryPageAllocator& allocator);
			void clean(MemoryPageAllocator& allocator);
			std::type_index id() const { return m_layout.id; }

			ReadWrapper read_lock() noexcept;
			void read_unlock() noexcept;

		private:

			ComponentMemoryPageDesc* allocate_page(MemoryPageAllocator& allocator) const;
			void free_page(MemoryPageAllocator& allocator, ComponentMemoryPageDesc*) noexcept;
			void release_component(ComponentMemoryPageDesc* desc, size_t index);
			std::tuple<ComponentMemoryPageDesc*, size_t> find_available_block(MemoryPageAllocator& allocator);
			void finish_construction(EntityImpPtr owner, void (*deconstructor)(void*) noexcept, ComponentMemoryPageDesc* desc, size_t index);

			Tool::type_layout m_layout;
			size_t m_page_space;
			size_t m_component_count;
			ComponentMemoryPageDesc* m_record_top_page = nullptr;
			ComponentMemoryPageDesc* m_record_last_page = nullptr;
			ComponentMemoryPageDesc* m_available_top_page = nullptr;
			std::mutex m_record_mutex;
			size_t m_record_count = 0;
			std::vector<std::tuple<ComponentMemoryPageDesc*, size_t>> m_constructed_comps;
			std::vector<std::tuple<ComponentMemoryPageDesc*, size_t>> m_need_deleted_comps;

			std::shared_mutex m_poll_mutex;
			uint64_t m_version = 0;
			size_t m_poll_count = 0;
			ComponentMemoryPageDesc* m_poll_top_page = nullptr;
		};

		template<typename Type, typename ...Parameters> Type& SimilerComponentPool::construction(MemoryPageAllocator& allocator, EntityImpPtr owner, Parameters&& ...para)
		{
			assert(is_same_layout(typeid(Type), alignof(Type), sizeof(Type)));
			std::lock_guard lg(m_record_mutex);
			ComponentMemoryPageDesc* desc;
			size_t index;
			std::tie(desc, index) = find_available_block(allocator);
			Type* result = new (desc->m_components + sizeof(Type) * index) Type(std::forward<Parameters>(para)...);
			finish_construction(std::move(owner), [](void* target) noexcept {reinterpret_cast<Type*>(target)->~Type(); }, desc, index);
			return *result;
		}

		struct SimilerComponentPool::ReadWrapper
		{
			struct Iterator;
			bool is_same_layout(Tool::type_layout layout) const noexcept { return m_ref->is_same_layout(layout); }
			uint64_t get_version() const noexcept { return m_ref->m_version; }
			size_t get_component_count() const noexcept { return m_ref->m_poll_count; }
			std::type_index id() const noexcept { return m_ref->id(); }
			operator bool() { return m_ref; }
			ReadWrapper() : m_ref(nullptr) {}
			Iterator get_iterator() const noexcept;
		private:
			friend struct SimilerComponentPool;
			ReadWrapper(SimilerComponentPool& ref) : m_ref(&ref) {}
			SimilerComponentPool* m_ref;
		};

		struct SimilerComponentPool::ReadWrapper::Iterator
		{
			Iterator(ComponentMemoryPageDesc* desc, size_t size, size_t count);
			Iterator& operator++() noexcept;
			bool operator==(const Iterator& ite) const noexcept;
			operator bool() const noexcept;
			Implement::EntityImpPtr* get_entity() const noexcept;
			template<typename CompT> CompT* get_component() const noexcept { return static_cast<CompT*>(get_component_imp()); }
		private:
			void* get_component_imp() const noexcept;
			ComponentMemoryPageDesc* m_desc;
			size_t m_comp_count;
			size_t m_comp_size;
			size_t m_index;
		};

		struct ComponentPool
		{
			template<typename Type, typename ...Parameters> Type& construction(EntityImpPtr owner, Parameters&& ...para);
			template<typename Type> bool deconstruction(EntityImpPtr owner) { return deconstruction(std::move(owner), typeid(Type), alignof(Type), sizeof(Type)); }
			~ComponentPool();
			bool read_lock(const std::set<Tool::type_layout>&);
			std::tuple<bool, SimilerComponentPool::ReadWrapper> read_lock_and_update_version(const std::set<Tool::type_layout>&, std::map<std::type_index, size_t>& version);
			void read_unlock(const std::set<Tool::type_layout>&);
			SimilerComponentPool::ReadWrapper read_lock_single(std::type_index);
			void read_unlock(std::type_index);
		private:
			bool deconstruction(EntityImpPtr owner, Tool::type_layout layout);
			SimilerComponentPool* find_pool(Tool::type_layout layout);
			MemoryPageAllocator allocator;
			std::shared_mutex m_components_mutex;
			std::map<std::type_index, SimilerComponentPool> m_components;
		};

		template<typename Type, typename ...Parameters> Type& ComponentPool::construction(EntityImpPtr owner, Parameters && ...para)
		{
			auto re = find_pool(Tool::type_layout::create<Type>());
			return re->construction(allocator, std::move(owner), std::forward<Parameters>(para)...);
		}
	}

	// component *************************************************************************
	
	static constexpr size_t min_page_comp_count = 16;
	static constexpr size_t system_memory_control_block_obligate = 200;
	static constexpr size_t memory_page_space = 4096;
	static constexpr size_t memory_page_max_cache_count = 3;

	namespace Implement
	{

		struct MemoryPageAllocator
		{
			static std::tuple<size_t, size_t> calculate_space(size_t align, size_t size);
			void* allocate(size_t mulity);
			void release(void* input, size_t mulity);
			~MemoryPageAllocator();
		private:
			struct RawPageHead
			{
				RawPageHead* m_next_page = nullptr;
			};
			std::mutex m_page_mutex;
			std::vector<RawPageHead*> m_pages;
		};

		struct SimilerComponentPool;

		struct ComponentMemoryPageDesc
		{
			struct Control
			{
				void (*m_deconstructor)(void*) = nullptr;
				EntityImpPtr m_owner;
				operator bool() const { return m_deconstructor != nullptr && m_owner; }
			};
			const SimilerComponentPool* m_owner = nullptr;
			ComponentMemoryPageDesc* m_front_page = nullptr;
			ComponentMemoryPageDesc* m_next_page = nullptr;

			ComponentMemoryPageDesc* m_next_available_page = nullptr;

			size_t m_available_count = 0;

			size_t m_poll_count = 0;

			Control* m_control = nullptr;
			EntityImpPtr* m_poll_owner = nullptr;
			std::byte* m_components = nullptr;
			ComponentMemoryPageDesc* m_poll_next_page;
		};

		struct SimilerComponentPool
		{
			struct ReadWrapper;
			SimilerComponentPool(Tool::type_layout layout);
			SimilerComponentPool(const SimilerComponentPool&) = delete;

			bool is_same_layout(Tool::type_layout input) const noexcept {
				return input == m_layout;
			}

			template<typename Type, typename ...Parameters> Type& construction(MemoryPageAllocator& allocator, EntityImpPtr owner, Parameters&& ...para);
			void deconstruction(ComponentMemoryPageDesc*, size_t index);
			
			~SimilerComponentPool();
			void update(MemoryPageAllocator& allocator);
			void clean(MemoryPageAllocator& allocator);
			std::type_index id() const { return m_layout.id; }

			ReadWrapper read_lock() noexcept;
			void read_unlock() noexcept;

		private:

			ComponentMemoryPageDesc* allocate_page(MemoryPageAllocator& allocator) const;
			void free_page(MemoryPageAllocator& allocator, ComponentMemoryPageDesc*) noexcept;
			void release_component(ComponentMemoryPageDesc* desc, size_t index);
			std::tuple<ComponentMemoryPageDesc*, size_t> find_available_block(MemoryPageAllocator& allocator);
			void finish_construction(EntityImpPtr owner, void (*deconstructor)(void*) noexcept, ComponentMemoryPageDesc* desc, size_t index);
			
			Tool::type_layout m_layout;
			size_t m_page_space;
			size_t m_component_count;
			ComponentMemoryPageDesc* m_record_top_page = nullptr;
			ComponentMemoryPageDesc* m_record_last_page = nullptr;
			ComponentMemoryPageDesc* m_available_top_page = nullptr;
			std::mutex m_record_mutex;
			size_t m_record_count = 0;
			std::vector<std::tuple<ComponentMemoryPageDesc*, size_t>> m_constructed_comps;
			std::vector<std::tuple<ComponentMemoryPageDesc*, size_t>> m_need_deleted_comps;

			std::shared_mutex m_poll_mutex;
			uint64_t m_version = 0;
			size_t m_poll_count = 0;
			ComponentMemoryPageDesc* m_poll_top_page = nullptr;
		};

		template<typename Type, typename ...Parameters> Type& SimilerComponentPool::construction(MemoryPageAllocator& allocator, EntityImpPtr owner, Parameters&& ...para)
		{
			assert(is_same_layout(typeid(Type), alignof(Type), sizeof(Type)));
			std::lock_guard lg(m_record_mutex);
			ComponentMemoryPageDesc* desc;
			size_t index;
			std::tie(desc, index) = find_available_block(allocator);
			Type* result = new (desc->m_components + sizeof(Type) * index) Type(std::forward<Parameters>(para)...);
			finish_construction(std::move(owner), [](void* target) noexcept {reinterpret_cast<Type*>(target)->~Type(); }, desc, index);
			return *result;
		}

		struct SimilerComponentPool::ReadWrapper
		{
			struct Iterator;
			bool is_same_layout(Tool::type_layout layout) const noexcept { return m_ref->is_same_layout(layout); }
			uint64_t get_version() const noexcept { return m_ref->m_version; }
			size_t get_component_count() const noexcept { return m_ref->m_poll_count; }
			std::type_index id() const noexcept { return m_ref->id(); }
			operator bool() { return m_ref; }
			ReadWrapper() : m_ref(nullptr) {}
			Iterator get_iterator() const noexcept;
		private:
			friend struct SimilerComponentPool;
			ReadWrapper(SimilerComponentPool& ref) : m_ref(&ref) {}
			SimilerComponentPool* m_ref;
		};

		struct SimilerComponentPool::ReadWrapper::Iterator
		{
			Iterator(ComponentMemoryPageDesc* desc, size_t size, size_t count);
			Iterator& operator++() noexcept;
			bool operator==(const Iterator& ite) const noexcept;
			operator bool() const noexcept;
			Implement::EntityImpPtr* get_entity() const noexcept;
			template<typename CompT> CompT* get_component() const noexcept { return static_cast<CompT*>(get_component_imp()); }
		private:
			void* get_component_imp() const noexcept;
			ComponentMemoryPageDesc* m_desc;
			size_t m_comp_count;
			size_t m_comp_size;
			size_t m_index;
		};

		struct ComponentPool
		{
			template<typename Type, typename ...Parameters> Type& construction(EntityImpPtr owner, Parameters&& ...para);
			template<typename Type> bool deconstruction(EntityImpPtr owner) { return deconstruction(std::move(owner), typeid(Type), alignof(Type), sizeof(Type)); }
			~ComponentPool();
			bool read_lock(const std::set<Tool::type_layout>&);
			std::tuple<bool, SimilerComponentPool::ReadWrapper> read_lock_and_update_version(const std::set<Tool::type_layout>&, std::map<std::type_index, size_t>& version);
			void read_unlock(const std::set<Tool::type_layout>&);
			SimilerComponentPool::ReadWrapper read_lock_single(std::type_index);
			void read_unlock(std::type_index);
		private:
			bool deconstruction(EntityImpPtr owner, Tool::type_layout layout);
			SimilerComponentPool* find_pool(Tool::type_layout layout);
			MemoryPageAllocator allocator;
			std::shared_mutex m_components_mutex;
			std::map<std::type_index, SimilerComponentPool> m_components;
		};

		template<typename Type, typename ...Parameters> Type& ComponentPool::construction(EntityImpPtr owner, Parameters&& ...para)
		{
			auto re = find_pool(Tool::type_layout::create<Type>());
			return re->construction(allocator, std::move(owner), std::forward<Parameters>(para)...);
		}
	}

	// filter ****************************************************

	namespace Implement
	{
		template<typename T, typename = std::void_t<>>
		struct CPD_ThreadSafe {
			static constexpr bool value = false;
		};

		template<typename T>
		struct CPD_ThreadSafe<T, std::enable_if_t<std::is_same_v<bool, std::remove_const_t<decltype(T::pro_thread_safe)>>>> {
			static constexpr bool value = T::pro_thread_safe;
		};

		template<typename ...AT> struct ComponentInfoExtractor
		{
			static void list_type_rwinfo(std::map<std::type_index, bool>& result) {}
			static void list_type_info(std::set<std::type_index>& result) {}
		};

		template<typename T, typename ...AT> struct ComponentInfoExtractor<T, AT...>
		{
			static void list_type_rwinfo(std::map<std::type_index, bool>& result)
			{
				if constexpr (!CPD_ThreadSafe<T>::value)
				{
					if constexpr (!std::is_const_v<T>)
						result[typeid(T)] = true;
					else
						result.insert({ typeid(T), false });
				}
				ComponentInfoExtractor<AT...>::list_type_rwinfo(result);
			}

			static void list_type_info(std::set<std::type_index>& result)
			{
				result.insert(typeid(T));
				ComponentInfoExtractor<AT...>::list_type_info(result);
			}
		};

	}


	// filter *****************************************************************

	struct EntityWrapper;

	struct Entity
	{
		operator bool() const noexcept { return m_ptr; }
	private:
		friend EntityWrapper;
		Entity(Implement::EntityImpPtr ptr) : m_ptr(std::move(ptr)) {}
		Implement::EntityImpPtr m_ptr;
	};

	struct EntityWrapper
	{
		operator Entity() { return Entity{ *m_entity}; };
	private:
		Implement::EntityImpPtr* m_entity;
	};

	template<typename ...CompT> struct EntityFilter;

	namespace Implement
	{

		template<typename Type> std::tuple<std::type_index, size_t, size_t> make_info_tuple()
		{
			return { typeid(Type), alignof(Type), sizeof(Type) };
		}

		template<typename ...Comp> struct ComponentRequireInfo
		{
			static const std::set<std::tuple<std::type_index, size_t, size_t>>& info() noexcept { return m_info; }
		private:
			static std::set<std::tuple<std::type_index, size_t, size_t>> m_info;
		};

		template<typename ...Comp> std::set<std::tuple<std::type_index, size_t, size_t>> ComponentRequireInfo<Comp...>::m_info = {
			make_info_tuple<Comp>()...
		};

		template<size_t total_index, size_t index, typename ...CompT> struct ComponentPointerTranstlate
		{
			bool operator()(std::tuple<std::add_pointer_t<CompT>...>& output, std::array<std::tuple<ComponentMemoryPageDesc*, size_t>, total_index>& input)
			{
				static_assert(total_index >= sizeof...(CompT) && index <= total_index);
				return true;
			}
		};

		template<size_t total_index, size_t index, typename CurComT, typename ...OtherComp> 
		struct ComponentPointerTranstlate<total_index, index, CurComT, OtherComp...>
		{
			bool operator()(
				std::tuple< std::add_pointer_t<CurComT>, std::add_pointer_t<OtherComp>...>& output, 
				std::array<std::tuple<ComponentMemoryPageDesc*, size_t>, total_index>& input
				)
			{
				static_assert(total_index >= sizeof...(OtherComp) + 1 && index <= total_index);
				ComponentMemoryPageDesc* desc;
				size_t comp_index;
				std::tie(desc, comp_index) = input[index];
				if (desc != nullptr)
				{
					std::get<index>(output) = static_cast<std::add_pointer_t<OtherComp>>(desc->m_components + sizeof(CurComT) * comp_index);
					return ComponentPointerTranstlate<total_index, index + 1, OtherComp...>{}(output, input);
				}
				return false;
			}
		};

		template<typename Func, typename Tuple, size_t ...index> void apply(Func&& f, Tuple& t, std::index_sequence<index...>)
		{
			std::forward<Func>(f)(*std::get<index>(t)...);
		}

		template<typename ...CompT> struct EntityFilterImp
		{
			template<typename Func> bool operator<<(Func&& func);
		private:
			EntityFilterImp(EntityImpPtr* input = nullptr) : m_entity(input) {  }
			EntityImpPtr* m_entity;
			friend struct EntityFilter<CompT...>;
		};

		template<typename ...CompT> template<typename Func> bool EntityFilterImp<CompT...>::operator<<(Func&& func)
		{
			if (m_entity != nullptr && (*m_entity))
			{
				auto result = (*m_entity)->read_tuple<CompT...>();
				std::tuple<std::add_pointer_t<CompT>...> m_storage;
				if (ComponentPointerTranstlate<sizeof...(CompT), 0, CompT...>{}(m_storage, result))
				{
					apply(std::forward<Func>(func), m_storage, std::index_sequence_for<CompT...>{});
					return true;
				}
			}
			return false;
		}
	}

	template<typename ...CompT> struct EntityFilter
	{
		Implement::EntityFilterImp<CompT...> operator()(EntityWrapper& wrapper) { return Implement::EntityFilterImp<CompT...>{ m_available ? wrapper.m_entity : nullptr }; }
	private:
		EntityFilter() = default;
		void lock(Implement::ComponentPool& pool);
		void unlock(Implement::ComponentPool& pool);
		bool m_available = false;
	};

	template<typename ...CompT> void EntityFilter<CompT...>::lock(Implement::ComponentPool& pool)
	{
		m_available = pool.read_lock(Implement::ComponentInfoExtractor<CompT...>::info());
	}

	template<typename ...CompT> void EntityFilter<CompT...>::unlock(Implement::ComponentPool& pool)
	{
		if(m_available)
			pool.unlock(Implement::ComponentInfoExtractor<CompT...>::info());
		m_available = false;
	}

	template<typename ...AT> struct Filter;
	template<typename CompT, typename ...AT> struct Filter<CompT, AT...>
	{
		template<typename Func> size_t operator<<(Func&& func);
	private:
		Filter();
		void lock(Implement::ComponentPool& pool);
		void unlock(Implement::ComponentPool& pool);
		std::map<std::type_index, size_t> m_version;
		std::vector<std::tuple<EntityWrapper, std::add_pointer_t<AT>...>> m_storaged_component;
		bool m_available = false;
	};

	template<typename CompT, typename ...AT> Filter<CompT, AT...>::Filter() : m_version({ {typeid(CompT), 0}, {typeid(CompT), 0}... }) {}

	template<typename CompT, typename ...AT> void Filter<CompT, AT...>::lock(Implement::ComponentPool& pool)
	{
		auto re = pool.read_lock_and_update_version(Implement::ComponentInfoExtractor<CompT...>::info(), m_version);
		m_available = std::get<0>(re);
		if (m_available)
		{
			if (std::get<1>(re))
			{
				m_storaged_component.clear();
				auto ite = std::get<1>(re).get_iterator();
				while (ite)
				{
					auto imp = ite.get_entity();
					auto read = (*imp)->read_tuple<CompT, AT...>();
					std::tuple<std::add_pointer_t<CompT>, std::add_pointer_t<AT>...> storage;
					if (Implement::ComponentPointerTranstlate<sizeof...(AT) + 1, 0, CompT, AT...>{}(storage, read))
						m_storaged_component.push_back(std::tuple_cat(std::make_tuple(EntityWrapper{ite}), storage));
				}
			}
		}
	}

	template<typename CompT, typename ...AT> void Filter<CompT, AT...>::unlock(Implement::ComponentPool& pool)
	{
		if (m_available)
		{
			pool.read_unlock(Implement::ComponentInfoExtractor<CompT...>::info());
			m_available = false;
		}
	}

	template<typename Func, typename Tuple, size_t ...index> decltype(auto) apply_with_entity(Func&& f, Tuple& t, std::index_sequence<index...>)
	{
		return std::forward<Func>(f)(std::get<0>(t), *std::get<index>(t)...);
	}

	template<typename CompT, typename ...AT> template<typename Func> 
	size_t Filter<CompT, AT...>::operator<<(Func&& func)
	{
		size_t count = 0;
		if (m_available)
		{
			for (auto& ite : m_storaged_component)
			{
				count += 1;
				if constexpr (std::is_same_v<decltype(apply_with_entity(std::forward<Func>(func), ite, std::index_sequence_for<CompT, AT...>{})), bool > )
				{
					bool cont = apply_with_entity(std::forward<Func>(func), ite, std::index_sequence_for<CompT, AT...>{});
					if (!cont)
						break;
				}
				else {
					apply_with_entity(std::forward<Func>(func), ite, std::index_sequence_for<CompT, AT...>{});
				}
			}
		}
	}

	template<typename CompT> struct Filter<CompT>
	{
		template<typename Func> size_t operator<<(Func&& func);
	private:
		Filter() = default;
		void lock(Implement::ComponentPool& pool);
		void unlock(Implement::ComponentPool& pool);
		Implement::SimilerComponentPool::ReadWrapper::Iterator m_ite;
	};

	template<typename CompT> void Filter<CompT>::lock(Implement::ComponentPool& pool)
	{
		auto re = pool.read_lock_single(typeid(CompT));
		if (re)
			m_ite = re.get_iterator();
	}

	template<typename CompT> void Filter<CompT>::unlock(Implement::ComponentPool& pool)
	{
		if (m_ite)
		{
			pool.read_unlock(typeid(CompT));
			m_ite = {};
		}
	}

	template<typename CompT> template<typename Func> 
	size_t Filter<CompT>::operator<<(Func&& func)
	{
		uint64_t count = 0;
		while(m_ite)
		{
			++count;
			if constexpr (std::is_same_v<decltype(std::forward<Func>(func)(EntityWrapper{ m_ite.get_entity() }, m_ite.get_component<CompT>())), bool > )
			{
				bool conti = std::forward<Func>(func)(EntityWrapper{ m_ite.get_entity() }, m_ite.get_component<CompT>());
				if (!conti)
					break;
			}
			else {
				std::forward<Func>(func)(EntityWrapper{ m_ite.get_entity() }, m_ite.get_component<CompT>());
			}
		}
		return count;
	}
	

	// Gobal Component ***********************
	namespace Implement
	{
		template<typename Type> struct GobalComponentImp;

		struct GobalComponentInterface : Tool::intrusive_object<GobalComponentInterface>
		{
			GobalComponentInterface(std::type_index id, bool thread_safe) noexcept;
			virtual ~GobalComponentInterface() = default;
			std::type_index id() const { return m_id; }
			bool is_thread_safe() const { return m_thread_safe; }
			template<typename Type> Type& cast();
		private:
			const std::type_index m_id;
			const bool m_thread_safe;
		};

		template<typename Type> struct GobalComponentImp : GobalComponentInterface
		{
			template<typename ...Parameter> GobalComponentImp(Parameter&& ...);
			operator Type& () { return m_storage; }
		private:
			Type m_storage;
		};

		template<typename Type> template<typename ...Parameter> GobalComponentImp<Type>::GobalComponentImp(Parameter&& ... para)
			: GobalComponentInterface(typeid(Type), CPD_ThreadSafe<Type>::value), m_storage(std::forward<Parameter>(para)...)
		{}

		template<typename Type> Type& GobalComponentInterface::cast()
		{
			assert(id() == typeid(Type));
			return static_cast<GobalComponentImp<Type>&>(*this);
		}

		using GobalComponentInterfacePtr = Tool::intrusive_ptr<GobalComponentInterface>;

		struct GobalComponentPool
		{
			struct ReadWrapper;
			ReadWrapper read_lock();
			void read_unlock();
			template<typename CompT, typename ...Parameter> CompT& construction(Parameter&& ...p);
			void deconstruction(std::type_index id);
			void update();
		private:
			std::shared_mutex m_component_mutex;
			std::map<std::type_index, GobalComponentInterfacePtr> m_all_component;
			std::mutex m_update_mutex;
			std::list<std::variant<GobalComponentInterfacePtr, std::type_index>> m_update_components;
		};

		template<typename CompT, typename ...Parameter> CompT& GobalComponentPool::construction(Parameter && ...p)
		{
			GobalComponentInterfacePtr ptr = new GobalComponentImp<CompT>();
		}

		struct GobalComponentPool::ReadWrapper
		{
			GobalComponentInterfacePtr read(std::type_index id);
			ReadWrapper() : m_ref(nullptr) {}
		private:
			friend struct GobalComponentPool;
			ReadWrapper(GobalComponentPool* ref) : m_ref(ref) {}
			GobalComponentPool* m_ref;
		};
	}


	// Event ****************************************************
	namespace Implement
	{
		struct EventListInterface : Tool::intrusive_object<EventListInterface>
		{
			Tool::type_layout layout() const noexcept { return m_layout; }
			virtual ~EventListInterface();
			virtual void update() = 0;
			EventListInterface(Tool::type_layout layout);
			bool is_avalible() const noexcept;
			void feed() noexcept;
		private:
			Tool::type_layout m_layout;
			std::atomic_bool m_heart_beat;
		};

		using EventListInterfacePtr = Tool::intrusive_ptr<EventListInterface>;

		template<typename EventT> struct EventListImp : EventListInterface
		{
			virtual void update() override;
			size_t push(EventT type);
		private:
			std::shared_mutex m_read_mutex;
			std::vector<EventT> m_read_list;
			std::mutex m_write_mutex;
			std::vector<EventT> m_write_list;
		};

		template<typename EventT> void EventListImp<EventT>::update()
		{
			std::unique_lock ul(m_read_mutex);
			std::lock_guard lg(m_write_mutex);
			std::swap(m_read_list, m_write_list);
			m_write_list.clear();
			m_heart_beat = false;
		}

		template<typename EventT> size_t EventListImp<EventT>::push(EventT type)
		{
			std::lock_guard lg(m_write_mutex);
			m_write_list.push_back(std::move(type));
			return m_write_list.size();
		}

		struct EventListPool
		{
		private:
			std::shared_mutex m_events_mutex;
			std::map<std::type_index, EventListInterfacePtr> m_events;
		};
	}

	template<typename Type> struct EventProvider
	{

	private:
		void lock(Implement::EventListPool& pool);
		void unlock(Implement::EventListPool& pool);
	};








	// system *******************************************************************

	struct context;

	namespace Implement
	{

		struct SystemInterface : Tool::intrusive_object<SystemInterface>
		{
			virtual ~SystemInterface();
		};

		template<typename ComT> struct SystemElementWrapper
		{
			Tool::intrusive_ptr<Implement::GobalComponentImp<ComT>> m_gobale;
		};


		template<typename ...ComT> struct SystemElementStorage
		{

		};

		template<typename ComT, typename ...OtherComT> struct SystemElementStorage<ComT, OtherComT...>
		{
			Tool::intrusive_ptr<GobalComponentInterface> m_compont;
		};

		//template<typename ...ComT, typename ...OtherComT> struct 



		

		template<typename SystemT> struct SystemImp : SystemInterface
		{
			SystemT m_storage;
			
		};
	}

}
*/