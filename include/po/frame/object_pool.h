#pragma once
#include "../tool/tool.h"
#include <map>
#include <typeindex>
namespace PO::ECSFramework
{
	namespace Implement
	{
		struct resource_page;

		struct element_scription
		{
			void* const resource;
			resource_page* const belong_page;
			element_scription* waitting_list_next;
			bool being_used;
		};

		class resource_page_manager;

		struct resource_page
		{
			const size_t page_size;
			const size_t element_count;
			resource_page_manager* belong_manager;
			resource_page* last_page;
		};

		class resource_page_manager
		{
			std::mutex mutex;
			element_scription* avalible_top;
			resource_page* page_top;
			const size_t type_size;
			const size_t type_alignas;
			const size_t real_size;
			const size_t real_alignas;
			void allcoate_resource_page(size_t min_count = 10);
		public:
			resource_page_manager(size_t size_s, size_t alignas_s);
			resource_page_manager(resource_page_manager&& spm);
			void* allocate();
			void release(element_scription*) noexcept;
			~resource_page_manager();
			bool is_same_size(size_t size, size_t aligna) const noexcept {
				return (size == type_size) && (aligna == type_alignas);
			}
		};
	}

	class object_pool;

	class object_pool
	{
		PO::Tool::scope_lock<std::map<std::type_index, Implement::resource_page_manager>> all_resource;
	public:
		void* allocate(std::type_index, size_t type_size, size_t alignas_size);
		template<typename type, typename ...construction_para>
		type* allocate(construction_para&&... cp) {
			auto* tem = allocate(typeid(type), sizeof(type), alignof(type));
			try {
				return new (tem) type{ std::forward<construction_para>(cp)... };
			}
			catch (...)
			{
				release(tem);
				throw;
			}
		};
		static void release(void* resource_ptr) noexcept;
	};

	template<typename derive_type>
	class object_pool_base_deleter
	{
	public:
		~object_pool_base_deleter()
		{
			std::string oi = typeid(derive_type).name();
			object_pool::release(static_cast<derive_type*>(this));
		}
	};
}