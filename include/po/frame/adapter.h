#pragma once
#include "../tool/tool.h"
#include "../tool/auto_adapter.h"
#include <typeindex>
#include <map>
#include <memory>
#include "viewer.h"
namespace PO
{
	class plugins; //store all plugin and renderer
	class self; //message

	struct adapter_interface {
		std::type_index ti;
		std::type_index index() const { return ti; }
		adapter_interface(std::type_index t) : ti(t) {}
	};

	namespace Implement {

		
		template<typename renderer_t> struct adapter_store_t : adapter_interface {

			std::function<void(self&, plugins&, viewer&, renderer_t&)> init;
			std::function<void(self&, plugins&, viewer&, renderer_t&, duration)> tick;

			adapter_store_t(
				std::function<void(self&, plugins&, viewer&, renderer_t&)> i,
				std::function<void(self&, plugins&, viewer&, renderer_t&, duration)> t
			) : adapter_interface(typeid(renderer_t)), init(std::move(i)), tick(std::move(t)) {}

			template<typename T, typename ...AT>
			adapter_store_t& bind_init(T&& t, AT&& ...at) {
				init_function = Tool::auto_bind_function<void(self&, plugins&, viewer&, renderer_t&), Tool::unorder_adapt>(std::forward<T>(t), std::forward<AT>(at)...);
				return *this;
			}

			template<typename T, typename ...AT>
			adapter_store_t& bind_tick(T&& t, AT&& ...at) {
				init_function = Tool::auto_bind_function<void(self&, plugins&, viewer&, renderer_t&, duration), Tool::unorder_adapt>(std::forward<T>(t), std::forward<AT>(at)...);
				return *this;
			}
		};
	}

	template<typename renderer_t, typename ptr, typename init_t, typename tick_t> std::shared_ptr<adapter_interface> make_member_adapter(ptr* p, init_t&& i, tick_t&& t) {
		return std::make_shared<Implement::adapter_store_t<renderer_t>>(
			Tool::auto_bind_function<void(self&, plugins&, viewer&, renderer_t&), Tool::unorder_adapt>(std::forward<init_t>(i), p),
			Tool::auto_bind_function<void(self&, plugins&, viewer&, renderer_t&, duration), Tool::unorder_adapt>(std::forward<tick_t>(t), p)
		);
	}

	/*
	template<typename renderer_t, typename ptr, typename init_t, typename tick_t> Implement::adapter_store_t<renderer_t> make_member_adapter_tick(ptr* p, tick_t t) {
		return {
			std::function<void(self&, plugins&, viewer&, renderer_t&)>{},
			Tool::auto_bind_function<void(void(self&, plugins&, viewer&, renderer_t&, duration)), Tool::unorder_adapt>(std::move(t), p)
		};
	}
	*/

	using adapter_map = std::vector<std::shared_ptr<adapter_interface>>;

	namespace Implement{
		struct value_table_element {
			virtual ~value_table_element() = default;
		};

		template<typename T> struct value_table_element_implement : value_table_element {
			T& t;
			value_table_element_implement(T& i) : t(i) {}
			operator T& () { return t; }
		};
	}

	struct value_table
	{
		std::map<std::type_index, std::shared_ptr<Implement::value_table_element>> mapping;
		value_table(std::initializer_list<decltype(mapping)::value_type> i) : mapping(i) {}

		template<typename type> bool find() const {
			return mapping.find(typeid(type)) != mapping.end();
		}

		template<typename type> Tool::optional<std::reference_wrapper<type>> get() {
			auto po = mapping.find(typeid(type));
			if (po == mapping.end()) return {};
			return { *static_cast<Implement::value_table_element_implement<type>*>(po->second.get()) };
		}
	};

	template<typename type, typename input_type>
	std::pair<std::type_index, std::shared_ptr<Implement::value_table_element>> make_value_table(input_type& t) {
		return {
			typeid(type),
			std::make_shared<Implement::value_table_element_implement<type>>(t)
		};
	}


}