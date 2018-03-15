#pragma once
#include "../tool/tool.h"
#include "object_pool.h"
#include <iostream>
#include <array>
namespace PO::ECSFramework
{
	class context;

	class other_entity {};
	class singleton {};

	namespace Implement
	{

		class vision
		{
			size_t vision_number = 0;
		public:
			void update() { vision_number++; }
			bool different(const vision& t) {
				if (vision_number != t.vision_number)
					return (vision_number = t.vision_number, true);
				return false;
			}
		};


		template<typename this_type, typename ...type> bool is_all_true(this_type tt, type... t)
		{
			return tt;
		}

		template<typename this_type, typename other_type, typename ...type> bool is_all_true(this_type tt, other_type ot, type... t)
		{
			return is_all_true(tt && ot, t...);
		}

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

		struct type_index_view
		{
			const std::type_index* view = nullptr;
			size_t view_count = 0;
			operator bool() const { return view_count != 0; }
			bool operator ==(type_index_view tiv) const noexcept;
			bool operator < (type_index_view tiv) const noexcept;
			bool have(std::type_index ti) const noexcept;
			bool is_collided(const type_index_view& tiv) const noexcept;
			const std::type_index& operator[](size_t index) const noexcept { return view[index]; }
			size_t size() const noexcept { return view_count; }
		};

		template<typename ...type>
		struct type_array
		{
			std::array<std::type_index, sizeof...(type)> array_buffer = { typeid(type)... };
			type_array() { std::sort(array_buffer.begin(), array_buffer.end()); }
			type_index_view view() const noexcept { return type_index_view{ array_buffer.data(), array_buffer.size()}; }
		};

		template<>
		struct type_array<>
		{
			type_array() { }
			type_index_view view() const noexcept { return type_index_view{ nullptr, 0 }; }
		};

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

		template<typename ...write_type> struct system_type_holder_initor
		{
			std::array<std::type_index, sizeof...(write_type)> holder = { typeid(write_type)... };
			system_type_holder_initor() { std::sort(holder.begin(), holder.begin() + sizeof...(write_type)); }
			operator type_index_view () const noexcept { return type_index_view{ holder.data(), sizeof...(write_type) }; }
		};

		template<> struct system_type_holder_initor<>
		{
			system_type_holder_initor() {}
			operator type_index_view () const noexcept { return type_index_view{ nullptr, 0 }; }
		};

		template<typename ...write_type> struct system_type_holder
		{
			static system_type_holder_initor<write_type...> holder;
			operator type_index_view () const noexcept { return holder; }
		};
		template<typename ...write_type> system_type_holder_initor<write_type...> system_type_holder<write_type...>::holder;

	}

}