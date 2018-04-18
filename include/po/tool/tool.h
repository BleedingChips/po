#pragma once
#include "tmpcall.h"
#include <assert.h>
#include <atomic>
#include <mutex>
#include <typeindex>
#include <optional>
namespace PO
{
	namespace Tool
	{
		namespace Implement
		{
			template<typename T> struct base_value_inherit {
				T data;
				base_value_inherit() noexcept {}
				base_value_inherit(T t) noexcept : data(t) {}
				operator T&() noexcept { return data; }
				operator T() const noexcept  { return data; }
				base_value_inherit& operator=(T t) noexcept { data = t; return *this; }
			};
		}

		template<typename T> using inherit_t = std::conditional_t<
			std::is_arithmetic<T>::value,
			Implement::base_value_inherit<T>, T
		>;

		/* static_mapping */
		namespace Implement
		{
			template<typename equal_ope, typename less_ope, size_t s, size_t o, typename ...input>
			struct static_mapping_implement
			{
				template<typename T, typename equal_func, typename default_func>
				decltype(auto) operator()(T&& t, equal_func&& ef, default_func&& df) const
				{
					using pick_type = TmpCall::call<TmpCall::append<input...>, TmpCall::select_index<std::integral_constant<size_t, (s + o) / 2>>, TmpCall::self>;
					if (equal_ope{}(Tmp::itself<pick_type>(), std::forward<T>(t)))
					{
						return ef(Tmp::itself<pick_type>());
					}
					else if (less_ope{}(Tmp::itself<pick_type>(), std::forward<T>(t)))
					{
						return static_mapping_implement<equal_ope, less_ope, s, (s + o) / 2, input...>{}(std::forward<T>(t), std::forward<equal_func>(ef), std::forward<default_func>(df));
					}
					else
					{
						return static_mapping_implement<equal_ope, less_ope, (s + o) / 2 + 1, o, input...>{}(std::forward<T>(t), std::forward<equal_func>(ef), std::forward<default_func>(df));
					}
				}
			};

			template<class equal_ope, class less_ope, size_t s, typename ...input>
			struct static_mapping_implement<equal_ope, less_ope, s, s, input...>
			{
				template<typename T, typename equal_func, typename default_func>
				decltype(auto) operator()(T&& t, equal_func&& ef, default_func&& df) const
				{
					return df();
				}
			};
		}

		template<typename equal_ope, typename less_ope>
		struct make_static_mapping
		{
			template<typename ...input> struct in
			{
				using type = Implement::static_mapping_implement<equal_ope, less_ope, 0, sizeof...(input), input...>;
			};
		};

		template<class equal_ope, class less_ope, typename ...input>
		using static_mapping_t = Implement::static_mapping_implement<less_ope, equal_ope, 0, sizeof...(input), input...>;

		template<typename T> class aligned_class
		{
			char storage[sizeof(T) + alignof(T) - 1];
			void* get_pointer() const {
				size_t space = sizeof(T) + alignof(T)-1;
				void* ptr = storage;
				std::align(alignof(T), sizeof(T), ptr, space);
				return ptr;
			}

		public:

			operator T& () { return *static_cast<T*>(get_pointer());  }
			operator const T& () const { return *static_cast<const T*>(get_pointer()); }

			T* operator->() { return static_cast<T*>(get_pointer()); }
			const T* operator->() const { return static_cast<const T*>(get_pointer()); }

			template<typename ...AT> aligned_class(AT&& ...at) {
				new (get_pointer()) T(std::forward<AT>(at)...);
			}

			~aligned_class()
			{
				static_cast<T*>(get_pointer())->~T();
			}
		};

		namespace Implement
		{
			template<size_t s, typename ...T> struct max_align_implement
			{
				static constexpr size_t value = s;
			};

			template<size_t s, typename K, typename ...T> struct max_align_implement<s, K, T...>
			{
				static constexpr size_t value = max_align_implement<(s > alignof(K) ? s : alignof(K)), T...>::value;
			};
		}

		template<typename ...T> struct max_align : public  Implement::max_align_implement<0, T...> {};

		template<typename value_type> struct stack_list
		{
			value_type& type_ref;
			stack_list* front;
			stack_list(value_type& ref, stack_list* f = nullptr) noexcept : type_ref(ref), front(f) {}
		};

		template<typename value_type, typename callable_function, typename ...other_type> decltype(auto) make_stack_list(callable_function&& ca, stack_list<value_type>* sl = nullptr) noexcept
		{
			return ca(sl);
		}

		template<typename value_type, typename callable_function, typename ...other_type> decltype(auto) make_stack_list(callable_function&& ca, stack_list<value_type>* sl, value_type& ref, other_type&& ...ot) noexcept
		{
			stack_list<value_type> temporary{ ref, sl };
			return make_stack_list<value_type>(std::forward<callable_function>(ca), &temporary, std::forward<other_type>(ot)...);
		}

		template<typename value_type, typename callable_function, typename ...other_type> decltype(auto) make_stack_list(callable_function&& ca, value_type& ref, other_type&& ...ot) noexcept
		{
			stack_list<value_type> temporary{ ref };
			return make_stack_list<value_type>(std::forward<callable_function>(ca), &temporary, std::forward<other_type>(ot)...);
		}

		template<template<typename ...> class implement_t> class deflection_ptr
		{
			const std::type_index original_info;

		public:

			deflection_ptr(const std::type_index& original) noexcept : original_info(original) {}
			const std::type_index& id() const noexcept { return original_info; }
			template<typename T> bool is() const noexcept {
				using type = std::decay_t<T>;
				return original_info == typeid(type);
			}

			template<typename T> std::decay_t<T>& cast() {
				using type = std::decay_t<T>;
				return static_cast<type&>(static_cast<implement_t<type>&>(*this));
			}

			template<typename T> const std::decay_t<T>& cast() const {
				using type = std::decay_t<T>;
				return static_cast<const type&>(static_cast<const implement_t<type>&>(*this));
			}

			template<typename T, typename ...AT> bool cast(T&& t, AT&& ...at) noexcept
			{
				using funtype = Tmp::pick_func<typename Tmp::degenerate_func<Tmp::extract_func_t<T>>::type>;
				static_assert(funtype::size == 1, "only receive one parameter");
				using true_type = std::decay_t<typename funtype::template out<Tmp::itself>::type>;
				//static_assert(std::is_base_of<base_interface, true_type>::value, "need derived form base_interface.");
				if (is<true_type>())
					return (t(cast<true_type>(), std::forward<AT>(at)...), true);
				return false;
			}
			virtual ~deflection_ptr() = default;

		};

		class atomic_reference_count
		{
			std::atomic_size_t ref = 0;

		public:

			void wait_touch(size_t targe_value) const noexcept;

			bool try_add_ref() noexcept;

			void add_ref() noexcept
			{
				assert(static_cast<std::ptrdiff_t>(ref.load(std::memory_order_relaxed)) >= 0);
				ref.fetch_add(1, std::memory_order_relaxed);
			}

			bool sub_ref() noexcept
			{
				assert(static_cast<std::ptrdiff_t>(ref.load(std::memory_order_relaxed)) >= 0);
				return ref.fetch_sub(1, std::memory_order_relaxed) == 1;
			}

			atomic_reference_count() noexcept : ref(0) {}
			atomic_reference_count(const atomic_reference_count&) = delete;
			atomic_reference_count& operator= (const atomic_reference_count&) = delete;
			~atomic_reference_count() { assert(ref.load(std::memory_order_relaxed)==0); }
		};

		template<typename type, typename deleter_type = std::default_delete<type>> 
		class intrusive_ptr
		{
			type* data;
			deleter_type del;
		public:
			const deleter_type& deleter() const { return del; }
			bool operator== (const intrusive_ptr& ip) const noexcept { return data == ip.data; }
			bool operator!= (const intrusive_ptr& ip) const noexcept { return data != ip.data; }
			operator bool() const noexcept { return data != nullptr; }
			intrusive_ptr(type* t, deleter_type dt = deleter_type{}) : data(t), del(dt) {
				if (data != nullptr) {
					data->add_ref();
				}
			}
			intrusive_ptr() noexcept  : data(nullptr) {}
			intrusive_ptr(const intrusive_ptr& ip) noexcept : data(ip.data), del(ip.del) {
				if (data != nullptr)
					data->add_ref();
			}

			template<typename other_type, typename = std::void_t<std::enable_if_t<std::is_base_of_v<type, other_type>>>>
			
			/*
			intrusive_ptr(const intrusive_ptr<other_type>& ip) noexcept : data(ip.data), del(deleter_type{}) {
				if (data != nullptr)
					data->add_ref();
			}*/

			intrusive_ptr(intrusive_ptr&& ip) noexcept : data(ip.data), del(std::move(ip.del))
			{
				ip.data = nullptr;
			}
			void reset() noexcept
			{
				if (data != nullptr && data->sub_ref())
				{
					del(data);
				}
				data = nullptr;
			}
			~intrusive_ptr() {
				if (data != nullptr && data->sub_ref())
				{
					del(data);
				}
			}
			intrusive_ptr& operator=(intrusive_ptr ip)  noexcept
			{
				reset();
				data = ip.data;
				del = ip.del;
				ip.data = nullptr;
				return *this;
			}
			/*
			template<typename cast_type> intrusive_ptr<cast_type> cast() const {
				return intrusive_ptr<cast_type> { data, deleter };
			}
			*/
			operator type* () noexcept { return data; }
			operator const type* () const noexcept { return data; }
			type* operator->() noexcept { return data; }
			const type* operator->() const noexcept { return data; }
			type& operator* () noexcept { return *data; }
			const type& operator*() const noexcept { return *data; }
			template<typename other_type> operator intrusive_ptr<other_type>() const noexcept { return intrusive_ptr<other_type>{data }; }
		};

		template<typename T> class stack_ref;
		template<typename T> class stack_ref_ptr;

		struct inherit_construct_t {};

		namespace Implement
		{
			struct stack_ref_control_block
			{

				enum class State : int8_t
				{
					Construction,
					Ready,
					Destruction,
					Missing
				};

				std::atomic<State> state;
				atomic_reference_count using_count;
				atomic_reference_count ref;

				stack_ref_control_block() : state(State::Missing) {}

				//必须保证第一个被调用
				void start_construction() noexcept { state.store(State::Construction, std::memory_order::memory_order_release); }
				void finish_construction() noexcept { state.store(State::Ready, std::memory_order::memory_order_release); }
				void start_destruction() noexcept;
				void finish_destruction() noexcept { state.store(State::Missing, std::memory_order::memory_order_release); }


				bool try_add_ref() noexcept { return ref.try_add_ref(); }
				void add_ref() noexcept { return ref.add_ref(); }
				bool sub_ref() noexcept { return ref.sub_ref(); }

				// avalible: if it can be call, means state == Ready; need_wait: means state == Construction;
				void try_add_using_ref(bool& avalible, bool& need_wait) noexcept;
				//call if try_add_using_ref -> avalible is true!
				void sub_using_ref() noexcept;

			};

			class stack_ref_head
			{
				intrusive_ptr<Implement::stack_ref_control_block> block;
			public:
				operator intrusive_ptr<Implement::stack_ref_control_block>() { return block; }
				operator const intrusive_ptr<Implement::stack_ref_control_block>() const { return block; }
				stack_ref_head();
				virtual ~stack_ref_head();
				void finish_construction() { block->finish_construction(); }
				void start_destruction() { block->start_destruction(); }
			};
		}

		template<typename T> class stack_ref_ptr
		{
			intrusive_ptr<Implement::stack_ref_control_block> block;
			T* ref_ptr;
		public:
			stack_ref_ptr() noexcept : ref_ptr(nullptr) {}
			stack_ref_ptr(intrusive_ptr<Implement::stack_ref_control_block> src, T* ref_ptr) noexcept : block(src), ref_ptr(ref_ptr) {}
			stack_ref_ptr(const stack_ref_ptr& srp) noexcept : stack_ref_ptr(srp.block, srp.ref_ptr) {}

			stack_ref_ptr(stack_ref_ptr&& srp) noexcept : block(std::move(srp.block)), ref_ptr(ref_ptr) {}

			template<typename T>
			stack_ref_ptr(const stack_ref_ptr<T>& srp) noexcept : block(srp.block), ref_ptr(ref_ptr) {}

			template<typename T>
			stack_ref_ptr(stack_ref_ptr<T>&& srp) noexcept : block(std::move(srp.block)), ref_ptr(ref_ptr) {}

			stack_ref_ptr& operator=(stack_ref_ptr srp) noexcept
			{
				block = std::move(srp.block);
				ref_ptr = srp.ref_ptr;
				return *this;
			}

			void reset() { block.reset(); }

			template<typename callable_object>
			bool try_ref(callable_object&& obj) noexcept(noexcept(obj(*ref_ptr)))
			{
				bool avalible, need_wait;
				if (block && (block->try_add_using_ref(avalible, need_wait), avalible))
				{
					try {
						obj(static_cast<T&>(*ref_ptr));
					}
					catch (...) {
						block->sub_using_ref();
						throw;
					}
					block->sub_using_ref();
					return true;
				}
				return false;
			}

			template<typename callable_object>
			bool try_ref(callable_object&& obj, bool& need_wait) noexcept(noexcept(obj(*ref_ptr)))
			{
				bool avalible;
				if (block && (block->try_add_using_ref(avalible, need_wait), avalible))
				{
					try {
						obj(static_cast<T&>(*ref_ptr));
					}
					catch (...) {
						block->sub_using_ref();
						throw;
					}
					block->sub_using_ref();
					return true;
				}
				return false;
			}

			template<typename callable_object>
			bool try_ref(callable_object&& obj) const noexcept(noexcept(obj(*ref_ptr)))
			{
				bool avalible, need_wait;
				if (block && (block->try_add_using_ref(avalible, need_wait), avalible))
				{
					try {
						obj(static_cast<const T&>(*ref_ptr));
					}
					catch (...) {
						block->sub_using_ref();
						throw;
					}
					block->sub_using_ref();
					return true;
				}
				return false;
			}

			template<typename callable_object>
			bool try_ref(callable_object&& obj, bool& need_wait) const noexcept(noexcept(obj(*ref_ptr)))
			{
				bool avalible;
				if (block && (block->try_add_using_ref(avalible, need_wait), avalible))
				{
					try {
						obj(static_cast<const T&>(*ref_ptr));
					}
					catch (...) {
						block->sub_using_ref();
						throw;
					}
					block->sub_using_ref();
					return true;
				}
				return false;
			}

		};

		template<typename T> class stack_ref : protected Implement::stack_ref_head, public inherit_t<T>
		{
		public:
			operator stack_ref_ptr<T>() { return stack_ref_ptr<T>{  static_cast<Implement::stack_ref_control_block*>(static_cast<Implement::stack_ref_head&>(*this)), static_cast<T*>(this) }; }
			template<typename ...AT> stack_ref(AT&& ...at) : inherit_t<T>(std::forward<AT>(at)...) { Implement::stack_ref_head::finish_construction(); }
			~stack_ref() { Implement::stack_ref_head::start_destruction(); }
		};

		template<typename callable_object_t, typename mutex_t> decltype(auto) simple_lock(callable_object_t&& cj, mutex_t& m) {
			std::lock_guard<std::remove_extent_t<mutex_t>> lg(m);
			return std::forward<callable_object_t>(cj)();
		}

		template<typename callable_object_t, typename mutex_t, typename ...o_mutex_t> decltype(auto) simple_lock(callable_object_t&& cj, mutex_t& m, o_mutex_t& ...om) {
			std::lock_guard<std::remove_extent_t<mutex_t>> lg(m);
			return simple_lock(std::forward<callable_object_t>(cj), om...);
		}

		template<typename T> struct replace_void { using type = T; };
		template<> struct replace_void<void> { using type = Tmp::itself<void>; };

		template<typename T, typename mutex_t = std::mutex> class scope_lock
		{
			T data;
			mutable mutex_t mutex;
		public:
			template<typename ...construction_para>  scope_lock(construction_para&& ...cp) : data(std::forward<construction_para>(cp)...) {}
			using type = T;
			using mutex_type = mutex_t;

			using type = T;

			T exchange(T t) noexcept {
				std::lock_guard<mutex_t> lg(mutex);
				T tem(std::move(data));
				data = std::move(t);
				return tem;
			}

			T copy() const noexcept {
				std::lock_guard<mutex_t> lg(mutex);
				return data;
			}

			T move() && noexcept {
				std::lock_guard<mutex_t> lg(mutex);
				T tem(std::move(data));
				return tem;
			}

			scope_lock& equal(T t) noexcept
			{
				std::lock_guard<mutex_t> lg(mutex);
				data = std::move(t);
				return *this;
			}

			template<typename callable_object> decltype(auto) lock(callable_object&& obj) noexcept(noexcept(std::forward<callable_object>(obj)(static_cast<T&>(data))))
			{
				std::lock_guard<mutex_t> lg(mutex);
				return std::forward<callable_object>(obj)(static_cast<T&>(data));
			}
			template<typename callable_object> decltype(auto) lock(callable_object&& obj) const noexcept(noexcept(std::forward<callable_object>(obj)(static_cast<const T&>(data))))
			{
				std::lock_guard<mutex_t> lg(mutex);
				return std::forward<callable_object>(obj)(static_cast<const T&>(data));
			}

			template<typename callable_object>  auto try_lock(callable_object&& obj)
				noexcept(noexcept(std::forward<callable_object>(obj)(static_cast<T&>(data))))
				-> std::conditional_t<
					std::is_void_v<decltype(std::forward<callable_object>(obj)(static_cast<T&>(data)))>,
					bool,
					std::optional<decltype(std::forward<callable_object>(obj)(static_cast<T&>(data)))>
				>
			{
				if (mutex.try_lock())
				{
					std::lock_guard<mutex_t> lg(mutex, std::adopt_lock);
					if constexpr(std::is_void_v<decltype(std::forward<callable_object>(obj)(static_cast<T&>(data)))>)
					{
						return true;
					}else
						return{ std::forward<callable_object>(obj)(static_cast<T&>(data)) };
				}
				if constexpr(std::is_void_v<decltype(std::forward<callable_object>(obj)(static_cast<T&>(data)))>)
				{
					return false;
				}
				else
					return {};
			}

			template<typename other_mutex_t, typename other_type, typename callable_object> auto lock_with(scope_lock<other_type, other_mutex_t>& sl, callable_object&& obj)
				noexcept(noexcept(std::forward<callable_object>(obj)(data, sl.data)))
				-> decltype(std::forward<callable_object>(obj)(data, sl.data))
			{
				std::lock(mutex, sl.mutex);
				std::lock_guard<mutex_t> lg(mutex, std::adopt_lock);
				std::lock_guard<other_mutex_t> lg2(sl.mutex, std::adopt_lock);
				return std::forward<callable_object>(obj)(data, sl.data);
			}

		};

	} 
}