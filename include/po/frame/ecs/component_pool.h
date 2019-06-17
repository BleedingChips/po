#pragma once
#include "entity.h"
#include "memory.h"
#include <set>
#include <deque>
namespace PO::ECS::Implement
{
	struct SimilerComponentPool;
	struct ComponentMemoryPageDesc
	{
		struct Control
		{
			void (*deconstructor)(void*) = nullptr;
			EntityInterfacePtr owner;
		};
		static ComponentMemoryPageDesc* allocate(MemoryPageAllocator& allocator, const MemoryPageAllocator::SpaceResult& result, size_t component_count, const SimilerComponentPool* pool);
		static std::tuple<ComponentMemoryPageDesc*, ComponentMemoryPageDesc*> free_page(MemoryPageAllocator& allocator, const MemoryPageAllocator::SpaceResult& result, ComponentMemoryPageDesc*) noexcept;
		void release_component(size_t index);
		bool try_release_component(size_t index);
		size_t construct_component(void(*constructor)(void*, void*), void* parameter, EntityInterface* entity, void(*deconstructor)(void*) noexcept);
		std::tuple<ComponentMemoryPageDesc*, size_t> update(size_t index);
		size_t poll_count() const noexcept { return m_poll_count; }
		size_t available_count() const noexcept { return m_available_count; }
		ComponentMemoryPageDesc*& next_page() noexcept { return m_next_page; }
		ComponentMemoryPageDesc*& front_page() noexcept { return m_next_page; }
		const TypeLayout& layout() const noexcept { return m_layout; }
		EntityInterface** entitys() noexcept { return m_entitys; }
		std::byte* components() noexcept { return m_components; }
	private:
		ComponentMemoryPageDesc(const SimilerComponentPool*, size_t component_count) noexcept;
		const SimilerComponentPool* m_owner;
		const TypeLayout& m_layout;
		const size_t m_component_count;
		ComponentMemoryPageDesc* m_front_page = nullptr;
		ComponentMemoryPageDesc* m_next_page = nullptr;
		size_t m_available_count = 0;
		size_t m_poll_count = 0;
		Control* m_control = nullptr;
		EntityInterface** m_entitys = nullptr;
		std::byte* m_components = nullptr;
	};

	struct SimilerComponentPool
	{

		static std::tuple<MemoryPageAllocator::SpaceResult, size_t> calculate_space(size_t align, size_t size) noexcept;

		SimilerComponentPool(MemoryPageAllocator& allocator, const TypeLayout& layout);
		SimilerComponentPool(const SimilerComponentPool&) = delete;

		const TypeLayout& layout() const noexcept { return m_layout; }
		bool is_same_layout(const TypeLayout& input) const noexcept { return input == m_layout; }

		void deconstruction(ComponentMemoryPageDesc*, size_t index);
		//Contructor construct_component(EntityImpPtr, void(*deconstructor)(void*) noexcept);
		//void finish_construt(Contructor, bool state);
		void construction_component(EntityInterface*, void(*constructor)(void*, void*), void* parasmeter, void (*deconstructor)(void*) noexcept);


		~SimilerComponentPool();
		void update();

		std::tuple<size_t, ComponentMemoryPageDesc*, uint64_t> read_lock(std::shared_lock<std::shared_mutex>* lock) noexcept;
		static void next_desc(ComponentPoolReadWrapper& corw);

	private:

		MemoryPageAllocator& m_allocator;
		TypeLayout m_layout;
		MemoryPageAllocator::SpaceResult m_page_space;
		size_t m_component_count;
		ComponentMemoryPageDesc* m_top_page = nullptr;
		std::mutex m_record_mutex;
		std::vector<std::tuple<ComponentMemoryPageDesc*, size_t>> m_constructed_comps;
		std::vector<std::tuple<ComponentMemoryPageDesc*, size_t>> m_need_deleted_comps;

		std::shared_mutex m_poll_mutex;
		uint64_t m_version = 0;
		size_t m_poll_count = 0;
	};

	struct ComponentPool : ComponentPoolInterface
	{
		virtual bool lock(ComponentPoolReadWrapper&, size_t count, const TypeLayout* layout, uint64_t* version, size_t mutex_size, void* mutex) override;
		virtual void next(ComponentPoolReadWrapper&) override;
		virtual void unlock(size_t count, size_t mutex_size, void* mutex) noexcept override;
		virtual void construct_component(const TypeLayout& layout, void(*constructor)(void*, void*), void* data, EntityInterface*, void(*deconstructor)(void*) noexcept) override;
		virtual bool deconstruct_component(EntityInterface*, const TypeLayout&) noexcept override;
		ComponentPool(MemoryPageAllocator&) noexcept;
		~ComponentPool();
		void update();
		void clean_all();
	private:
		SimilerComponentPool* find_pool(const TypeLayout& layout);
		MemoryPageAllocator& m_allocator;
		std::shared_mutex m_components_mutex;
		std::map<TypeLayout, SimilerComponentPool> m_components;
	};
}