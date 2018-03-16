#pragma once
#include "system.h"
#include "../platform/asynchronous_affairs.h"
#include "../platform/platform.h"
#include <deque>
#include <set>
#include <map>
namespace PO::ECSFramework
{

	



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
		entity create_entity() { return ref.create_entity(); };
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

}