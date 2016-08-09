#pragma once
#include <utility>

namespace PO {
	namespace Tool {

		/*----- type_container -----*/
		template<typename ...AT> struct type_container {};

		template<size_t ...i> struct index_container {};
		template<size_t i> struct index_container<i>:std::integral_constant<size_t, i> {};


		/*----- itself -----*/
		template<typename T> struct itself
		{
			using type = T;
		};
		template<typename T> using itself_t = T;

		/*----- dig -----*/
		/*
		namespace Assistant
		{
			template<size_t i, typename input> struct dig_execute
			{
				using type = typename dig_execute<i - 1, typename input::type >::type;
			};
			template< typename input> struct dig_execute<0,input>
			{
				using type = input;
			};
		}

		template<size_t i> struct dig
		{
			template<typename input> using in_t = typename Assistant::dig_execute<i, input>::type;
		};
		template<size_t i, typename input> using dig_t = typename Assistant::dig_execute<i, input>::type;
		*/

		/*----- extract -----*/
		namespace Assistant
		{
			template<template<typename ...> class output, typename input> struct extract_final;
			template<template<typename ...> class output, template<typename ...> class input_container, typename ...input> struct extract_final<output, input_container<input...>>
			{
				using type = output<input...>;
			};
		}
		template<template<typename ...> class output> struct extract
		{
			template<typename input > using in_t = typename Assistant::extract_final<output, input>::type;
		};
		template<template<typename...> class output, typename input> using extract_t = typename Assistant::extract_final<output, input>::type;


		/*----- instant -----*/
		namespace Assistant
		{
			template<template<typename ...> class output, typename ...input> struct instant_execute
			{
				using type = output<input...>;
			};
		}
		template<template<typename ...> class output, typename ...input> struct instant
		{
			template<typename ...other_input> using in = instant<output, input..., other_input...>;
			template<typename ...other_input> using in_t = typename Assistant::instant_execute<output, input..., other_input...>::type;
			template<typename ...other_input> using front_in = instant<output, other_input..., input...>;
			template<typename ...other_input> using front_in_t = typename Assistant::instant_execute<output, other_input..., input...>::type;
		};
		template<template<typename ...> class output, typename ...input> using instant_t = typename Assistant::instant_execute<output, input... >::type;


		/*----- packet -----*/
		namespace Assistant
		{
			template<template<typename...> class rule, template<typename...> class output, typename ...input> struct packet_execute
			{
				using type = output<rule<input>...>;
			};
		}
		template<template<typename...> class rule, template<typename...> class output, typename ...input> struct packet
		{
			template<typename ...other_input> using in = packet<rule, output, input..., other_input...>;
			template<typename ...other_input> using in_t = typename Assistant::packet_execute<rule, output, input..., other_input... >::type;
			template<typename ...other_input> using front_in = packet<rule, output, other_input..., input...>;
			template<typename ...other_input> using front_in_t = typename Assistant::packet_execute<rule, output, other_input..., input... >::type;
			template<typename ...packet_input_> using rule_in = packet<instant<rule, packet_input_...>::template front_in_t, output, input...>;
			template<typename ...packet_input_> using rule_in_t = output< rule< packet_input_..., input >...>;
			template<typename ...packet_input_> using rule_front_in_t = output< rule< packet_input_..., input >...>;
			template<typename ...packet_input_> using rulet_front_in = packet<instant<rule, packet_input_...>::template in_t, output, input...>;
		};
		template<template<typename> class packet_, template<typename...> class output, typename ...AT> using packet_t = typename Assistant::packet_execute<packet_, output, AT...>::type;


		/*----- for_packet -----*/
		namespace Assistant
		{
			template<typename result, typename last_input, template<typename ...> class rule, template<typename ...> class next_rule, template<typename...> class output, typename ...input> struct for_packet_execute_2;

			template<typename ...result, typename last_input, template<typename ...> class rule, template<typename ...> class next_rule, template<typename...> class output>
			struct for_packet_execute_2<Tool::type_container<result...>,last_input,rule,next_rule,output>
			{
				using type = output<result...>;
			};

			template<typename ...result, typename last_input, template<typename ...> class rule, template<typename ...> class next_rule, template<typename...> class output, typename this_type,typename ...input>
			struct for_packet_execute_2<Tool::type_container<result...>, last_input, rule, next_rule, output, this_type, input...>
			{
				using type = typename for_packet_execute_2<Tool::type_container<result..., rule<last_input, this_type>>, next_rule<last_input, this_type>, rule, next_rule, output, input...>::type;
			};

			template<template<typename...> class first_rule, template<typename ...> class first_next_rule, template<typename ...> class rule, template<typename ...> class next_rule, template<typename...> class output, typename ...input>
			struct for_packet_execute
			{
				using type = output<>;
			};

			template<template<typename...> class first_rule, template<typename ...> class first_next_rule, template<typename ...> class rule, template<typename ...> class next_rule, template<typename...> class output,typename this_type, typename ...input>
			struct for_packet_execute < first_rule, first_next_rule, rule, next_rule, output, this_type, input...>
			{
				using type = typename for_packet_execute_2<Tool::type_container<first_rule<this_type> >, first_next_rule<this_type>, rule, next_rule, output, input...>::type;
			};

		}
		template<template<typename...> class first_rule, template<typename ...> class first_next_rule, template<typename ...> class rule, template<typename ...> class next_rule, template<typename...> class output, typename ...input>
		struct for_packet
		{
			template<typename ...other_input> using in = for_packet<first_rule, first_next_rule, rule, next_rule, output, input..., other_input... >;
			template<typename ...other_input> using in_t = typename  Assistant::for_packet_execute<first_rule, first_next_rule, rule, next_rule, output, input..., other_input... >::type;
			template<typename ...other_input> using front_in = for_packet<first_rule, first_next_rule, rule, next_rule, output, other_input..., input... >;
			template<typename ...other_input> using front_in_t = typename  Assistant::for_packet_execute<first_rule, first_next_rule, rule, next_rule, output, other_input..., input... >::type;

			template<typename ...other_input> using first_rule_in = for_packet< instant<first_rule, other_input... >::template front_in_t, first_next_rule, rule, next_rule, output, input... >;
			template<typename ...other_input> using first_rule_in_t = typename  Assistant::for_packet_execute<instant<first_rule, other_input... >::template front_in_t, first_next_rule, rule, next_rule, output, input... >::type;
			template<typename ...other_input> using first_rule_front_in = for_packet< instant<first_rule, other_input... >::template in_t, first_next_rule, rule, next_rule, output, input... >;
			template<typename ...other_input> using first_rule_front_in_t = typename  Assistant::for_packet_execute<instant<first_rule, other_input... >::template in_t, first_next_rule, rule, next_rule, output, input... >::type;

			template<typename ...other_input> using first_next_rule_in = for_packet<first_rule, instant<first_next_rule, other_input...>::template front_in_t, rule, next_rule, output, input... >;
			template<typename ...other_input> using first_next_rule_in_t = typename Assistant::for_packet_execute<first_rule, instant<first_next_rule, other_input...>::template front_in_t, rule, next_rule, output, input... >::type;
			template<typename ...other_input> using first_next_rule_front_in = for_packet<first_rule, instant<first_next_rule, other_input...>::template in_t, rule, next_rule, output, input... >;
			template<typename ...other_input> using first_next_rule_front_in_t = typename Assistant::for_packet_execute<first_rule, instant<first_next_rule, other_input...>::template in_t, rule, next_rule, output, input... >::type;

			template<typename ...other_input> using rule_in = for_packet<first_rule, first_next_rule, instant<rule, other_input...>::template front_in_t, next_rule, output, input... >;
			template<typename ...other_input> using rule_in_t = typename  Assistant::for_packet_execute<first_rule, first_next_rule, instant<rule, other_input...>::template front_in_t, next_rule, output, input... >::type;
			template<typename ...other_input> using rule_front_in = for_packet<first_rule, first_next_rule, instant<rule, other_input...>::template in_t, next_rule, output, input... >;
			template<typename ...other_input> using rule_front_in_t = typename  Assistant::for_packet_execute<first_rule, first_next_rule, instant<rule, other_input...>::template in_t, next_rule, output, input... >::type;

			template<typename ...other_input> using next_rule_in = for_packet<first_rule, first_next_rule, rule, instant<next_rule, other_input...>::template front_in_t, output, input... >;
			template<typename ...other_input> using next_rule_in_t = typename  Assistant::for_packet_execute<first_rule, first_next_rule, rule, instant<next_rule, other_input...>::template front_in_t, output, input... >::type;
			template<typename ...other_input> using next_rule_front_in = for_packet<first_rule, first_next_rule, rule, instant<next_rule, other_input...>::template in_t, output, input... >;
			template<typename ...other_input> using next_rule_front_in_t = typename  Assistant::for_packet_execute<first_rule, first_next_rule, rule, instant<next_rule, other_input...>::template in_t, output, input... >::type;
		};
		template<template<typename...> class first_rule, template<typename ...> class first_next_rule, template<typename ...> class rule, template<typename ...> class next_rule, template<typename...> class output, typename ...input>
		using for_packet_t = typename  Assistant::for_packet_execute<first_rule, first_next_rule, rule, next_rule, output, input...>::type;



		/*----- value_and -----*/
		template< bool def , bool ...other > struct value_and : public std::conditional<def,value_and<other...>,std::false_type>::type{};
		template< bool def> struct value_and<def> : public std::conditional<def, std::true_type, std::false_type>::type{};
		
		/*----- value_or -----*/
		template< bool def, bool ...other > struct value_or : public std::conditional<def, std::true_type, value_or<other...>>::type {};
		template< bool def> struct value_or<def> : public std::conditional<def, std::true_type, std::false_type>::type{};

		/*----- is_one_of -----*/
		template<typename T, typename ...AT> struct is_one_of : std::false_type {};
		template<typename T, typename I, typename ...AT> struct is_one_of<T,I,AT...> : is_one_of<T,AT...> {};
		template<typename T, typename ...AT> struct is_one_of<T, T, AT...> : std::true_type {};

		template<typename T, typename ...AT> struct is_not_one_of : std::true_type {};
		template<typename T, typename I, typename ...AT> struct is_not_one_of<T, I, AT...> : is_not_one_of<T, AT...> {};
		template<typename T, typename ...AT> struct is_not_one_of<T, T, AT...> : std::false_type {};

		/*----- is_repeat -----*/
		template<typename ...AT> struct is_repeat : public std::false_type {};
		template<typename T, typename ...AT> 
		struct is_repeat <T, AT...>: std::conditional< is_one_of<T,AT...>::value , std::true_type, is_repeat < AT...>  >::type{};

		/*----- statement_if -----*/
		namespace Assistant
		{
			template<bool> struct statement_if_struct
			{
				template<typename F, typename P, typename ...AT> static decltype(auto) run(F&& t, P&& p, AT&&... at) { return t(std::forward<AT>(at)...); }
			};

			template<> struct statement_if_struct<false>
			{
				template<typename T, typename P, typename ...AT> static decltype(auto) run(T&& t, P&& p, AT&&... at) { return p(std::forward<AT>(at)...); }
			};

			template<bool s, typename T = int> struct statement_if_execute
			{
				T t;
				template<typename ...AT> decltype(auto) operator()(AT&&... at) { return t(std::forward<AT>(at)...); }
				template<bool other_s, typename K> statement_if_execute& elseif_(K&& k) {
					return *this;
				}
				template<typename K> statement_if_execute& else_(K&& k) {
					return *this;
				}
			};

			template<typename T> struct statement_if_execute<false, T>
			{
				T t;
				template<typename ...AT> decltype(auto) operator()(AT&&... at) { }
				template<bool other_s, typename K> decltype(auto) elseif_(K&& k) {
					return statement_if_execute<other_s, K>{std::forward<K>(k)};
				}
				template<typename K> decltype(auto) else_(K&& k) {
					return statement_if_execute<true, K>{std::forward<K>(k)};
				}
			};
		}

		template<bool s, typename T, typename P, typename ...AK> decltype(auto) statement_if(T&& t, P&& p, AK&& ...ak) { return Assistant::statement_if_struct<s>::run(std::forward<T>(t),std::forward<P>(p),std::forward<AK>(ak)...); }
		template<bool s, typename T> decltype(auto) statement_if(T&& t) { return Assistant::statement_if_execute<s, T>{std::forward<T>(t)}; }


		/*----- filter -----*/
		namespace Assistant
		{

			template<template<typename ...>  class rule, template<typename ...> class output, typename result, typename ...input > struct filter_execute;
			template<template<typename ...>  class rule, template<typename ...> class output, typename ...result > struct filter_execute<rule, output, Tool::type_container<result...>>
			{
				using type = output<result...>;
			};
			template<template<typename ...>  class rule, template<typename ...> class output, typename ...result, typename this_input, typename ...input >
			struct filter_execute<rule, output, Tool::type_container<result...>, this_input,input...>
			{
				using type = typename filter_execute<rule, output, std::conditional_t<rule<this_input>::value, Tool::type_container<result..., this_input>, Tool::type_container<result...> >, input...>::type;
			};
		}
		
		template<template<typename ...> class rule, template<typename ...> class output, typename ...input> struct filter
		{
			template<typename ...other_input> using in = filter<rule, output, input..., other_input...>;
			template<typename ...other_input> using in_t = typename Assistant::filter_execute<rule, output, Tool::type_container<>, input..., other_input...>::type;
			template<typename ...other_input> using front_in = filter<rule, output, other_input ..., input...>;
			template<typename ...other_input> using front_in_t = typename Assistant::filter_execute<rule, output, Tool::type_container<>, other_input ..., input ...>::type;
			template<typename ...other_input> using rule_in = filter<instant<rule,other_input...>::template front_in_t, output, input... >;
			template<typename ...other_input> using rule_in_t = typename Assistant::filter_execute<instant<rule, other_input...>::template front_in_t, output, Tool::type_container<>, input...>::type;
			template<typename ...other_input> using rule_front_in = filter<instant<rule, other_input...>::template in_t, output, input... >;
			template<typename ...other_input> using rule_front_in_t = typename Assistant::filter_execute<instant<rule, other_input...>::template in_t, output, Tool::type_container<>, input...>::type;
		};
		template<template<typename ...> class rule, template<typename ...> class output, typename ...input> using filter_t = typename Assistant::filter_execute<rule, output, Tool::type_container<>, input...>::type;
		

		/*----- reverse -----*/
		namespace Assistant
		{
			template<template<typename ...> class container, typename ...AT> struct reverse_execute
			{
				template<typename ...AT2> struct result
				{
					using type = container<AT2...>;
				};
			};
			template<template<typename ...> class container, typename T, typename ...AT> struct reverse_execute<container, T, AT...>
			{
				template<typename ...AT2> struct result
				{
					using type = typename reverse_execute<container, AT...>::template result<T, AT2...>::type;
				};
			};
		}
		template<template<typename ...> class container, typename ...AT> struct reverse
		{
			template<typename ...AT2> using with = reverse<container, AT..., AT2...>;
			template<typename ...AT2> using with_t = typename with<AT2...>::type;
			using type = typename Assistant::reverse_execute<container, AT... >::template result<>::type;
		};
		template<template<typename ...> class container, typename ...AT> using reverse_t = typename reverse< container, AT...>::type;

		
		/*----- localizer -----*/
		namespace Assistant
		{

			template<bool, template<size_t ...> class rule, size_t this_index, size_t ...input > struct localizer_execute_helper
			{
				using type = typename rule<input...>::type;
			};

			template< template<size_t ...> class rule, size_t this_index, size_t ...input > struct localizer_execute_helper<true,rule,this_index,input...>
			{
				using type = typename rule<input..., this_index>::type;
			};

			template<size_t cur, template<typename ...> class rule, template<size_t...> class index_output, typename ...AT> struct localizer_execute
			{
				template<size_t ...AT2> struct result
				{
					using type = index_output<AT2...>;
				};
			};
			template<size_t cur, template<typename ...> class rule, template<size_t...> class index_output, typename T, typename ...AT> 
			struct localizer_execute<cur, rule, index_output, T, AT...>
			{
				template<size_t ...AT2> struct result
				{
					using type = typename localizer_execute_helper< rule<T>::value, localizer_execute<cur + 1, rule, index_output, AT...>::template result, cur, AT2...>::type;
				};
			};
		}

		template<size_t start_index, template<typename ...> class rule, template<size_t...> class index_output, typename ...input> struct localizer
		{
			template<typename ...other_input> using in = localizer<start_index, rule, index_output, input..., other_input...>;
			template<typename ...other_input> using in_t = typename Assistant::localizer_execute<start_index, rule, index_output, input..., other_input... >::template result<>::type;
			template<typename ...other_input> using front_in = localizer<start_index, rule, index_output, other_input..., input...>;
			template<typename ...other_input> using front_in_t = typename Assistant::localizer_execute<start_index, rule, index_output, other_input..., input... >::template result<>::type;
			template<typename ...input_other> using rule_in = localizer<start_index, instant<rule, input_other...>::template fornt_in_t, index_output, input...>;
			template<typename ...input_other> using rule_in_t = typename Assistant::localizer_execute<start_index, instant<rule, input_other...>::template fornt_in_t, index_output, input...>::template result<>::type;
			template<typename ...input_other> using rule_front_in = localizer<start_index, instant<rule, input_other...>::template in_t, index_output, input...>;
			template<typename ...input_other> using rule_front_in_t = typename Assistant::localizer_execute<start_index, instant<rule, input_other...>::template in_t, index_output, input...>::template result<>::type;
		};
		template<size_t start_index, template<class ...> class role, template<size_t...> class index_outpu, typename ...input> using localizer_t = typename Assistant::localizer_execute<start_index, role, index_outpu, input... >::template result<>::type;


		/*----- index_separate -----*/
		namespace Assistant
		{
			template<template<typename...> class output, template <size_t...> class rule, typename ...input> struct index_separate_execute
			{
				template<typename ...res > struct result
				{
					using type = output<  res... >;
				};
			};
			template<template<typename...> class output, template <size_t...> class rule, template <size_t ...> class input_rule, size_t ...this_input, typename ...input> struct index_separate_execute<output, rule, input_rule<this_input...>, input...>
			{
				template<typename ...res > struct result
				{
					using type = typename index_separate_execute<output, rule, input...>::template result<res..., rule<this_input>...>::type;
				};
			};
		}
		template<template<typename...> class output, template <size_t...> class rule, typename ...input> struct index_separate
		{
			template <typename ...other_input> using in = index_separate<output, rule, input..., other_input... >;
			template <typename ...other_input> using in_t = typename Assistant::index_separate_execute<output, rule, input..., other_input... >::template result<>::type;
			template <typename ...other_input> using front_in = index_separate<output, rule, other_input..., input... >;
			template <typename ...other_input> using front_in_t = typename Assistant::index_separate_execute<output, rule, other_input ..., input ... >::template result<>::type;
		};
		template<template<typename...> class output, template <size_t...> class rule, typename ...input> using index_separate_t = typename Assistant::index_separate_execute<output, rule, input...>::template result<>::type;

		/*----- index_merga -----*/
		namespace Assistant
		{
			template<template <size_t...> class rule, typename ...input> struct index_merga_execute
			{
				template<size_t ...index>
				struct result
				{
					using type = rule<index...>;
				};
			};
			template<template <size_t...> class rule, template<size_t...> class this_rule, size_t ...input_index, typename ...input> struct index_merga_execute<rule, this_rule<input_index...>, input...>
			{
				template<size_t ...other_index> struct result
				{
					using type = typename index_merga_execute < rule, input...>::template result<other_index..., input_index... >::type;
				};
			};
		}
		template<template <size_t...> class output, typename ...input> struct index_merga
		{
			template <typename ...other_input> using in = index_merga<output, input..., other_input... >;
			template <typename ...other_input> using in_t = typename Assistant::index_merga_execute<output, input..., other_input... >::template result<>::type;
			template <typename ...other_input> using front_in = index_merga<output, other_input..., input... >;
			template <typename ...other_input> using front_in_t = typename Assistant::index_merga_execute<output, other_input ..., input ... >::template result<>::type;
		};
		template<template<size_t...> class output, typename ...input> using index_merga_t = typename Assistant::index_merga_execute<output, input...>::template result<>::type;

		/*----- picker -----*/
		namespace Assistant
		{
			template<size_t index, typename this_input, typename ...other_input> struct picker_execute
			{
				static_assert(sizeof...(other_input) < index, "PO::Tool::picker the number of input type is less then require");
				using type = typename picker_execute<index - 1, other_input...>::type;
			};

			template<typename this_input, typename ...other_input> struct picker_execute<0, this_input, other_input...>
			{
				using type = this_input;
			};
		}

		template<size_t i, typename ...input> struct picker
		{
			template<typename ...other_input> using in = picker<i, input..., other_input...>;
			template<typename ...other_input> using in_t = typename Assistant::picker_execute<i, input..., other_input...>::type;
			template<typename ...other_input> using front_in = picker<i, other_input..., input...>;
			template<typename ...other_input> using front_in_t = typename Assistant::picker_execute<i, other_input..., input...>::type;
		};
		template<size_t index, typename ...input> using picker_t = typename Assistant::picker_execute<index, input...>::type;

		

		/*----- pick_parameter -----*/
		template<size_t i> struct pick_parameter
		{
			template<typename this_parameter, typename ...parameter>
			static decltype(auto) in(this_parameter&& tp, parameter&& ... pa) 
			{
				static_assert(i <= sizeof...(pa), "pick_parameter overflow");
				return pick_parameter<i - 1>::in(std::forward<parameter>(pa)...); 
			}
		};

		template<> struct pick_parameter<0>
		{
			template<typename this_parameter, typename ...parameter>
			static decltype(auto) in(this_parameter&& tp, parameter&& ... pa) { return std::forward<this_parameter>(tp); }
		};

		/*----- selector -----*/
		//template<typename in>

	}
}