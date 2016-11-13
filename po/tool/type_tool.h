#pragma once
#include <utility>
#include <functional>
namespace PO {
	namespace Tool {

		/*----- container -----*/
		template<typename ...AT> struct type_container 
		{ 
			using size = std::integral_constant<size_t, sizeof...(AT)>; 
			template<typename ...AT2> using call = type_container<AT..., AT2...>;
		};

		template<size_t ...i> struct index_container {};
		template<size_t i> struct index_container<i> : std::integral_constant<size_t, i> {};

		template<typename T, typename K> struct value_bigger
		{
			using type = std::conditional_t< ( T::value > K::value ), T, K > ;
		};
		template<typename T, typename K> using value_bigger_t = typename value_bigger<T, K>::type;

		template<size_t i, typename t> struct serial_type : std::integral_constant<size_t, i>
		{
			using type = t;
		};
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

		template<template<typename ...> class out, typename input> struct sperate_index_type;
		template<template<typename ...> class out, size_t ...input, template<size_t...> class tank> struct sperate_index_type<out, tank<input...>>
		{
			using type = out<index_container<input>...>;
		};
		template<template<typename ...> class out, typename input> using sperate_index_type_t =
			typename sperate_index_type<out, input>::type;

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

		/* link */
		namespace Assistant
		{
			template<template<typename ...> class output, typename early_input, typename ...input> struct link_execute;
			template<template<typename ...> class output, typename ...early_input> struct link_execute<output, type_container<early_input...>>
			{
				using type = output<early_input...>;
			};
			template<template<typename ...> class output, typename ...early_input, typename ...this_input, template<typename ...> class tank, typename ...input> 
			struct link_execute<output, type_container<early_input...>, tank<this_input...>, input...>
			{ 
				using type = typename link_execute<output, type_container<early_input..., this_input...>, input... >::type;
			};
		}
		template<template<typename ...> class output, typename ...input> using link_t = typename Assistant::link_execute<output, type_container<>, input...>::type;

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
			template<template<size_t...> class index_con, template<typename ...> class out, typename ...input> struct select_execute<index_con<>, out, input... >
			{
				using type = out<>;
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

		template<typename ret, typename own, typename ...parameter> struct function_object_extract_type
		{
			using return_t = ret;
			using owner_t = own;
			using is_able_extract = std::integral_constant<bool, true>;
			template<template<typename... > class output> using parameter_out = output<parameter...>;
		};

		template<> struct function_object_extract_type<void, void, void>
		{
			using return_t = void;
			using owner_t = void;
			using is_able_extract = std::integral_constant<bool, false>;
			template<template<typename... > class output> using parameter_out = output<>;
		};
		
		namespace Assistant
		{
			template<typename function> struct function_pointer_type;
			template<typename ret, typename ...parameter > struct function_pointer_type<ret(parameter...)> { using type = function_object_extract_type<ret, void, parameter...>; };
			template<typename ret, typename ...parameter > struct function_pointer_type<ret(*)(parameter...)> { using type = function_object_extract_type<ret, void, parameter...>; };
			template<typename ret, typename owner, typename ...parameter > struct function_pointer_type<ret(owner::*)(parameter...)> { using type = function_object_extract_type<ret, owner, parameter...>; };
			template<typename ret, typename owner, typename ...parameter > struct function_pointer_type<ret(owner::*)(parameter...)volatile> { using type = function_object_extract_type<ret, volatile owner, parameter...>; };
			template<typename ret, typename owner, typename ...parameter > struct function_pointer_type<ret(owner::*)(parameter...) const> { using type = function_object_extract_type<ret, const owner, parameter...>; };
			template<typename ret, typename owner, typename ...parameter > struct function_pointer_type<ret(owner::*)(parameter...) volatile const> { using type = function_object_extract_type<ret, const owner, parameter...>; };
			template<typename ret, typename owner, typename ...parameter > struct function_pointer_type<ret(owner::*)(parameter...) &> { using type = function_object_extract_type<ret, owner&, parameter...>; };
			template<typename ret, typename owner, typename ...parameter > struct function_pointer_type<ret(owner::*)(parameter...) volatile &> { using type = function_object_extract_type<ret, owner&, parameter...>; };
			template<typename ret, typename owner, typename ...parameter > struct function_pointer_type<ret(owner::*)(parameter...) const &> { using type = function_object_extract_type<ret, const owner&, parameter...>; };
			template<typename ret, typename owner, typename ...parameter > struct function_pointer_type<ret(owner::*)(parameter...) volatile const &> { using type = function_object_extract_type< ret, const owner&, parameter...>; };
			template<typename ret, typename owner, typename ...parameter > struct function_pointer_type<ret(owner::*)(parameter...) && > { using type = function_object_extract_type<ret, owner&&, parameter...>; };
			template<typename ret, typename owner, typename ...parameter > struct function_pointer_type<ret(owner::*)(parameter...) volatile &&  > { using type = function_object_extract_type<ret, owner&&, parameter...>; };
			template<typename ret, typename owner, typename ...parameter > struct function_pointer_type<ret(owner::*)(parameter...) const &&> { using type = function_object_extract_type<ret, const owner&&, parameter...>; };
			template<typename ret, typename owner, typename ...parameter > struct function_pointer_type<ret(owner::*)(parameter...) volatile const &&> { using type = function_object_extract_type<ret, const owner&&, parameter...>; };
			
			template<typename translate> struct function_object_reset_normale_function;
			template<typename ret, typename owner, typename ...parameter> struct function_object_reset_normale_function<function_object_extract_type<ret, owner, parameter...>>
			{
				using type = function_object_extract_type<ret, void, parameter...>;
			};
		}
		
		template<typename function_type> class funtion_obejct_extract
		{
			using ft = std::remove_reference_t<function_type>;
			template<typename K> 
			static typename Assistant::function_object_reset_normale_function<typename Assistant::function_pointer_type<decltype(&K::operator())>::type>::type
				func(Assistant::function_pointer_type<decltype(&K::operator())>*);
			template<typename K> static typename Assistant::function_pointer_type<K>::type func(Assistant::function_pointer_type<K>*);
			template<typename K> static function_object_extract_type<void, void, void> func(...);
		public:
			using type = decltype(func<ft>(nullptr));
		};
		template<typename function_type> using funtion_obejct_extract_t = typename funtion_obejct_extract<function_type>::type;


		namespace Assistant
		{
			template<typename function, typename ...paramer> class is_callable_execute
			{
				template<typename fun, typename ...pa> static std::true_type fun(decltype(std::invoke(std::declval<fun>(), std::declval<pa>()...))*);
				template<typename fun, typename ...pa> static std::false_type fun(...);
			public:
				using type = decltype(fun<function, paramer...>(nullptr));
			};

		}
		template<typename function, typename ...paramer> using is_callable = typename Assistant::is_callable_execute<function, paramer...>::type;

		/*  combine */
		namespace Assistant
		{
			template<template<typename ...> class func, typename ...input_other> struct compare_execute 
			{
				static_assert(sizeof...(input_other) > 0, "compare need 1 or more input");
			};
			template<template<typename ...> class func, typename input, typename ...input_other> struct compare_execute<func, input, input_other...> { using type = input; };
			template<template<typename ...> class func, typename input, typename other_input, typename ...input_other> struct compare_execute<func, input, other_input, input_other...> 
			{ 
				using type = typename compare_execute< func, func<input, other_input>, input_other... >::type;
			};
		}
		
		template<template<typename ...> class func, typename ...input> struct compare
		{
			template<typename ...o_input> using in = compare<func, input..., o_input...>;
			template<typename ...o_input> using in_t = typename Assistant::compare_execute<func, input..., o_input...>::type;
			template<typename ...o_input> using front_in = compare<func, o_input..., input...>;
			template<typename ...o_input> using front_in_t = typename Assistant::compare_execute<func, o_input..., input...>::type;
			template<typename ...o_input> using rule_in = compare< instant<func, o_input...>:: template front_in_t, input... >;
			template<typename ...o_input> using rule_in_t = typename Assistant::compare_execute<instant<func, o_input...>:: template front_in_t, input...>::type;
			template<typename ...o_input> using rule_front_in = compare< instant<func, o_input...>:: template in_t, input... >;
			template<typename ...o_input> using rule_front_in_t = typename Assistant::compare_execute<instant<func, o_input...>:: template in_t, input...>::type;
		};


		/* call */
		namespace Assistant
		{
			template<typename T> 
			struct is_func_have_template_call_inside
			{
				template<template<typename...> class func> struct str;
				template<typename P> static std::true_type func(str<P::template call>*);
				template<typename P> static std::false_type func(...);
				static constexpr bool value = decltype(func<T>(nullptr))::value;
			};


			struct call_execute_empty {};

			template<typename func_type, typename replace_type, typename replace_para, typename input_para> struct call_execute;
			template<typename func_type, typename replace_type, typename ...replace_para, typename ...input_para> 
			struct call_execute<func_type, replace_type, type_container<replace_para...>, type_container<input_para...>>
			{
				static_assert(is_func_have_template_call_inside<func_type>::value, "func_type of call need to have a template class name call");
				template<typename ...other_input> using in = 
					call_execute<func_type, replace_type, type_container<replace_para..., other_input...>, type_container<input_para...> >;
				template<typename ...other_input> using in_t =
					call_execute<typename replace_type::template replace<func_type, replace_para..., other_input... >::type, call_execute_empty, type_container<>, type_container<input_para...> >;
				template<typename ...other_input> using int_t_t =
					typename replace_type::template replace<func_type, replace_para..., other_input... >::type::template call<input_para...>::type;

				template<typename ...other_input> using front =
					call_execute<func_type, replace_type, type_container<other_input..., replace_para...>, type_container<input_para...> >;
				template<typename ...other_input> using front_t =
					call_execute<typename replace_type::template replace<func_type, other_input..., replace_para... >::type, call_execute_empty, type_container<>, type_container<input_para...> >;
				template<typename ...other_input> using front_t_t =
					typename replace_type::template replace<func_type, other_input..., replace_para...  >::type::template call<input_para...>::type;
				
				template<typename new_replace_type, typename ...new_replace_type_para> using replace =
					typename call_execute<typename replace_type::template replace<func_type, replace_para... >::type, new_replace_type, type_container<new_replace_type_para...>, type_container<input_para...>>;
			};

			template<typename func_type, typename ...replace_para, typename ...input_para>
			struct call_execute<func_type, call_execute_empty, type_container<replace_para...>, type_container<input_para...>>
			{
				template<typename ...other_input> using in =
					call_execute<func_type, call_execute_empty, type_container<replace_para...>, type_container<input_para..., other_input...> >;
				template<typename ...other_input> using in_t =
					typename func_type::template call<input_para..., other_input...>::type;

				template<typename ...other_input> using front =
					call_execute<func_type, call_execute_empty, type_container<replace_para...>, type_container<other_input..., input_para...> >;
				
				template<typename ...other_input> using front_t =
					typename func_type::template call<other_input ..., input_para ...>::type;

				template<typename new_replace_type, typename ...new_replace_type_para> using replace =
					call_execute<func_type, new_replace_type, type_container<new_replace_type_para...>, type_container<input_para...>>;
			};
		}

		template<typename func_type> struct call_func 
		{
			template<typename old_func_type> struct replace
			{
				using type = func_type;
			};
		};

		struct call_default
		{
			template<typename ...input> struct call
			{
				using type = type_container<input...>;
			};
		};

		template<typename func_type = call_default, typename ...input> using call = Assistant::call_execute<func_type, Assistant::call_execute_empty, type_container<>, type_container<input...> >;
		template<typename func_type = call_default, typename ...input> using call_t = typename func_type::template call<input...>;

		template<template<typename ...> class less_ope, template<typename ...> class output> struct sort
		{
			template<typename ...input> struct call
			{
				using type = output<input...>;
			};
		};

		/* sperate_input  */
		template<typename func_type> struct sperate_input
		{
			static_assert(Assistant::is_func_have_template_call_inside<func_type>::value, "func_type of sperate_input need to have a template class name call");
			template<typename ...input> struct call
			{
				using type = typename link_t<func_type::template call, input...>::type;
			};
		};
		

		/* index_replace */
		namespace Assistant
		{
			template<size_t i, typename T, template<typename ...> class output, typename front, typename middle, typename last> struct index_replace_execute;
			template<size_t i, typename T, template<typename ...> class output, typename ...front, typename middle, typename pre_last, typename ...last>
			struct index_replace_execute<i, T, output, type_container<front...>, middle, type_container<pre_last, last...>>
			{
				using type = typename index_replace_execute<i - 1, T, output, type_container<front..., middle>, pre_last, type_container<last...>>::type;
			};
			template<typename T, template<typename ...> class output, typename ...front, typename middle, typename ...last>
			struct index_replace_execute<0, T, output, type_container<front...>, middle, type_container<last...>>
			{
				using type = output<front..., T, last...>;
			};
			template<typename T, template<typename ...> class output, typename ...front, typename middle, typename pre_last, typename ...last>
			struct index_replace_execute<0, T, output, type_container<front...>, middle, type_container<pre_last, last...>>
			{
				using type = output<front..., T, pre_last, last...>;
			};
		}
		
		template<size_t i, typename T, template<typename ...> class output> struct index_replace
		{
			template<typename first, typename ...input> struct call
			{
				using type = typename Assistant::index_replace_execute<i, T, output, type_container<>, first, type_container<input...>>::type;
			};
		};
		
		/* index_swap */
		template<size_t i, size_t i2, template<typename ...> class output> struct index_swap
		{
			template<typename ...input> struct call
			{
				using pre = picker_t<i, input...>;
				using last = picker_t<i2, input...>;
				using type = typename index_replace<i2, pre, index_replace<i, last, output>::template call>::template call<input...>::type::type;
			};
		};

		/* static_mapping */
		namespace Assistant
		{
			template<template<typename...> class less_ope, template<typename ...> class equal_ope, template<typename...> class ope, typename default_op, size_t s, size_t o, typename ...input>
			struct static_mapping_execute
			{
				template<typename T, typename ...AP>
				decltype(auto) operator()(T&& t, AP&&... ap)
				{
					using pick_type = picker_t<(s + o) / 2, input...>;
					if (equal_ope<pick_type>{}(std::forward<T>(t)))
					{
						return ope<pick_type>{}(std::forward<AP>(ap)...);
					}
					else if (less_ope<pick_type>{}(std::forward<T>(t)))
					{
						return static_mapping_execute<less_ope, equal_ope, ope, default_op, s, (s + o) / 2, input...>{}(std::forward<T>(t), std::forward<AP>(ap)...);
					}
					else
					{
						return static_mapping_execute<less_ope, equal_ope, ope, default_op, (s + o) / 2 + 1, o, input...>{}(std::forward<T>(t), std::forward<AP>(ap)...);
					}
				}
			};

			template<template<typename...> class less_ope, template<typename ...> class equal_ope, template<typename...> class ope, typename default_op, size_t s, typename ...input>
			struct static_mapping_execute<less_ope, equal_ope, ope, default_op, s, s, input...>
			{
				template<typename T, typename ...AP>
				decltype(auto) operator()(T&& t, AP&&... ap)
				{
					return default_op{}(std::forward<AP>(ap)...);
				}
			};
		}
		template<template<typename...> class less_ope, template<typename ...> class equal_ope, template<typename...> class ope, typename default_op>
		struct static_mapping
		{
			template<typename ...input> struct call
			{
				using type = Assistant::static_mapping_execute<less_ope, equal_ope, ope, default_op, 0, sizeof...(input), input...>;
			};
		};
		
		template<template<typename...> class less_ope, template<typename ...> class equal_ope, template<typename...> class ope, typename default_op, typename ...input>
		using static_mapping_t = Assistant::static_mapping_execute<less_ope, equal_ope, ope, default_op, 0, sizeof...(input), input...>;

		namespace Assistant
		{
			template<size_t s, template<typename...> class output, typename output_type, typename other_type> struct add_serial_execute;
			template<size_t s, template<typename...> class output, typename ...output_type, typename this_type, typename ...other_type> 
			struct add_serial_execute<s, output, type_container<output_type...>, type_container<this_type, other_type...>>
			{
				using type = typename add_serial_execute<s + 1, output, type_container<output_type..., serial_type<s, this_type> >, type_container<other_type...>>::type;
			};
			template<size_t s, template<typename...> class output, typename ...output_type>
			struct add_serial_execute<s, output, type_container<output_type...>, type_container<>>
			{
				using type = output<output_type...>;
			};
		}

		template<size_t s, template<typename...> class output> struct add_serial
		{
			template<typename ...input> struct call
			{
				using type = typename Assistant::add_serial_execute<s, output, type_container<>, type_container<input...>>::type;
			};
		};
		template<size_t s, template<typename...> class output, typename ...input> using add_serial_t = typename Assistant::add_serial_execute<s, output, type_container<>, type_container<input...>>::type;
		
		template<typename T> struct less_serial { bool operator()(size_t i) { return i < T::value; } };
		template<typename T> struct equal_serial { bool operator()(size_t i) { return i == T::value; } };
	}
}