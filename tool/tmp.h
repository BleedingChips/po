#pragma once
#include <utility>
#include <type_traits>
#include <functional>
#include <tuple>
namespace PO
{
	namespace Tmp
	{

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

		template<bool its, bool ...oth> struct bool_and : bool_and<oth...> {};
		template<bool ...oth> struct bool_and<false, oth...> : std::false_type {};
		template<bool its> struct bool_and<its> : std::integral_constant<bool, its> {};

		template<bool its, bool ...oth> struct bool_or : bool_or<oth...> {};
		template<bool ...oth> struct bool_or<true, oth...> : std::true_type {};
		template<bool its> struct bool_or<its> : std::integral_constant<bool, its> {};

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

		namespace Implement
		{
			template<typename T> struct have_value_implement
			{
				template<typename P> static std::true_type func(std::integral_constant<decltype(P::value), P::value>*);
				template<typename P> static std::false_type func(...);
				using type = decltype(func<T>(nullptr));
			};
		}

		template<typename T> struct have_value : Implement::have_value_implement<T>::type {};
		

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

		template<typename T> struct is_integral_constant : std::false_type {};
		template<typename T, T v> struct is_integral_constant<std::integral_constant<T, v>> : std::true_type {};

		template<typename T> struct is_integer_sequence : std::false_type {};
		template<typename T, T ...v> struct is_integer_sequence<std::integer_sequence<T, v...>> : std::true_type {};

	}
}