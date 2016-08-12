#pragma once
#include <functional>
#include <string>
#include <memory>
#include <mutex>
#include <assert.h>
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
			class ref_count
			{
				size_t strong_ref;
				size_t all_ref;
			public:
				ref_count() :strong_ref(0), all_ref(0) {}
				void add_storng_ref() noexcept
				{
					++strong_ref;
					++all_ref;
					assert(strong_ref <= all_ref);
				}
				void add_weak_ref() noexcept
				{
					++all_ref;
					assert(strong_ref <= all_ref);
				}
				void sub_storng_ref() noexcept
				{
					assert(strong_ref > 0 && all_ref > 0);
					--strong_ref;
					--all_ref;
				}
				void sub_weak_ref() noexcept
				{
					assert(all_ref > 0);
					--all_ref;
					assert(strong_ref <= all_ref);
				}
				bool zero_all_ref() const noexcept { return all_ref == 0; }
				bool zero_storng_ref() const noexcept { return strong_ref == 0; }
			};

			template<typename lock_type = std::mutex>
			class ref_count_ts :ref_count
			{
				lock_type mutex;
			public:
				ref_count_ts() = default;
				void add_storng_ref() noexcept { std::lock_guard<lock_type> guard(mutex); ref_count::add_storng_ref(); }
				void add_weak_ref() noexcept { std::lock_guard<lock_type> guard(mutex); ref_count::add_weak_ref(); }
				void sub_storng_ref() noexcept { std::lock_guard<lock_type> guard(mutex); ref_count::sub_storng_ref(); }
				void sub_weak_ref() noexcept { std::lock_guard<lock_type> guard(mutex); ref_count::sub_weak_ref(); }
				
				template<typename fun_obj>
				void sub_weak_ref_if_zero_unlock(fun_obj&& fo) 
				{ 
					bool call = false;
					{
						std::lock_guard<lock_type> guard(mutex);
						ref_count::sub_weak_ref();
						call = ref_count::zero_all_ref();
					}
					if (call)
					{
						std::forward<fun_obj>(fo)();
					}
				}

				template<typename fun_obj, typename fun_obj2>
				void sub_storng_ref_if_zero_unlock(fun_obj&& fo, fun_obj2&& fo2) 
				{
					bool call_strong = false;
					bool call_weak = false;
					{
						std::lock_guard<lock_type> guard(mutex);
						ref_count::sub_storng_ref();
						call_strong = ref_count::zero_storng_ref();
						call_weak = ref_count::zero_all_ref();
					}
					if (call_strong) std::forward<fun_obj>(fo)();
					if (call_weak) std::forward<fun_obj2>(fo2)();
				}


				bool zero_all_ref() const noexcept { std::lock_guard<lock_type> guard(mutex); return ref_count::zero_all_ref(); }
				bool zero_storng_ref() const noexcept { std::lock_guard<lock_type> guard(mutex); ref_count::zero_storng_ref(); }
				template<typename func_obj>
				void lock(fun_obj&& fo)
				{
					std::lock_guard<lock_type> guard(mutex);
					fo(static_cast<ref_count&>(*this));
				}
			};
		}

		class exist_flag
		{
			Assistant::ref_count* rf;
			friend class exist_flag_weak;
		public:
			operator bool() const { return rf != nullptr && !rf->zero_storng_ref(); }
			exist_flag() : rf(nullptr) { }
			exist_flag(exist_flag&& ef) : rf(ef.rf) { ef.rf = nullptr; }
			~exist_flag() 
			{ 
				if (rf != nullptr) 
				{
					rf->sub_storng_ref();
				} 
			}
		};

		class exist_flag_weak
		{
			Assistant::ref_count* rf;
		public:
			exist_flag_weak() : rf(nullptr) { }
			exist_flag_weak(const exist_flag& ef) : rf(ef.rf) { if (rf != nullptr) rf->add_weak_ref(); }
			exist_flag_weak(exist_flag_weak&& ef) : rf(ef.rf) { ef.rf = nullptr; }
			exist_flag_weak& operator=(const exist_flag& ef)
			{
				if (rf != nullptr)
				{
					rf->sub_weak_ref();
					if (rf->zero_all_ref())
						delete rf;
				}
				rf = ef.rf;
				if (rf != nullptr)
				{
					rf->add_weak_ref();
				}
			}
			exist_flag_weak& operator=(const exist_flag_weak& ef)
			{
				if (rf != nullptr)
				{
					rf->sub_weak_ref();
					if (rf->zero_all_ref())
						delete rf;
				}
				rf = ef.rf;
				if (rf != nullptr)
				{
					rf->add_weak_ref();
				}
			}
			~exist_flag_weak()
			{
				if (rf != nullptr)
				{
					rf->sub_weak_ref();
					if (rf->zero_all_ref())
						delete rf;
				}
			}
			operator bool() const { return rf != nullptr && !rf->zero_all_ref(); }
		};


	}
}

inline std::basic_string<char16_t> operator ""_str(const char16_t* str, size_t count)
{
	return std::basic_string<char16_t>(str);
}