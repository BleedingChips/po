#include <list>
#include <functional>
#include <map>
#include <memory>
#include "auto_adapter.h"
namespace PO
{
	namespace Mail
	{
		namespace Assistant
		{
			template<typename target_type> struct mail_create_funtion_ptr_execute;
			template<typename ret, typename ...fun_para> struct mail_create_funtion_ptr_execute<ret(fun_para...)>
			{
				template<typename fun_obj, typename ...input> static decltype(auto) create(fun_obj&& fo, input&&... in)
				{
					static_assert(std::is_member_function_pointer<fun_obj>::value || sizeof...(input) >= 1, "PO::Mail::Assistant::mail_create_funtion_ptr_execute need a ref of the owner of the member function");
					return Tool::statement_if< Tool::is_callable<fun_obj, fun_para...>::value && (sizeof...(input) == 0)>
						(
							[](auto&& fun) {return std::make_shared<std::function<fun_ret(fun_para...)>>(std::forward<decltype(func) && >(func)); },
							[](auto&& fun, auto&& ...in)
					{
						return Adapter::auto_adapt_bind_function<ret(fun_para...), Adapter::unorder_adapt>(std::forward<decltype(fun)&&>(fun), std::forward<decltype(in)>(in)...);
					},
							std::forward<fun_obj>(fo), std::forward<input>(in)...
						);
				}
			};

			
			template<typename func_type> class mail_box;
			template<typename func_type> class receipt
			{
				std::shared_ptr<std::function<func_type>> func;
				friend class mail_box<func_type>;
			public:
				//operator std::weak_ptr<std::function<func_type>>() { return func; }
				void cancle() { func.reset(); }
				operator bool() const { return func; }
				//receipt(const std::shared_ptr<func_type>& ss) :func(ss) {}
				//receipt(std::shared_ptr<func_type>&& ss) :func(std::move(ss)) {}
				receipt() = default;
				receipt(receipt&&) = default;
				receipt(const receipt&) = default;
				template<typename func_obj, typename ...input> receipt(func_obj&& fo, input&&... in) :func(
					std::make_shared<std::function<func_type>>(
						Assistant::mail_create_funtion_ptr_execute<func_type>::create(std::forward<func_obj>(fo), std::forward<input>(in)...)
						)
				) {};
				receipt& operator= (const receipt&) = default;
				receipt& operator= (receipt&&) = default;
			};

			
			template<typename func_type> class mail_box
			{
				std::weak_ptr<std::function<func_type>> func;
			public:
				mail_box() = default;
				mail_box(const mail_box&) = default;
				mail_box(mail_box&&) = default;
				mail_box& operator= (mail_box&&) = default;
				mail_box& operator= (const mail_box&) = default;
				operator bool() const { return !func.expired(); }

				mail_box& operator= (const receipt<func_type>& r) { func = r.func; return *this; }

				mail_box(const receipt<func_type>& r) :func(r.func) {}

				template<typename ...parameter>
				decltype(auto) raw_call(parameter&& ...pa)
				{
					auto ip = func.lock();
					return (*ip)(std::forward<parameter>(pa)...);
				}

				template<typename ...parameter>
				bool operator() (parameter&&... fp)
				{
					auto ip = func.lock();
					if (ip)
					{
						(*ip)(std::forward<fp>(fp)...);
						return true;
					}
					return false;
				}
				
				template<typename T, typename ...parameter>
				bool capture(T&& f, parameter&&... fp)
				{
					auto ip = func.lock();
					if (ip)
					{
						Tool::statement_if < std::is_same<typename std::function<func_type>::result_type, void >::value  >
							(
								[](auto&& f, auto&& a, auto&& ...para){std::forward<decltype(a) && >(a)(std::forward<decltype(para)&&>(para)...); std::forward<decltype(f)>(f)(); },
								[](auto&& f, auto&& a, auto&& ...para){std::forward<decltype(f)>(f)(std::forward<decltype(a) && >(a)(std::forward<decltype(para) && >(para)...)); },
								std::forward<T>(t), *ip, std::forward<parameter>(fp)...
								);
						return true;
					}
					return false;
				}
				
			};

		}

		template<typename func_type> class single_mail
		{
			Assistant::mail_box<func_type> func;
		public:
			using receipt = Assistant::receipt<func_type>;
			template<typename func_obj, typename ...input> decltype(auto) bind(func_obj&& fo, input&& ...in)
			{
				receipt re(std::forward<func_obj>(fo),std::forward<input>(in)...);
				func = re;
				return re;
			}
			operator bool() const { return !func.expired(); }
			template<typename ...para>
			bool operator()(para&&... fp)
			{
				return func(std::forward<para>(fp)...);
			}
			template<typename T, typename ...parameter>
			bool capture(T&& f, parameter&&... fp)
			{
				return func.capture(std::forward<T>(t), std::forward<parameter>(fp)...);
			}
		};

		template<typename fun_type> class list_mail
		{
			std::list<Assistant::mail_box<fun_type>> all_mail;
			using store_type = std::list<Assistant::mail_box<fun_type>>;
		public:

			list_mail() = default;
			list_mail(const list_mail&) = default;
			list_mail(list_mail&&) = default;
			list_mail& operator=(const list_mail&) = default;
			list_mail& operator=(list_mail&&) = default;

			class receipt:private Assistant::receipt<fun_type>
			{
				typename store_type::iterator this_ite;
				store_type*  const target_list;
				friend class list_mail;
			public:
				template<typename fun_obj,typename ...para>
				receipt(typename store_type::iterator ei, store_type* tl, Assistant::receipt<fun_type>&& fc) : this_ite(ri), target_list(tl), Assistant::receipt<fun_type>(std::move(fc)) {}
				receipt(typename store_type::iterator ei, store_type* tl, const Assistant::receipt<fun_type>& fc) : this_ite(ri), target_list(tl), Assistant::receipt<fun_type>(fc) {}
				receipt() = default;
				receipt(const receipt&) = default;
				receipt(receipt&&) = default;
				void cancle() { Assistant::receipt<fun_type>::cancle(); end_ite = typename store_type::iterator(); target_list = nullptr; }
				operator bool() const { return Assistant::receipt<fun_type>::operator bool(); }
			};

			template<typename fun_ob, typename ...input>
			decltype(auto) bind(fun_ob&& func, input&& ...k)
			{
				Assistant::receipt<fun_type> tem(std::forward<fun_obj>(fo), std::forward<input>(k)...);
				all_mail.push_back(tem);
				auto this_ite = --all_mail.end();
				return receipt(this_ite, &all_mail, std::move(tem));
			}

			template<typename ...input>
			void operator() (input&&... ip)
			{
				for (auto ite = all_mail.begin(); ite != all_mail.end();)
				{
					if ((*ite)(std::forward<input>(ip)...))
						++ite;
					else
						st.erase(ite++);
				}
			}

			template<typename T, typename ...input>
			void capture(T&& t, input&&... fp)
			{
				for (auto ite = all_mail.begin(); ite != all_mail.end();)
				{
					if (*ite)
					{
						if (Tool::statement_if<std::is_same<typename std::function<fun_type>::result_type, void>::value>
							(
								[&ite, &fp...](auto&& fu) {(*ite).raw_call(std::forward<input>(fp)...); return fu(); },
								[&ite, &fp...](auto&& fu) {return fu((*ite).raw_call(std::forward<input>(fp)...)); },
								std::forward<T>(t)
								)
							)
							break;
						++ite;
					}
					else
						st.erase(ite++);
				}
			}

			bool set_front(const receipt& re)
			{
				if (static_cast<bool>(re) && ( re.target_list == &all_mail ))
				{
					all_mail.splice(all_mail.begin(), std::move(all_mail), re.this_ite);
					return true;
				}
				return false;
			}

			bool set_back(const receipt& re)
			{
				if (static_cast<bool>(re) && (re.target_list == &all_mail))
				{
					all_mail.splice(all_mail.end(), std::move(all_mail), re.this_ite);
					return true;
				}
				return false;
			}

			bool set_front_of(const receipt& front_of_this, const receipt& target)
			{
				if (static_cast<bool>(front_of_this) && static_cast<bool>(target) && (front_of_this.target_list == &all_mail) && (target.target_list == &all_mail))
				{
					all_mail.splice(front_of_this.this_ite, std::move(all_mail), target.this_ite);
					return true;
				}
				return false;
			}

			bool erase(receipt& re)
			{
				if (static_cast<bool>(re) && (re.target_list == &all_mail))
				{
					all_mail.erase(re.this_ite);
					re.target_list = nullptr;
					re.this_ite = typename store_type::iterator();
					return true;
				}
				return false;
			}

		};
	}
}