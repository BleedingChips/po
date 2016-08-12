#pragma once
#include <functional>
#include "type_tool.h"
#include "tool.h"
#include "auto_adapter.h"
#include <memory>
#include <list>
namespace PO
{
	namespace Tool
	{
		namespace Mail
		{

			namespace Assistant
			{
				template<typename fun_obj>
				struct box_obj_control
				{
					exist_flag control;
					exist_flag_weak receipt_flag;
					fun_obj func;
				};
			}

			template<typename fun_type> class list_box;
			template<typename fun_ret,typename ...fun_para> class list_box<fun_ret(fun_para...)>
			{
				using store = std::list<Assistant::box_obj_control>;
				store control_list;
			public:
				class receipt
				{
					exist_flag rec_control;
					exist_flag_weak obj_control;

				public:

					operator bool() 

					template<typename T> bool bind(T&& t)
					{
						using fun_type = Tool::funtion_detect_t<T>;
						if (!func_list.expired())
						{
							auto list_ptr = func_list.lock();
							func = std::make_shared<std::function<fun_type>>(std::forward<T>(t));
							list_ptr->push_back(func);
							this_ite = --list_ptr->end();
							end_ite = list_ptr->end();
							return true;
						}
						return false;
					}
				};


				void operator()(fun_para... it)
				{
					if (func_list)
					{
						store& fl = *func_list;
						for (auto po = fl.begin(); po != fl.end(); )
						{
							if (po->expired())
								fl.erase(po++);
							else {
								auto fun = (po++)->lock();
								(*fun)(it...);
							}
						}
					}
				}

				template<typename break_if>
				void operator()(break_if&& bi, fun_para... it)
				{
					if (func_list)
					{
						store& fl = *func_list;
						for (auto po = fl.begin(); po != fl.end(); )
						{
							if (po->expired())
								fl.erase(po++);
							else {
								auto fun = (po++)->lock();
								if (std::forward<break_if>(bi)((*fun)(it...)))
									break;
							}
						}
					}
				}
			};

			















			/*


			namespace Assistant
			{
				template<typename T> struct mail_control;

				template<typename T> struct box_element;
				template<typename RE, typename ...PA>
				struct box_element<RE(PA...)>
				{
					exist_flag_weak exist;
					std::function<RE(PA...)> func;
					
					bool operator()(RE& ret, PA... pa) 
					{
						bool resault = false;
						return exist.lock_if(
							[&,this]() 
						{
							if ( static_cast<bool>(func))
							{
								ret = func(std::forward<PA>(pa)...);
								resault = true;
							}
						}
						) && resault;
					}

					bool operator()(PA... pa)
					{
						bool resault = false;
						return exist.lock_if(
							[&, this]()
						{
							if ( static_cast<bool>(func))
							{
								func(std::forward<PA>(pa)...);
								resault = true;
							}
						}
						) && resault;
					}
				};

				template<typename T> class box_store;

				template<typename RE,typename ...PA> class box_store<RE(PA...)>
				{
					std::recursive_mutex box_mutex;
					std::list< box_element<RE(PA...)> > list;

				public:

					using iterator = typename std::list< box_element<RE(PA...)>>::iterator;
					using const_iterator = typename std::list< box_element<RE(PA...)>>::const_iterator;
					using identity = std::pair<const_iterator, const_iterator>;

					box_store() = default;
					box_store(const box_store&) = default;
					box_store(box_store&&) = default;

					template<typename T>
					void lock(T&& t)
					{
						box_mutex.lock();
						t(list);
						box_mutex.unlock();
					}

					identity end()
					{
						box_mutex.lock();
						auto po = std::make_pair(list.end(), list.end());
						box_mutex.unlock();
						return po;
					}

					identity begin()
					{
						box_mutex.lock();
						auto po = std::make_pair(list.begin(), list.end());
						box_mutex.unlock();
						return po;
					}

					identity push_back(box_element<RE(PA...)>&& be)
					{
						box_mutex.lock();
						list.push_back(std::move(be));
						auto po = std::make_pair(--list.end(),list.end());
						box_mutex.unlock();
						return po;
					}

					bool set_front_of(identity id,identity id2)
					{
						box_mutex.lock();
						bool is = (id.second == list.end() && id2.second == list.end());
						if (is)
						{
							list.splice(id2.first, std::move(list), id.first);
						}
						box_mutex.unlock();
						return is;
					}

					bool set_front(identity id)
					{
						box_mutex.lock();
						bool is = (id.second == list.end());
						if (is)
						{
							list.splice(list.begin(), std::move(list), id.first);
						}
						box_mutex.unlock();
						return is;
					}

					bool set_back(identity id)
					{
						box_mutex.lock();
						bool is = (id.second == list.end());
						if (is)
						{
							list.splice(list.end(), std::move(list), id.first);
						}
						box_mutex.unlock();
						return is;
					}

					void operator()(PA... at)
					{
						box_mutex.lock();
						if (!list.empty())
						{
							auto ptr = list.begin();
							auto end = --list.end();
							bool exit = true;
							while (exit)
							{
								exit = (ptr != end);
								if (!(*ptr)(std::forward<PA>(at)...))
									list.erase(ptr++);
								else ++ptr;
							}
						}
						box_mutex.unlock();
					}

					template<typename T>
					void operator()(T&& t,PA... at)
					{
						box_mutex.lock();
						if (!list.empty())
						{
							auto ptr = list.begin();
							auto end = --list.end();
							bool exit = true;
							while (exit)
							{
								exit = (ptr != end);
								RE resault;
								if (!(*ptr)(resault,std::forward<PA>(at)...))
									list.erase(ptr++);
								else if (t(resault))
										++ptr;
								else break;
							}
						}
						box_mutex.unlock();
					}
					
				};

			}

			template<typename T> class box; 
			template<typename RE,typename ...PA> class box<RE(PA...)>
			{
				Assistant::box_store<RE(PA...)> store;
				exist_flag exist;
				template<typename T> friend class receipt;
			public:
				box() = default;
				box(const box&) = default;
				box(box&&) = default;
				void operator()(PA... at)
				{
					store(std::forward<PA>(at)...);
				}
				template<typename T>
				void operator()(T&& t,PA... at)
				{
					store(std::forward<T>(t), std::forward<PA>(at)...);
				}
			};

			template<typename T> class receipt;
			template<typename RE, typename ...PA> class receipt<RE(PA...)>
			{
				exist_flag receipt_exist;
				exist_flag_weak box_exist;
				Assistant::box_store<RE(PA...)>* store_ptr;
				typename Assistant::box_store<RE(PA...)>::identity id;

			public:

				void bind_box(box<RE(PA...)>& b)
				{
					b.exist.make_exist_if_not();
					box_exist = b.exist;
					id = b.store.end();
					store_ptr = &b.store;
				}

				bool set_front()
				{
					bool seted = false;
					return box_exist.lock_if(
						[&seted,this] {
						if (store_ptr != nullptr)
						{
							store_ptr->set_front(id);
							seted = true;
						}
					}
					) && seted;
				}

				bool set_back()
				{
					bool seted = false;
					return box_exist.lock_if(
						[&seted, this] {
						if (store_ptr != nullptr)
						{
							store_ptr->set_back(id);
							seted = true;
						}
					}
					) && seted;
				}

				bool set_front(const receipt& r)
				{
					bool seted = false;
					return box_exist.lock_if(
						[&, this] {
						if (store_ptr != nullptr)
						{
							store_ptr->set_front_of(id,r.id);
							seted = true;
						}
					}
					) && seted;
				}

				bool bind(const std::function<RE(PA...)>& fun)
				{
					bool resault = false;
					return box_exist.lock_if(
						[&,this]()
					{
						if (store_ptr != nullptr)
						{
							receipt_exist.make_exist();
							id = store_ptr->push_back(
								Assistant::box_element<RE(PA...)>{ receipt_exist, fun}
							);
							resault = true;
						}
					}
					) && resault;
				}

				bool bind(std::function<RE(PA...)>&& fun)
				{
					bool resault = false;
					return box_exist.lock_if(
						[&, this]()
					{
						if (store_ptr != nullptr)
						{
							receipt_exist.make_exist();
							id = store_ptr->push_back(
								Assistant::box_element<RE(PA...)>{ receipt_exist, std::move(fun)}
							);
							resault = true;
						}
					}
					) && resault;
				}

			};


			*/
		}
	}
	




			/*
			struct mail_control
			{
				mail_control(){}
				mail_control(const mail_control&) = default;
				mail_control(mail_control&&) = default;
				virtual void set_front() = 0;
				virtual void set_back() = 0;
			};
			
			class receipt
			{
				std::shared_ptr<mail_control> control;
			public:
				receipt() {}
				receipt(const std::shared_ptr<mail_control>& c) :control(c) {}
				receipt(const receipt&) = default;
				receipt(receipt&&) = default;
				receipt& operator=(const receipt&) = default;
				operator bool() const { return static_cast<bool>(control); }
				void recall() { control.reset(); }
				void set_front() {
					if (control)
						control->set_front();
				}
				void set_back() {
					if (control)
						control->set_back();
				}
			};

			template<typename T> struct box_element;
			template<typename RE,typename ...PA>
			struct box_element<RE(PA...)>
			{
				std::weak_ptr<mail_control> receipt;
				std::function<RE(PA...)> func;
				operator bool() const { return !receipt.expired() && static_cast<bool>(func); }
				RE operator()(PA... pa) { return func(std::forward<PA>(pa)...); }
				std::shared_ptr<mail_control> get_control() { return mail_control.lock(); };
			};

			template<typename T> struct mail_control_substance :mail_control
			{
				std::list<box_element<T>>& list_ref;
				typename std::list<box_element<T>>::iterator this_ite;
				virtual void set_front() override
				{
					list_ref.splice(list_ref.begin(), std::move(list_ref), this_ite);
				}
				virtual void set_back() override
				{
					list_ref.splice(list_ref.end(), std::move(list_ref), this_ite);
				}
				mail_control_substance(std::list<box_element<T>>& ref, typename std::list<box_element<T>>::iterator i) :list_ref(ref), this_ite(i) {}
				mail_control_substance(const mail_control_substance&) = default;
			};

			template<typename T> class boxs;
			template<typename RT,typename ...PA> 
			class boxs<RT(PA...)>
			{
				std::list<box_element<RT(PA...)>> be_list;
			public:
				operator bool() const { return be_list.empty(); }

				template<typename T>
				void operator()(PA... pa, T t)
				{
					auto begin = be_list.begin();
					auto end = be_list.end();
					for (auto po = be_list.begin(); po != be_list.end();)
					{
						if (*po)
						{
							if (!t((*po++)(std::forward<PA>(pa)...)))
								break;
						}
						else
							be_list.erase(po++);
					}
				}

				void operator()(PA... pa)
				{
					std::list<box_element<RT(PA...)>> pos_list;
					for (auto po = be_list.begin(); po != be_list.end();)
					{
						if (*po)
						{
							(*po++)(std::forward<PA>(pa)...);
						}
						else
							be_list.erase(po++);
					}
				}

				template<typename T>
				receipt bind(T&& t)
				{
					std::shared_ptr<mail_control_substance<RT(PA...)>> tem = std::make_shared<mail_control_substance<RT(PA...)>>(be_list,be_list.end());
					be_list.push_back( box_element<RT(PA...)>{ tem, t });
					tem->this_ite = --be_list.end();
					return receipt(tem);
				}

				template<typename T>
				void stort(T t)
				{
					be_list.sort(t);
				}

				void clear() { be_list.clear(); }
			};
		}*/















		/*
		namespace Mail
		{
			
			class owner
			{

				class mail_function_index { virtual ~mail_function_index() {} };

				template<typename T>
				class mail_function :mail_function_index
				{
					std::function<T> func;
				public:
					template<typename ...AT> decltype(auto) operator()(AT&&... at) { return func(std::forward<AT>(at)...); }
					template<typename AT>  mail_function(AT&& at) :func(std::forward<AT>(at)) {}
				};

				template<typename T> friend class mail;

				std::shared_ptr<mail_function_index> index;
			public:
				bool recall() { if (index) { index.reset(); return true; } return false; }
				owner() {}
				owner(owner&& ow) :index(std::move(ow.index)) {}
				owner& operator= (owner&& ow) { index = std::move(ow.index); return *this; }
			};

			template<typename T> class mail {};
			template<typename T,typename ...PT> class mail<T(PT...)>
			{
				std::list<std::weak_ptr< owner::mail_function<T(PT...)>> > all_func;
			public:
				void operator()(PT ...pt) 
				{
					for (auto po = all_func.begin(); po != all_func.end(); )
					{
						if (auto shared = po->lock())
						{
							shared->operator()( std::forward<PT>(pt)...  );
							++po;
						}
						else {
							all_func.erase(po++);
						}
					}
				}

				template<typename K>
				void   (PT... pt, K&& k)
				{

				}
			};

		}
		*/
		/*
		class mail_owner
		{
			
		public:
			mail_owner() :rwef(nullptr) {}
		};*/



		/*
		namespace Assistant
		{
			template<int i, typename T, typename K, int ...oi> struct mail_type_allocator;

			template<int i, typename T, typename K, typename ...AT, typename ...AK, int ...oi>
			struct mail_type_allocator<i, deque_<T, AT...>, deque_<K, AK...>, oi...>
			{
				typedef typename mail_type_allocator<i + 1, deque_<AT...>, deque_<K, AK...>, oi...>::type type;
			};

			template<int i, typename T, typename ...AT, typename ...AK, int ...oi>
			struct mail_type_allocator<i, deque_<T, AT...>, deque_<T, AK...>, oi...>
			{
				static_assert(sizeof...(AT) >= sizeof...(AK), "parameter type dismatch.");
				typedef typename mail_type_allocator<i + 1, deque_<AT...>, deque_<AK...>, oi...,i>::type type;
			};

			template<int i, typename ...AK, int ...oi>
			struct mail_type_allocator<i, deque_<AK...>, deque_<>, oi...>
			{
				typedef typename std::integer_sequence<int,oi...> type;
			};

			template<int i> struct replace_placeholder;

			#define REPLACE_PLACEHOLDER(i) template<> struct replace_placeholder<i> { auto operator() () { return std::placeholders::_##i; } };
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

			template<typename T> class mail_function_allocator;
			template<typename T, typename ...AT> class mail_function_allocator<T(AT...)>
			{
				template<typename C, int ...i> auto bing_assistant(C&& c, std::integer_sequence<int,i...>)
				{
					return std::move(std::function<T(AT...)>(std::bind(c, replace_placeholder<i+1>()()...)));
				}
				template<typename C, typename K,int ...i> auto bing_assistant(C&& c, K* k, std::integer_sequence<int,i...>)
				{
					return std::move(std::function<T(AT...)>(std::bind(c,k, typename replace_placeholder<decltype(std::placeholders::_1), i + 1>::type()...)));
				}
			public:
				template<typename C> auto operator()(C&& t) { return std::move(std::function<T(AT...)>(std::forward<C>(t))); };
				template<typename C, typename ...AC> auto operator()(C(*func)(AC...))
				{
					static_assert(sizeof...(AT) >= sizeof...(AC), "unable to bind function");
					return std::move(bing_assistant(func, typename mail_type_allocator<0,deque_<AT...>, deque_<AC...>>::type()));
				}
				template<typename C, typename K,typename ...AC,typename P> auto operator()(C(K::*func)(AC...),P* t)
				{
					return std::move(bing_assistant(func,t, typename mail_type_allocator<0, deque_<AT...>, deque_<AC...>>::type()));
				}
			};
		}

		class mail_index
		{
			struct index_t {};
			template<typename T> friend class mail;
			std::shared_ptr<index_t> all_index;
		public:
			template<typename T> mail_index(T&& t) :all_index(std::forward<T>(t)) {}
			mail_index() {}
			void disconnect() { all_index.reset(); }
		};

		template<typename T>  class mail;
		template<typename ...AT> 
		class mail<void(AT...)>
		{
			std::list<std::pair<std::weak_ptr<mail_index>,std::function<void(AT...)>>> all_mail;

		public:

			template<typename ...PT> void operator()(PT&& ... pt) 
			{
				for (auto po = all_mail.begin(); po != all_mail.end();)
				{
					auto tem = po->first.lock();
					if (tem)
					{
						po->second(pt...);
						++po;
					}
					else {
						all_mail.erase(po++);
					}
				}
			}

			void claer() 
			{
				all_mail.clear();
			}

			template<typename T> mail_index bind(T&& t)
			{ 
				std::shared_ptr < mail_index::index_t > tem = std::make_shared< mail_index::index_t >();
				auto temf = Assistant::mail_function_allocator<void(AT...)>()(t);
				all_mail.push_back(std::make_pair( tem, std::move(temf)));
				return std::move(mail_index(tem));
			}

			template<typename T,typename K,typename ...AP,typename P> mail_index bind(T (K::*func)(AP...),P* k)
			{
				std::shared_ptr<mail_index::index_t> tem = std::make_shared< mail_index::index_t >();
				auto temf = Assistant::mail_function_allocator<void(AT...)>()(func,k);
				all_mail.push_back(std::make_pair(tem, std::move(temf)));
				return std::move(mail_index(tem));
			}


		};


		/*template<typename T> class single_mail;

		template<typename ...AT>
		class single_mail<void(AT...)>
		{
			std::pair<std::weak_ptr<index>, std::function<void(AT...)>> sin_mail;
		public:
			template<typename ...PT> void operator()(PT&& ... pt)
			{
				auto tem = sin_mail.first.lock();
				if (tem)
				{
					sin_mail(std::forward<PT>(pt)...);
				}
			}
			void claer()
			{
				sin_mail.first.reset();
			}
			template<typename T> mail_index bind(T&& t)
			{
				std::shared_ptr<mail_index::index> tem = std::make_shared< mail_index::index >();
				auto temf = Assistant::mail_function_allocator<void(AT...)>()(t);
				sin_mail.first = tem;
				sin_mail.second = std::move(temf);
				return std::move(mail_index(tem));
			}
			template<typename T, typename K, typename ...AP, typename P> mail_index bind(T(K::*func)(AP...), P* k)
			{
				std::shared_ptr<mail_index::index> tem = std::make_shared< mail_index::index >();
				auto temf = Assistant::mail_function_allocator<void(AT...)>()(func, k);
				sin_mail.first = tem;
				sin_mail.second = std::move(temf);
				return std::move(mail_index(tem));
			}
		};*/



		/*class mail_box_index {};

		template<typename T = void> class mail_box :public T
		{
			std::shared_ptr<mail_box_index> index;
			template<typename ...AT> friend class mail;
		public:
			template<typename ...AT>
			mail_box(AT&&... at) :T(std::forward<AT>(at)...), index(std::make_shared<mail_box_index>()) {}
		};

		template<> class mail_box<void>
		{
			std::shared_ptr<mail_box_index> index;
			template<typename ...AT> friend class mail;
		public:
			mail_box() :index(std::make_shared<mail_box_index>()) {}
		};

		template<typename ...AT>
		class mail
		{
			struct mail_function
			{
				std::weak_ptr<mail_box_index> index;
				virtual void execute(AT... at) = 0;
				mail_function(const std::shared_ptr<mail_box_index>& k) :index(k) {}
				operator bool() const { return !index.expired(); }
				virtual bool equal(const mail_function&) const = 0;
				bool operator == (const mail_function& mf) const { return mf.index.lock() == index.lock() && equal(mf); }
			};

			template<typename T>
			struct mail_function_execute :public mail_function
			{
				T& t;
				void (T::*ui)(AT...);
				bool equal(const mail_function& mf) const
				{
					try {
						const mail_function_execute& mfe = dynamic_cast<const mail_function_execute&>(mf);
						return ui == mfe.ui;
					}
					catch (...)
					{
						return false;
					}
				}
				virtual void execute(AT... at)
				{
					(t.*ui)(at...);
				}
				mail_function_execute(const std::shared_ptr<mail_box_index>& c, T& k, void (T::*u)(AT...))
					:mail_function(c), t(k), ui(u) {}
			};
			std::list<std::shared_ptr<mail_function>> mail_index;
			std::shared_ptr<mail_box_index> index;
		public:
			template<typename T, typename K>
			void connect(mail_box<T>& t, void (K::*func)(AT...))
			{
				static_assert(
					std::is_same<K, T>::value || std::is_base_of<K, T>::value,
					"the object is not able to call the function pointer."
					);
				mail_index.push_back(std::make_shared<mail_function_execute<K>>(t.mail_box<T>::index, t, func));
			}

			template<typename T, typename K>
			void connect(T& t, void (K::*func)(AT...))
			{
				static_assert(std::is_base_of<mail_box<void>, T>::value, "the object must derives from mail_box<> or mail_box<void>.");
				static_assert(
					std::is_same<K, T>::value || std::is_base_of<K, T>::value,
					"the object is not able to call the function pointer."
					);
				mail_index.push_back(std::make_shared<mail_function_execute<K>>(t.mail_box<void>::index, t, func));
			}

			void connect(mail& t)
			{
				mail_index.push_back(std::make_shared<mail_function_execute<mail>>(t.index, t, &mail::sent));
			}

			void sent(AT... at)
			{
				for (auto Poi = mail_index.begin(); Poi != mail_index.end();)
				{
					if (*(*Poi))
					{
						(*(Poi++))->execute(at...);
					}
					else {
						mail_index.erase(Poi++);
					}
				}
			}
			void duplicate_checking()
			{
				for (auto Poi = mail_index.begin(); Poi != mail_index.end(); ++Poi)
				{
					auto Poi2 = Poi;
					for (++Poi2; Poi2 != mail_index.end();)
					{
						if (**Poi == **Poi2)
							mail_index.erase(Poi2++);
						else
							++Poi2;
					}
				}
			}
			mail() :index(std::make_shared<mail_box_index>()) {}
		};*/
	//}
	
}
