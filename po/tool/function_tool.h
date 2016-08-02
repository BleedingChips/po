#pragma once
#include <functional>
#include "type_tool.h"
namespace PO
{
	namespace Tool
	{
		template<size_t i> struct replace_placeholder;

#define REPLACE_PLACEHOLDER(i) template<> struct replace_placeholder<i> { decltype(auto) operator() () { return std::placeholders::_##i; } };
		REPLACE_PLACEHOLDER(1)
			REPLACE_PLACEHOLDER(2)
			REPLACE_PLACEHOLDER(3)
			REPLACE_PLACEHOLDER(4)
			REPLACE_PLACEHOLDER(5)
			REPLACE_PLACEHOLDER(6)
			REPLACE_PLACEHOLDER(7)
			REPLACE_PLACEHOLDER(8)
			REPLACE_PLACEHOLDER(9)
			REPLACE_PLACEHOLDER(10)
			REPLACE_PLACEHOLDER(11)
			REPLACE_PLACEHOLDER(12)
			REPLACE_PLACEHOLDER(13)
			REPLACE_PLACEHOLDER(14)
			REPLACE_PLACEHOLDER(15)
			REPLACE_PLACEHOLDER(16)
			REPLACE_PLACEHOLDER(17)
			REPLACE_PLACEHOLDER(18)
			REPLACE_PLACEHOLDER(19)
			REPLACE_PLACEHOLDER(20)
#undef REPLACE_PLACEHOLDER

		template<size_t i, typename T1, typename T2, size_t ...oi> struct parameter_locator;
		template<size_t i,typename FT1,typename ...OT1,typename FT2,typename ...OT2,size_t ...oi>
		struct parameter_locator<i, type_container<FT1, OT1...>, type_container<FT2, OT2...>, oi...>
		{
			static_assert(i>=1,"index of parameter should >= 1");
			static_assert(sizeof...(OT1) >= sizeof...(OT2), "unsuitable parameter size");
			using resoult = typename parameter_locator<i + 1, type_container<OT1...>, type_container<FT2, OT2...>, oi...>::resoult;
		};
		template<size_t i, typename FT1, typename ...OT1, typename ...OT2, size_t ...oi>
		struct parameter_locator<i, type_container<FT1, OT1...>, type_container<FT1, OT2...>, oi...>
		{
			static_assert(i >= 1, "index of parameter should >= 1");
			static_assert(sizeof...(OT1) >= sizeof...(OT2), "unsuitable parameter size");
			using resoult = typename parameter_locator<i + 1, type_container<OT1...>, type_container< OT2...>, oi..., i>::resoult;
		};

		template<size_t i, typename ...OT1, size_t ...oi>
		struct parameter_locator<i, type_container<OT1...>, type_container<>, oi...>
		{
			static_assert(i >= 1, "index of parameter should >= 1");
			using resoult = index_array_<oi...>;
		};

		template<typename RT,typename ...PA,size_t ...i> 
		decltype(auto) adjector( RT(*func)(PA...),index_array_<i...>)
		{
			return std::bind(func, replace_placeholder<i>()()...);
		}
		template<typename RT, typename ...PA, typename T, size_t ...i>
		decltype(auto) adjector(RT(T::*func)(PA...),T* t, index_array_<i...>)
		{
			return std::bind(func, t, replace_placeholder<i>()()...);
		}

		template<typename T> struct parameter_adjector;
		template<typename RT, typename ...PA> struct parameter_adjector<RT(PA...)>
		{
			template<typename RT2, typename ...PA2> 
			static decltype(auto) adject(RT2(*pa)(PA2...))
			{
				return adjector(pa, typename parameter_locator<1,deque_<PA...>,deque_<PA2...>>::resoult());
			}
			template<typename RT2, typename ...PA2,typename T>
			static decltype(auto) adject(RT2(T::*pa)(PA2...),T* t)
			{
				return adjector(pa,t, typename parameter_locator<1, deque_<PA...>, deque_<PA2...>>::resoult());
			}
		};
	}
}
