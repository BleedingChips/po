#pragma once
#include "entity.h"
#include "memory.h"
#include <set>
#include <deque>
namespace PO::ECS::Implement
{
	struct TypeLayoutArray
	{
		const TypeLayout* layouts = nullptr;
		size_t count = 0;
		bool operator<(const TypeLayoutArray&) const noexcept;
		bool operator==(const TypeLayoutArray&) const noexcept;
		TypeLayoutArray& operator=(const TypeLayoutArray&) = default;
		TypeLayoutArray(const TypeLayoutArray&) = default;
		const TypeLayout& operator[](size_t index) const noexcept
		{
			assert(index < count);
			return layouts[index];
		}
	};

	struct TypeGroud;

	struct StorageBlock
	{
		struct Control
		{
			std::tuple<void (*)(void*) noexcept, void (*)(void*, void*)>* tool;
			void* data;
		};
		static StorageBlock* create(const TypeGroud* owner);
		static void free(StorageBlock* owner) noexcept;
	private:
		const TypeGroud* m_owner;
		StorageBlock* m_next = 0;
		size_t available_count = 0;
		Control* control_start = nullptr;
		EntityInterface** entity_start = nullptr;
	};

	struct TypeGroud
	{
		TypeLayoutArray layouts() const noexcept { return m_type_layouts; }
		static TypeGroud* create(MemoryPageAllocator& allocator, TypeLayoutArray array);
		static void free(TypeGroud*);

		MemoryPageAllocator::SpaceResult space() const noexcept { return m_space; }
		size_t max_count() const noexcept { return m_max_count; }
		MemoryPageAllocator& allocator() const noexcept { return m_allocator; }

	private:

		TypeGroud(MemoryPageAllocator&, TypeLayoutArray);
		~TypeGroud();
		
		TypeLayoutArray m_type_layouts;
		StorageBlock* m_start_block = nullptr;
		StorageBlock* m_next_block = nullptr;

		MemoryPageAllocator& m_allocator;
		MemoryPageAllocator::SpaceResult m_space;
		size_t m_max_count;
	};

	struct InitHistory
	{
		bool is_construction;
		TypeLayout type;
		void* data;
	};

	struct ComponentPool : ComponentPoolInterface
	{

		virtual void construct_component(
			const TypeLayout& layout, void(*constructor)(void*, void*), void* data, 
			EntityInterface*, void(*deconstructor)(void*) noexcept, void(*mover)(void*, void*)
		) override;
		void update();
		void clean();

	private:

		struct InitBlock
		{
			void* start_block;
			void* last_block;
			size_t last_available_count;
		};

		struct InitHistory
		{
			bool is_construction;
			TypeLayout type;
			void* data;
		};

		MemoryPageAllocator& m_allocator;
		std::map<TypeLayoutArray, TypeGroud*> m_data;
		std::map<EntityInterfacePtr, std::vector<InitHistory>> m_init_history;
		std::vector<InitBlock> m_init_block;
	};










	/*
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
	*/
}