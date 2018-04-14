#pragma once
namespace PO::Graphic
{
	namespace Implement
	{
		template<typename T, typename ...OT> struct alignas(16) shader_storage_holder
		{
			alignas(16) T data;
			alignas(16) shader_storage_holder<OT...> next;
			template<typename TT, typename ...CT> shader_storage_holder(TT&& tt, CT&& ...ct) :data(std::forward<TT>(tt)), next(std::forward<CT>(ct)...) {}
			shader_storage_holder() {}
			T& get_implement(std::integral_constant<size_t, 0>) noexcept { return data; }
			template<size_t index> decltype(auto) get_implement(std::integral_constant<size_t, index>) { return next.get_implement(std::integral_constant<size_t, index - 1>{}); }
			const T& get_implement(std::integral_constant<size_t, 0>) const noexcept { return data; }
			template<size_t index> decltype(auto) get_implement(std::integral_constant<size_t, index>) const noexcept { return next.get_implement(std::integral_constant<size_t, index - 1>{}); }
		};

		template<typename T> struct alignas(16) shader_storage_holder<T>
		{
			alignas(16) T data;
			template<typename TT> shader_storage_holder(TT&& tt) :data(std::forward<TT>(tt)) {}
			shader_storage_holder() {}
			T& get_implement(std::integral_constant<size_t, 0>) noexcept { return data; }
			const T& get_implement(std::integral_constant<size_t, 0>) const noexcept { return data; }
		};

		template<size_t i>
		struct alignas(16) fill_buffer
		{
			alignas(16) std::array<char, i> buffer;
		};

		template<typename ... AT>
		struct alignas(16) shader_storage
		{
			shader_storage_holder <fill_buffer<128>> empty_buffer;
		};

		template<typename T, typename ... AT>
		class alignas(16) shader_storage<T, AT...>
		{
			std::conditional_t<
				sizeof(shader_storage_holder<T, AT...>) >= 128,
				shader_storage_holder<T, AT...>,
				shader_storage_holder<T, AT..., fill_buffer<128 - sizeof(Implement::shader_storage_holder<T, AT...>)>>
			> buffer;
		public:
			template<size_t i> decltype(auto) get() noexcept { return buffer.get_implement(std::integral_constant<size_t, i>{}); }
			template<size_t i> decltype(auto) get() const noexcept { return buffer.get_implement(std::integral_constant<size_t, i>{}); }
			template<typename ...AT>
			shader_storage(AT&& ...at) : buffer(std::forward<AT>(at)...) {}
		};

	}



}