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
				template<typename K>
				static std::true_type func(std::integral_constant<decltype(&K::operator()), &K::operator()>*);
				template<typename K>
				static std::false_type func(...);
			public :
				static constexpr bool value = decltype(func<T>(nullptr))::value;
			};

			template<typename T, typename P, typename ...AT> struct function_type 
			{
				static constexpr bool is_member = std::is_same<P, void>::value;
				template<template<typename ...> class out> using out_parameter = out<AT...>;
			};


			template<typename T> class function_object_parameter_detect_
			{
				static_assert(function_object_parameter_able_detect_<T>::value, "can not be detect_");
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
				using type = decltype(func(&T::operator()));
			};

			template<size_t ... index, typename fun_obj, typename ...in_para>
			decltype(auto) app_adapter_func(Tool::index_container<index...>, fun_obj&& fo, in_para&&... ip)
			{
				return fo(Tool::pick_parameter<index>::in(std::forward<ip>...)...);
			}

		}

		template<typename func_object,typename ...input>
		decltype(auto) auto_adapter_by_order(func_object&& fo, input&&... input)
		{
			return Tool::statement_if<!Assistant::function_object_parameter_able_detect_<func_object>::value>
				(
					[&input...](auto&& fun_object) {  return fun_object(std::forward<input>(input)...); },
					[&input...](auto&& fun_object) {

			},
					std::forward<func_object>(fo)
					);
		}
	

		/*
		template<typename T>
		class auto_adapter
		{
			std::function<T> func;
			template<typename P,size_t ...io,typename ...AP> void alloc(P&& p,std::integer_sequence<size_t, io...>,AP&& ...ap)
			{
				func = std::bind(std::forward<P>(p),std::forward<AP>(ap)..., Assistant::replace_placeholder<io>::return_placeholder()...);
			}
		public:
			template<typename K>
			auto_adapter(K&& k) 
			{
				//auto fun = std::forward<K>(k);


				PO::Tool::statement_if< Assistant::function_object_parameter_able_detect_<std::remove_extent_t<K>>::value >
					(
						[this](auto&& a) {
					alloc(
						std::forward<decltype(a) && >(a),
						typename Assistant::type_allocator <
						1,
						typename Assistant::pick_parameter<T>::type,
						typename Assistant::function_object_parameter_detect_ < std::remove_reference_t<std::remove_all_extents_t<decltype(a)>> >::type
						>::type()
						);
				},
						[this](auto&& a) {
					func = std::forward<decltype(a) && >(a);
				},
					k
					);
			}
			template<typename K,typename ...AK>
			auto_adapter(K(*fun)(AK...))
			{
				alloc(fun, Assistant::type_allocator < 1, typename Assistant::pick_parameter<T>::type, type_tuple<AK...> >::type());
			}
			template<typename K, typename P,typename ...AK, typename ...AC>
			auto_adapter(K(P::*fun)(AK...), P* p, AC&& ...ac)
			{
				alloc(fun, Assistant::type_allocator < 1, typename Assistant::pick_parameter<T>::type, type_tuple<AK...> >::type(),p);
			}
			auto_adapter(const auto_adapter&) = default;
			auto_adapter(auto_adapter&&) = default;
			auto_adapter& operator=(const auto_adapter&) = default;
			auto_adapter& operator=(auto_adapter&&) = default;
			template<typename ...AT> decltype(auto) operator()(AT&& ...at) { return func(std::forward<AT>(at)...); }
		};

		namespace Assistant
		{
			template<size_t i> struct adapter_pick_parameter_
			{
				template<typename T,typename ...AT>
				static decltype(auto) pick(T&& t,AT&& ...at)
				{
					static_assert(i <= sizeof...(AT),"parameter index overflow");
					return adapter_pick_parameter_<i-1>::pick(std::forward<AT>(at)...);
				}
			};
			template<> struct adapter_pick_parameter_<0>
			{
				template<typename T, typename ...AT>
				static decltype(auto) pick(T&& t, AT&& ...at)
				{
					return std::forward<T>(t);
				}
			};

			template<typename T,size_t ...i, typename ...AT> decltype(auto) adapter_by_order_execute( T&& t,std::index_sequence<i...>,AT&& ... at )
			{
				return t(adapter_pick_parameter_<i>::pick(std::forward<AT>(at)...)...);
			}
			template<typename T, typename ...AP, typename K, typename P,size_t ...i, typename ...AT> decltype(auto) adapter_by_order_execute(T(K::*t)(AP...),  std::index_sequence<i...>, P *p, AT&& ... at)
			{
				return (p->*t)(adapter_pick_parameter_<i>::pick(std::forward<AT>(at)...)...);
			}
		}

		inline int textc(int) { return 1; }

		template<typename T, typename ...AT> decltype(auto) adapt_by_order(T&& t, AT&& ...at)
		{
			return statement_if<
				Assistant::function_object_parameter_able_detect_<std::remove_reference_t<T>>::value
			>(
				[](auto&& fun,auto&& ...at)
			{
				using dector_type = typename Assistant::function_object_parameter_detect_< std::remove_reference_t<decltype(fun)&&>>::type;
				using index_type = typename Assistant::type_allocator<0, type_tuple<AT...>, dector_type>::type;
				return Assistant::adapter_by_order_execute
					(
						std::forward<decltype(fun)&&>(fun),
						index_type(),
						std::forward<decltype(at)&&>(at)...
						);
			},
				[](auto fun,auto&& ...at)
			{
				return fun(std::forward<decltype(at)&&>(at)...);
			},
				std::forward<T>(t),std::forward<AT>(at)...
				);
		}

		template<typename T,typename ...AP, typename ...AT> decltype(auto) adapt_by_order(T (t)(AP...), AT&& ...at)
		{
			using index_type = typename Assistant::type_allocator<0, type_tuple<AT...>, type_tuple<AP...>>::type;
			return Assistant::adapter_by_order_execute
					(
						t,
						index_type(),
						std::forward<decltype(at) && >(at)...
						);
		}
		template<typename T, typename K, typename P, typename ...AP, typename ...AT> decltype(auto) adapt_by_order(T(K::*t)(AP...), P *p, AT&& ...at)
		{
			using index_type = typename Assistant::type_allocator<0, type_tuple<AT...>, type_tuple<AP...>>::type;
			return Assistant::adapter_by_order_execute
				(
					t,
					index_type(),
					p,
					std::forward<decltype(at) && >(at)...
					);
		}*/
	}
}
