#pragma once
#include "tool.h"
#include <atomic>
#include <mutex>
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

		template<typename T>
		class completeness :private Assistant::completeness_head, public std::remove_reference_t<T>
		{
			friend class completeness_ref;

			template<typename ...AT> completeness(std::true_type, AT&& ...at) : std::remove_reference_t<T>(static_cast<const Assistant::completeness_head&>(*this), std::forward<AT>(at)...)
			{
				Assistant::completeness_head::data->finish_construct();
			}
			template< typename ...AT> completeness(std::false_type, AT&& ...at) : std::remove_reference_t<T>(std::forward<AT>(at)...)
			{
				Assistant::completeness_head::data->finish_construct();
			}

		public:

			template<typename ...AT> completeness(AT&& ...at) : completeness(
				std::integral_constant<bool, std::is_constructible<std::remove_reference_t<T>, const Assistant::completeness_head&, AT... >::value>(),
				std::forward<AT>(at)...)
			{

			}

			~completeness()
			{
				Assistant::completeness_head::data->start_destruct();
			}
		};


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
			template<typename T>
			completeness_ref(const completeness<T>& cpd) : completeness_ref(static_cast<const Assistant::completeness_head&>(cpd)) {}
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

			template<typename T>
			completeness_ref& operator=(const completeness<T>& cpd) { return operator=(static_cast<const Assistant::completeness_head&>(cpd)); }

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
			auto lock_if(T&& fun) -> Tool::optional<std::conditional_t<std::is_same<decltype(fun()),void>::value, Tmp::itself<void>, decltype(fun())>>
			{
				if (operator bool() && data->add_read_ref())
				{
					//Tool::destructor de([this]() {data->del_read_ref(); });
					at_scope_exit ase([this]() {data->del_read_ref(); });
					return statement_if<std::is_same<decltype(fun()), void>::value>
						(
							[](auto& f) { f(); return Tmp::itself<void>{}; },
							[](auto& f) {return f(); },
							fun
							);
				}
				return {};
			}
			template<typename T>
			auto try_lock_if(T&& fun) ->Tool::optional<std::conditional_t<std::is_same<decltype(fun()), void>::value, Tmp::itself<void>, decltype(fun())>>
			{
				if (operator bool() && data->try_add_read_ref())
				{
					//Tool::destructor de([this]() {data->del_read_ref(); });
					at_scope_exit ase([this]() {data->del_read_ref(); });
					return statement_if<std::is_same<decltype(fun()), void>::value>
						(
							[](auto& f) { f(); return Tmp::itself<void>{}; },
							[](auto& f) {return f(); },
							fun
							);
				}
				return {};
			}
			~completeness_ref()
			{
				drop();
			}
		};

		template<typename T, typename mutex = std::mutex> struct scope_lock
		{
			T data;
			mutex lock_mutex;
		public:
			template<typename fun> auto lock(fun&& f) -> decltype(f(data))
			{
				std::lock_guard<mutex> lg(lock_mutex);
				return f(data);
			}
			template<typename fun> auto try_lock(fun&& f) -> Tool::optional<decltype(f(data))>
			{
				if (lock_mutex.try_lock())
				{
					Tool::at_scope_exit ase({ &}() { lock_mutex.unlock(); })
					return{ f(data) };
				}
				return{};
			}
			template<typename ...AT> scope_lock(AT&&... at) :data(std::forward<AT>(at)...) {}
		};

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
				auto operator()(para... pa)
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
				auto operator()(para... pa) -> Tool::optional<std::conditional_t<std::is_same<ret, void>::value, Tmp::itself<void>, decltype(func(pa...))>>
				{
					return{ func(pa...) };
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
									[](auto&& f, auto&& pa) {return f(pa); },
									std::forward<ask>(ak), *op
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
					cout << "66666 " << da.end() - start << endl;
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
										[](auto&& f, auto&& pa) {return f(pa); },
										std::forward<ask>(ak), *op
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

#include "thread_tool.hpp"