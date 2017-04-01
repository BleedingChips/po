#pragma once
#include <utility>
#include <type_traits>
#include <functional>
namespace PO
{
	namespace Tmp
	{
		template<typename T> T type_value();
		template<typename ... t> struct set_t 
		{
			static constexpr size_t size = sizeof...(t);
		};
		template<size_t ... i> struct set_i 
		{
			static constexpr size_t size = sizeof...(i);
		};
		template<size_t i> struct set_i<i> : std::integral_constant<size_t, i> 
		{
			static constexpr size_t size = 1;
		};
		template<typename t, typename l> struct label
		{
			using label_t = l;
			using original_t = t;
		};

		/* is_one_of */
		template<typename T, typename ...AT> struct is_one_of : std::false_type {};
		template<typename T, typename I, typename ...AT> struct is_one_of<T, I, AT...> : is_one_of<T, AT...> {};
		template<typename T, typename ...AT> struct is_one_of<T, T, AT...> : std::true_type {};

		template<typename T, typename ...AT> struct is_not_one_of : std::true_type {};
		template<typename T, typename I, typename ...AT> struct is_not_one_of<T, I, AT...> : is_not_one_of<T, AT...> {};
		template<typename T, typename ...AT> struct is_not_one_of<T, T, AT...> : std::false_type {};

		/* is_repeat */
		template<typename ...AT> struct is_repeat : public std::false_type {};
		template<typename T, typename ...AT>
		struct is_repeat <T, AT...> : std::conditional< is_one_of<T, AT...>::value, std::true_type, is_repeat < AT...>  >::type {};

		//to do std::in_place
		template<typename T>
		struct itself 
		{
			using type = T;
			decltype(auto) operator() () { return *static_cast<T*>(nullptr); }
			template<typename ...AT> struct in_t
			{
				using type = T;
			};
			itself() {}
			itself(const itself&) {}
		};

		namespace Implement
		{
			template<typename T> struct type_extract_implement;
			template<typename T> struct type_extract_implement<itself<T>> { using type = T; };
		}

		template<typename T> using type_extract_t = typename Implement::type_extract_implement<T>::type;

		template<typename T> using pure_type = std::remove_const_t<std::remove_reference_t<T>>;


		/* instant */
		namespace Implement
		{
			template<template<typename ...> class output, typename ...input> struct instant_implemenmt
			{
				using type = output<input...>;
			};
		}
		template<template<typename ...> class output, typename ...input> struct instant
		{
			template<typename ...other_input> using in = instant<output, input..., other_input...>;
			template<typename ...other_input> using in_t = typename Implement::instant_implemenmt<output, input..., other_input...>::type;
			template<typename ...other_input> using in_t_t = typename Implement::instant_implemenmt<output, input..., other_input...>::type::type;
			template<typename ...other_input> using front_in = instant<output, other_input..., input...>;
			template<typename ...other_input> using front_in_t = typename Implement::instant_implemenmt<output, other_input..., input...>::type;
			template<typename ...other_input> using front_in_t_t = typename Implement::instant_implemenmt<output, other_input..., input...>::type::type;
		};
		template<template<typename ...> class output, typename ...input> using instant_t = typename Implement::instant_implemenmt<output, input... >::type;
	
		namespace Implement
		{
			template<typename T, typename K> struct bigger_value_implement
			{
				using type = std::conditional_t < (T::value < K::value), K, T >;
			};
		}
		template<typename T, typename K> using bigger_value = typename Implement::bigger_value_implement<T, K>::type;

		template<template<typename ...> class out> struct extract_lable
		{
			template<typename ... i> struct with_implement
			{
				using type = out<typename i::label...>;
			};
			template<typename ...i> using with = typename with_implement<i...>::type;
		};

		template<template<typename ...> class out> struct extract_type
		{
			template<typename ... i> struct with_implement
			{
				using type = out<typename i::original_t...>;
			};
			template<typename ...i> using with = typename with_implement<i...>::type;
		};

		namespace Implement
		{
			template<typename ...AT>
			struct void_implement
			{
				using type = void;
			};
		}

		template<typename ...AT> using void_t = typename Implement::void_implement<AT...>::type;

		namespace Implement
		{
			struct static_map_illegal {};
			template<typename ...T> struct is_all_pair : std::true_type {};
			template<typename T, typename ...OT> struct is_all_pair<T, OT...> : std::false_type {};
			template<typename K, typename T, typename ...OT> struct is_all_pair<std::pair<K,T>, OT...> : is_all_pair<OT...> {};
		}

		template<typename ...T> struct static_map
		{
			static_assert(Implement::is_all_pair<T...>::value, "static_map only receipt std::pair as parameter");
		};
		template<typename T> struct is_static_map :std::false_type {};
		template<typename ...T> struct is_static_map<static_map<T...>> :std::true_type {};

		namespace Implement
		{
			template<typename T, typename index> struct find_static_map_implement;
			template<typename index> struct find_static_map_implement<static_map<>, index> { using type = static_map_illegal; };
			template<typename A, typename B, typename ...T, typename index> struct find_static_map_implement<static_map<std::pair<A,B>, T...>, index> { using type = typename find_static_map_implement<static_map<T...>, index>::type; };
			template<typename B, typename ...T, typename index> struct find_static_map_implement<static_map<std::pair<index, B>, T...>, index> { using type = B; };

			template<typename T, typename upper, typename index, typename value> struct set_static_map_implement;

			template<typename ...upper, typename index, typename value> 
			struct set_static_map_implement<static_map<>, static_map<upper...>, index, value> { using type = static_map<upper..., std::pair<index, value>>; };

			template<typename A, typename B, typename ...T, typename ...upper, typename index, typename value> 
			struct set_static_map_implement<static_map<std::pair<A, B>, T...>, static_map<upper...>, index, value>
			{
				using type = typename set_static_map_implement<static_map<T...>, static_map<upper..., std::pair<A,B>>, index, value>::type; 
			};
			
			template<typename B, typename ...T, typename ...upper, typename index, typename value>
			struct set_static_map_implement<static_map<std::pair<index, B>, T...>, static_map<upper...>, index, value> { using type = static_map<upper..., std::pair<index,value>, T...>; };
		}

		template<typename T, typename index> using find_static_map = typename Implement::find_static_map_implement<T, index>::type;
		template<typename T, typename index, typename value> using set_static_map = typename Implement::set_static_map_implement<T, static_map<>, index, value>::type;
		

		/*----- function_detect -----*/
		/*
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

		namespace Implement
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
			static typename Implement::function_object_reset_normale_function<typename Implement::function_pointer_type<decltype(&K::operator())>::type>::type
				func(Implement::function_pointer_type<decltype(&K::operator())>*);
			template<typename K> static typename Implement::function_pointer_type<K>::type func(Implement::function_pointer_type<K>*);
			template<typename K> static function_object_extract_type<void, void, void> func(...);
		public:
			using type = decltype(func<ft>(nullptr));
		};
		template<typename function_type> using funtion_obejct_extract_t = typename funtion_obejct_extract<function_type>::type;
		*/

		template<typename fun_type> struct pick_func;
		template<typename ret, typename ...para> struct pick_func<ret(para...)>
		{
			using return_t = ret;
			template<template<typename...> class o> using out = o<para...>;
			static constexpr size_t size = sizeof...(para);
		};

		template<typename fun_obj> struct degenerate_func;
		template<typename fun_ret, typename ...fun_para> struct degenerate_func<fun_ret(fun_para...)> { using type = fun_ret(fun_para...); };
		template<typename fun_ret, typename ...fun_para> struct degenerate_func<fun_ret(fun_para...) &> { using type = fun_ret(fun_para...); };
		template<typename fun_ret, typename ...fun_para> struct degenerate_func<fun_ret(fun_para...) &&> { using type = fun_ret(fun_para...); };
		template<typename fun_ret, typename ...fun_para> struct degenerate_func<fun_ret(fun_para...) const > { using type = fun_ret(fun_para...); };
		template<typename fun_ret, typename ...fun_para> struct degenerate_func<fun_ret(fun_para...) const& > { using type = fun_ret(fun_para...); };
		template<typename fun_ret, typename ...fun_para> struct degenerate_func<fun_ret(fun_para...) const&& > { using type = fun_ret(fun_para...); };
		template<typename fun_ret, typename ...fun_para> struct degenerate_func<fun_ret(fun_para...) volatile> { using type = fun_ret(fun_para...); };
		template<typename fun_ret, typename ...fun_para> struct degenerate_func<fun_ret(fun_para...) const volatile> { using type = fun_ret(fun_para...); };
		template<typename fun> using degenerate_func_t = typename degenerate_func<fun>::type;


		template<typename fun_obj> struct extract_func
		{
			using type = typename extract_func<decltype(&std::remove_reference_t<std::remove_const_t<fun_obj>>::operator())>::type;
		};
		template<typename fun_re, typename ...fun_para> struct extract_func<fun_re(fun_para...)>
		{
			using type = fun_re(fun_para...);
		};
		template<typename owner, typename func_type> struct extract_func<func_type owner::*>
		{
			using type = std::enable_if_t<std::is_function<func_type>::value, func_type>;
		};
		template<typename owner, typename func_type> struct extract_func<func_type owner::*&>
		{
			using type = std::enable_if_t<std::is_function<func_type>::value, func_type>;
		};
		template<typename owner, typename func_type> struct extract_func<func_type owner::*&&>
		{
			using type = std::enable_if_t<std::is_function<func_type>::value, func_type>;
		};
		template<typename owner, typename func_type> struct extract_func<func_type owner::* const &>
		{
			using type = std::enable_if_t<std::is_function<func_type>::value, func_type>;
		};
		template<typename fun> using extract_func_t = typename extract_func<fun>::type;

		template<typename fun> struct is_member_function_pointer : std::false_type {};
		template<typename fun, typename obj> struct is_member_function_pointer<fun obj::*> : std::true_type {};
		template<typename fun, typename obj> struct is_member_function_pointer<fun obj::*&> : std::true_type {};
		template<typename fun, typename obj> struct is_member_function_pointer<fun obj::*&&> : std::true_type {};
		template<typename fun, typename obj> struct is_member_function_pointer<fun obj::*const &> : std::true_type {};

		namespace Implement
		{
			template<typename function, typename ...paramer> class is_callable_execute
			{
				template<typename fun, typename ...pa> static std::true_type fun(decltype(std::invoke(std::declval<fun>(), std::declval<pa>()...))*);
				template<typename fun, typename ...pa> static std::false_type fun(...);
			public:
				using type = decltype(fun<function, paramer...>(nullptr));
			};
		}
		template<typename function, typename ...paramer> using is_callable = typename Implement::is_callable_execute<function, paramer...>::type;

		/*----- pick_parameter -----*/
		template<size_t i> struct pick_parameter
		{
			template<typename this_parameter, typename ...parameter>
			decltype(auto) operator()(this_parameter&& tp, parameter&& ... pa) const noexcept
			{
				static_assert(i <= sizeof...(pa), "pick_parameter overflow");
				return pick_parameter<i - 1>{}(std::forward<parameter>(pa)...);
			}
		};

		template<> struct pick_parameter<0>
		{
			template<typename this_parameter, typename ...parameter>
			decltype(auto) operator()(this_parameter&& tp, parameter&& ... pa) const noexcept { return std::forward<this_parameter>(tp); }
		};
	}

	
	namespace TmpCall
	{

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
			/*

			template<typename ...T> struct call_implemenmt
			{
				static_assert(sizeof...(T) >= 2, "call need at last an input and an output statement");
			};

			template<typename T, typename P, typename ...O> struct call_implemenmt<T, P, O...>
			{
				template<typename K, typename H> static auto func_3(typename H::template in<K>*) -> typename H::template in<K>;
				template<typename K, typename H> static auto func_3(typename H::template in<K>*) -> typename H::template in<K>;
				template<typename K, typename H> static auto func_2(typename H::template in<K>*) -> typename H::template in<K>;
				template<typename K, typename H> static auto func_2(...)-> decltype(func_3<K, H>(nullptr));
				template<typename K, typename H> static auto func(typename K::template out<H::template in>::type*) -> typename K::template out<H::template in>::type;
				template<typename K, typename H> static auto func(...) -> decltype(func_2<K, H>(nullptr));
				using next_statement = decltype(func<T, P>(nullptr));
				static_assert(!std::is_same<next_statement, statement_illegal>::value, "call statement input illegal");
				using type = typename process_statement<Tmp::static_map<>, next_statement, O...>::type;
			};
			*/
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

		template<template<typename...> class f> struct func_t
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

		template<template<typename...>class o> using bind_t = func_t<o>;

		template<template<size_t ...> class f> struct func_i
		{
			template<size_t ...i> struct in
			{
				using type = f<i...>;
				template<template<typename ...> class o> struct out
				{
					using type = o<f<i...>>;
				};
			};
		};

		template<template<size_t...>class o> using bind_i = func_i<o>;

		template<typename ...T> struct in_t
		{
			template<template<typename...> class o> struct out
			{
				using type = o<T...>;
			};
		};

		template<template<typename...> class output> struct out_t
		{
			template<typename ...T> struct in
			{
				using type = output<T...>;
			};
		};

		template<typename T, T ...n> struct append_i
		{
			template<T... i> struct in
			{
				template<template<T ...> class o> struct out
				{
					using type = o<i..., n...>;
				};
			};
		};

		template<typename ...O> struct append_t
		{
			template<typename ...T> struct in
			{
				template<template<typename...> class o> struct out
				{
					using type = o<T..., O...>;
				};
			};
			template<template<typename...> class o> struct out
				{
					using type = o<O...>;
				};
		};

		template<typename ...T> struct sperate_call_t
		{
			template<typename ...input> struct in
			{
				template<template<typename ...> class o> struct out
				{
					using type = o< call<in_t<input>, T...>... >;
				};
			};
		};

		struct sperate_t
		{
			template<typename T> struct in;
			template<typename ...T, template<typename ...> class o> struct in<o<T...>>
			{
				template<template<typename ...> class ou> struct out
				{
					using type = ou<T...>;
				};
			};
		};

		struct self_t
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
			template<template<typename ...> class output, typename input, size_t s, size_t e> struct make_range_set_t_implemenmt;
			template<template<typename ...> class output, typename ...input, size_t s, size_t e > struct make_range_set_t_implemenmt<output, Tmp::set_t<input...>, s, e>
			{
				using type = typename make_range_set_t_implemenmt<output, Tmp::set_t<input..., Tmp::set_i<s>>, (s + (s > e ? -1 : 1)), e>::type;
			};
			template<template<typename ...> class output, typename ...input, size_t s> struct make_range_set_t_implemenmt<output, Tmp::set_t<input...>, s, s>
			{
				using type = output<input...>;
			};
		}

		template<size_t s, size_t e> struct make_range_t
		{
			template<template<typename ...> class o> struct out
			{
				using type = typename Implement::make_range_set_t_implemenmt<o, Tmp::set_t<>, s, e>::type;
			};
		};

		namespace Implement
		{
			template<typename result, template<size_t...> class o, typename ...out>
			struct extract_value_t_implement;
			template<size_t ... i, template<size_t...> class o>
			struct extract_value_t_implement<Tmp::set_i<i...>, o>
			{
				using type = o<i...>;
			};
			template<size_t ...i, template<size_t...> class o, template<size_t ...> class this_tank, size_t ...ti, typename ...ot>
			struct extract_value_t_implement<Tmp::set_i<i...>, o, this_tank<ti...>, ot...>
			{
				using type = typename extract_value_t_implement<Tmp::set_i<i..., ti...>, o, ot...>::type;
			};
		}

		struct extract_value_t
		{
			template<typename ...T> struct in
			{
				template<template<size_t...> class o> struct out
				{
					using type = typename Implement::extract_value_t_implement<Tmp::set_i<>, o, T...>::type;
				};
			};
		};

		template<template<size_t ...> class da> struct sperate_value_i
		{
			template<size_t ...i> struct in
			{
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

}