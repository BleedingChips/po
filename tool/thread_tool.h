#pragma once
#include "tool.h"
#include <atomic>
#include <mutex>
#include <vector>
#include <thread>
#include <chrono>
#include <iostream>
namespace PO
{
	namespace Tool
	{
		template<typename T> class completeness;
		class completeness_ref;
		namespace Assistant
		{
			enum class completeness_state : int8_t
			{
				Construction,
				Ready,
				Destruction,
				Missing
			};

			struct completeness_head_data_struct
			{
				mutable std::mutex mutex;
				completeness_state state = completeness_state::Construction;
				size_t ref = 0;
				size_t read_ref = 0;
				operator bool() const
				{
					std::lock_guard<decltype(mutex)> lg(mutex);
					return state != completeness_state::Missing && state != completeness_state::Destruction;
				}
				bool try_add_read_ref();
				bool add_read_ref();
				void del_read_ref() { std::lock_guard<decltype(mutex)> lg(mutex); --(read_ref); }

				inline void add_ref() { std::lock_guard<decltype(mutex)> lg(mutex); ++(ref); }
				bool del_ref() { std::lock_guard<decltype(mutex)> lg(mutex); return --(ref)==0; }

				void start_construct() 
				{
					std::lock_guard<decltype(mutex)> lg(mutex);
					state = completeness_state::Construction;
					++(ref);
				}

				void finish_construct()
				{
					std::lock_guard<decltype(mutex)> lg(mutex);
					state = completeness_state::Ready;
				}

				void start_destruct();
				bool finish_destruct()
				{
					std::lock_guard<decltype(mutex)> lg(mutex);
					state = completeness_state::Missing;
					return --ref == 0;
				}
			};

			class completeness_head
			{
				completeness_head_data_struct* data;
				
				completeness_head() :data(new completeness_head_data_struct)
				{
					data->start_construct();
				}
				~completeness_head() { if (data->finish_destruct()) delete data; }
				template<typename T> friend class Tool::completeness;
				friend class Tool::completeness_ref;
			};
		}

		class completeness_ref
		{
			Assistant::completeness_head_data_struct* data;
			void drop()
			{
				if (data != nullptr)
				{
					if (data->del_ref())
						delete data;
					data = nullptr;
				}
			}
		public:
			operator bool() const { return data != nullptr && *data; }
			operator bool()
			{
				if (data != nullptr)
				{
					if (*data)
						return true;
					else {
						if (data->del_ref())
							delete data;
						data = nullptr;
					}
				}
				return false;
			}
			completeness_ref(const Assistant::completeness_head& cpd) :data(cpd.data)
			{
				data->add_ref();
			}
			/*
			template<typename T>
			completeness_ref(const completeness<T>& cpd) : completeness_ref(static_cast<const Assistant::completeness_head&>(cpd)) {}
			*/
			completeness_ref() :data(nullptr) {}
			completeness_ref(const completeness_ref& cpd) :data(cpd.data)
			{
				if (data != nullptr)
				{
					data->add_ref();
				}
			}

			completeness_ref(completeness_ref&& cpf) :data(cpf.data)
			{
				cpf.data = nullptr;
			}

			completeness_ref& operator=(const completeness_ref& cpf)
			{
				completeness_ref tem(cpf);
				drop();
				data = tem.data;
				if (data != nullptr)
				{
					data->add_ref();
				}
				return *this;
			}

			completeness_ref& operator=(completeness_ref&& cpf)
			{
				completeness_ref tem(std::move(cpf));
				drop();
				data = tem.data;
				tem.data = nullptr;
				return *this;
			}

			completeness_ref& operator=(const Assistant::completeness_head& chd)
			{
				drop();
				data = chd.data;
				data->add_ref();
				return *this;
			}

			void reset()
			{
				drop();
			}

			template<typename T>
			auto lock_if(T&& fun) -> Tool::optional_t<decltype(fun())>
			{
				if (operator bool() && data->add_read_ref())
				{
					at_scope_exit ase([this]() {data->del_read_ref(); });
					return{ return_optional_t(fun) };
				}
				return{};
			}

			template<typename T>
			auto try_lock_if(T&& fun) -> Tool::optional_t<decltype(fun())>
			{
				if (operator bool() && data->try_add_read_ref())
				{
					//Tool::destructor de([this]() {data->del_read_ref(); });
					at_scope_exit ase([this]() {data->del_read_ref(); });
					return{ return_optional_t(fun) };
				}
				return{};
			}
			~completeness_ref()
			{
				drop();
			}
		};

		template<typename T>
		class completeness :private Assistant::completeness_head, public std::decay_t<T>
		{
			friend class completeness_ref;

			template<typename ...AT> completeness(std::true_type, AT&& ...at) : std::decay_t<T>(static_cast<const Assistant::completeness_head&>(*this), std::forward<AT>(at)...)
			{
				Assistant::completeness_head::data->finish_construct();
			}
			template< typename ...AT> completeness(std::false_type, AT&& ...at) : std::decay_t<T>(std::forward<AT>(at)...)
			{
				Assistant::completeness_head::data->finish_construct();
			}

		public:

			operator completeness_ref() const 
			{
				return completeness_ref{static_cast<const Assistant::completeness_head&>(*this)};
			}

			template<typename ...AT> completeness(AT&& ...at) : completeness(
				std::integral_constant<bool, std::is_constructible<std::decay_t<T>, const completeness_ref&, AT... >::value>(),
				std::forward<AT>(at)...)
			{

			}

			~completeness()
			{
				Assistant::completeness_head::data->start_destruct();
			}
		};
		
		template<typename T, typename mutex = std::mutex> struct scope_lock
		{
			T data;
			mutable mutex lock_mutex;
		public:
			using type = T;
			using mutex_type = mutex;

			template<typename fun> decltype(auto) lock(fun&& f)
			{
				std::lock_guard<mutex> lg(lock_mutex);
				return f(data);
			}
			template<typename fun> decltype(auto) lock(fun&& f) const
			{
				std::lock_guard<mutex> lg(lock_mutex);
				return f(data);
			}

			template<typename fun> auto try_lock(fun&& f) -> Tool::optional_t<decltype(f())>
			{
				if (lock_mutex.try_lock())
				{
					Tool::at_scope_exit ase({ &}() { lock_mutex.unlock(); })
					return{ return_optional_t(f) };
				}
				return{};
			}
			template<typename fun> auto try_lock(fun&& f) const -> Tool::optional_t<decltype(f())>
			{
				if (lock_mutex.try_lock())
				{
					Tool::at_scope_exit ase({ &}() { lock_mutex.unlock(); })
						return{ return_optional_t(f) };
				}
				return{};
			}
			template<typename ...AT> scope_lock(AT&&... at) :data(std::forward<AT>(at)...) {}
		};

		template<typename T, typename K, typename F>
		auto lock_scope_look(T&& t, K&& k, F&& f)
		{
			return t.lock([&f, &k](auto& tt) mutable {
				return k.lock([&tt, &f](auto& kk) mutable{
					return f(tt, kk);
				});
			});
		}

		namespace Implement
		{
			struct mail_control
			{
				bool avalible = true;
			};

			template<typename T> struct mail_element;
			template<typename ret, typename ...para>
			struct mail_element<ret(para...)>
			{
				std::function<ret(para...)> func;
				std::shared_ptr<scope_lock<mail_control>> cont;
				completeness_ref cr;
				mail_element(std::function<ret(para...)> f, std::shared_ptr<scope_lock<mail_control>> s, completeness_ref c)
					: func(std::move(f)), cont(std::move(s)), cr(std::move(c)) {}
				operator bool() const { return cont && cont->lock([](mail_control& mc) {return mc.avalible; }); }
				decltype(auto) operator()(para... pa)
				{
					auto op = cr.lock_if(
						[&, this]() 
					{
						return func(pa...);
					}
					);
					if (!op)
						cont->lock([](mail_control& i) {i.avalible = false; });
					return op;
				}
			};
			template<typename T> struct mail_element_without_ref;
			template<typename ret, typename ...para>
			struct mail_element_without_ref<ret(para...)>
			{
				std::function<ret(para...)> func;
				std::shared_ptr<scope_lock<mail_control>> cont;
				mail_element_without_ref(std::function<ret(para...)> f, std::shared_ptr<scope_lock<mail_control>> s)
					: func(std::move(f)), cont(std::move(s)) {}
				operator bool() const { return cont && cont->lock([](mail_control& mc) {return mc.avalible; }); }
				auto operator()(para... pa) -> Tool::optional_t<decltype(func(pa...))>
				{
					return{ return_optional_t(func, pa...)};
				}
			};
		}
		struct receiption
		{
			std::shared_ptr<scope_lock<Implement::mail_control>> cont;
		};

		template<typename fun, typename ...vector_para> struct mail;
		template<typename ret, typename ...para, typename ...vector_para>
		struct mail<ret(para...), vector_para...>
		{
			using ele_ref = Implement::mail_element<ret(para...)>;
			using ele = Implement::mail_element_without_ref<ret(para...)>;
			using tank_type = std::vector<
				Tool::variant<ele_ref, ele>,
				vector_para...
			>;
			scope_lock<tank_type> input;
			scope_lock<tank_type> store;
			receiption bind(std::function<ret(para...)> pa, completeness_ref cr)
			{
				auto con = std::make_shared<scope_lock<Implement::mail_control>>();
				input.lock(
					[&](auto& i) {
					i.emplace_back(Tmp::itself<ele_ref>, std::move(pa), con, std::move(cr)); 
				}
				);
				return{con};
			}
			receiption bind(std::function<ret(para...)> pa)
			{
				auto con = std::make_shared<scope_lock<Implement::mail_control>>();
				input.lock(
					[&](auto& i) {
					i.emplace_back(Tmp::itself<ele>{}, std::move(pa), con);
				}
				);
				return{ con };
			}
			template<typename ask>
			void operator()(ask&& ak, para... pa)
			{
				store.lock(
					[&ak, &pa..., this](tank_type& da)
				{
					da.erase(std::remove_if(da.begin(), da.end(), [](auto& u) 
					{
						bool ava = true; 
						if (u)
							u.call([&ava](auto& t) {ava = !static_cast<bool>(t); });
						return ava;
					}), da.end());
					bool need_break = false;
					
					for (Tool::variant<ele_ref, ele>& ptr : da)
					{
						ptr.call(
							[&need_break, &ak, &pa...](auto& i) 
						{
							auto op = i(pa...);
							if (op) need_break = !Tool::statement_if<std::is_same<ret, void>::value>
								(
									[](auto&& f, auto&& pa) {f(); return false; },
									[](auto&& f, auto&& pa) {return f(*pa); },
									std::forward<ask>(ak), op
									);
						}
						);
						if (need_break)
							break;
					}
					
					auto start = input.lock(
						[&da](tank_type& aa)
					{
						aa.erase(std::remove_if(aa.begin(), aa.end(), [](auto& u) 
						{
							bool ava = true;
							if (u)
								u.call([&ava](auto& t) {ava = !static_cast<bool>(t); });
							return ava;
						}), aa.end());
						auto insert =  da.insert(da.end(), std::make_move_iterator(aa.begin()), std::make_move_iterator(aa.end()));
						aa.clear();
						return insert;
					}
					);
					if(!need_break)
						for (; start != da.end(); ++start)
						{
							start->call(
								[&need_break, &ak, &pa...](auto& i)
							{
								auto op = i(pa...);
								if (op) need_break = !Tool::statement_if<std::is_same<ret, void>::value>
									(
										[](auto&& f, auto&& pa) {f(); return false; },
										[](auto&& f, auto&& pa) {return f(*pa); },
										std::forward<ask>(ak), op
									);
							}
							);
							if (need_break)
								break;
						}
				}
				);
			}
		};

		struct thread_task_operator
		{
			struct thread_task_data
			{
				bool need_joined;
				std::vector<std::function<bool(void)>> task;
			};
			scope_lock<thread_task_data> task;
			std::vector<std::function<bool(void)>> calling_buffer;
			std::thread main_thread;
			std::atomic_bool exit;
			bool need_join;
			std::chrono::milliseconds ref_duration;
			void main_thread_execute();
		public:
			thread_task_operator() noexcept : exit(false), need_join(false), ref_duration(1000){}
			~thread_task_operator() { exit = true; if (main_thread.joinable()) main_thread.join(); }
			void add_task(std::function<bool(void)> f);
		};

		template<typename T> struct binding_value_output : T{
			
		};

		namespace Implement
		{
			template<typename T, typename mutex = std::mutex> struct bingding_value_input_implement
			{
				Tool::scope_lock<std::tuple<T, uint64_t>, mutex> data;
				//operator T& () { return data.lock([](decltype(data)::type& u) {return u.store; }); }
				operator const T& () const { return data.lock([](decltype(data)::type& u) {return u.store; }); }
				bingding_value_input_implement& operator= (const T& t) {
					return *this;
				}
			};
		}

		template<typename T, typename mutex = std::mutex> using binding_value_input = completeness<Implement::bingding_value_input_implement<T, mutex>>;

		

		template<typename T, typename mutex = std::mutex> struct value_binding_control
		{
			std::shared_ptr<scope_lock<std::tuple<uint64_t, T>, mutex>> control_ptr;

			value_binding_control() {}
			value_binding_control(const value_binding_control&) = default;
			value_binding_control(value_binding_control&&) = default;
			value_binding_control& operator=(const value_binding_control&) = default;
			value_binding_control& operator=(value_binding_control&&) = default;

			value_binding_control(const T& t) : control_ptr(std::make_shared<scope_lock<std::tuple<uint64_t, T>, mutex>>(std::make_tuple(1, t))) {}
			value_binding_control(T&& t) : control_ptr(std::make_shared<scope_lock<std::tuple<uint64_t, T>, mutex>>(std::make_tuple(1, std::move(t)))) {}

			uint64_t operator=(const T& t) {
				if (!control_ptr)
					return (control_ptr = std::make_shared<scope_lock<std::tuple<uint64_t, T>, mutex>>(std::make_tuple(1, t)), 1);
				else
					return control_ptr->lock([&t](decltype(*control_ptr)::type& tuple) {
					std::get<1>(tuple) = t;
					auto vision = ++std::get<0>(tuple);
					if (vision == 0) std::get<0>(tuple) = 1;
					return std::get<0>(tuple);
				});
			}

			uint64_t operator=(T&& t) {
				if (!control_ptr)
					return (control_ptr = std::make_shared<scope_lock<std::tuple<uint64_t, T>, mutex>>(std::make_tuple(1, std::move(t))), 1);
				else
					return control_ptr->lock([&t](decltype(*control_ptr)::type& tuple) {
					std::get<1>(tuple) = std::move(t);
					auto vision = ++std::get<0>(tuple);
					if (vision == 0) std::get<0>(tuple) = 1;
					return std::get<0>(tuple);
				});
			}

		};

	}
}

namespace std
{
	template<typename T, typename K> void swap(PO::Tool::scope_lock<T>& t, PO::Tool::scope_lock<K>& k)
	{
		t.lock(
			[&](T& t) 
		{
			k.lock(
				[&t](K& k) 
			{
				swap(t, k);
			}
			);
		}
		);
	}
}