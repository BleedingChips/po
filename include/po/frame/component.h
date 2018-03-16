#pragma once
#include "../tool/tool.h"
#include "object_pool.h"
namespace PO::ECSFramework
{
	namespace Implement
	{

		struct ptr_base
		{
			virtual void* data() noexcept = 0;
			virtual const void* data() const noexcept = 0;
			virtual ~ptr_base() = default;
			virtual bool is_avalible() const noexcept = 0;
			virtual bool is_same_id(std::type_index ti) const noexcept = 0;
			virtual std::type_index id() const noexcept = 0;
			template<typename T> T& cast() noexcept { return *static_cast<T*>(data()); }
			template<typename T> const T& cast() const noexcept { return *static_cast<const T*>(data()); }

		};

		template<typename T> struct default_deleter { void operator ()(T* da) const noexcept { if (da != nullptr) { da->~T(); } } };
		template<typename T> using ecs_intrusive_ptr = PO::Tool::intrusive_ptr<T, default_deleter<T>>;
		template<typename T> using ecs_unique_ptr = std::unique_ptr<T, default_deleter<T>>;

		class base_res
		{
			bool living = true;
		public:
			void destory() noexcept { living = false; }
			bool is_avalible() const noexcept { return living; }
		};
	}



	using component_res = Implement::base_res;
	class entity;

	//using entity_ptr = Implement::ecs_intrusive_ptr<entity>;
	//using entity_ptr_const = Implement::ecs_intrusive_ptr<const entity>;

	namespace Implement
	{




		struct component_inside_ptr : public ptr_base {};

		template<typename implement_t>
		class component_implement :
			public component_inside_ptr, public implement_t
		{
			static_assert(std::is_base_of_v<component_res, implement_t>);
		public:
			virtual void* data() noexcept { return static_cast<implement_t*>(this); }
			virtual const void* data() const noexcept { return static_cast<const implement_t*>(this); }
			virtual bool is_avalible() const noexcept { return implement_t::is_avalible(); }
			virtual bool is_same_id(std::type_index ti) const noexcept { return ti == typeid(implement_t); }
			virtual std::type_index id() const noexcept { return typeid(implement_t); }
			template<typename ...construction_para>
			component_implement(construction_para&&... at) : implement_t(std::forward<construction_para>(at)...) { }
			~component_implement() = default;
		};

		class component_holder
			: public object_pool_base_deleter<component_holder>
		{
			mutable Tool::atomic_reference_count ref;
			
			friend class PO::Tool::intrusive_ptr<component_holder, default_deleter<component_holder>>;
			friend class PO::Tool::intrusive_ptr<const component_holder, default_deleter<const component_holder>>;
			void* data() noexcept { return ptr->data(); }
			const void* data() const noexcept { return ptr->data(); }
		public:
			ecs_unique_ptr<component_inside_ptr> ptr;
			void add_ref() const noexcept { ref.add_ref(); }
			bool sub_ref() const noexcept { return ref.sub_ref(); }
			bool is_avalible() const noexcept { return ptr && ptr->is_avalible(); };
			bool is_same_id(std::type_index ti) const noexcept { return ptr->is_same_id(ti); }
			std::type_index id() const noexcept { return ptr->id(); }
			template<typename T> T& cast() noexcept { assert(is_same_id(typeid(T))); return *static_cast<T*>(data()); }
			template<typename T> const T& cast() const noexcept { assert(is_same_id(typeid(T))); return *static_cast<const T*>(data()); }
			void clear() noexcept { ptr.reset(); }
		};

		using component_holder_ptr = ecs_intrusive_ptr<component_holder>;

	}

}