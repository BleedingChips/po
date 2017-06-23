#pragma once
#include "frame/adapter.h"
namespace PO {
	class form_constraint
	{
		std::atomic_bool virtual_function_ready;
	public:
		virtual Respond ask_for_respond_mt(event& e) = 0;
		virtual Respond ask_for_respond(event& e) = 0;
		virtual Respond respond(event& e) { return Respond::Pass; }
		virtual bool available() const = 0;
		bool ready() const { return virtual_function_ready; }
		void end_construction() { virtual_function_ready = true; }
		void start_destruction() { virtual_function_ready = false; }
		form_constraint() : virtual_function_ready(false) {}
		form_constraint(const form_constraint&) : virtual_function_ready(false) {}
	};

	namespace Implement {

		template<typename form_t> struct have_avalible 
		{
			template<typename P> static std::true_type func(
				std::enable_if_t<
					std::is_same<
						decltype(((const form_t*)(nullptr))->available()),
						bool
					>::value
				>*
			);
			template<typename P> static std::false_type func(...);
			static constexpr bool value = decltype(func<form_t>(nullptr))::value;
		};

		template<typename form_t> struct have_make_value_table
		{
			template<typename P> static std::true_type func(std::enable_if_t<std::is_same<decltype(((form_t*)(nullptr))->mapping()), value_table>::value>*);
			template<typename P> static std::false_type func(...);
			static constexpr bool value = decltype(func<form_t>(nullptr))::value;
		};
	}

	template<typename form_t> struct form {
		static_assert(Implement::have_avalible<form_t>::value, "form should need an memeber function \'bool available() const\'");
		static_assert(Implement::have_make_value_table<form_t>::value, "form should need an memeber function \'value_table mapping()\'");
	};

}
