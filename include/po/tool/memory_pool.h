#pragma once
#include <mutex>
#include "tool.h"
#include <map>
#include <iostream>
namespace PO::Tool
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
			const resource_page_manager* const belong_manager;
			resource_page* last_page;
		};

		class resource_page_manager
		{
			std::mutex mutex;
			element_scription* avalible_top;
			resource_page* page_top;
			void allcoate_resource_page(size_t min_count = 10);
		public:
			const size_t type_size;
			const size_t type_alignas;
			resource_page_manager(size_t size_s, size_t alignas_s);
			resource_page_manager(resource_page_manager&& spm);
			void* allocate();
			void release(element_scription*) noexcept;
			~resource_page_manager();
		};
	}

	struct memory_pool_key
	{
		size_t type_size;
		size_t alignas_size;
		bool operator==(const memory_pool_key& rmk) const noexcept { return rmk.type_size == type_size && rmk.alignas_size == alignas_size; }
		bool operator<(const memory_pool_key& rmk) const noexcept { return rmk.type_size < type_size || rmk.alignas_size < alignas_size; }
	};

	namespace Implement
	{
		template<typename final_type>
		struct memory_pool_packet_base
		{
			~memory_pool_packet_base()
			{
				memory_pool::instance.release(static_cast<final_type*>(this));
			}
		};

		template<typename packet>
		struct memory_pool_packet :memory_pool_packet_base<memory_pool_packet<packet>>, packet
		{
			static_assert(std::has_virtual_destructor_v<packet>, "memory_pool_packet need virtual destructor");
			//template<typename ...construction_para> memory_pool_packet(construction_para&& ...cp) : memory_pool_packet_base({ this }), packet(std::forward<construction_para>(cp)...) {}
			using packet::packet;
		};
	}

	class memory_pool
	{
		Tool::scope_lock<std::map<memory_pool_key, Implement::resource_page_manager>> all_resource;
	public:
		memory_pool() {}
		void* allocate(memory_pool_key key);

		template<typename packet_type, typename ...construction_type> packet_type* allocate_packet(construction_type&& ...ct)
		{
			using type = Implement::memory_pool_packet<packet_type>;
			void* ptr = allocate({sizeof(type), alignof(type)});
			try {
				type* final = new(ptr) type{ std::forward<construction_type>(ct)... };
				return final;
			}
			catch (...)
			{
				release(ptr);
				throw;
			}
		}

		/*
		template<typename type, typename ...construction_para> Tool::intrusive_ptr<type> allocate_intrusice_ptr(construction_para&& ...cp)
		{
			return Tool::intrusive_ptr<type>{ new(allocate({ sizeof(type), alignof(type) })) type{ std::forward<construction_para>(cp)... }, memory_pool_deleter::deleter };
		}
		*/
		
		template<typename T> void* allocate() { return allocate({sizeof(T), alignof(T)}); }
		void release(void* resource_ptr) noexcept;
	};

	
}