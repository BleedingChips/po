#pragma once
#include <utility>

namespace PO {
	namespace Tool {

		/*----- container -----*/
		template<typename ...AT> struct type_container {};

		template<size_t ...i> struct index_container {};
		template<size_t i> struct index_container<i>:std::integral_constant<size_t, i> {};


		/*----- make_index_range -----*/
		namespace Assistant
		{
			template<typename result, template<size_t...> class out, size_t cur, size_t to> struct make_index_range_execute;
			template<size_t ...result, template<size_t...> class out, size_t cur, size_t to> struct make_index_range_execute<Tool::index_container<result...>, out, cur, to>
			{
				using type = typename make_index_range_execute<Tool::index_container<result..., cur>, out, cur + (cur < to ? +1 : -1), to >::type;
			};
			template<size_t ...result, template<size_t...> class out, size_t to> struct make_index_range_execute<Tool::index_container<result...>, out, to, to>
			{
				using type = out<result...>;
			};
		}

		template<template<size_t ...> class out, size_t start, size_t to> struct make_index_range
		{
			using type = typename Assistant::make_index_range_execute<index_container<>, out, start, to>::type;
		};
		template<template<size_t ...> class out, size_t start, size_t to> using make_index_range_t = typename Assistant::make_index_range_execute<index_container<>, out, start, to>::type;




		/*----- itself -----*/
		template<typename T> struct itself
		{
			using type = T;
		};
		template<typename T> using itself_t = T;

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
			template<typename ...other_input> using in_t_t = typename Assistant::instant_execute<output, input..., other_input...>::type::type;
			template<typename ...other_input> using front_in = instant<output, other_input..., input...>;
			template<typename ...other_input> using front_in_t = typename Assistant::instant_execute<output, other_input..., input...>::type;
			template<typename ...other_input> using front_in_t_t = typename Assistant::instant_execute<output, other_input..., input...>::type::type;
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
			struct for_packet_execute_2<type_container<result...>,last_input,rule,next_rule,output>
			{
				using type = output<result...>;
			};

			template<typename ...result, typename last_input, template<typename ...> class rule, template<typename ...> class next_rule, template<typename...> class output, typename this_type,typename ...input>
			struct for_packet_execute_2<type_container<result...>, last_input, rule, next_rule, output, this_type, input...>
			{
				using type = typename for_packet_execute_2<type_container<result..., rule<last_input, this_type>>, next_rule<last_input, this_type>, rule, next_rule, output, input...>::type;
			};

			template<template<typename...> class first_rule, template<typename ...> class first_next_rule, template<typename ...> class rule, template<typename ...> class next_rule, template<typename...> class output, typename ...input>
			struct for_packet_execute
			{
				using type = output<>;
			};

			template<template<typename...> class first_rule, template<typename ...> class first_next_rule, template<typename ...> class rule, template<typename ...> class next_rule, template<typename...> class output,typename this_type, typename ...input>
			struct for_packet_execute < first_rule, first_next_rule, rule, next_rule, output, this_type, input...>
			{
				using type = typename for_packet_execute_2<type_container<first_rule<this_type> >, first_next_rule<this_type>, rule, next_rule, output, input...>::type;
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
			template<template<typename ...>  class rule, template<typename ...> class output, typename ...result > struct filter_execute<rule, output, type_container<result...>>
			{
				using type = output<result...>;
			};
			template<template<typename ...>  class rule, template<typename ...> class output, typename ...result, typename this_input, typename ...input >
			struct filter_execute<rule, output, type_container<result...>, this_input,input...>
			{
				using type = typename filter_execute<rule, output, std::conditional_t<rule<this_input>::value, type_container<result..., this_input>, type_container<result...> >, input...>::type;
			};
		}
		
		template<template<typename ...> class rule, template<typename ...> class output, typename ...input> struct filter
		{
			template<typename ...other_input> using in = filter<rule, output, input..., other_input...>;
			template<typename ...other_input> using in_t = typename Assistant::filter_execute<rule, output, type_container<>, input..., other_input...>::type;
			template<typename ...other_input> using front_in = filter<rule, output, other_input ..., input...>;
			template<typename ...other_input> using front_in_t = typename Assistant::filter_execute<rule, output, type_container<>, other_input ..., input ...>::type;
			template<typename ...other_input> using rule_in = filter<instant<rule,other_input...>::template front_in_t, output, input... >;
			template<typename ...other_input> using rule_in_t = typename Assistant::filter_execute<instant<rule, other_input...>::template front_in_t, output, type_container<>, input...>::type;
			template<typename ...other_input> using rule_front_in = filter<instant<rule, other_input...>::template in_t, output, input... >;
			template<typename ...other_input> using rule_front_in_t = typename Assistant::filter_execute<instant<rule, other_input...>::template in_t, output, type_container<>, input...>::type;
		};
		template<template<typename ...> class rule, template<typename ...> class output, typename ...input> using filter_t = typename Assistant::filter_execute<rule, output, type_container<>, input...>::type;
		

		/*----- reverse -----*/
		namespace Assistant
		{
			template<template<typename ...> class output, typename result, typename ...AT> struct reverse_execute;
			template<template<typename ...> class output, typename ...result> struct reverse_execute<output, type_container<result...>>
			{
				using type = output<result...>;
			};
			template<template<typename ...> class output, typename ...result,typename this_type, typename ...AT> struct reverse_execute<output, type_container<result...>, this_type, AT...>
			{
				using type = typename reverse_execute<output, type_container<this_type, result...>, AT... >::type;
			};
		}
		template<template<typename ...> class output, typename ...input> struct reverse
		{
			template<typename ...other_input> using in = reverse<output, input..., other_input...>;
			template<typename ...other_input> using in_t = typename Assistant::reverse_execute<output,type_container<>, input..., other_input...>::type;
			template<typename ...other_input> using front_in = reverse<output, other_input..., input...>;
			template<typename ...other_input> using front_in_t = typename Assistant::reverse_execute<output, type_container<>, other_input..., input...>::type;
		};
		template<template<typename ...> class output, typename ...input> using reverse_t = typename Assistant::reverse_execute<output, type_container<>, input...>::type;

		
		/*----- localizer -----*/
		namespace Assistant
		{
			template<size_t cur, template<typename ...> class rule, template<size_t...> class index_output, typename result, typename ...AT> struct localizer_execute;
			template<size_t cur, template<typename ...> class rule, template<size_t...> class index_output, size_t ...result> struct localizer_execute<cur,rule,index_output, index_container<result...>>
			{
				using type = index_output<result...>;
			};
			template<size_t cur, template<typename ...> class rule, template<size_t...> class index_output, size_t ...result,typename this_type, typename ...AT> struct localizer_execute<cur, rule, index_output, index_container<result...>,this_type, AT...>
			{
				using type = typename localizer_execute<cur + 1, rule, index_output, std::conditional_t<rule<this_type>::value, index_container<result..., cur>, index_container<result...> > , AT...>::type;
			};
		}

		template<size_t start_index, template<typename ...> class rule, template<size_t...> class index_output, typename ...input> struct localizer
		{
			template<typename ...other_input> using in = localizer<start_index, rule, index_output, input..., other_input...>;
			template<typename ...other_input> using in_t = typename Assistant::localizer_execute<start_index, rule, index_output, index_container<>, input..., other_input... >::type;
			template<typename ...other_input> using front_in = localizer<start_index, rule, index_output, other_input..., input...>;
			template<typename ...other_input> using front_in_t = typename Assistant::localizer_execute<start_index, rule, index_output, index_container<>, other_input..., input... >::type;
			template<typename ...input_other> using rule_in = localizer<start_index, instant<rule, input_other...>::template fornt_in_t, index_output, input...>;
			template<typename ...input_other> using rule_in_t = typename Assistant::localizer_execute<start_index, instant<rule, input_other...>::template fornt_in_t, index_output, index_container<>, input...>::type;
			template<typename ...input_other> using rule_front_in = localizer<start_index, instant<rule, input_other...>::template in_t, index_output, input...>;
			template<typename ...input_other> using rule_front_in_t = typename Assistant::localizer_execute<start_index, instant<rule, input_other...>::template in_t, index_output, index_container<>, input...>::type;
		};
		template<size_t start_index, template<class ...> class role, template<size_t...> class index_outpu, typename ...input> using localizer_t = typename Assistant::localizer_execute<start_index, role, index_outpu, index_container<>, input... >::type;


		/*----- index_separate -----*/
		namespace Assistant
		{
			template<template<typename...> class output, template <size_t...> class rule, typename result, typename ...input> struct index_separate_execute;
			template<template<typename...> class output, template <size_t...> class rule, typename ...result> struct index_separate_execute<output,rule,type_container<result...>>
			{
				using type = output<result...>;
			};
			template<template<typename...> class output, template <size_t...> class rule, typename ...result, template <size_t ...> class input_rule, size_t ...this_input, typename ...input> struct index_separate_execute<output, rule, type_container<result...>, input_rule<this_input...>, input...>
			{
				using type = typename index_separate_execute<output, rule, type_container<result..., input_rule<this_input>...>, input...>::type;
			};
		}
		template<template<typename...> class output, template <size_t...> class rule, typename ...input> struct index_separate
		{
			template <typename ...other_input> using in = index_separate<output, rule, input..., other_input... >;
			template <typename ...other_input> using in_t = typename Assistant::index_separate_execute<output, rule, type_container<>, input..., other_input... >::type;
			template <typename ...other_input> using front_in = index_separate<output, rule, type_container<>, other_input..., input... >;
			template <typename ...other_input> using front_in_t = typename Assistant::index_separate_execute<output, rule, type_container<>, other_input ..., input ... >::type;
		};
		template<template<typename...> class output, template <size_t...> class rule, typename ...input> using index_separate_t = typename Assistant::index_separate_execute<output, rule, type_container<>, input...>::type;

		/*----- index_merga -----*/
		namespace Assistant
		{
			template<template <size_t...> class rule, typename result, typename ...input> struct index_merga_execute;
			template<template <size_t...> class rule, size_t ...result> struct index_merga_execute<rule,index_container<result...>>
			{
				using type = rule<result...>;
			};
			template<template <size_t...> class rule, size_t ...result, template<size_t...> class this_rule, size_t ...input_index, typename ...input> struct index_merga_execute<rule, index_container<result...>, this_rule<input_index...>, input...>
			{
				using type = typename index_merga_execute<rule, index_container<result..., input_index...>, input...>::type;
			};
		}
		template<template <size_t...> class output, typename ...input> struct index_merga
		{
			template <typename ...other_input> using in = index_merga<output, input..., other_input... >;
			template <typename ...other_input> using in_t = typename Assistant::index_merga_execute<output, index_container<>, input..., other_input... >::type;
			template <typename ...other_input> using front_in = index_merga<output, other_input..., input... >;
			template <typename ...other_input> using front_in_t = typename Assistant::index_merga_execute<output, index_container<>, other_input ..., input ... >::type;
		};
		template<template<size_t...> class output, typename ...input> using index_merga_t = typename Assistant::index_merga_execute<output, index_container<>, input...>::type;

		/*----- picker -----*/
		namespace Assistant
		{
			template<size_t index, typename this_input, typename ...other_input> struct picker_execute
			{
				static_assert(sizeof...(other_input) >= index, "PO::picker the number of input type is less then require");
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

		/*----- replace -----*/
		template<typename out, typename input> struct replace
		{
			using type = out;
		};
		template<typename out, typename input> using replace_t = out;



		/*----- selector -----*/
		namespace Assistant
		{
			template<typename index, template<typename ...> class out, typename ...input> struct select_execute;
			template<size_t ...index, template<size_t...> class index_con, template<typename ...> class out, typename ...input> struct select_execute<index_con<index...>, out, input... >
			{
				using type = out<typename picker_execute<index, input...>::type... >;
			};
		}
		template<typename index, template<typename ...> class out, typename ...input> struct select
		{
			template<typename ...other_input> using in = select<index, out, input..., other_input...>;
			template<typename ...other_input> using in_t = typename Assistant::select_execute<index, out, input..., other_input...>::type;
			template<typename ...other_input> using front_in = select<index, out, other_input..., input...>;
			template<typename ...other_input> using front_in_t = typename Assistant::select_execute<index, out, other_input..., input...>::type;
		};
		template<typename index, template<typename ...> class out, typename ...input> using select_t = typename Assistant::select_execute<index, out, input...>::type;


		/*----- function_detect -----*/
		template<bool able_detect, typename ret, typename owner, typename ...parameter> struct funtion_type :std::integral_constant<bool, able_detect>
		{
			using return_type = ret;
			using owner_type = owner;
			template<template<typename... > class output> using parameter_out = output<parameter...>;
		};

		namespace Assistant
		{
			template<typename function> struct function_pointer_type;
			template<typename ret, typename ...parameter > struct function_pointer_type<ret(parameter...)> { using type = funtion_type<true, ret, void, parameter...>; };
			template<typename ret, typename ...parameter > struct function_pointer_type<ret(*)(parameter...)> { using type = funtion_type<true, ret, void, parameter...>; };
			template<typename ret, typename owner, typename ...parameter > struct function_pointer_type<ret(owner::*)(parameter...)> { using type = funtion_type<true, ret, owner, parameter...>; };
			template<typename ret, typename owner, typename ...parameter > struct function_pointer_type<ret(owner::*)(parameter...)volatile> { using type = funtion_type<true, ret, volatile owner, parameter...>; };
			template<typename ret, typename owner, typename ...parameter > struct function_pointer_type<ret(owner::*)(parameter...) const> { using type = funtion_type<true, ret, const owner, parameter...>; };
			template<typename ret, typename owner, typename ...parameter > struct function_pointer_type<ret(owner::*)(parameter...) volatile const> { using type = funtion_type<true, ret, const owner, parameter...>; };
			template<typename ret, typename owner, typename ...parameter > struct function_pointer_type<ret(owner::*)(parameter...) &> { using type = funtion_type<true, ret, owner&, parameter...>; };
			template<typename ret, typename owner, typename ...parameter > struct function_pointer_type<ret(owner::*)(parameter...) volatile &> { using type = funtion_type<true, ret, owner&, parameter...>; };
			template<typename ret, typename owner, typename ...parameter > struct function_pointer_type<ret(owner::*)(parameter...) const &> { using type = funtion_type<true, ret, const owner&, parameter...>; };
			template<typename ret, typename owner, typename ...parameter > struct function_pointer_type<ret(owner::*)(parameter...) volatile const &> { using type = funtion_type<true, ret, const owner&, parameter...>; };
			template<typename ret, typename owner, typename ...parameter > struct function_pointer_type<ret(owner::*)(parameter...) && > { using type = funtion_type<true, ret, owner&&, parameter...>; };
			template<typename ret, typename owner, typename ...parameter > struct function_pointer_type<ret(owner::*)(parameter...) volatile &&  > { using type = funtion_type<true, ret, owner&&, parameter...>; };
			template<typename ret, typename owner, typename ...parameter > struct function_pointer_type<ret(owner::*)(parameter...) const &&> { using type = funtion_type<true, ret, const owner&&, parameter...>; };
			template<typename ret, typename owner, typename ...parameter > struct function_pointer_type<ret(owner::*)(parameter...) volatile const &&> { using type = funtion_type<true, ret, const owner&&, parameter...>; };
			template<typename translate> struct function_object_translate;
			template<bool able_detect, typename ret, typename owner, typename ...parameter> struct function_object_translate<funtion_type<able_detect, ret, owner, parameter...>>
			{
				using type = funtion_type<able_detect, ret, void, parameter...>;
			};
		}
		
		template<typename function_type> class funtion_detect
		{
			using ft = std::remove_reference_t<function_type>;
			template<typename K> 
			static typename Assistant::function_object_translate<typename Assistant::function_pointer_type<decltype(&K::operator())>::type>::type 
				func(Assistant::function_pointer_type<decltype(&K::operator())>*);
			template<typename K> static typename Assistant::function_pointer_type<K>::type func(Assistant::function_pointer_type<K>*);
			template<typename K> static funtion_type<false, void, void> func(...);
		public:
			using type = decltype(func<ft>(nullptr));
		};
		template<typename function_type> using funtion_detect_t = typename funtion_detect<function_type>::type;

		/*----- apply_function_object -----*/
		namespace Assistant
		{
			template<typename T> T& get_ref(T* in) { return *in; }
			template<typename T> decltype(auto) get_ref(T&& in) { return std::forward<T>(in); }
		}

		template<typename fun_object, typename ...input> decltype(auto) apply_function_object(fun_object&& fo, input&&... in)
		{
			using fun_type = funtion_detect_t<fun_object>;
			static_assert(std::is_same<typename fun_type::owner_type, void>::value ||  std::is_object<fun_object>::value || sizeof...(in) >= 1, "PO::Tool::app_adapter_func need a ref of the owner for its member function");
			return Tool::statement_if< std::is_same<typename fun_type::owner_type, void >::value >
				(
					[](auto&& fobj, auto&& ...in) { return std::forward<decltype(fobj) && >(fobj)(std::forward<decltype(in) && >(in)...); },
					[](auto&& fobj, auto&& owner, auto&& ...input) {
				static_assert(
					std::is_convertible<std::remove_pointer_t<decltype(owner)&&>&, typename fun_type::owner_type& >::value, 
					"PO::Tool::app_adapter_func need a avalible ref for its member function"
					);
				return (Assistant::get_ref(owner).*std::forward<decltype(fobj) && >(fobj))(std::forward<decltype(input) && >(input)...);
			},
					std::forward<fun_object>(fo), std::forward<input>(in)...
				);
		}

	}
}