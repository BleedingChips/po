#include "memory_pool.h"
namespace PO::Tool
{
	namespace Implement
	{

		void resource_page_manager::allcoate_resource_page(size_t min_count)
		{
			size_t min_total_size = sizeof(resource_page) + (sizeof(element_scription) + type_size + type_alignas) * min_count;
			size_t total_size = (min_total_size / 1024 + (min_total_size % 1024 == 0 ? 0 : 1)) * 1024;
			size_t element_count = (total_size - sizeof(resource_page)) / (sizeof(element_scription) + type_size + type_alignas);
			std::byte* data = new std::byte[total_size];

			resource_page* rp = new (data) resource_page{ total_size,  element_count, this, page_top };
			page_top = rp;
			element_scription* element_start = reinterpret_cast<element_scription*>(rp + 1);
			std::byte* resource_start = reinterpret_cast<std::byte*>(element_start + element_count);


			element_scription* last_element = avalible_top;
			for (size_t count = 0; count < element_count; ++count)
			{
				void* resource_start_v = resource_start + sizeof(nullptr_t);
				size_t space = sizeof(nullptr_t) * 2;
				std::align(type_alignas, type_size, resource_start_v, space);
				void* resource = reinterpret_cast<std::byte*>(resource_start_v);
				element_scription* temporary = new (element_start) element_scription{ resource, rp, last_element, false };
				*(reinterpret_cast<element_scription**>(resource) - 1) = temporary;
				last_element = element_start;
				++element_start;
				resource_start += (type_size + type_alignas);
			}
			avalible_top = last_element;
		}


		resource_page_manager::resource_page_manager(size_t type, size_t aligned) :
			type_size(type), type_alignas(aligned), avalible_top(nullptr), page_top(nullptr)
		{
			//allcoate_resource_page();
		}

		void* resource_page_manager::allocate()
		{
			std::lock_guard<std::mutex> lg(mutex);
			if (avalible_top == nullptr)
				allcoate_resource_page();
			assert(avalible_top != nullptr);
			element_scription* result = avalible_top;
			avalible_top = avalible_top->waitting_list_next;
			result->waitting_list_next = nullptr;
			result->being_used = true;
			return result->resource;
		}

		void resource_page_manager::release(element_scription* ptr) noexcept
		{
			assert(ptr != nullptr);
			std::lock_guard<std::mutex> lg(mutex);
			ptr->waitting_list_next = avalible_top;
			ptr->being_used = false;
			avalible_top = ptr;
		}

		resource_page_manager::resource_page_manager(resource_page_manager&& spm) : type_size(spm.type_size), type_alignas(spm.type_alignas)
		{
			std::lock_guard<std::mutex>(spm.mutex);
			avalible_top = spm.avalible_top;
			page_top = spm.page_top;
			spm.avalible_top = nullptr;
			spm.page_top = nullptr;
		}

		resource_page_manager::~resource_page_manager()
		{
			std::lock_guard<std::mutex> lg(mutex);
			while (page_top != nullptr)
			{
				resource_page* current = page_top;
				page_top = current->last_page;
				assert([&]()->bool {
					element_scription* rpe = (element_scription *)(current + 1);
					for (size_t i = 0; i < current->element_count; ++i)
					{
						if (rpe[i].being_used)
							return false;
					}
					return true;
				}()
					);
				delete current;
			}
		}
	}

	void* memory_pool::allocate(memory_pool_key key)
	{
		using lock_type = typename decltype(all_resource)::type;
		Implement::resource_page_manager& ref = all_resource.lock([&](lock_type& lt) -> Implement::resource_page_manager& {
			auto ite = lt.find(key);
			if (ite != lt.end())
				return ite->second;
			else
			{
				auto ite = lt.try_emplace(key, key.type_size, key.alignas_size);
				assert(ite.second);
				return (ite.first)->second;
			}
		});
		return ref.allocate();
	}

	void memory_pool::release(void* resource_ptr) noexcept
	{
		assert(resource_ptr != nullptr);
		using lock_type = typename decltype(all_resource)::type;
		Implement::element_scription* ptr = *(reinterpret_cast<Implement::element_scription**>(resource_ptr) - 1);
		assert(ptr->belong_page != nullptr && ptr->resource != nullptr && ptr->being_used && ptr->waitting_list_next == nullptr && ptr->resource == resource_ptr);
		auto manager = ptr->belong_page->belong_manager;
		memory_pool_key key{ manager->type_size , manager->type_alignas };
		all_resource.lock([&](lock_type& lt) {
			auto ite = lt.find(key);
			if (ite != lt.end())
				ite->second.release(ptr);
			else
				assert(false);
		});
	}
}