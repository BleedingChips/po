#pragma once
#include <tuple>
#include <mutex>
#include <vector>
namespace PO::ECS::Implement
{
	
	struct MemoryPageAllocator
	{

		MemoryPageAllocator(size_t storage_page_count = 4) noexcept;
		~MemoryPageAllocator();

		struct SpaceResult
		{
			size_t space;
			size_t allocate_flag;
		};

		static SpaceResult recalculate_space(size_t require_space);
		void* allocate(SpaceResult flag);
		void release(void* buffer, SpaceResult flag) noexcept;
		
	private:
		struct RawPageHead
		{
			RawPageHead* m_next_page = nullptr;
		};
		std::mutex m_page_mutex;
		std::vector<std::tuple<RawPageHead*, size_t>> m_pages;
		uint64_t m_require_storage;
	};
}