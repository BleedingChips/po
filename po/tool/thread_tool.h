#pragma once
#include "tool.h"
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
				std::mutex mutex;
				completeness_state state = completeness_state::Construction;
				std::condition_variable cv;
				size_t ref = 0;
				size_t read_ref = 0;
			};

			class completeness_head
			{
				completeness_head_data_struct* data;
				completeness_head() :data(new completeness_head_data_struct)
				{
					std::lock_guard<decltype(data->mutex)> lg(data->mutex);
					data->state = completeness_state::Construction;
					++(data->ref);
				}
				completeness_head(const completeness_head&) :completeness_head() {}
				~completeness_head()
				{
					size_t res = 0;
					{
						std::lock_guard<decltype(data->mutex)> lg(data->mutex);
						res = --data->ref;
						data->state = completeness_state::Missing;
					}
					data->cv.notify_all();
					if (res == 0)
						delete data;
				}
				template<typename T> friend class Tool::completeness;
				friend class Tool::completeness_ref;
			};
		}

		template<typename T>
		class completeness_construct_selecter : public std::remove_reference_t<T>
		{
			template<typename ref, typename ...AT> completeness_construct_selecter(std::true_type, ref&& re, AT&& ...at) : std::remove_reference_t<T>(std::forward<ref>(re), std::forward<AT>(at)...) {}
			template<typename ref, typename ...AT> completeness_construct_selecter(std::false_type, ref&& re, AT&& ...at) : std::remove_reference_t<T>(std::forward<AT>(at)...) {}
		public:
			template<typename ...AT>  completeness_construct_selecter(Tool::completeness_ref&& cpr, AT&& ...at) : completeness_construct_selecter(
				std::integral_constant<bool,
				std::is_constructible<std::remove_reference_t<T>, completeness_ref, AT...>::value
				>(),
				std::move(cpr), std::forward<AT>(at)...
			) {}

			template<typename ...AT>  completeness_construct_selecter(const Tool::completeness_ref& cpr, AT&& ...at) : completeness_construct_selecter(
				std::integral_constant<bool,
				std::is_constructible<std::remove_reference_t<T>, completeness_ref, AT...>::value
				>(),
				cpr, std::forward<AT>(at)...
			) {}
		};

		template<typename T>
		class completeness :private Assistant::completeness_head, public std::remove_reference_t<T>
		{
			friend class completeness_ref;

			template<typename ref, typename ...AT> completeness(std::true_type, ref&& re, AT&& ...at) : std::remove_reference_t<T>(std::forward<ref>(re), std::forward<AT>(at)...) 
			{
				std::unique_lock<decltype(Assistant::completeness_head::data->mutex)> lg(Assistant::completeness_head::data->mutex);
				Assistant::completeness_head::data->state = Assistant::completeness_state::Ready;
				lg.unlock();
				Assistant::completeness_head::data->cv.notify_all();
			}
			template<typename ref, typename ...AT> completeness(std::false_type, ref&& re, AT&& ...at) : std::remove_reference_t<T>(std::forward<AT>(at)...) 
			{
				std::unique_lock<decltype(Assistant::completeness_head::data->mutex)> lg(Assistant::completeness_head::data->mutex);
				Assistant::completeness_head::data->state = Assistant::completeness_state::Ready;
				lg.unlock();
				Assistant::completeness_head::data->cv.notify_all();
			}

		public:

			completeness(completeness&& cp) : std::remove_reference_t<T>(std::move(cp))
			{
				std::unique_lock<decltype(Assistant::completeness_head::data->mutex)> lg(Assistant::completeness_head::data->mutex);
				Assistant::completeness_head::data->state = Assistant::completeness_state::Ready;
				lg.unlock();
				Assistant::completeness_head::data->cv.notify_all();
			}

			completeness(const completeness& cp) : std::remove_reference_t<T>(cp)
			{
				std::unique_lock<decltype(Assistant::completeness_head::data->mutex)> lg(Assistant::completeness_head::data->mutex);
				Assistant::completeness_head::data->state = Assistant::completeness_state::Ready;
				lg.unlock();
				Assistant::completeness_head::data->cv.notify_all();
			}

			template<typename ...AT> completeness(AT&& ...at) : completeness(  std::integral_constant<bool, std::is_constructible<std::remove_reference_t<T>, Assistant::completeness_head&, AT... >::value>(), static_cast<Assistant::completeness_head&>(*this), std::forward<AT>(at)...)
			{

			}

			~completeness()
			{
				std::unique_lock<decltype(Assistant::completeness_head::data->mutex)> ul(Assistant::completeness_head::data->mutex);
				Assistant::completeness_head::data->state = Assistant::completeness_state::Destruction;
				Assistant::completeness_head::data->cv.wait(ul, [this]() {return Assistant::completeness_head::data->read_ref == 0; });
				ul.unlock();
				data->cv.notify_all();
			}
		};

		class completeness_ref
		{
			mutable Assistant::completeness_head_data_struct* data;
		public:
			operator bool() const { return data != nullptr; }
			completeness_ref(const Assistant::completeness_head& cpd) :data(cpd.data)
			{
				std::unique_lock<decltype(data->mutex)> ul(data->mutex);
				++(data->ref);
			}
			completeness_ref() :data(nullptr) {}
			completeness_ref(const completeness_ref& cpd) :data(cpd.data)
			{
				if (data != nullptr)
				{
					std::unique_lock<decltype(data->mutex)> ul(data->mutex);
					++(data->ref);
				}
			}
			completeness_ref(completeness_ref&& cpf) :data(cpf.data)
			{
				cpf.data = nullptr;
			}
			template<typename T> completeness_ref(const completeness<T>& cp) : completeness_ref(static_cast<const Assistant::completeness_head&>(cp)) {}
			completeness_ref& operator=(const completeness_ref& cpf)
			{
				completeness_ref tem(cpf);
				drop();
				data = tem.data;
				if (data != nullptr)
				{
					std::unique_lock<decltype(data->mutex)> ul(data->mutex);
					++data->ref;
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
			template<typename T>
			completeness_ref& operator=(const completeness<T>&& cp)
			{
				return operator&=(static_cast<const Assistant::completeness_head&>(cp));
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
					std::unique_lock<decltype(data->mutex)> ul(data->mutex);
					data->cv.wait(ul, [this]() { return data->state != Assistant::completeness_state::Construction; });
					if (data->state == Assistant::completeness_state::Ready)
					{
						++data->read_ref;
						ul.unlock();
						destructor de([&]()
						{
							ul.lock();
							--data->read_ref;
							ul.unlock();
							data->cv.notify_one();
						});
						fun();
						return true;
					}
					else {
						auto res = --data->ref;
						ul.unlock();
						ul.release();
						if (res == 0)
							delete data;
						data = nullptr;
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
					if (data->state == Assistant::completeness_state::Ready)
					{
						++data->read_ref;
						data->mutex.unlock();
						destructor de([this]()
						{
							data->mutex.lock();
							--data->read_ref;
							data->mutex.unlock();
							data->cv.notify_one();
						});
						fun();
						return true;
					}
					else if (data->state != Assistant::completeness_state::Construction)
					{
						auto res = --data->ref;
						data->mutex.unlock();
						if (res == 0)
							delete data;
						data = nullptr;
					}else
						data->mutex.unlock();
				}
				return false;
			}
			~completeness_ref()
			{
				drop();
			}
		};

		template<typename T> class completeness_ptr
		{
			Tool::completeness_ref ref;

			

		public:
			T* ptr = nullptr;

			template<typename K>
			decltype(auto) lock_if(K&& k)
			{
				ref.lock_if(std::forward<K>(k));
			}

			template<typename K>
			decltype(auto) try_lock_if(K&& k)
			{
				ref.try_lock_if(std::forward<K>(k));
			}

			completeness_ptr() = default;

			operator bool() const { return ptr != nullptr && ref; }
			template<typename K>
			completeness_ptr(completeness<K>& c) :ref(c), ptr(&c) {}

			template<typename K>
			completeness_ptr(const completeness_ptr<K>& c) : ref(c.ref), ptr(c.ptr) {}

			template<typename K>
			completeness_ptr(completeness_ptr<K>&& c) : ref(std::move(c.ref)), ptr(c.ptr) { c.ptr = nullptr; }
			
			template<typename K>
			completeness_ptr& operator=(const completeness<K>& c)
			{
				ref = c;
				ptr = &c;
				return *this;
			}

			template<typename K>
			completeness_ptr& operator=(completeness_ptr<K>&& c)
			{
				ref = std::move(c);
				ptr = c.ptr;
				c.ptr = nullptr;
				return *this;
			}

			template<typename K>
			completeness_ptr& operator=(const completeness_ptr<K>& c)
			{
				ref = c;
				ptr = c.ptr;
				return *this;
			}
		};

	}
}
