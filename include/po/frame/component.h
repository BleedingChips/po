#pragma once
#include "../tool/tool.h"
#include "object_pool.h"
namespace PO::ECSFramework
{
	namespace Implement
	{

		class ptr_base
		{
			void* raw_data;
			std::type_index defined_id;
		public:
			ptr_base(void* data, std::type_index id) : raw_data(data), defined_id(id) {}
			virtual ~ptr_base() = default;
			virtual bool is_avalible() const noexcept = 0;
			bool is_same_id(std::type_index ti) const noexcept { return ti == id(); }
			std::type_index id() const noexcept { return defined_id; }
			template<typename T> T& cast() noexcept { return *static_cast<T*>(raw_data); }
			template<typename T> const T& cast() const noexcept { return *static_cast<const T*>(raw_data); }
		};

		template<typename T> struct default_deleter { void operator ()(T* da) const noexcept { if (da != nullptr) { da->~T(); } } };
		template<typename T> using ecs_intrusive_ptr = PO::Tool::intrusive_ptr<T, default_deleter<T>>;
		template<typename T> using ecs_unique_ptr = std::unique_ptr<T, default_deleter<T>>;

		struct base_res
		{
			bool avalible = true;
			void destory() noexcept { avalible = false; }
			bool is_avalible() const noexcept { return avalible; }
		};
	}



	using component_res = Implement::base_res;
	class entity;

	//using entity_ptr = Implement::ecs_intrusive_ptr<entity>;
	//using entity_ptr_const = Implement::ecs_intrusive_ptr<const entity>;

	namespace Implement
	{

		struct component_inside_ptr : public ptr_base {
			using ptr_base::ptr_base;
		};

		template<typename implement_t>
		class component_implement :
			public component_inside_ptr, public implement_t
		{
			static_assert(std::is_base_of_v<component_res, implement_t>);
		public:
			virtual bool is_avalible() const noexcept { return implement_t::is_avalible(); }
			template<typename ...construction_para>
			component_implement(construction_para&&... at) :component_inside_ptr(static_cast<implement_t*>(this), typeid(implement_t) ),  implement_t(std::forward<construction_para>(at)...) { }
			~component_implement() = default;
		};

		class component_holder
			: public object_pool_base_deleter<component_holder>
		{
			mutable Tool::atomic_reference_count ref;
			ecs_unique_ptr<component_inside_ptr> ptr;
		public:
			component_holder(ecs_unique_ptr<component_inside_ptr> p) : ptr(std::move(p)) {}
			void add_ref() const noexcept { ref.add_ref(); }
			bool sub_ref() const noexcept { return ref.sub_ref(); }
			bool is_avalible() const noexcept { return ptr && ptr->is_avalible(); };
			bool is_same_id(std::type_index ti) const noexcept { return ptr->is_same_id(ti); }
			std::type_index id() const noexcept { return ptr->id(); }
			template<typename T> T& cast() noexcept { return ptr->cast<T>(); }
			template<typename T> const T& cast() const noexcept { ptr->cast<T>(); }
			void clear() noexcept { ptr.reset(); }
		};

		using component_holder_ptr = ecs_intrusive_ptr<component_holder>;

	}

}