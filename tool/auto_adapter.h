#pragma once
#include <functional>
#include "tool.h"
namespace PO
{
	namespace Tool
	{
		
		class unorder_adapt
		{
			template<typename T, typename K> struct combine_implement;//TODO static_assert
			template<size_t ...s1, size_t s2, size_t... s2o> struct combine_implement<std::integer_sequence<size_t, s1...>, std::integer_sequence<size_t, s2, s2o...>>
			{
				using type = typename std::conditional_t<
					Tmp::is_one_of<
						std::integral_constant<size_t, s2>, 
						std::integral_constant<size_t, s1>...
					>::value,
					combine_implement<std::integer_sequence<size_t, s1...>, std::integer_sequence<size_t, s2o...>>,
					Tmp::itself<std::integer_sequence<size_t, s1..., s2>>
				>::type;
			};

			template<typename T, typename K> using combine_implement_t = typename combine_implement<T, K>::type;
		public:
			template<typename T, typename K> using match = std::is_convertible<T, K>;
			template<typename ...T> struct combine
			{
				using type = TmpCall::call<TmpCall::append<std::integer_sequence<size_t>, T...>, TmpCall::combine<TmpCall::make_func_t<combine_implement_t>>, TmpCall::self>;
			};
		};
		
		class order_adapt
		{
			template<typename out, typename in> struct reverser;
			template<size_t ...out, size_t t, size_t... in> struct reverser<std::integer_sequence<size_t, out...>, std::integer_sequence<size_t, t, in...>>
			{
				using type = typename reverser<std::integer_sequence<size_t, t, out...>, std::integer_sequence<size_t, in...>>::type;
			};
			template<size_t ...out> struct reverser<std::integer_sequence<size_t, out...>, std::integer_sequence<size_t>>
			{
				using type = std::integer_sequence<size_t, out...>;
			};

			template<typename T, typename K> struct combine_implement;//TODO static_assert
			template<size_t s2, size_t... s2o> struct combine_implement<std::integer_sequence<size_t>, std::integer_sequence<size_t, s2, s2o...>>
			{
				using type = std::integer_sequence<size_t, s2>;
			};
			template<size_t s1, size_t ...s1o, size_t s2, size_t... s2o> struct combine_implement<std::integer_sequence<size_t, s1, s1o...>, std::integer_sequence<size_t, s2, s2o...>>
			{
				using type = typename std::conditional_t <
					Tmp::is_one_of<std::integral_constant<size_t, s2>, std::integral_constant<size_t, s1>, std::integral_constant<size_t, s1o>...>::value || (s1 > s2),
					combine_implement<std::integer_sequence<size_t, s1, s1o...>, std::integer_sequence<size_t, s2o...>>,
					Tmp::itself<std::integer_sequence<size_t, s2, s1, s1o...>>
				>::type;
			};

			template<typename T, typename K> using combine_implement_t = typename combine_implement<T, K>::type;
		public:
			template<typename T, typename K> using match = std::is_convertible<T, K>;
			template<typename ...T> struct combine
			{
				using type = typename reverser<std::integer_sequence<size_t>, TmpCall::call<TmpCall::append<std::integer_sequence<size_t>, T...>, TmpCall::combine<TmpCall::make_func_t<combine_implement_t>>, TmpCall::self>>::type;
			};
		};

		namespace Implement
		{
			template<typename T> struct add_one;
			template<size_t ...i> struct add_one<std::integer_sequence<size_t, i...>>
			{
				using type = std::integer_sequence<size_t, (i + 1)...>;
			};

			template<template<typename ...> class role> struct analyze_match_implement
			{
				template<typename T, typename ...AT> struct in
				{
					using type = TmpCall::call<TmpCall::append<AT...>, TmpCall::localizer<TmpCall::make_func_front<role, T>>, TmpCall::self>;
				};
			};

			template<bool, typename adapt_type, typename func, typename ...par> struct analyze_implement
			{
				using func_para_append = typename Tmp::pick_func<Tmp::degenerate_func_t<Tmp::extract_func_t<func>>>::template out<TmpCall::append>;
				using type = typename TmpCall::call<func_para_append,
					TmpCall::sperate_call<
						TmpCall::append<par...>,
						analyze_match_implement<adapt_type::template match>
					>,
					TmpCall::make_func_t<adapt_type::template combine>
				>;
			};

			template<typename adapt_type, typename func, typename member_ptr, typename ...par> 
			struct analyze_implement<true, adapt_type, func, member_ptr, par...>
			{
				using func_para_append = typename Tmp::pick_func<Tmp::degenerate_func_t<Tmp::extract_func_t<func>>>::template out<TmpCall::append>;

				using type = typename TmpCall::call<
					func_para_append,
					TmpCall::sperate_call<
						TmpCall::append<par...>,
						analyze_match_implement<adapt_type::template match>
					>,
					TmpCall::make_func_t<adapt_type::template combine>//,
				>;
			};

			template<bool is_member_func, size_t ...i, typename fun, typename par> decltype(auto) auto_adapter_implement(std::integral_constant<bool, is_member_func>, std::integer_sequence<size_t, i...>, fun&& f, par pa)
			{
				return std::invoke(std::forward<fun>(f), std::get<is_member_func ? (i + 1) : (i)>(pa)...);
			}
		}		
		
		template<typename adapter_type, typename func_object, typename ...input>
		decltype(auto) auto_adapter(func_object&& fo, input&&... in)
		{
			return statement_if<Tmp::is_callable<func_object, input...>::value>
				(
					[](auto&& fot, auto&& ...ini) { return std::invoke(std::forward<decltype(fot) && >(fot), std::forward<decltype(ini) && >(ini)...); },
					[](auto&& fot, auto&& ...ini)
			{
				using index = typename Implement::analyze_implement<Tmp::is_member_function_pointer<func_object>::value, adapter_type, decltype(fot) && , decltype(ini) && ...>::type;
				return Implement::auto_adapter_implement(
					std::integral_constant<bool, Tmp::is_member_function_pointer<func_object>::value>{},
					index{}, 
					std::forward<decltype(fot) && >(fot), std::forward_as_tuple(std::forward<decltype(ini) && >(ini)...));
			},
					std::forward<func_object>(fo), std::forward<input>(in)...
				);
		}

		//TODO need to finish auto bind


		/*
		template<typename func_object, typename ...input>
		decltype(auto) auto_adapter_unorder(func_object&& fo, input&&... in)
		{
			return auto_adapter<unorder_adapt>(std::forward<func_object>(fo), std::forward<input>(in)...);
		}

		template<typename func_object, typename ...input>
		decltype(auto) auto_adapter_order(func_object&& fo, input&&... in)
		{
			return auto_adapter<order_adapt>(std::forward<func_object>(fo), std::forward<input>(in)...);
		}

		namespace Implement
		{

			//template<typename target, >


			template<typename ...AT> struct auto_bind_function_implement
			{
				template<typename adapter_type, typename func, typename ...AK> decltype(auto) bind(func&& f, AK&& ...ak) const noexcept
				{
					return [=](AT ...a) { return auto_adapter<adapter_type>(f, ak..., a...); };
				}
			};

			template<typename ...AT> struct auto_bind_member_function_implement
			{
				template<typename adapter_type, typename func, typename owner, typename ...AK> decltype(auto) bind(func&& f, owner&& o,AK&& ...ak) const noexcept
				{
					return [=, &o](AT ...a) { return auto_adapter<adapter_type>(f, o, ak..., a...); };
				}
			};
		}

		template<typename target, typename adapter_type, typename fun_obj, typename ...input> decltype(auto) auto_bind_function(fun_obj&& fo, input&&... in)
		{
			static_assert(!std::is_member_function_pointer<fun_obj>::value || sizeof...(input) >= 1, "PO::Mail::Assistant::mail_create_funtion_ptr_execute need a ref of the owner of the member function");

			return statement_if<std::is_member_function_pointer<fun_obj>::value>
				(
					[](auto&& fun_objt, auto&& owner, auto&& ...inputt) mutable
			{
				return typename Tmp::pick_func<Tmp::degenerate_func_t<Tmp::extract_func_t<target>>>::template out<Implement::auto_bind_member_function_implement>{}.template bind<adapter_type>
					(
						std::forward<decltype(fun_objt) && >(fun_objt),
						std::forward<decltype(owner) && >(owner),
						std::forward<decltype(inputt) && >(inputt)...
						);
			},
					[](auto&& fun_objt, auto&& ...inputt) mutable
			{
				return typename Tmp::pick_func<Tmp::degenerate_func_t<Tmp::extract_func_t<target>>>::template out<Implement::auto_bind_function_implement>{}.template bind<adapter_type>
					(
						std::forward<decltype(fun_objt) && >(fun_objt),
						std::forward<decltype(inputt) && >(inputt)...
						);
			},
				std::forward<fun_obj>(fo), std::forward<input>(in)...
				);
				*/





			/*

			return statement_if<!std::is_member_function_pointer<fun_obj>::value && sizeof...(input) == 0 && Tmp::is_callable<fun_obj, input...>::value >
				(
					[](auto&& fun_obj)
			{
				return std::forward<decltype(fun_obj) && >(fun_obj);
			}).elseif_ < std::is_member_function_pointer<fun_obj>::value>(
				[](auto&& fun_objt, auto&& owner, auto&& ...inputt) mutable
			{
				return[fun_objt, &owner, inputt...](auto&& ...intt) mutable { return auto_adapter<adapter_type>(fun_objt, owner, inputt..., std::forward<decltype(intt) && >(intt)...); };
			}
				).else_(
					[](auto&& fun_objt, auto&& ...inputt)
			{
				return[fun_objt, inputt...](auto&& ...intt) mutable { return auto_adapter<adapter_type>(fun_objt, inputt..., std::forward<decltype(intt) && >(intt)...); };
			}
				)(
					std::forward<fun_obj>(fo), std::forward<input>(in)...
					);
					*/
		//}
	}
}
