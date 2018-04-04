#pragma once
#include "component.h"
namespace PO::ECSFramework
{
	namespace Implement
	{

		class entity_implement
		{
			std::unordered_map<std::type_index, Implement::component_holder_ptr> component_set;
		public:
			void insert(Implement::component_holder_ptr) noexcept;
			bool check_exist(std::type_index) const noexcept;
			Implement::component_holder_ptr get_component(std::type_index ti) noexcept;
			Implement::component_holder_ptr get_component(std::type_index ti) const noexcept;
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
			void insert(Implement::component_holder_ptr chp) noexcept { return shift()->insert(std::move(chp)); }
			Implement::component_holder_ptr get_component(std::type_index ti) noexcept { return shift()->get_component(ti); }
			Implement::component_holder_ptr get_component(std::type_index ti) const noexcept { return shift()->get_component(ti); }
			void destory() noexcept;
			operator bool() const noexcept { return avalible; }
		};

		using entity_ptr = Implement::ecs_intrusive_ptr<entity_ref>;
		template<typename implement> struct system_requirement_storage;
		template<typename implement> struct filter;
	}

	class entity
	{
		Implement::entity_ptr ptr;
		friend class context_implement;
		template<typename implement> friend struct Implement::system_requirement_storage;
		friend class context;
		template<typename implement> friend struct Implement::filter;
		entity(Implement::entity_ptr eip) : ptr(std::move(eip)) {}
	public:
		entity(const entity&) = default;
		entity() = default;
		entity(entity&&) = default;
		entity& operator=(entity e) { ptr = std::move(e.ptr); return *this; }
		operator bool() const noexcept { return ptr && *ptr; }
		//void destory() { ptr->destory(); ptr.reset(); }
		bool check_exist(std::type_index ti) const noexcept { return ptr->check_exist(ti); }
	};
}