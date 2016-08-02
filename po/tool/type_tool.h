#pragma once
#include <utility>
namespace PO {
	namespace Tool {

		namespace Assistant
		{
			struct tool_void {};
		}

		/*----- type_container -----*/
		template<typename ...AT> struct type_container {};

		template<size_t ...i> struct index_container {};


		/*----- itself -----*/
		template<typename T> struct itself
		{
			using type = T;
		};
		template<typename T> using itself_t = T;

		/*----- dig -----*/
		template<size_t i,typename T = Assistant::tool_void> struct dig
		{
			using type = dig<i - 1, typename T::type>;
		};
		template<typename T> struct dig<0,T>
		{
			using type = T;
		};
		template<size_t i> struct dig<i, Assistant::tool_void>
		{
			template<typename T> using with = dig<i, T>;
			template<typename T> using with_t = typename with<T>::type;
		};
		template<> struct dig<0, Assistant::tool_void>
		{
			template<typename T> using with = dig<0, T>;
			template<typename T> using with_t = typename with<T>::type;
		};
		template<size_t i, typename T> using dig_t = typename dig<i, T>::type;


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


		/*----- packet_front -----*/
		namespace Assistant
		{

			template<typename last_input, template<typename ...> class rule, template<typename ...> class next_rule, template<typename...> class output, typename ...input>
			struct for_packet_execute_2
			{
				template<typename ...out> struct resault
				{
					using type = output<out...>;
				};
			};

			template<typename last_input, template<typename ...> class rule, template<typename ...> class next_rule, template<typename...> class output,typename this_type, typename ...input>
			struct for_packet_execute_2<last_input, rule,next_rule,output,this_type, input...>
			{
				template<typename ...out> struct resault
				{
					using type = typename for_packet_execute_2< next_rule<last_input, this_type>, rule, next_rule, output, input...>::template resault<out..., rule<last_input, this_type> >::type;
				};
			};

			template<template<typename...> class first_rule, template<typename ...> class first_next_rule, template<typename ...> class rule, template<typename ...> class next_rule, template<typename...> class output, typename ...input>
			struct for_packet_execute
			{
				using type = output<>;
			};

			template<template<typename...> class first_rule, template<typename ...> class first_next_rule, template<typename ...> class rule, template<typename ...> class next_rule, template<typename...> class output,typename this_type, typename ...input>
			struct for_packet_execute < first_rule, first_next_rule, rule, next_rule, output, this_type, input...>
			{
				using type = typename for_packet_execute_2<first_next_rule<this_type>, rule, next_rule, output, input...>::template resault<first_rule<this_type>>::type;
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
		/*
		namespace Assistant
		{
			template<template<typename ...>  class container, template<typename ...> class role, typename ...AT > struct filter_execute
			{
				template<typename ...AT2>
				struct resault
				{
					using type = container<AT2...>;
				};
			};

			template<template<typename ...>  class container, template<typename ...> class role, typename T, typename ...AT > struct filter_execute<container,role,T,AT...>
			{
				template<typename ...AT2>
				struct resault
				{
					using type = typename std::conditional_t<
						role<T>::value,
						instant<filter_execute<container, role, AT...>::template resault, AT2..., T>,
						instant<filter_execute<container, role, AT...>::template resault, AT2...>
					>::type::type;
				};
			};
		}
		template<template<typename ...> class role, template<typename ...> class container, typename ...AT> struct filter
		{
			template<typename ...AT2> using with = filter<role, container, AT..., AT2...>;
			template<typename ...AT2> using with_t = typename with<AT2...>::type;
			using type = typename Assistant::filter_execute<container, role, AT... >::template resault<>::type;
		};
		template<template<typename ...> class role, template<typename ...> class container, typename ...AT> using filter_t = typename filter<role, container, AT...>::type;
		*/

		/*----- reverse -----*/
		namespace Assistant
		{
			template<template<typename ...> class container, typename ...AT> struct reverse_execute
			{
				template<typename ...AT2> struct resault
				{
					using type = container<AT2...>;
				};
			};
			template<template<typename ...> class container, typename T, typename ...AT> struct reverse_execute<container, T, AT...>
			{
				template<typename ...AT2> struct resault
				{
					using type = typename reverse_execute<container, AT...>::template resault<T, AT2...>::type;
				};
			};
		}
		template<template<typename ...> class container, typename ...AT> struct reverse
		{
			template<typename ...AT2> using with = reverse<container, AT..., AT2...>;
			template<typename ...AT2> using with_t = typename with<AT2...>::type;
			using type = typename Assistant::reverse_execute<container, AT... >::template resault<>::type;
		};
		template<template<typename ...> class container, typename ...AT> using reverse_t = typename reverse< container, AT...>::type;

		
		/*----- localizer -----*/
		namespace Assistant
		{
			template<size_t cur, template<typename ...> class rule, template<size_t...> class index_output, typename ...AT> struct localizer_execute
			{
				template<size_t ...AT2> struct resault
				{
					using type = index_output<AT2...>;
				};
			};
			template<size_t cur, template<typename ...> class rule, template<size_t...> class index_output, typename T, typename ...AT> 
			struct localizer_execute<cur, rule, index_output, T, AT...>
			{
				template<size_t ...AT2> struct resault
				{
					using type = typename std::conditional_t<
						rule<T>::value,
						typename localizer_execute<cur + 1, rule, index_output, AT...>::template resault<AT2..., cur>,
						typename localizer_execute<cur + 1, rule, index_output, AT...>::template resault<AT2...>
					>::type;
				};
			};
		}

		template<size_t start_index, template<typename ...> class rule, template<size_t...> class index_output, typename ...input> struct localizer
		{
			template<typename ...other_input> using in = localizer<start_index, rule, index_output, input..., other_input...>;
			template<typename ...other_input> using in_t = typename Assistant::localizer_execute<start_index, rule, index_output, input..., other_input... >::template resault<>::type;
			template<typename ...other_input> using front_in = localizer<start_index, rule, index_output, other_input..., input...>;
			template<typename ...other_input> using front_in_t = typename Assistant::localizer_execute<start_index, rule, index_output, other_input..., input... >::template resault<>::type;
			template<typename ...input_other> using rule_in = localizer<start_index, instant<rule, input_other...>::template fornt_in_t, index_output, input...>;
			template<typename ...input_other> using rule_in_t = typename Assistant::localizer_execute<start_index, instant<rule, input_other...>::template fornt_in_t, index_output, input...>::template resault<>::type;
			template<typename ...input_other> using rule_front_in = localizer<start_index, instant<rule, input_other...>::template in_t, index_output, input...>;
			template<typename ...input_other> using rule_front_in_t = typename Assistant::localizer_execute<start_index, instant<rule, input_other...>::template in_t, index_output, input...>::template resault<>::type;
		};
		template<size_t start_index, template<class ...> class role, template<size_t...> class index_outpu, typename ...input> using localizer_t = typename Assistant::localizer_execute<start_index, role, index_outpu, input... >::template resault<>::type;


		/*
		template<size_t start_index, template<class ...> class rule, template<size_t...> class index_output, typename ...input>
		struct localizer
		{
			template<typename ...input_other> using in = localizer<start_index, rule, index_output, input..., input_other... >;
			template<typename ...input_other> using in_t = localizer<start_index, rule, index_output, input..., input_other... >;
		};

		template<size_t i,template<class ...> class role, typename ...AT> using localizer_t = typename Assistant::localizer_execute<i, role, AT... >::template resault<>::type;
		*/

		/*----- extract_index -----*/
		namespace Assistant
		{
			template<template<size_t ...index > class output, typename ...input> struct extract_index_execute
			{
				template<size_t ...all_index> struct resault
				{
					using type = output<all_index...>;
				};
			};
			template<template<size_t ...index > class output, template<size_t ...index > class input, size_t ...input_index ,typename ...other_input> struct extract_index_execute<output,input<input_index...>,other_input... >
			{
				template<size_t ...all_index> struct resault
				{
					using type = typename extract_index_execute<output, other_input...>::template resault<all_index..., input_index...>::type;
				};
			};
		}
		template<template<size_t ... > class output,typename ...input> struct extract_index
		{
			template<typename ...other_input> using in = extract_index<output, input..., other_input... >;
			template<typename ...other_input> using in_t = typename Assistant::extract_index_execute<output, input..., other_input... >::template resault<>::type;
			template<typename ...other_input> using front_in = extract_index<output, other_input..., input... >;
			template<typename ...other_input> using front_in_t = typename Assistant::extract_index_execute<output, other_input ..., input ... >::template resault<>::type;
		};
		template<template<size_t ... > class output, typename ...input> using extract_index_t = typename Assistant::extract_index_execute<output, input...>::template resault<>::type;
	}
}