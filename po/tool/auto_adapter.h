#pragma once
#include <functional>
#include "type_tool.h"
namespace PO
{
	namespace Tool
	{

		class adapter_unorder
		{
			template<size_t ...oi> struct find_index1 { static_assert(sizeof...(oi) != 0, "adapter_unorder meet empty index"); };
			template<size_t i> struct find_index1<i> { using type = index_container<i>; };
			template<size_t i, size_t o, size_t ...oi> struct find_index1<i, o, oi...> { using type = typename find_index1< (i<o) ? i : o, oi...>::type; };
			template<typename T, typename K> struct less :std::integral_constant<bool, T::value<K::value>{};
			template <typename v1, typename v2> struct find_index2
			{
				using type = index_separate_t <
					filter<
					index_separate_t<instant<is_not_one_of>::template in, index_container, v1>::template front_in_t,
					instant<index_merga<find_index1>::template in_t>::template in_t_t
					>::template in_t,
					index_container,
					v2
				>;
			};
			template<typename v1, typename v2> using find_index3 = index_merga_t<index_container, v1, typename find_index2<v1, v2>::type>;
		public:
			template<typename ...input> using adapter = for_packet_t<
				instant<index_merga<find_index1>::template in_t>::template in_t_t,
				instant<index_merga<find_index1>::template in_t>::template in_t_t,
				instant<find_index2>::template in_t_t,
				find_index3,
				index_merga<index_container>::template in_t,
				input...
			>;
		};

		class adapter_order
		{
			template<size_t ...oi> struct find_index1 { static_assert(sizeof...(oi) != 0, "adapter_order meet empty index"); };
			template<size_t i> struct find_index1<i> { using type = index_container<i>; };
			template<size_t i, size_t o, size_t ...oi> struct find_index1<i, o, oi...> { using type = typename find_index1< (i<o) ? i : o, oi...>::type; };

			template<size_t ...oi> struct find_index2 { static_assert(sizeof...(oi) >= 2, "adapter_order meet empty index"); };
			template<size_t i, size_t o> struct find_index2<i, o> { static_assert(o > i, "adapter_order unable adapter"); using type = index_container<o>; };
			template<size_t i, size_t o, size_t e, size_t ...ui> struct find_index2<i, o, e, ui...>
			{ 
				using type = typename find_index2<i, ((e > i) && (e < o) || (o < i) && (e > i)) ? e : o, ui... >::type;
			};
		public:
			template<typename ...input> using adapter = for_packet_t<
				instant<index_merga<find_index1>::template in_t>::template in_t_t,
				instant<index_merga<find_index1>::template in_t>::template in_t_t,
				instant<index_merga<find_index2>::template in_t>::template in_t_t,
				instant<index_merga<find_index2>::template in_t>::template in_t_t,
				index_merga<index_container>::template in_t,
				input...
			>;
		};


		/*----- auto_adapter -----*/
		namespace Assistant
		{

			template<size_t ... index, typename fun_obj, typename ...input>
			decltype(auto) auto_adapter_execute(Tool::index_container<index...>, fun_obj&& fo, input&& ...in)
			{
				return Tool::apply_function_object(std::forward<fun_obj>(fo),
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
				template<typename in> static instant<in::template match> fun(instant<in::template match>*);
				template<typename in> static instant<std::is_convertible> fun(...);
			public:
				using match = decltype(fun<T>(nullptr));
			};

			template<typename T> class did_adapter_type_have_adapter
			{
				template<typename in> static std::true_type fun(instant<in::template adapter>*);
				template<typename in> static std::false_type fun(...);
			public:
				static constexpr bool value = decltype(fun<T>(nullptr))::value;
			};

		}
		

		template<typename adapter_type, typename func_object, typename ...input> class adapter_analyze
		{

			static_assert(Assistant::did_adapter_type_have_adapter<adapter_type>::value, "adapter_type need to be have template class call \"adapter\"");

			using fun_type = funtion_detect_t<func_object>;

			template<typename target, typename ...in> using match = localizer_t<0, typename Assistant::get_adapter_type_match<adapter_type>::match::template in<target>::template in_t, index_container, in...>;

			static_assert(std::is_same<typename fun_type::owner_type, void>::value || sizeof...(input) >= 1, "PO::Tool::auto_adapter need a ref of the owner for its member function");

			using pick_index = make_index_range_t<Tool::index_container, (std::is_same<typename fun_type::owner_type, void>::value ? 0 : 1), sizeof...(input)>;

			using pre_index = typename fun_type::template parameter_out
				<
					packet<
						select_t<
							pick_index,
							instant<match>::template in,
							input...
						>::template front_in_t,
					adapter_type::template adapter
					>::template in_t
				>;

		public:

			using type = std::conditional_t<
				fun_type::value,
				typename  Assistant::auto_adapter_index_execute<
					std::is_same<typename fun_type::owner_type, void>::value,
					pre_index
				>::type,
				make_index_range_t<index_container, 0, sizeof...(input)>
			>;
		};
		template<typename adapter_type, typename func_object, typename ...input> using adapter_analyze_t = typename adapter_analyze<adapter_type, func_object, input...>::type;


		template<typename adapter_type, typename func_object,typename ...input>
		decltype(auto) auto_adapter(func_object&& fo, input&&... in)
		{
			return Assistant::auto_adapter_execute(adapter_analyze_t<adapter_type,func_object,input...>(),std::forward<func_object>(fo),std::forward<input>(in)...);
		}

	}
}
