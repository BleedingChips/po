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
			bool lock_if(T&& fun)
			{
				if (operator bool() && data->add_read_ref())
				{
					//Tool::destructor de([this]() {data->del_read_ref(); });
					at_scope_exit ase([this]() {data->del_read_ref(); });
					fun();
					return true;
				}
				return false;
			}
			template<typename T>
			bool try_lock_if(T&& fun)
			{
				if (operator bool() && data->try_add_read_ref())
				{
					//Tool::destructor de([this]() {data->del_read_ref(); });
					at_scope_exit ase([this]() {data->del_read_ref(); });
					fun();
					return true;
				}
				return false;
			}
			~completeness_ref()
			{
				drop();
			}
		};

		namespace Assistant
		{
			template<typename T> struct mail_ts_base
			{
				std::mutex mutex;
				completeness_ref self_ref;
				completeness_ref func_ref;
				std::function<T> func;

			public:

				mail_ts_base(const completeness_ref& r) :self_ref(r) {}

				struct receiption_implement //: mail_receiption_base
				{
					completeness_ref ref;
					operator bool() const { return ref; }
					operator bool() { return ref; }
					receiption_implement() {}
					receiption_implement(const completeness_ref& wp) : ref(wp) {}
					//receiption_implement(const receiption&) = default;
					//receiption_implement(receiption&&) = default;
					//receiption_implement& operator=(const receiption_implement&) = default;
					//receiption_implement& operator=(receiption_implement&&) = default;
				};

				template<typename K>
				decltype(auto) bind(const completeness_ref& ref, K&& k)
				{
					/*
					std::lock_guard<decltype(mutex)> lg(mutex);
					static_assert(std::is_constructible<std::function<T>, K&&>::value, "mail unable bind this fun.");
					//std::unique_ptr<completeness<receiption_implement>> rec_ptr = std::make_unique<completeness<receiption_implement>>();
					func = std::forward<K>(k);
					func_ref = ref;
					return receiption{ self_ref };
					*/
				}

				template<typename K, typename ...AT>
				bool lock_if_capture(K&& k, AT&&... at)
				{
					/*
					std::lock_guard<decltype(mutex)> lg(mutex);
					return func_ref.lock_if(
						[&, this]() {
						Tool::statement_if < std::is_same<typename std::function<T>::result_type, void >::value  >
							(
								[](auto&& f, auto& a, auto&& ...para) {a(std::forward<decltype(para) && >(para)...); std::forward<decltype(f)&&>(f)(); },
								[](auto&& f, auto& a, auto&& ...para) {std::forward<decltype(f) && >(f)(a(std::forward<decltype(para) && >(para)...)); },
								std::forward<K>(k), *func, std::forward<AT>(at)...
								);
					}
					);
					*/
					return true;
				}

				template<typename ...AT>
				bool lock_if(AT&&... at)
				{
					/*
					std::lock_guard<decltype(mutex)> lg(mutex);
					return func_ref.lock_if(
						[&, this]() {
						(*func)(std::forward<AT>(at)...);
					}
					);
					*/
				}

				template<typename K, typename ...AT>
				bool try_lock_if_capture(K&& k, AT&&... at)
				{
					/*
					std::lock_guard<decltype(mutex)> lg(mutex);
					return func_ref.try_lock_if(
						[&, this]() {
						Tool::statement_if < std::is_same<typename std::function<T>::result_type, void >::value  >
							(
								[](auto&& f, auto& a, auto&& ...para) {a(std::forward<decltype(para) && >(para)...); std::forward<decltype(f)>(f)(); },
								[](auto&& f, auto& a, auto&& ...para) {std::forward<decltype(f) && >(f)(a(std::forward<decltype(para) && >(para)...)); },
								std::forward<K>(k), *func, std::forward<AT>(at)...
								);
					}
					);
					*/
				}
				template<typename ...AT>
				bool try_lock_if(AT&&... at)
				{
					/*
					std::lock_guard<decltype(mutex)> lg(mutex);
					return func_ref.try_lock_if(
						[&, this]() {
						(*func)(std::forward<AT>(at)...);
					}
					);
					*/
				}
			};
		}
		
		template<typename T> using mail_ts = completeness<Assistant::mail_ts_base<T>>;

	}
}
#include "thread_tool.hpp"