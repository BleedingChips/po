#pragma once
#include <map>
#include <exception>
#include <typeindex>
#include "frame\define.h"
#include "frame\viewer.h"
#include "tool\auto_adapter.h"
namespace PO {

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
	}

	template<typename form_t> struct form {
		static_assert(Implement::have_avalible<form_t>::value, "form should need an memeber function \'bool available() const\'");
		static_assert(Implement::have_make_value_table<form_t>::value, "form should need an memeber function \'value_table mapping()\'");
	};

}
