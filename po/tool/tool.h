#pragma once
#include <functional>
#include <string>
#include <memory>
#include <mutex>
namespace PO
{
	namespace Tool
	{
		class destructor
		{
			std::function<void(void)> func;
		public:
			template<typename T>  destructor(T&& t) :func(std::forward<T>(t)) {}
			~destructor() { func(); }
		};

		namespace Assistant
		{
			struct exist_flag_control
			{
				std::mutex mutex;
				size_t count = 0;
				size_t count_all = 0;
			};
		}

		class exist_flag
		{

			Assistant::exist_flag_control* ref = nullptr;
			friend class exist_flag_weak;

		public:

			exist_flag() {}

			exist_flag(const exist_flag& ef)
			{
				ref = ef.ref;
				if (ref != nullptr)
				{
					ref->mutex.lock();
					++ref->count;
					++ref->count_all;
					ref->mutex.unlock();
				}
			}

			void make_exist()
			{
				if (ref != nullptr)
				{
					ref->mutex.lock();
					if (ref->count_all == 1)
					{
						ref->mutex.unlock();
						delete ref;
					}
					else {
						--ref->count;
						--ref->count_all;
						ref->mutex.unlock();
					}
				}
				ref = new Assistant::exist_flag_control;
				++ref->count;
				++ref->count_all;
			}

			bool make_exist_if_not() 
			{
				if (ref == nullptr)
				{
					ref = new Assistant::exist_flag_control;
					++ref->count;
					++ref->count_all;
					return true;
				}
				return false;
			}

			bool resert() 
			{
				if (ref != nullptr)
				{
					ref->mutex.lock();
					if (ref->count_all == 1)
					{
						ref->mutex.unlock();
						delete ref;
					}
					else {
						--ref->count_all;
						--ref->count;
						ref->mutex.unlock();
					}
					ref = nullptr;
				}
			}

			exist_flag(exist_flag&& ef)
			{
				ref = ef.ref;
				ef.ref = nullptr;
			}

			~exist_flag()
			{
				if (ref != nullptr)
				{
					ref->mutex.lock();
					if (ref->count_all == 1)
					{
						ref->mutex.unlock();
						delete ref;
					}
					else {
						--ref->count;
						--ref->count_all;
						ref->mutex.unlock();
					}
				}
			}
		};

		class exist_flag_weak
		{

			Assistant::exist_flag_control* ref = nullptr;
			void reset(Assistant::exist_flag_control* r)
			{
				if (ref != nullptr)
				{
					ref->mutex.lock();
					if (ref->count_all == 1)
					{
						ref->mutex.unlock();
						delete ref;
					}
					else {
						--ref->count_all;
						ref->mutex.unlock();
					}
				}
				ref = r;
				if (ref != nullptr)
				{
					ref->mutex.lock();
					++ref->count_all;
					ref->mutex.unlock();
				}
			}
		public:

			exist_flag_weak() {}

			operator bool() const 
			{
				if (ref != nullptr)
				{
					ref->mutex.lock();
					bool value = ref->count != 0;
					ref->mutex.unlock();
					return value;
				}
				return false;
			}

			template<typename T>
			bool lock_if(T&& t)
			{
				if (ref != nullptr)
				{
					ref->mutex.lock();
					t();
					ref->mutex.unlock();
					return true;
				}
				return false;
			}

			exist_flag_weak(const exist_flag_weak& ef)
			{
				ref = ef.ref;
				if (ref != nullptr)
				{
					ref->mutex.lock();
					++ref->count_all;
					ref->mutex.unlock();
				}
			}

			exist_flag_weak(const exist_flag& ef)
			{
				ref = ef.ref;
				if (ref != nullptr)
				{
					ref->mutex.lock();
					++ref->count_all;
					ref->mutex.unlock();
				}
			}

			exist_flag_weak& operator=(const exist_flag& ef)
			{
				reset(ef.ref);
				return *this;
			}

			exist_flag_weak& operator=(const exist_flag_weak& ef)
			{
				reset(ef.ref);
				return *this;
			}

			exist_flag_weak& operator=(exist_flag_weak&& ef)
			{
				reset(nullptr);
				ref = ef.ref;
				ef.ref = nullptr;
				return *this;
			}

			exist_flag_weak(exist_flag_weak&& ef)
			{
				ref = ef.ref;
				ef.ref = nullptr;
			}

			~exist_flag_weak()
			{
				if (ref != nullptr)
				{
					ref->mutex.lock();
					if (ref->count_all == 1)
					{
						ref->mutex.unlock();
						delete ref;
					}
					else {
						--ref->count_all;
						ref->mutex.unlock();
					}
				}
			}
		};
	}
}

inline std::basic_string<char16_t> operator ""_str(const char16_t* str, size_t count)
{
	return std::basic_string<char16_t>(str);
}