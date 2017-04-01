#pragma once
#include "tmp.h"
namespace PO
{
	namespace TmpCall
	{
		namespace Implement
		{
			template<typename T, typename ...in> struct is_callable_function_implement
			{
				template<typename K, typename ...i> static std::true_type func(typename K::template in<i...>*);
				template<typename K, typename ...i> static std::false_type func(...);
				using type = decltype(func<T, in...>(nullptr));
			};
		}

		template<typename T, typename ...in> struct is_callable_function : Implement::is_callable_function_implement<T, in...>::type {};

		namespace Implement
		{
			struct statement_illegal {};

			template<typename S, typename T, typename ...O> struct process_statement
			{
				using type = T;
				using stack = S;
			};

			template<typename S, typename T, typename P, typename ...OT> struct process_statement<S, T, P, OT...>
			{
				template<typename A, typename K, typename H> static auto func_2(
					std::pair<typename H::template in_statement<A, K>::stack, typename H::template in_statement<A, K>::type>*
					//void*
				)
					->std::pair<typename H::template in_statement<A, K>::stack, typename H::template in_statement<A, K>::type>;

				template<typename A, typename K, typename H> static auto func_2(...)->statement_illegal;

				template<typename A, typename K, typename H> static auto func(
					std::pair<A, typename K::template out<H::template in>::type>*
				)
					->std::pair<A, typename K::template out<H::template in>::type>;
				template<typename A, typename K, typename H> static auto func(...) -> decltype(func_2<A, K, H>(nullptr));

				using next_statement = decltype(func<S, T, P>(nullptr));

				static_assert(!std::is_same<next_statement, statement_illegal>::value, "call statement illegal");
				using type = typename process_statement<typename next_statement::first_type, typename next_statement::second_type, OT...>::type;
				using stack = typename process_statement<typename next_statement::first_type, typename next_statement::second_type, OT...>::stack;
			};

			template<typename T> struct call_implemenmt_pick_result
			{
				template<typename K> static auto func(typename K::type*) -> typename K::type;
				template<typename K> static auto func(...)->statement_illegal;
				using type = decltype(func<T>(nullptr));
				static_assert(!std::is_same<type, statement_illegal>::value, "unavailable output of call");
			};

			
		}

		template<typename ...T> using call = typename Implement::call_implemenmt_pick_result<typename Implement::process_statement<Tmp::static_map<>, T...>::type>::type;

		template<typename ... func> struct def_func
		{
			template<typename s, typename in> struct in_statement
			{
				using type = typename Implement::process_statement<Tmp::static_map<>, in, func...>::type;
				using stack = s;
			};
		};

		template<typename ...T> struct append
		{
			template<template<typename...> class o> struct out
			{
				using type = o<T...>;
			};
			template<typename ...K> struct in
			{
				template<template<typename...> class o> struct out
				{
					using type = o<K..., T...>;
				};
			};
		};

		template<typename ...T> struct sperate_call
		{
			template<typename ...input_t> struct in
			{
				template<template<typename ...> class o> struct out
				{
					using type = o<call<append<input_t>, T...>... >;
				};
			};
		};

		template<template<typename...> class f> struct make_func
		{
			template<typename ...i> struct in
			{
				using type = f<i...>;
				template<template<typename ...> class o> struct out
				{
					using type = o<f<i...>>;
				};
			};
		};

		

		template<typename ...T> struct front_append
		{
			template<template<typename...> class o> struct out
			{
				using type = o<T...>;
			};
			template<typename ...K> struct in
			{
				template<template<typename...> class o> struct out
				{
					using type = o<T..., K...>;
				};
			};
		};

		template<typename F> struct packet
		{
			static_assert(is_callable_function<F>::value, "packet only accept callable function");
			template<typename ...T> struct in
			{
				using type = typename F::template in<T...>;
				template<template<typename...> class o> struct out
				{
					using type = o<typename F::template in<T...>>;
				};
			};
		};

		struct unpacket
		{
			template<typename ...T> struct in
			{
				static_assert(sizeof...(T) < 0, "TmpCall unpacket only accept type like tank<T...>");
			};
			template<typename ...T, template<typename ...> class o> struct in<o<T...>>
			{
				template<template<typename ...> class ou> struct out
				{
					using type = ou<T...>;
				};
			};
		};

		struct self
		{
			template<typename ...T> struct in
			{
				static_assert(sizeof...(T) == 1, "self can not handle mulity output");
			};
			template<typename T> struct in<T>
			{
				using type = T;
				template<template<typename...> class o> struct out
				{
					using type = o<T>;
				};
			};
		};

		namespace Implement
		{
			template<template<typename ...> class output, typename result, typename s, typename e> struct make_range_implemenmt;
			template<template<typename ...> class output, typename ...input, typename s, typename e> 
			struct make_range_implemenmt<output, std::tuple<input...>, s, e>
			{
				using type = typename make_range_implemenmt<
					output, 
					std::tuple<input..., std::integral_constant<decltype(s::value), s::value>>,
					std::integral_constant<decltype(s::value), (s::value + (s::value > e::value ? -1 : 1))>, 
					e
				>::type;
			};
			template<template<typename ...> class output, typename ...input, typename e> 
			struct make_range_implemenmt<output, std::tuple<input...>, e, e>
			{
				using type = output<input...>;
			};
		}

		template<typename S, typename E> struct make_range
		{
			static_assert(Tmp::bool_and<Tmp::have_value<S>::value, Tmp::have_value<E>::value>::value, "make_range accept type have T::value");
			static_assert(std::is_same<decltype(S::value), decltype(E::value)>::value, "make_range need the same type of value");
			template<template<typename ...> class o> struct out
			{
				using type = typename Implement::make_range_implemenmt<o, std::tuple<>, S, E>::type;
			};
		};
		template<size_t s, size_t e> using make_range_size_t = make_range<std::integral_constant<size_t, s>, std::integral_constant<size_t, e>>;

		struct sperate_value
		{
			template<typename ...T> struct in
			{
				static_assert(sizeof...(T) < 0, "TmpCall unpacket only accept type like tank<T...>");
				template<template<typename ...> class o> struct out
				{
					using type = o<da<i>...>;
				};
			};

			template<typename ...T> struct in
			{
				static_assert(sizeof...(T) < 0, "TmpCall unpacket only accept type like tank<T...>");
				template<template<typename ...> class o> struct out
				{
					using type = o<da<i>...>;
				};
			};
		};

		struct bind_value_t
		{
			template<typename ...i> struct in
			{
				template<template<decltype(i::value)...> class o> struct output
				{
					using type = o<i::value...>;
				};
			};
		};

		namespace Implement
		{
			template<size_t s, template<typename...> class output, typename output_type, typename other_type> struct label_serial_t_implemenmt;
			template<size_t s, template<typename...> class output, typename ...output_type, typename this_type, typename ...other_type>
			struct label_serial_t_implemenmt<s, output, Tmp::set_t<output_type...>, Tmp::set_t<this_type, other_type...>>
			{
				using type = typename label_serial_t_implemenmt<s + 1, output, Tmp::set_t<output_type..., Tmp::label<this_type, Tmp::set_i<s>> >, Tmp::set_t<other_type...>>::type;
			};
			template<size_t s, template<typename...> class output, typename ...output_type>
			struct label_serial_t_implemenmt<s, output, Tmp::set_t<output_type...>, Tmp::set_t<>>
			{
				using type = output<output_type...>;
			};
		}

		template<size_t s = 0>
		struct label_serial_t
		{
			template<typename ...i> struct in
			{
				template<template<typename ...> class o> struct out
				{
					using type = typename Implement::label_serial_t_implemenmt<s, o, Tmp::set_t<>, Tmp::set_t<i...>>::type;
				};
			};
		};

		namespace Implement
		{
			template<size_t index, typename this_input, typename ...other_input> struct pick_single_t_implemenmt
			{
				static_assert(sizeof...(other_input) >= index, "picker the number of input type is less then require");
				using type = typename pick_single_t_implemenmt<index - 1, other_input...>::type;
			};

			template<typename this_input, typename ...other_input> struct pick_single_t_implemenmt<0, this_input, other_input...>
			{
				using type = this_input;
			};
		}

		template<size_t i>
		struct pick_single_t
		{
			template<typename ...input> struct in
			{
				static_assert(sizeof...(input) >= i, "picker the number of input type is less then require");
				template<template<typename ...> class o> struct out
				{
					using type = o< typename Implement::pick_single_t_implemenmt<i, input...>::type >;
				};
			};
		};


		namespace Implement
		{
			template<template<typename ... > class role, template<typename ...> class out, typename output, typename ...input> struct pick_if_t_implemenmt;
			template<template<typename ... > class role, template<typename ...> class out, typename ...output> struct pick_if_t_implemenmt<role, out, Tmp::set_t<output...>>
			{
				using type = out<output...>;
			};
			template<template<typename ... > class role, template<typename ...> class out, typename ...output, typename this_input, typename ...input> struct pick_if_t_implemenmt<role, out, Tmp::set_t<output...>, this_input, input...>
			{
				using type = typename pick_if_t_implemenmt<role, out, std::conditional_t< role<this_input>::value, Tmp::set_t<output..., this_input>, Tmp::set_t<output...>>, input...>::type;
			};
		}


		template<template<typename ... > class role>
		struct pick_if_t
		{
			template<typename ...input> struct in
			{
				template<template<typename...> class o> struct out
				{
					using type = typename Implement::pick_if_t_implemenmt<role, o, Tmp::set_t<>, input...>::type;
				};
			};
		};

		namespace Implement
		{
			template<template<typename ...> class out, typename output, typename ...input> struct pick_func_if_t_cut_implemenmt;
			template<template<typename ...> class out, typename ...output, typename this_input, typename ...input> struct pick_func_if_t_cut_implemenmt<out, Tmp::set_t<output...>, this_input, input...>
			{
				using type = typename pick_func_if_t_cut_implemenmt<out,
					std::conditional_t< this_input::first_type::value, Tmp::set_t<output..., typename this_input::second_type>, Tmp::set_t<output...>>,
					input...>::type;
			};

			template<template<typename ...> class out, typename ...output> struct pick_func_if_t_cut_implemenmt<out, Tmp::set_t<output...>>
			{
				using type = out<output...>;
			};

			template<typename S, typename ...func> struct pick_func_if_implement
			{
				template<typename input> struct in
				{
					using type = std::pair<typename process_statement<S, append_t<input>, func...>::type::type, input>;
				};
			};

			struct pick_func_if_t_filter_implemenmt
			{
				template<typename ...i> struct in
				{
					template<template<typename...> class o> struct out
					{
						using type = typename pick_func_if_t_cut_implemenmt<o, Tmp::set_t<>, i...>::type;
					};
				};
			};
		}

		template<typename ...func> struct pick_func_if_t
		{
			template<typename s, typename state> struct in_statement
			{
				using type = typename Implement::process_statement<s, state, sperate_call_t<typename Implement::pick_func_if_implement<s, func...>>, Implement::pick_func_if_t_filter_implemenmt >::type;
				using stack = s;
			};
		};

		template<typename index> struct push_stack
		{
			template<typename s, typename state> struct in_statement
			{
				using type = state;
				using stack = Tmp::set_static_map<s, index, state>;
			};
		};

		template<typename index> struct pop_stack
		{
			template<typename s, typename state> struct in_statement
			{
				using type = Tmp::find_static_map<s, index>;
				using stack = s;
			};
		};

		template<typename sta> struct replace_register
		{
			template<typename s, typename state> struct in_statement
			{
				using type = sta;
				using stack = s;
			};
		};

		namespace Implement
		{
			template<template<typename...> class role, template<typename...> class out, typename ...T> struct combine_t_implement
			{
				using type = out<>;
			};

			template<template<typename...> class role, template<typename...> class out, typename this_type, typename other_type, typename ...T>
			struct combine_t_implement<role, out, this_type, other_type, T...>
			{
				using type = typename combine_t_implement<role, out, role<this_type, other_type>, T...>::type;
			};

			template<template<typename...> class role, template<typename...> class out, typename this_type>
			struct combine_t_implement<role, out, this_type>
			{
				using type = out<this_type>;
			};
		}

		template<template<typename ...> class role>
		struct combine_t
		{
			template<typename ...i> struct in
			{
				template<template<typename...> class o> struct out
				{
					using type = typename Implement::combine_t_implement<role, o, i...>::type;
				};
			};
		};

		struct label_size_t
		{
			template<typename ...i> struct in
			{
				template<template<typename ...> class o> struct out
				{
					using type = o<Tmp::label<i, Tmp::set_i<sizeof(i)>>...>;
				};
			};
		};

		struct replace_by_label_t
		{
			template<typename ...i> struct in
			{
				template<template<typename ...> class o> struct out
				{
					using type = o<typename i::label_t...>;
				};
			};
		};

		namespace Implement
		{
			template<size_t s, template<typename...> class role, template<size_t ...> class o, typename out, typename ...i> struct localizer_implement;
			template<size_t s, template<typename...> class role, template<size_t ...> class o, size_t ...out, typename this_type, typename ...i>
			struct localizer_implement<s, role, o, Tmp::set_i<out...>, this_type, i...>
			{
				using type = typename localizer_implement<s + 1, role, o,
					std::conditional_t<role<this_type>::value, Tmp::set_i<out..., s>, Tmp::set_i<out...>>,
					i...
				>::type;
			};
			template<size_t s, template<typename...> class role, template<size_t ...> class o, size_t ...out>
			struct localizer_implement<s, role, o, Tmp::set_i<out...>>
			{
				using type = o<out...>;
			};
		}

		template<size_t s, template<typename...> class role> struct localizer_t
		{
			template<typename ...i> struct in
			{
				template<template<size_t ...> class o> struct out
				{
					using type = typename Implement::localizer_implement<s, role, o, Tmp::set_i<>, i...>::type;
				};
			};
		};
}