#pragma once
#include <functional>
#include "tool.h"
namespace PO
{
	namespace Tool
	{




		template<typename adapter_type, typename func_object, typename ...input>
		decltype(auto) auto_adapter(func_object&& fo, input&&... in)
		{
			return statement_if<Tmp::is_callable<func_object, input...>::value>
				(
					[](auto&& fot, auto&& ...ini) { return std::invoke(std::forward<decltype(fot) && >(fot), std::forward<decltype(ini) && >(ini)...); },
					[](auto&& fot, auto&& ...ini)
			{
				using index = analyzer_t<adapter_type, decltype(fot) && , decltype(ini) && ...>;
				return Assistant::auto_adapter_execute(index(), std::forward<decltype(fot) && >(fot), std::forward<decltype(ini) && >(ini)...);
			},
					std::forward<func_object>(fo), std::forward<input>(in)...
				);
		}

		template<typename target, typename adapter_type, typename fun_obj, typename ...input> decltype(auto) auto_adapt_bind_function(fun_obj&& fo, input&&... in)
		{
			static_assert(!std::is_member_function_pointer<fun_obj>::value || sizeof...(input) >= 1, "PO::Mail::Assistant::mail_create_funtion_ptr_execute need a ref of the owner of the member function");

			return statement_if<!std::is_member_function_pointer<fun_obj>::value && sizeof...(input) == 0 && is_callable<fun_obj, input...>::value >
				(
					[](auto&& fun_obj)
			{
				return std::forward<decltype(fun_obj) && >(fun_obj);
			}).elseif_ < std::is_member_function_pointer<fun_obj>::value>(
				[](auto&& fun_objt, auto&& owner, auto&& ...inputt)
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
		}





		/*
		class unorder_adapt
		{
			template<size_t ...oi> struct find_index1 { static_assert(sizeof...(oi) != 0, "adapter_unorder meet empty index"); };
			template<size_t i> struct find_index1<i> { using type = Tool::index_container<i>; };
			template<size_t i, size_t o, size_t ...oi> struct find_index1<i, o, oi...> { using type = typename find_index1< (i<o) ? i : o, oi...>::type; };
			template<typename T, typename K> struct less :std::integral_constant<bool, T::value<K::value>{};
			template <typename v1, typename v2> struct find_index2
			{
				using type = Tool::index_separate_t <
					Tool::filter<
					Tool::index_separate_t<Tool::instant<Tool::is_not_one_of>::template in, Tool::index_container, v1>::template front_in_t,
					Tool::instant<Tool::index_merga<find_index1>::template in_t>::template in_t_t
					>::template in_t,
					Tool::index_container,
					v2
				>;
			};
			template<typename v1, typename v2> using find_index3 = Tool::index_merga_t<Tool::index_container, v1, typename find_index2<v1, v2>::type>;
		public:
			template<typename ...input> using adapter = Tool::for_packet_t<
				Tool::instant<Tool::index_merga<find_index1>::template in_t>::template in_t_t,
				Tool::instant<Tool::index_merga<find_index1>::template in_t>::template in_t_t,
				Tool::instant<find_index2>::template in_t_t,
				find_index3,
				Tool::index_merga<Tool::index_container>::template in_t,
				input...
			>;
		};

		class order_adapt
		{
			template<size_t ...oi> struct find_index1 { static_assert(sizeof...(oi) != 0, "adapter_order meet empty index"); };
			template<size_t i> struct find_index1<i> { using type = Tool::index_container<i>; };
			template<size_t i, size_t o, size_t ...oi> struct find_index1<i, o, oi...> { using type = typename find_index1< (i<o) ? i : o, oi...>::type; };

			template<size_t ...oi> struct find_index2 { static_assert(sizeof...(oi) >= 2, "adapter_order meet empty index"); };
			template<size_t i, size_t o> struct find_index2<i, o> { static_assert(o > i, "adapter_order unable adapter"); using type = Tool::index_container<o>; };
			template<size_t i, size_t o, size_t e, size_t ...ui> struct find_index2<i, o, e, ui...>
			{ 
				using type = typename find_index2<i, (((e > i) && (e < o)) || ((o < i) && (e > i))) ? e : o, ui... >::type;
			};
		public:
			template<typename ...input> using adapter = Tool::for_packet_t<
				Tool::instant<Tool::index_merga<find_index1>::template in_t>::template in_t_t,
				Tool::instant<Tool::index_merga<find_index1>::template in_t>::template in_t_t,
				Tool::instant<Tool::index_merga<find_index2>::template in_t>::template in_t_t,
				Tool::instant<Tool::index_merga<find_index2>::template in_t>::template in_t_t,
				Tool::index_merga<Tool::index_container>::template in_t,
				input...
			>;
		};


		namespace Assistant
		{

			template<size_t ... index, typename fun_obj, typename ...input>
			decltype(auto) auto_adapter_execute(Tool::index_container<index...>, fun_obj&& fo, input&& ...in)
			{
				static_assert(Tool::is_callable<fun_obj, Tool::picker_t<index, input...>...>::value, "PO::Adapter::Assistant::auto_adapter_execute can call function with those");
				return std::invoke(std::forward<fun_obj>(fo),
					Tool::pick_parameter<index>::in(std::forward<input>(in)...)...
				);
			}

			template<bool, typename index> struct auto_adapter_index_execute;
			template<size_t ...index> struct auto_adapter_index_execute<false,Tool::index_container<index...>>
			{
				using type = Tool::index_container<0, (index + 1)...>;
			};
			template<size_t ...index> struct auto_adapter_index_execute<true, Tool::index_container<index...>>
			{
				using type = Tool::index_container<index...>;
			};
			

			template<typename T> class get_adapter_type_match
			{
				template<typename in> static Tool::instant<in::template match> fun(Tool::instant<in::template match>*);
				template<typename in> static Tool::instant<std::is_convertible> fun(...);
			public:
				using match = decltype(fun<T>(nullptr));
			};

			template<typename T> class did_adapter_type_have_adapter
			{
				template<typename in> static std::true_type fun(Tool::instant<in::template adapter>*);
				template<typename in> static std::false_type fun(...);
			public:
				static constexpr bool value = decltype(fun<T>(nullptr))::value;
			};

			template<bool, typename match_, template<typename...> class adapter, typename func_obj, typename ...input> struct analyzer_execute_2
			{
				template<typename target, typename ...in> using match = Tool::localizer_t<0, match_::template in<target>::template in_t, Tool::index_container, in...>;
				using type = typename func_obj::template parameter_out
					<
						Tool::packet<
							Tool::instant<match, input...>::template front_in_t,
							adapter
						>::template in_t
					>;
			};

			template<typename match_, template<typename...> class adapter, typename func_obj, typename owner, typename ...input> struct analyzer_execute_2<true, match_, adapter, func_obj, owner, input...>
			{
				template<typename target, typename ...in> using match = Tool::localizer_t<1, match_::template in<target>::template in_t, Tool::index_container, in...>;
				using pre_index = typename func_obj::template parameter_out
					<
						Tool::packet<
							Tool::instant<match, input...>::template front_in_t,
							adapter
						>::template in_t
					>;
				using type = Tool::index_merga_t<Tool::index_container, Tool::index_container<0>, pre_index>;
			};



			template<bool, typename match_, template<typename...> class adapter, typename func_obj, typename ...input> struct analyzer_execute
			{
				using type = typename analyzer_execute_2 < std::is_member_function_pointer<std::remove_reference_t<func_obj>>::value, match_, adapter, Tool::funtion_obejct_extract_t<func_obj>, input... >::type;
			};

			template<typename match_, template<typename...> class adapter, typename func_obj, typename ...input> struct analyzer_execute<true, match_, adapter,func_obj, input...>
			{
				using type = Tool::make_index_range_t<Tool::index_container, 0, sizeof...(input)>;
			};

		}
		

		template<typename adapter_type, typename func_object, typename ...input> struct analyzer
		{
			static_assert(Assistant::did_adapter_type_have_adapter<adapter_type>::value, "PO::Adapter::analyzer need to have template class call \"adapter\"");
			static_assert(std::is_member_function_pointer<func_object>::value || sizeof...(input) >= 1, "PO::Adapter::analyzer need a ref of the owner for its member function");
			using type = typename Assistant::analyzer_execute<Tool::is_callable<func_object, input...>::value && !Assistant::did_adapter_type_have_adapter<adapter_type>::value, typename Assistant::get_adapter_type_match<adapter_type>::match, adapter_type::template adapter, func_object, input...>::type;
		};
		template<typename adapter_type, typename func_object, typename ...input> using analyzer_t = typename analyzer<adapter_type, func_object, input...>::type;


		template<typename adapter_type, typename func_object,typename ...input>
		decltype(auto) auto_adapter(func_object&& fo, input&&... in)
		{
			return Tool::statement_if<Tool::is_callable<func_object, input...>::value>
				(
					[](auto&& fot, auto&& ...ini) { return std::invoke(std::forward<decltype(fot) && >(fot), std::forward<decltype(ini) && >(ini)...); },
					[](auto&& fot, auto&& ...ini) 
			{ 
				using index = analyzer_t<adapter_type, decltype(fot)&&, decltype(ini)&&...>;
				return Assistant::auto_adapter_execute(index(), std::forward<decltype(fot) && >(fot), std::forward<decltype(ini) && >(ini)...);
			},
					std::forward<func_object>(fo), std::forward<input>(in)...
					);
		}

		template<typename target, typename adapter_type, typename fun_obj, typename ...input> decltype(auto) auto_adapt_bind_function(fun_obj&& fo, input&&... in)
		{
			static_assert(!std::is_member_function_pointer<fun_obj>::value || sizeof...(input) >= 1, "PO::Mail::Assistant::mail_create_funtion_ptr_execute need a ref of the owner of the member function");
			
			return Tool::statement_if<!std::is_member_function_pointer<fun_obj>::value && sizeof...(input) == 0 && Tool::is_callable<fun_obj, input...>::value >
				(
					[](auto&& fun_obj)
			{
				return std::forward<decltype(fun_obj) && >(fun_obj);
			}).elseif_ < std::is_member_function_pointer<fun_obj>::value>(
						[](auto&& fun_objt, auto&& owner, auto&& ...inputt)
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
		}
		*/
	}
}
