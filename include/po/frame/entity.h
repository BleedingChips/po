#pragma once
#include "component.h"
namespace PO::ECSFramework
{
	namespace Implement
	{
		class entity_implement :
			object_pool_base_deleter<entity_implement>
		{
		public:
			mutable PO::Tool::atomic_reference_count ref;
			std::unordered_map<std::type_index, Implement::component_holder_ptr> component_set;
			bool avalible;
			void add_ref() const noexcept { ref.add_ref(); }
			bool sub_ref() const noexcept { return ref.sub_ref(); }
			friend class PO::Tool::intrusive_ptr<entity, Implement::default_deleter<entity>>;
			friend class PO::Tool::intrusive_ptr<const entity, Implement::default_deleter<const entity>>;
			void insert(Implement::component_holder_ptr) noexcept;
			friend class context_implement;

			bool check_exist(std::type_index) const noexcept;
			Implement::component_holder_ptr get_component(std::type_index ti) noexcept;
			Implement::component_holder_ptr get_component(std::type_index ti) const noexcept;
			bool is_avalible() const noexcept { return avalible; }
			entity_implement() : avalible(true) {}
			void destory() noexcept { avalible = false; }
		};

		using entity_implement_ptr = Implement::ecs_intrusive_ptr<entity_implement>;
		template<typename implement> struct system_requirement_storage;
	}

	class entity
	{
		Implement::entity_implement_ptr ptr;
		friend class context_implement;
		template<typename implement> friend struct Implement::system_requirement_storage;
		friend class context;
	public:
		entity(Implement::entity_implement_ptr eip) : ptr(std::move(eip)) {}
		entity(const entity&) = default;
		entity() = default;
		entity(entity&&) = default;
		entity& operator=(entity e) { ptr = std::move(e.ptr); return *this; }
		operator bool() const noexcept { return ptr && ptr->is_avalible(); }
		void destory() { ptr->destory(); ptr.reset(); }
		bool check_exist(std::type_index ti) const noexcept { return ptr->check_exist(ti); }
	};
}