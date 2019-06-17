#include "memory.h"
#include "component_pool.h"
namespace PO::ECS::Implement
{
	constexpr size_t system_memory_control_block_obligate = 200;
	constexpr size_t memory_page_space = 2048;

	MemoryPageAllocator::MemoryPageAllocator(size_t storage_page_count) noexcept
		: m_require_storage(storage_page_count)
	{}


	MemoryPageAllocator::~MemoryPageAllocator()
	{
		std::lock_guard lg(m_page_mutex);
		for (auto ite : m_pages)
		{
			auto [head, index] = ite;
			while (head != nullptr)
			{
				auto cur = head;
				head = head->m_next_page;
				cur->~RawPageHead();
				delete[] reinterpret_cast<std::byte*>(cur);
			}
		}
		m_pages.clear();
	}

	MemoryPageAllocator::SpaceResult MemoryPageAllocator::recalculate_space(size_t require_space)
	{
		size_t mulity = 1;
		while (memory_page_space * mulity < require_space + system_memory_control_block_obligate)
			mulity *= 1;
		return { memory_page_space * mulity - system_memory_control_block_obligate, mulity };
	}

	void* MemoryPageAllocator::allocate(SpaceResult flag)
	{
		assert(flag.allocate_flag >= 1);
		std::lock_guard lg(m_page_mutex);
		if (m_pages.size() < flag.allocate_flag)
			m_pages.resize(flag.allocate_flag, { nullptr, 0 });
		auto& [head, count] = m_pages[flag.allocate_flag - 1];
		if (head == nullptr)
			return new std::byte[flag.allocate_flag * memory_page_space - system_memory_control_block_obligate];
		else {
			RawPageHead* old = head;
			head = head->m_next_page;
			old->~RawPageHead();
			--count;
			return old;
		}
	}

	void MemoryPageAllocator::release(void* input, SpaceResult flag) noexcept
	{
		std::lock_guard lg(m_page_mutex);
		assert(m_pages.size() >= flag.allocate_flag);
		auto& [head, count] = m_pages[flag.allocate_flag - 1];
		if (count >= m_require_storage)
		{
			delete[](reinterpret_cast<std::byte*>(input));
		}
		else {
			RawPageHead* tar = new (input) RawPageHead{};
			tar->m_next_page = head;
			head = tar;
			++count;
		}
	}

	
}