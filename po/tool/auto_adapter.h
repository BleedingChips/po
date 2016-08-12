#pragma once
#include <functional>
#include "type_tool.h"
namespace PO
{
	namespace Tool
	{
		namespace Assistant
		{

			/*----- match_detect -----*/
			template<typename target_type, typename ...detect_type> struct match_detect
			{
				template<typename match_type> using match_ope = std::is_convertible<target_type, match_type>;
				using type = localizer_t<0, match_ope, Tool::index_container, detect_type...>;
			};
			template<typename target_type, typename ...detect_type> using match_detect_t = typename match_detect<target_type, detect_type...>::type;

			/*----- adapter_operator -----*/
			template<typename T, typename L> struct adapter_index_less : std::integral_constant<bool, (T::value < L::value)>{};
			template<typename v1, typename ...vo> struct adapter_find_min_index { using type = v1; };
			template<typename v1, typename v2, typename ...vo> struct adapter_find_min_index<v1, v2, vo...>
			{
				using type = typename adapter_find_min_index<std::conditional_t<adapter_index_less<v1, v2>::value, v1, v2 >, vo...>::type;
			};
			template<typename v1, typename ...vo> using adapter_find_min_index_t = typename adapter_find_min_index<v1, vo...>::type;

			template<typename v1, typename v2> struct adapter_order_rule
			{
				using type = Tool::index_separate_t<
					Tool::filter<Tool::instant<adapter_index_less, v1>::template in_t, adapter_find_min_index_t >::template in_t,
					Tool::index_container,
					v2
				>;
			};
			template<typename v1, typename v2> using adapter_order_rule_t = typename adapter_order_rule<v1, v2>::type;

			template<typename ...input> struct adapter_by_order
			{
				using type = Tool::for_packet_t<
					Tool::index_separate<adapter_find_min_index_t, Tool::index_container>::template in_t,
					Tool::index_separate<adapter_find_min_index_t, Tool::index_container>::template in_t,
					adapter_order_rule_t,
					adapter_order_rule_t,
					Tool::index_merga<Tool::index_container>::template in_t,
					input...
				>;
			};

			template<typename v1, typename v2> struct adapter_first_match_rule
			{
				using type = Tool::index_separate_t <
					Tool::filter<
						Tool::index_separate_t<Tool::instant<Tool::is_not_one_of>::template in,Tool::index_container,v1>::template front_in_t,
						adapter_find_min_index_t
					>::template in_t,
					Tool::index_container,
					v2
				>;
			};
			template<typename v1, typename v2> using adapter_first_match_rule_t = typename adapter_first_match_rule<v1, v2>::type;

			template<typename v1, typename v2> struct adapter_first_match_next_rule
			{
				using type = Tool::index_merga_t<Tool::index_container, v1, typename adapter_first_match_rule<v1, v2>::type>;
			};
			template<typename v1, typename v2> using adapter_first_match_next_rule_t = typename adapter_first_match_next_rule<v1, v2>::type;
			
			template<typename ...input> struct adapter_by_first_match
			{
				using type = Tool::for_packet_t<
					Tool::index_separate<adapter_find_min_index_t, Tool::index_container>::template in_t,
					Tool::index_separate<adapter_find_min_index_t, Tool::index_container>::template in_t,
					adapter_first_match_rule_t,
					adapter_first_match_next_rule_t,
					Tool::index_merga<Tool::index_container>::template in_t,
					input...
				>;
			};

		}
		
		template<typename ...input> using adapter_by_order_t = typename Assistant::adapter_by_order<input...>::type;
		template<typename ...input> using adapter_by_first_match_t = typename Assistant::adapter_by_first_match<input...>::type;


		/*----- auto_adapter -----*/
		namespace Assistant
		{

			template<size_t ... index, typename fun_obj, typename ...input>
			decltype(auto) auto_adapter_execute(Tool::index_container<index...>, fun_obj&& fo, input&& ...in)
			{
				return Tool::apply_function_object(std::forward<fun_obj>(fo),
					Tool::pick_parameter<index>::in(std::forward<input>(in)...)...
				);
			}

			template<bool, typename index> struct auto_adapter_index_execute;
			template<size_t ...index> struct auto_adapter_index_execute<false,Tool::index_container<index...>>
			{
				using type = Tool::index_container<0, (index + 1)...>;
			};
			template<size_t ...index> struct auto_adapter_index_execute<true, Tool::index_container<index...>>
			{
				using type = Tool::index_container<index...>;
			};

		}
		
		template<template<typename ...> class adapter_rule, typename func_object,typename ...input>
		decltype(auto) auto_adapter(func_object&& fo, input&&... in)
		{
			using fun_type = funtion_detect_t<func_object>;
			static_assert(std::is_same<typename fun_type::owner_type, void>::value || sizeof...(in) >= 1, "PO::Tool::auto_adapter need a ref of the owner for its member function");

			using pick_index = make_index_range_t<Tool::index_container, (std::is_same<typename fun_type::owner_type, void>::value ? 0 : 1), sizeof...(in)>;

			using pre_index = funtion_detect_t<func_object>::template parameter_out
				<
					packet<
						select_t<
							pick_index,
							Tool::instant<Assistant::match_detect_t>::template in,
							input...
						>::template front_in_t,
						adapter_rule
					>::template in_t
				>;
			
			using finnal_index = std::conditional_t<
				fun_type::value,
				typename  Assistant::auto_adapter_index_execute<
					std::is_same<typename fun_type::owner_type, void>::value,
					pre_index
				>::type,
				make_index_range_t<Tool::index_container, 0, sizeof...(in)>
			>;
			
			return Assistant::auto_adapter_execute(finnal_index(),std::forward<func_object>(fo),std::forward<input>(in)...);
			
		}

	}
}
