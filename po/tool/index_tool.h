#pragma once
#include <utility>
namespace PO
{
	namespace Tool
	{

		/*----- instant_index -----*/
		namespace Assistant
		{
			template<template<size_t ...> class output, size_t ...input> struct instant_index_execute
			{
				using type = output<input...>;
			};
		}

		template<template<size_t...> class output, size_t ...input> struct instant_index
		{
			template<size_t ...other_input> using in = instant_index<output, input..., other_input>;
			template<size_t ...other_input> using in_t = typename Assistant::instant_index_execute<output, input..., other_input...>::type;
			template<size_t ...other_input> using front_in = instant_index<output, other_input..., input... >;
			template<size_t ...other_input> using front_in_t = typename Assistant::instant_index_execute<output, other_input..., input...>::type;
		};


		namespace Assistant
		{
			template<size_t cur, template<size_t ...> class rule, template<size_t... > output, size_t ...input> struct filter_index_execute
			{
				template<size_t... res> struct resault
				{
					using type = output<input>;
				};
			};
			template<size_t cur, template<size_t ...> class rule, template<size_t... > output, size_t this_index, size_t ...input> struct filter_index_execute<cur,rule,output,this_index,input...>
			{
				template<size_t... res> struct resault
				{
					using type = typename std::conditional_t<
						rule<this_index>::value,
						filter_index_execute<rule, output, input...>::template resault<res..., cur>,
						filter_index_execute<rule, output, input...>::template resault<res...>,
					>::type;
				};
			};
		}


		template<size_t cur, template<size_t ...> class rule, template<size_t... > output, size_t ...input> struct filter_index
		{
			template<size_t ...other_input> using in = filter_index<cur, rule, output, input...,other_input...>;
			template<size_t ...other_input> using in_t = typename Assistant::filter_index_execute<cur, rule, output, input..., other_input...>::type;
			template<size_t ...other_input> using front_in = filter_index<cur, rule, output, other_input..., input...>;
			template<size_t ...other_input> using front_in_t = typename Assistant::filter_index_execute<cur, rule, output, other_input..., input...>::type;

			//template<size_t ...other_input> using rule_in = filter_index<cur,rule,output>
		};
	}
}
