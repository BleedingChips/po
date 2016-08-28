#pragma once
#include <functional>
#include "type_tool.h"
namespace PO
{
	namespace Tool
	{
		/*----- destructor -----*/
		class destructor
		{
			std::function<void(void)> func;
		public:
			template<typename T>  destructor(T&& t) :func(std::forward<T>(t)) {}
			~destructor() { func(); }
		};


		/*----- statement_if -----*/
		namespace Assistant
		{
			template<bool> struct statement_if_struct
			{
				template<typename F, typename P, typename ...AT> static decltype(auto) run(F&& t, P&& p, AT&&... at) { return std::forward<F>(t)(std::forward<AT>(at)...); }
			};

			template<> struct statement_if_struct<false>
			{
				template<typename T, typename P, typename ...AT> static decltype(auto) run(T&& t, P&& p, AT&&... at) { return std::forward<P>(p)(std::forward<AT>(at)...); }
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
					return statement_if_execute<other_s, K&&>{std::forward<K>(k)};
				}
				template<typename K> decltype(auto) else_(K&& k) {
					return statement_if_execute<true, K&&>{std::forward<K>(k)};
				}
			};
		}

		template<bool s, typename T, typename P, typename ...AK> decltype(auto) statement_if(T&& t, P&& p, AK&& ...ak) { return Assistant::statement_if_struct<s>::run(std::forward<T>(t), std::forward<P>(p), std::forward<AK>(ak)...); }
		template<bool s, typename T> decltype(auto) statement_if(T&& t) { return Assistant::statement_if_execute<s, T&&>{std::forward<T>(t)}; }



		/*----- pick_parameter -----*/
		template<size_t i> struct pick_parameter
		{
			template<typename this_parameter, typename ...parameter>
			static decltype(auto) in(this_parameter&& tp, parameter&& ... pa)
			{
				static_assert(i <= sizeof...(pa), "pick_parameter overflow");
				return pick_parameter<i - 1>::in(std::forward<parameter>(pa)...);
			}
		};

		template<> struct pick_parameter<0>
		{
			template<typename this_parameter, typename ...parameter>
			static decltype(auto) in(this_parameter&& tp, parameter&& ... pa) { return std::forward<this_parameter>(tp); }
		};

		namespace Assistant
		{
			template<typename T, typename P> struct auto_cast_able_dynamic_cast
			{
				template<typename K, typename I> static std::true_type fun(decltype(dynamic_cast<K>(std::declval<I>()))*);
				template<typename K, typename I> static std::false_type fun(...);
				using type = decltype(fun<T, P>(nullptr));
			};
		}

		/*----- auto_cast -----*/
		template<typename T, typename P> T auto_dynamic_cast(P&& data)
		{
#ifdef _DEBUG
			return statement_if<Assistant::auto_cast_able_dynamic_cast<T, P>::type::value>
				(
					[](auto&& u) {  cout << "call this" << endl; return dynamic_cast<T>(std::forward<decltype(u) && >(u)); },
					[](auto&& u) {  cout << "call this2" << endl; return static_cast<T>(std::forward<decltype(u) && >(u)); },
					std::forward<P>(data)
			);
#else
			return static_cast<T>(std::forward<decltype(u) && >(u));
#endif // DEBUG


		}
	}
}

inline std::basic_string<char16_t> operator ""_str(const char16_t* str, size_t count)
{
	return std::basic_string<char16_t>(str);
}