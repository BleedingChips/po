#pragma once
#include <functional>
#include "type_tool.h"
namespace PO
{
	namespace Tool
	{
		namespace Assistant
		{

			template<typename T> class function_object_parameter_able_detect_
			{
				using final_input = std::remove_reference_t<T>;
				template<typename K>
				static std::true_type func(std::integral_constant<decltype(&K::operator()), &K::operator()>*);
				template<typename K>
				static std::false_type func(...);
			public :
				static constexpr bool value = decltype(func<final_input>(nullptr))::value;
			};

			template<typename T, typename P, typename ...AT> struct function_type 
			{
				static constexpr bool is_member = std::is_same<P, void>::value;
				template<template<typename ...> class out> using out_parameter = out<AT...>;
			};


			template<typename T> class function_object_parameter_detect_
			{
				using final_input = std::remove_reference_t<T>;
				static_assert(function_object_parameter_able_detect_<final_input>::value, "can not be detect_");
				template< typename P, typename ...AT > static function_type<P, void, AT...> func(P(*)(AT...));
				template< typename P, typename L, typename ...AT> static function_type<P, L, AT...> func(P(L::*)(AT...));
				template< typename P, typename L, typename ...AT> static function_type<P, L, AT...> func(P(L::*)(AT...) const );
				template< typename P, typename L, typename ...AT> static function_type<P, L, AT...> func(P(L::*)(AT...) volatile);
				template< typename P, typename L, typename ...AT> static function_type<P, L, AT...> func(P(L::*)(AT...) const volatile);
				template< typename P, typename L, typename ...AT> static function_type<P, L, AT...> func(P(L::*)(AT...) const &&);
				template< typename P, typename L, typename ...AT> static function_type<P, L, AT...> func(P(L::*)(AT...) volatile &&);
				template< typename P, typename L, typename ...AT> static function_type<P, L, AT...> func(P(L::*)(AT...) const volatile &&);
				template< typename P, typename L, typename ...AT> static function_type<P, L, AT...> func(P(L::*)(AT...) &);
				template< typename P, typename L, typename ...AT> static function_type<P, L, AT...> func(P(L::*)(AT...) const &);
				template< typename P, typename L, typename ...AT> static function_type<P, L, AT...> func(P(L::*)(AT...) volatile &);
				template< typename P, typename L, typename ...AT> static function_type<P, L, AT...> func(P(L::*)(AT...) const volatile &);
			public:
				using type = decltype(func(&final_input::operator()));
			};

			template<size_t ... index, typename fun_obj, typename ...in_para>
			decltype(auto) app_adapter_func(Tool::index_container<index...>, fun_obj&& fo, in_para&&... ip)
			{
				return fo(Tool::pick_parameter<index>::in(std::forward<in_para>(ip)...)...);
			}

			template<size_t ... index, typename fun_ret, typename type, typename ...fun_para, typename ...in_para>
			decltype(auto) app_adapter_func(Tool::index_container<index...>, fun_ret(type::* func)(fun_para...), type& t, in_para&&... ip)
			{
				return (t.*func)(Tool::pick_parameter<index>::in(std::forward<in_para>(ip)...)...);
			}

			template<size_t ... index, typename fun_ret, typename type, typename ...fun_para, typename ...in_para>
			decltype(auto) app_adapter_func(Tool::index_container<index...>, fun_ret(type::* func)(fun_para...) &, type& t, in_para&&... ip)
			{
				return (t.*func)(Tool::pick_parameter<index>::in(std::forward<in_para>(ip)...)...);
			}

			template<size_t ... index, typename fun_ret, typename type, typename ...fun_para, typename ...in_para>
			decltype(auto) app_adapter_func(Tool::index_container<index...>, fun_ret(type::* func)(fun_para...) &&, type&& t, in_para&&... ip)
			{
				return (std::move(t).*func)(Tool::pick_parameter<index>::in(std::forward<in_para>(ip)...)...);
			}

			template<size_t ... index, typename fun_ret, typename type, typename ...fun_para, typename ...in_para>
			decltype(auto) app_adapter_func(Tool::index_container<index...>, fun_ret(type::* func)(fun_para...) const, const type& t, in_para&&... ip)
			{
				return (t.*func)(Tool::pick_parameter<index>::in(std::forward<in_para>(ip)...)...);
			}

			template<size_t ... index, typename fun_ret, typename type, typename ...fun_para, typename ...in_para>
			decltype(auto) app_adapter_func(Tool::index_container<index...>, fun_ret(type::* func)(fun_para...) const &, const type& t, in_para&&... ip)
			{
				return (t.*func)(Tool::pick_parameter<index>::in(std::forward<in_para>(ip)...)...);
			}

			template<size_t ... index, typename fun_ret, typename type, typename ...fun_para, typename ...in_para>
			decltype(auto) app_adapter_func(Tool::index_container<index...>, fun_ret(type::* func)(fun_para...) const && ,const type&& t, in_para&&... ip)
			{
				return (std::move(t).*func)(Tool::pick_parameter<index>::in(std::forward<in_para>(ip)...)...);
			}

			template<size_t ... index, typename fun_ret, typename type, typename ...fun_para, typename ...in_para>
			decltype(auto) app_adapter_func(Tool::index_container<index...>, fun_ret(type::* func)(fun_para...) volatile , type& t, in_para&&... ip)
			{
				return (t.*func)(Tool::pick_parameter<index>::in(std::forward<in_para>(ip)...)...);
			}

			template<size_t ... index, typename fun_ret, typename type, typename ...fun_para, typename ...in_para>
			decltype(auto) app_adapter_func(Tool::index_container<index...>, fun_ret(type::* func)(fun_para...) volatile &, type& t, in_para&&... ip)
			{
				return (t.*func)(Tool::pick_parameter<index>::in(std::forward<in_para>(ip)...)...);
			}

			template<size_t ... index, typename fun_ret, typename type, typename ...fun_para, typename ...in_para>
			decltype(auto) app_adapter_func(Tool::index_container<index...>, fun_ret(type::* func)(fun_para...) volatile && , type&& t, in_para&&... ip)
			{
				return (std::move(t).*func)(Tool::pick_parameter<index>::in(std::forward<in_para>(ip)...)...);
			}

			template<size_t ... index, typename fun_ret, typename type, typename ...fun_para, typename ...in_para>
			decltype(auto) app_adapter_func(Tool::index_container<index...>, fun_ret(type::* func)(fun_para...) volatile const, const type& t, in_para&&... ip)
			{
				return (t.*func)(Tool::pick_parameter<index>::in(std::forward<in_para>(ip)...)...);
			}

			template<size_t ... index, typename fun_ret, typename type, typename ...fun_para, typename ...in_para>
			decltype(auto) app_adapter_func(Tool::index_container<index...>, fun_ret(type::* func)(fun_para...) volatile const &, const type& t, in_para&&... ip)
			{
				return (t.*func)(Tool::pick_parameter<index>::in(std::forward<in_para>(ip)...)...);
			}

			template<size_t ... index, typename fun_ret, typename type, typename ...fun_para, typename ...in_para>
			decltype(auto) app_adapter_func(Tool::index_container<index...>, fun_ret(type::* func)(fun_para...) volatile const &&, const type&& t, in_para&&... ip)
			{
				return (std::move(t).*func)(Tool::pick_parameter<index>::in(std::forward<in_para>(ip)...)...);
			}



			template<typename target_type, typename ...detect_type> struct match_detect
			{
				template<typename match_type> using match_ope = std::is_constructible<target_type, match_type>;
				using type = localizer_t<0, match_ope, Tool::index_container, detect_type...>;
			};
			template<typename target_type, typename ...detect_type> using match_detect_t = typename match_detect<target_type, detect_type...>::type;

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


		template<template<typename ...> class adapter_rule, typename func_object,typename ...input>
		decltype(auto) auto_adapter(func_object&& fo, input&&... input_para)
		{
			return Tool::statement_if<!Assistant::function_object_parameter_able_detect_<func_object>::value>
				(
					[](auto&& fun_object,auto&& ...in) {  return fun_object(std::forward<decltype(in)&&>(in)...); },
					[](auto&& fun_object,auto&& ...in) {

				using index = typename Assistant::function_object_parameter_detect_<decltype(fun_object)>::type::template out_parameter
					<
						Tool::packet<
							Tool::instant<Assistant::match_detect_t, input...>::template front_in_t,
							adapter_rule
						>::template in_t
					>;
				return Assistant::app_adapter_func(index(), std::forward<decltype(fun_object) && >(fun_object), std::forward<decltype(in) && >(in)...);
			},
					std::forward<func_object>(fo),
				std::forward<input>(input_para)...
					);
		}

		
		template<template<typename ...> class adapter_rule, typename func_ret, typename ...func_parameter, typename ...input>
		decltype(auto) auto_adapter(func_ret (fo)(func_parameter...), input&&... input_para)
		{
			using index = adapter_rule< Tool::instant_t<Assistant::match_detect_t, func_parameter, input...>...>;
			Assistant::app_adapter_func(index(), fo, std::forward<input>(input_para)...);
		}

		
		template<template<typename ...> class adapter_rule, typename func_ret, typename type, typename ...func_parameter, typename ...input>
		decltype(auto) auto_adapter(func_ret(type::* fo)(func_parameter...), type& t,input&&... input_para)
		{
			using index = adapter_rule< Tool::instant_t<Assistant::match_detect_t, func_parameter, input...>...>;
			Assistant::app_adapter_func(index(), fo, t, std::forward<input>(input_para)...);
		}

		template<template<typename ...> class adapter_rule, typename func_ret, typename type, typename ...func_parameter, typename ...input>
		decltype(auto) auto_adapter(func_ret(type::* fo)(func_parameter...) &, type& t, input&&... input_para)
		{
			using index = adapter_rule< Tool::instant_t<Assistant::match_detect_t, func_parameter, input...>...>;
			Assistant::app_adapter_func(index(), fo, t, std::forward<input>(input_para)...);
		}

		template<template<typename ...> class adapter_rule, typename func_ret, typename type, typename ...func_parameter, typename ...input>
		decltype(auto) auto_adapter(func_ret(type::* fo)(func_parameter...) &&, type&& t, input&&... input_para)
		{
			using index = adapter_rule< Tool::instant_t<Assistant::match_detect_t, func_parameter, input...>...>;
			Assistant::app_adapter_func(index(), fo, std::move(t), std::forward<input>(input_para)...);
		}

		template<template<typename ...> class adapter_rule, typename func_ret, typename type, typename ...func_parameter, typename ...input>
		decltype(auto) auto_adapter(func_ret(type::* fo)(func_parameter...) const , const type& t, input&&... input_para)
		{
			using index = adapter_rule< Tool::instant_t<Assistant::match_detect_t, func_parameter, input...>...>;
			Assistant::app_adapter_func(index(), fo, t, std::forward<input>(input_para)...);
		}

		template<template<typename ...> class adapter_rule, typename func_ret, typename type, typename ...func_parameter, typename ...input>
		decltype(auto) auto_adapter(func_ret(type::* fo)(func_parameter...) const &, const type& t, input&&... input_para)
		{
			using index = adapter_rule< Tool::instant_t<Assistant::match_detect_t, func_parameter, input...>...>;
			Assistant::app_adapter_func(index(), fo, t, std::forward<input>(input_para)...);
		}

		template<template<typename ...> class adapter_rule, typename func_ret, typename type, typename ...func_parameter, typename ...input>
		decltype(auto) auto_adapter(func_ret(type::* fo)(func_parameter...) const && , const type&& t, input&&... input_para)
		{
			using index = adapter_rule< Tool::instant_t<Assistant::match_detect_t, func_parameter, input...>...>;
			Assistant::app_adapter_func(index(), fo, std::move(t), std::forward<input>(input_para)...);
		}

		template<template<typename ...> class adapter_rule, typename func_ret, typename type, typename ...func_parameter, typename ...input>
		decltype(auto) auto_adapter(func_ret(type::* fo)(func_parameter...) volatile , type& t, input&&... input_para)
		{
			using index = adapter_rule< Tool::instant_t<Assistant::match_detect_t, func_parameter, input...>...>;
			Assistant::app_adapter_func(index(), fo, t, std::forward<input>(input_para)...);
		}

		template<template<typename ...> class adapter_rule, typename func_ret, typename type, typename ...func_parameter, typename ...input>
		decltype(auto) auto_adapter(func_ret(type::* fo)(func_parameter...) volatile &, type& t, input&&... input_para)
		{
			using index = adapter_rule< Tool::instant_t<Assistant::match_detect_t, func_parameter, input...>...>;
			Assistant::app_adapter_func(index(), fo, t, std::forward<input>(input_para)...);
		}

		template<template<typename ...> class adapter_rule, typename func_ret, typename type, typename ...func_parameter, typename ...input>
		decltype(auto) auto_adapter(func_ret(type::* fo)(func_parameter...) volatile && , type&& t, input&&... input_para)
		{
			using index = adapter_rule< Tool::instant_t<Assistant::match_detect_t, func_parameter, input...>...>;
			Assistant::app_adapter_func(index(), fo, std::move(t), std::forward<input>(input_para)...);
		}

		template<template<typename ...> class adapter_rule, typename func_ret, typename type, typename ...func_parameter, typename ...input>
		decltype(auto) auto_adapter(func_ret(type::* fo)(func_parameter...) volatile const, const type& t, input&&... input_para)
		{
			using index = adapter_rule< Tool::instant_t<Assistant::match_detect_t, func_parameter, input...>...>;
			Assistant::app_adapter_func(index(), fo, t, std::forward<input>(input_para)...);
		}

		template<template<typename ...> class adapter_rule, typename func_ret, typename type, typename ...func_parameter, typename ...input>
		decltype(auto) auto_adapter(func_ret(type::* fo)(func_parameter...) volatile const &, const type& t, input&&... input_para)
		{
			using index = adapter_rule< Tool::instant_t<Assistant::match_detect_t, func_parameter, input...>...>;
			Assistant::app_adapter_func(index(), fo, t, std::forward<input>(input_para)...);
		}

		template<template<typename ...> class adapter_rule, typename func_ret, typename type, typename ...func_parameter, typename ...input>
		decltype(auto) auto_adapter(func_ret(type::* fo)(func_parameter...) volatile const &&, const type&& t, input&&... input_para)
		{
			using index = adapter_rule< Tool::instant_t<Assistant::match_detect_t, func_parameter, input...>...>;
			Assistant::app_adapter_func(index(), fo, std::move(t), std::forward<input>(input_para)...);
		}
	}
}
