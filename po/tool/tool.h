#pragma once
#include <functional>
#include <mutex>
#include <thread>
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
					[](auto&& u) { return dynamic_cast<T>(std::forward<decltype(u) && >(u)); },
					[](auto&& u) { return static_cast<T>(std::forward<decltype(u) && >(u)); },
					std::forward<P>(data)
			);
#else
			return static_cast<T>(std::forward<decltype(u) && >(u));
#endif // DEBUG
		}


		template<typename T> class completeness_protector;
		class completeness_protector_ref;
		namespace Assistant
		{
			enum class completeness_protector_state : int8_t
			{
				Construction,
				Ready,
				Destruction,
				Missing
			};

			struct completeness_protector_head_data_struct
			{
				std::mutex mutex;
				completeness_protector_state state = completeness_protector_state::Construction;
				size_t ref = 0;
				size_t read_ref = 0;
			};
			class completeness_protector_head
			{
				completeness_protector_head_data_struct* data;
				completeness_protector_head() :data(new completeness_protector_head_data_struct) 
				{ 
					std::lock_guard<decltype(data->mutex)> lg(data->mutex);
					data->state = completeness_protector_state::Construction;
					++(data->ref);
				}
				completeness_protector_head(completeness_protector_head&& cph) :data(new completeness_protector_head_data_struct)
				{
					std::lock_guard<decltype(data->mutex)> lg(data->mutex);
					++(data->ref);
					data->state = completeness_protector_state::Construction;
					std::swap(data,cph.data);
				}
				completeness_protector_head(const completeness_protector_head&) :data(new completeness_protector_head_data_struct)
				{
					std::lock_guard<decltype(data->mutex)> lg(data->mutex);
					data->state = completeness_protector_state::Construction;
					++(data->ref);
				}
				~completeness_protector_head() 
				{ 
					size_t res = 0;
					{
						std::lock_guard<decltype(data->mutex)> lg(data->mutex);
						res = --data->ref;
						data->state = completeness_protector_state::Missing;
					}
					if (res == 0)
						delete data;
				}
				template<typename T> friend class Tool::completeness_protector;
				friend class Tool::completeness_protector_ref;
			};
		}

		template<typename T>
		class completeness_protector_construct_selecter : public std::remove_reference_t<T>
		{
			template<typename ref, typename ...AT> completeness_protector_construct_selecter(std::true_type, ref&& re, AT&& ...at) : std::remove_reference_t<T>(std::forward<ref>(re), std::forward<AT>(at)...) {}
			template<typename ref, typename ...AT> completeness_protector_construct_selecter(std::false_type, ref&& re, AT&& ...at) : std::remove_reference_t<T>(std::forward<AT>(at)...) {}
		public:
			template<typename ...AT>  completeness_protector_construct_selecter(Tool::completeness_protector_ref&& cpr, AT&& ...at) : completeness_protector_construct_selecter(
				std::integral_constant<bool,
					std::is_constructible<std::remove_reference_t<T>, completeness_protector_ref, AT...>::value
				>(),
				std::move(cpr), std::forward<AT>(at)...
			) {}

			template<typename ...AT>  completeness_protector_construct_selecter(const Tool::completeness_protector_ref& cpr, AT&& ...at) : completeness_protector_construct_selecter(
				std::integral_constant<bool,
				std::is_constructible<std::remove_reference_t<T>, completeness_protector_ref, AT...>::value
				>(),
				cpr, std::forward<AT>(at)...
			) {}
		};

		template<typename T>
		class completeness_protector :private Assistant::completeness_protector_head, public completeness_protector_construct_selecter<T>
		{
			friend class completeness_protector_ref;
		public:
			completeness_protector() : completeness_protector_construct_selecter<T>(static_cast<Assistant::completeness_protector_head&>(*this)) 
			{
				std::lock_guard<decltype(Assistant::completeness_protector_head::data->mutex)> lg(Assistant::completeness_protector_head::data->mutex);
				Assistant::completeness_protector_head::data->state = Assistant::completeness_protector_state::Ready;
			}

			completeness_protector(completeness_protector&& cp) : Assistant::completeness_protector_head(std::move(cp)), completeness_protector_construct_selecter<T>(static_cast<Assistant::completeness_protector_head&>(*this),std::move(cp))
			{
				std::lock_guard<decltype(Assistant::completeness_protector_head::data->mutex)> lg(Assistant::completeness_protector_head::data->mutex);
				Assistant::completeness_protector_head::data->state = Assistant::completeness_protector_state::Ready;
			}

			completeness_protector(const completeness_protector& cp) : completeness_protector_construct_selecter<T>(static_cast<Assistant::completeness_protector_head&>(*this), std::move(cp))
			{
				std::lock_guard<decltype(Assistant::completeness_protector_head::data->mutex)> lg(Assistant::completeness_protector_head::data->mutex);
				Assistant::completeness_protector_head::data->state = Assistant::completeness_protector_state::Ready;
			}

			template<typename ...AT> completeness_protector(AT&& ...at) : completeness_protector_construct_selecter<T>(static_cast<Assistant::completeness_protector_head&>(*this), std::forward<AT>(at)...)
			{
				std::lock_guard<decltype(Assistant::completeness_protector_head::data->mutex)> lg(Assistant::completeness_protector_head::data->mutex);
				Assistant::completeness_protector_head::data->state = Assistant::completeness_protector_state::Ready;
			}
			~completeness_protector()
			{
				while (true)
				{
					Assistant::completeness_protector_head::data->mutex.lock();
					if (Assistant::completeness_protector_head::data->read_ref == 0)
					{
						Assistant::completeness_protector_head::data->state = Assistant::completeness_protector_state::Destruction;
						Assistant::completeness_protector_head::data->mutex.unlock();
						break;
					}
					else
					{
						Assistant::completeness_protector_head::data->mutex.unlock();
						std::this_thread::yield();
					}
				}
			}
		};

		template<>
		class completeness_protector<void> :private Assistant::completeness_protector_head
		{
			friend class completeness_protector_ref;
		public:
			completeness_protector()
			{
				std::lock_guard<decltype(Assistant::completeness_protector_head::data->mutex)> lg(Assistant::completeness_protector_head::data->mutex);
				Assistant::completeness_protector_head::data->state = Assistant::completeness_protector_state::Ready;
			}

			completeness_protector(completeness_protector&& cp) : Assistant::completeness_protector_head(std::move(cp))
			{
				std::lock_guard<decltype(Assistant::completeness_protector_head::data->mutex)> lg(Assistant::completeness_protector_head::data->mutex);
				Assistant::completeness_protector_head::data->state = Assistant::completeness_protector_state::Ready;
			}

			completeness_protector(const completeness_protector& cp) :completeness_protector() {}

			~completeness_protector()
			{
				while (true)
				{
					Assistant::completeness_protector_head::data->mutex.lock();
					if (Assistant::completeness_protector_head::data->read_ref == 0)
					{
						Assistant::completeness_protector_head::data->state = Assistant::completeness_protector_state::Destruction;
						Assistant::completeness_protector_head::data->mutex.unlock();
						break;
					}
					else
					{
						Assistant::completeness_protector_head::data->mutex.unlock();
						std::this_thread::yield();
					}
				}
			}
		};

		class completeness_protector_ref
		{
			Assistant::completeness_protector_head_data_struct* data;
		public:
			operator bool() const { return data != nullptr; }
			completeness_protector_ref(const Assistant::completeness_protector_head& cpd) :data(cpd.data)
			{
				data->mutex.lock();
				++(data->ref);
				data->mutex.unlock();
			}
			completeness_protector_ref() :data(nullptr) {}
			completeness_protector_ref(const completeness_protector_ref& cpd) :data(cpd.data)
			{
				if (data != nullptr)
				{
					data->mutex.lock();
					++(data->ref);
					data->mutex.unlock();
				}
			}
			completeness_protector_ref(completeness_protector_ref&& cpf) :data(cpf.data)
			{
				cpf.data = nullptr;
			}
			template<typename T> completeness_protector_ref(const completeness_protector<T>& cp) : completeness_protector_ref(static_cast<const Assistant::completeness_protector_head&>(cp)) {}
			completeness_protector_ref& operator=(const completeness_protector_ref& cpf)
			{
				completeness_protector_ref tem(cpf);
				drop();
				data = tem.data;
				if (data != nullptr)
				{
					data->mutex.lock();
					++data->ref;
					data->mutex.unlock();
				}
				return *this;
			}
			completeness_protector_ref& operator=(completeness_protector_ref&& cpf)
			{
				completeness_protector_ref tem(std::move(cpf));
				drop();
				data = tem.data;
				tem.data = nullptr;
				return *this;
			}
			template<typename T>
			completeness_protector_ref& operator=(const completeness_protector<T>&& cp)
			{
				return operator&=(static_cast<const Assistant::completeness_protector_head&>(cp));
			}
			void drop()
			{
				if (data != nullptr)
				{
					data->mutex.lock();
					size_t res = --data->ref;
					data->mutex.unlock();
					if (res == 0)
						delete data;
					data = nullptr;
				}
			}
			template<typename T>
			bool lock_if(T&& fun)
			{
				if (data != nullptr)
				{
					while (true)
					{
						data->mutex.lock();
						if (data->state == Assistant::completeness_protector_state::Ready)
						{
							++data->read_ref;
							data->mutex.unlock();
							destructor de([this]()
							{
								data->mutex.lock();
								--data->read_ref;
								data->mutex.unlock();
							});
							fun();
							return true;
						}
						else if(data->state != Assistant::completeness_protector_state::Destruction){
							data->mutex.unlock();
							std::this_thread::yield();
						}
						else {
							data->mutex.unlock();
							return false;
						}
					}
				}
				return false;
			}
			template<typename T>
			bool try_lock_if(T&& fun)
			{
				if (data != nullptr)
				{
					data->mutex.lock();
					if (data->state == Assistant::completeness_protector_state::Ready)
					{
						++data->read_ref;
						data->mutex.unlock();
						destructor de([this]()
						{
							data->mutex.lock();
							--data->read_ref;
							data->mutex.unlock();
						});
						fun();
						return true;
					}
					else
						data->mutex.unlock();
				}
				return false;
			}
			~completeness_protector_ref()
			{
				drop();
			}
		};
	}
}

inline std::basic_string<char16_t> operator ""_str(const char16_t* str, size_t count)
{
	return std::basic_string<char16_t>(str);
}