#pragma once
#include <map>
#include <exception>
#include <typeindex>
#include "frame\define.h"
#include "frame\viewer.h"
#include "tool\auto_adapter.h"
namespace PO {

	struct value_table
	{

		struct value_not_exist : std::exception
		{
			const char* what() const override;
		};

		std::map<std::type_index, void*> mapping;
		value_table(std::initializer_list<typename decltype(mapping)::value_type> il) : mapping(std::move(il)) {}

		template<typename type> bool find() const {
			return mapping.find(typeid(type)) != mapping.end();
		}

		template<typename type> std::remove_reference_t<type>& get() {
			auto po = mapping.find(typeid(type));
			if (po != mapping.end()) return { *static_cast<std::remove_reference_t<type>*>(po->second) };
			throw value_table::value_not_exist{};
		}
	};

	template<typename type, typename input_type>
	std::pair<std::type_index, void*> make_value_table(input_type& t) {
		return {
			typeid(type),
			static_cast<std::remove_reference_t<type>*>(&t)
		};
	}

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
