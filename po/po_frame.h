#pragma once
#include "tool\tool.h"
#include <atomic>
#include <future>
/*
struct form; 
[must
each form will be constructed in difference thread.
should be close form in destruction
should provide "bool operator()()" to show form need to be close.
]

struct form_view; 
[optional
usually call in diference thread.
should provide "form_view::form_view(form&)" and "form_view::form_view(const form_view&)".
construct in the same thread of form
form_view's life time is usually longer then form.
]

struct tick_vew;
[optional
call in the same thread with form.
should provide "tick_vew::tick_vew(form&)" and "tick_vew::tick_vew(const form_view&)".
construct in the same thread of form.
]


struct plugin_initializer; [optional]
struct initializer; [optional]
struct forbid_plugin; [optional]
*/

namespace PO
{
	namespace Assistant
	{
		template<typename T, typename = void> struct frame_have_form : std::false_type {};
		template<typename T> struct frame_have_form<T, std::void_t<typename T::form>> :std::true_type {};

		template<typename T, typename = void> struct frame_have_form_view : std::false_type {};
		template<typename T> struct frame_have_form_view
			<
				T, 
				std::void_t<
					typename T::form_view,
					std::enable_if_t< std::is_constructible<typename T::form_view, typename T::form&>::value && std::is_constructible<typename T::form_view, const typename T::form_view& >::value >
				>
			> :std::true_type {};

		template<typename T, typename = void> struct frame_have_form_avalible : std::false_type {};
		template<typename T> struct frame_have_form_avalible<T, std::void_t<decltype(T::form_avalible(std::declval<typename T::form&>()))>> :std::true_type {};

		//template<typename T, typename = void> struct frame_have_form_close : std::false_type {};
		//template<typename T> struct frame_have_form_close<T, std::void_t<decltype(T::form_close(std::declval<typename T::form&>()))>> :std::true_type {};

		template<typename T, typename = void> struct frame_have_plugin_initializer :std::false_type {};
		template<typename T> struct frame_have_plugin_initializer<T, std::void_t<typename T::plugin_initializer>> :std::true_type {};

		template<typename T, typename = void> struct frame_have_initializer :std::false_type {};
		template<typename T> struct frame_have_initializer<T, std::void_t<typename T::initializer>> :std::true_type {};

		template<typename T, typename = void> struct frame_have_forbid_plugin :std::false_type {};
		template<typename T> struct frame_have_forbid_plugin<T, std::void_t<typename T::forbid_plugin>> :std::true_type {};

		template<typename T, typename = void> struct frame_have_delegate_plugin :std::false_type {};
		template<typename T> struct frame_have_delegate_plugin < T, std::void_t<decltype(T::delegate_plugin(std::declval<typename T::form&>(), std::declval<std::function<void(void)>>)) >> :std::true_type {};
		
		template<typename T, typename = void> struct frame_have_tick_view :std::false_type {};
		template<typename T> struct frame_have_tick_view < T, std::void_t<decltype(T::tick_view) >> :std::integral_constant<bool,std::is_constructible<typename T::tick_view,typename T::form&>::value> {};

	}

	template<typename T> using frame_legal =
		std::integral_constant<bool,
		Assistant::frame_have_form<T>::value
		>;

	template<typename T> struct frame_static_assert
	{
		static_assert(Assistant::frame_have_form<T>::value, "frame need form");
		//static_assert(Assistant::frame_have_form_close<T>::value, "frame need form_close");
	};

	namespace Assistant
	{
		template<typename T, bool> struct frame_form_view: public T::form_view
		{
			typename T::form* form;
			frame_form_view(typename T::form* f) : T::form_view(f), form(f) {}
			frame_form_view(const frame_form_view&) = default;
		};

		template<typename T> struct frame_form_view<T,false>
		{
			typename T::form* form;
			frame_form_view(typename T::form* f) : form(f) {}
			frame_form_view(const frame_form_view&) = default;
		};

		template<typename T, bool> struct frame_tick_view : public T::tick_view
		{
			typename T::form* form;
			frame_tick_view(typename T::form* f) : T::tick_view(f), form(f) {}
			frame_tick_view(const frame_tick_view&) = default;
		};

		template<typename T> struct frame_tick_view<T, false>
		{
			typename T::form* form;
			frame_tick_view(typename T::form* f) : form(f) {}
			frame_tick_view(const frame_tick_view&) = default;
		};
	}

	template<typename T> struct frame: frame_static_assert<T>
	{
		using form = typename T::form;
		using form_view = Assistant::frame_form_view<T, Assistant::frame_have_form_view<T>::value>;
		using tick_view = Assistant::frame_tick_view<T, Assistant::frame_have_tick_view<T>::value>;
	};

	using duration = std::chrono::duration<long long, std::ratio<1, 1000>>;
}