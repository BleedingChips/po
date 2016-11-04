#pragma once
#include <functional>
#include <mutex>
#include <thread>
#include "type_tool.h"
#include <memory>
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
					[](auto&& u) { return dynamic_cast<T>(std::forward<decltype(u) && >(u)); },
					[](auto&& u) { return static_cast<T>(std::forward<decltype(u) && >(u)); },
					std::forward<P>(data)
					);
#else
			return static_cast<T>(std::forward<decltype(data) && >(data));
#endif // DEBUG
		}

		template<typename ...T> struct adapter
		{
			template<typename K>  adapter(K&& k) {}
		};

		template<typename P, typename ...T> struct adapter<P,T...> : adapter<T...>
		{
			P data;
			template<typename K>  adapter(K&& k) : adapter<T...>(std::forward<K>(k)), data(std::forward<K>(k)) {}
			template<typename D>
			decltype(auto) get()
			{
				static_assert(is_one_of<D, P, T...>::value, "what you get is not exist");
				return Tool::statement_if< std::is_same<D, P>::value >
					(
						[](auto& it) { return it.adapter<P,T...>::data;  },
						[](auto& it) { return it.adapter<T...>::template get<D>(); },
						*this
						);
			}
		};



		namespace Assistant
		{
			struct mail_receiption_base
			{
				virtual operator bool() const = 0;
				virtual ~mail_receiption_base() {}
			};
		}
		
		struct receiption
		{
			std::unique_ptr<Assistant::mail_receiption_base> ptr;
		public:
			operator bool() const { return static_cast<bool>(ptr) && *ptr; }
			void cancle() { ptr.reset(); }
			receiption() {}
			receiption(std::unique_ptr<Assistant::mail_receiption_base>&& p) :ptr(std::move(p)) {}
			receiption(const receiption&) = delete;
			receiption(receiption&&) = default;
			receiption& operator=(receiption&&) = default;
			receiption& operator=(std::unique_ptr<Assistant::mail_receiption_base>&& p)
			{
				ptr = std::move(p);
				return *this;
			}
		};

		template<typename T> struct mail
		{
			struct control {};
			std::weak_ptr<std::function<T>> func;
			std::shared_ptr<control> ref;
			struct receiption : Assistant::mail_receiption_base
			{
				std::shared_ptr<std::function<T>> func;
				std::weak_ptr<control> ref;
				operator bool() const { return static_cast<bool>(func) && !ref.expired(); }
				operator Tool::receiption() && { return Tool::receiption(std::make_unique<receiption>(*this)); }
				receiption(std::shared_ptr<std::function<T>>&& sp, const std::weak_ptr<control>& wp) :func(std::move(sp)), ref(wp) {}
				receiption(const receiption&) = default;
				receiption(receiption&&) = default;
				receiption& operator=(const receiption&) = default;
				receiption& operator=(receiption&&) = default;
			};
			operator bool() const { return !func.expired(); }
			template<typename ...AT>
			decltype(auto) operator()(AT&& ...at) { return (*(func.lock()))(std::forward<AT>(at)...); }
			template<typename K>
			decltype(auto) bind(K&& t)
			{
				if (!ref)
					ref = std::make_shared<control>();
				auto fun = std::make_shared<std::function<T>>(std::forward<K>(t));
				func = fun;
				return receiption{ std::move(fun), ref };
			}
		};

	}
}
