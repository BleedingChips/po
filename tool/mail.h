#pragma once
#include <list>
#include <functional>
#include <map>
#include <memory>
#include "thread_tool.h"
namespace PO
{
	namespace Mail
	{
		/*
		namespace Assistant
		{
			struct receiption_base
			{
				virtual operator bool() const = 0;
				virtual ~receiption_base() {}
			};
		}

		template<typename T> struct mail
		{
			struct control {};
			std::weak_ptr<std::function<T>> func;
			std::shared_ptr<control> ref;
			struct receiption: Assistant::receiption_base
			{
				std::shared_ptr<std::function<T>> func;
				std::weak_ptr<control> ref;
				operator bool() const { return static_cast<bool>(func) && !ref.expired(); }
			};
			operator bool() const { return !func.expired(); }
			template<typename ...AT>
			decltype(auto) operator()(AT&& ...at) { return func(std::forward<AT>(at)...); }
			template<typename K>
			decltype(auto) bind(K&& t)
			{
				if (!ref)
					ref = std::make_shared<control>();
				return receiption{std::make_shared<std::function<T>>(std::function<T>(std::forward<K>(t))),ref};
			}
		};
		*/
		/*
		template<typename T> struct mail_ts_base
		{
			Tool::completeness_ptr<std::function<T>> func;
		};

		template<typename T> struct mail_ts : Tool::completeness<mail_ts_base<T>>
		{
			struct receiption_ts : Assistant::receiption_base
			{
				Tool::completeness_ref mail_ref;
				Tool::completeness<function<T>> func;
			};
			template<typename K>
			decltype(auto) bind(K&& t)
			{
				if (!ref)
					ref = std::make_shared<control>();
				return receiption{ std::make_shared<std::function<T>>(std::function<T>(std::forward<K>(t))),ref };
			}
		}*/

		namespace Assistant
		{
			template<typename target_type> struct mail_create_funtion_ptr_execute;
			template<typename ret, typename ...fun_para> struct mail_create_funtion_ptr_execute<ret(fun_para...)>
			{
				template<typename fun_obj, typename ...input> static decltype(auto) create(fun_obj&& fo, input&&... in)
				{
					static_assert(!std::is_member_function_pointer<fun_obj>::value || sizeof...(input) >= 1, "PO::Mail::Assistant::mail_create_funtion_ptr_execute need a ref of the owner of the member function");
					return Tool::statement_if< Tool::is_callable<fun_obj, fun_para...>::value && (sizeof...(input) == 0)>
						(
							[](auto&& fun) 
					{
						return std::forward<decltype(fun) && >(fun); 
					},
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
								[](auto&& f, auto&& a, auto&& ...para){std::forward<decltype(f) &&>(f)(std::forward<decltype(a) && >(a)(std::forward<decltype(para) && >(para)...)); },
								std::forward<T>(f), *ip, std::forward<parameter>(fp)...
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
				return func.capture(std::forward<T>(f), std::forward<parameter>(fp)...);
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
				receipt(typename store_type::iterator ei, store_type* tl, Assistant::receipt<fun_type>&& fc) : this_ite(ei), target_list(tl), Assistant::receipt<fun_type>(std::move(fc)) {}
				receipt(typename store_type::iterator ei, store_type* tl, const Assistant::receipt<fun_type>& fc) : this_ite(ei), target_list(tl), Assistant::receipt<fun_type>(fc) {}
				receipt() = default;
				receipt(const receipt&) = default;
				receipt(receipt&&) = default;
				void cancle() { Assistant::receipt<fun_type>::cancle(); this_ite = typename store_type::iterator(); target_list = nullptr; }
				operator bool() const { return Assistant::receipt<fun_type>::operator bool(); }
			};

			template<typename fun_ob, typename ...input>
			decltype(auto) bind(fun_ob&& func, input&& ...k)
			{
				Assistant::receipt<fun_type> tem(std::forward<fun_ob>(func), std::forward<input>(k)...);
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
						all_mail.erase(ite++);
				}
			}

			template<typename T, typename ...input>
			void capture(T&& t, input&&... fp)
			{
				for (auto ite = all_mail.begin(); ite != all_mail.end();)
				{
					bool break_ = false;
					if (*ite && 
						Tool::statement_if<std::is_same<typename std::function<fun_type>::result_type, void>::value>
							(
								[&break_, &ite, &fp...](auto&& fu) {return (*ite).capture([&fu, &break_]() {break_ = fu(); }, std::forward<input>(fp)...); },
								[&break_, &ite, &fp...](auto&& fu) {return (*ite).capture([&fu, &break_](auto&& in) {break_ = fu(std::forward<decltype(in) && >(in)); }, std::forward<input>(fp)...); },
								std::forward<T>(t)
								)
							)
					{
						if(break_)
							break;
						++ite;
					}
					else
						all_mail.erase(ite++);
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
		
		namespace Assistant
		{
			template<typename func_type> class mail_box_completeness_request;
			template<typename func_type > class receipt_completeness_request : protected receipt<func_type>
			{
				Tool::completeness_ref cpf;
				friend class mail_box_completeness_request<func_type>;
			public:
				operator bool() const { return static_cast<bool>(cpf) && receipt<func_type>::operator bool(); }
				void cancle() { cpf.drop(); receipt<func_type>::cancle(); }
				receipt_completeness_request() = default;
				receipt_completeness_request(receipt_completeness_request&&) = default;
				receipt_completeness_request(const receipt_completeness_request&) = default;
				template<typename func_obj, typename ...input> receipt_completeness_request(const Tool::completeness_ref& cpr, func_obj&& fo, input&&... in) :cpf(cpr), receipt<func_type>(std::forward<func_obj>(fo),std::forward<input>(in)...) {};
				receipt_completeness_request& operator= (const receipt_completeness_request&) = default;
				receipt_completeness_request& operator= (receipt_completeness_request&&) = default;
			};

			template<typename func_type> class mail_box_completeness_request : protected mail_box<func_type>
			{
				Tool::completeness_ref cpr;
			public:
				mail_box_completeness_request() = default;
				mail_box_completeness_request(const mail_box_completeness_request&) = default;
				mail_box_completeness_request(mail_box_completeness_request&&) = default;
				mail_box_completeness_request(const receipt_completeness_request<func_type>& r) :mail_box<func_type>(r), cpr(r.cpf) {}
				mail_box_completeness_request& operator= (mail_box_completeness_request&&) = default;
				mail_box_completeness_request& operator= (const mail_box_completeness_request&) = default;
				operator bool() const { return static_cast<bool>(cpr) && mail_box<func_type>::operator bool(); }

				mail_box_completeness_request& operator= (const receipt_completeness_request<func_type>& r) { cpr = r.cpf; mail_box<func_type>::operator=(r); return *this; }

				template<typename ...parameter>
				bool operator() (parameter&&... fp)
				{
					bool res = false;
					return cpr.lock_if
					(
						[this,&res,&fp...]() 
					{
						res = mail_box<func_type>::operator()(std::forward<parameter>(fp)...);
					}
					) && res;
				}

				template<typename T, typename ...parameter>
				bool capture(T&& f, parameter&&... fp)
				{
					bool res = false;
					return cpr.lock_if(
						[&,this]()
					{
						res = mail_box<func_type>::capture(std::forward<T>(f), std::forward<parameter>(fp)...);
					}
					) && res;
				}

			};
		}

		template<typename fun_type> class single_mail_completeness_request
		{
			Assistant::mail_box_completeness_request<fun_type> func;
		public:
			using receipt = Assistant::receipt_completeness_request<fun_type>;
			template<typename func_obj, typename ...input> decltype(auto) bind(const Tool::completeness_ref& cpr, func_obj&& fo, input&& ...in)
			{
				receipt re(cpr, std::forward<func_obj>(fo), std::forward<input>(in)...);
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
				return func.capture(std::forward<T>(f), std::forward<parameter>(fp)...);
			}
		};
		

		template<typename fun_type> class list_mail_completeness_request
		{
			std::recursive_mutex mutex;
			std::list<Assistant::mail_box_completeness_request<fun_type>> all_mail;
			using store_type = std::list<Assistant::mail_box_completeness_request<fun_type>>;

		public:

			list_mail_completeness_request() = default;
			list_mail_completeness_request(const list_mail_completeness_request&) = default;
			list_mail_completeness_request(list_mail_completeness_request&&) = default;
			list_mail_completeness_request& operator=(const list_mail_completeness_request&) = default;
			list_mail_completeness_request& operator=(list_mail_completeness_request&&) = default;

			class receipt :private Assistant::receipt_completeness_request<fun_type>
			{
				typename store_type::iterator this_ite;
				store_type*  const target_list;
				friend class list_mail_completeness_request;
			public:
				template<typename fun_obj, typename ...para>
				receipt(typename store_type::iterator ei, store_type* tl, Assistant::receipt_completeness_request<fun_type>&& fc) : this_ite(ei), target_list(tl), Assistant::receipt_completeness_request<fun_type>(std::move(fc)) {}
				receipt(typename store_type::iterator ei, store_type* tl, const Assistant::receipt_completeness_request<fun_type>& fc) : this_ite(ei), target_list(tl), Assistant::receipt_completeness_request<fun_type>(fc) {}
				receipt() = default;
				receipt(const receipt&) = default;
				receipt(receipt&&) = default;
				void cancle() { Assistant::receipt_completeness_request<fun_type>::cancle(); this_ite = typename store_type::iterator(); target_list = nullptr; }
				operator bool() const { return Assistant::receipt_completeness_request<fun_type>::operator bool(); }
			};

			template<typename fun_ob, typename ...input>
			decltype(auto) bind(const Tool::completeness_ref& cpr, fun_ob&& func, input&& ...k)
			{
				std::lock_guard<std::recursive_mutex> lg(mutex);
				Assistant::receipt_completeness_request<fun_type> tem(cpr, std::forward<fun_ob>(func), std::forward<input>(k)...);
				all_mail.push_back(tem);
				auto this_ite = all_mail.rbegin().base();
				return receipt(this_ite, &all_mail, std::move(tem));
			}

			template<typename ...input>
			void operator() (input&&... ip)
			{
				std::lock_guard<std::recursive_mutex> lg(mutex);
				for (auto ite = all_mail.begin(); ite != all_mail.end();)
				{
					if ((*ite)(std::forward<input>(ip)...))
						++ite;
					else
						all_mail.erase(ite++);
				}
			}

			template<typename T, typename ...input>
			void capture(T&& t, input&&... fp)
			{
				std::lock_guard<std::recursive_mutex> lg(mutex);
				for (auto ite = all_mail.begin(); ite != all_mail.end();)
				{
					bool break_ = false;
					if (*ite &&
						Tool::statement_if<std::is_same<typename std::function<fun_type>::result_type, void>::value>
						(
							[&break_, &ite, &fp...](auto&& fu) {return (*ite).capture([&fu, &break_]() {break_ = fu(); }, std::forward<input>(fp)...); },
							[&break_, &ite, &fp...](auto&& fu) {return (*ite).capture([&fu, &break_](auto&& in) {break_ = fu(std::forward<decltype(in) && >(in)); }, std::forward<input>(fp)...); },
							std::forward<T>(t)
							)
						)
					{
						if (break_)
							break;
						++ite;
					}
					else
						all_mail.erase(ite++);
				}
			}

			bool set_front(const receipt& re)
			{
				std::lock_guard<std::recursive_mutex> lg(mutex);
				if (static_cast<bool>(re) && (re.target_list == &all_mail))
				{
					all_mail.splice(all_mail.begin(), std::move(all_mail), re.this_ite);
					return true;
				}
				return false;
			}

			bool set_back(const receipt& re)
			{
				std::lock_guard<std::recursive_mutex> lg(mutex);
				if (static_cast<bool>(re) && (re.target_list == &all_mail))
				{
					all_mail.splice(all_mail.end(), std::move(all_mail), re.this_ite);
					return true;
				}
				return false;
			}

			bool set_front_of(const receipt& front_of_this, const receipt& target)
			{
				std::lock_guard<std::recursive_mutex> lg(mutex);
				if (static_cast<bool>(front_of_this) && static_cast<bool>(target) && (front_of_this.target_list == &all_mail) && (target.target_list == &all_mail))
				{
					all_mail.splice(front_of_this.this_ite, std::move(all_mail), target.this_ite);
					return true;
				}
				return false;
			}

			bool erase(receipt& re)
			{
				std::lock_guard<std::recursive_mutex> lg(mutex);
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