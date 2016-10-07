#pragma once
#include "tool\thread_tool.h"
#include <thread>
#include <iostream>
#include "event\event.h"
#include <future>
#include <atomic>
#include <deque>
#include <memory>
#include <list>

/*

struct event
[must
carry the event infomation from form or input

should provide event().
should provide opertor=(const event&).
]



struct form;
[must

should provide bool is_avalible() to show whether this form should close, call in same thread.
should provide void bind_event(const PO::Tool::completeness_ref&, std::function<bool(event&)>&&) or bind_event(std::function<bool(event&)>&&)to get the event function from form_append.

optional provide bool check_tick(duration) to return weather enter plugin tick and form tick function, call in same thread.
optional provide void tick(duration) call in each tick, call in same thread.

each form will be constructed in difference thread.
should be close form in destruction.
]



struct view_outside;
[optional

should provide view_outside::view_outside(form&), this will call in the same thread of form.
should provide view_outside::view_outside(const view_outside&) or view_outside::view_outside(view_outside&), this will call in diference thread, may call many time.

viewer's life time is usually longer then form.
usually call in diference thread.
]

struct view_inside;
[optional

should provide view_inside::view_inside(form&).
should provide view_inside::view_inside(const view_inside&) or view_inside::view_inside(view_inside&).

construct and use in the same thread of form.
viewer's life time is same as form.
]

*/

namespace PO
{

	using duration = std::chrono::duration<long long, std::ratio<1, 1000>>;

	namespace Assistant
	{
		template<typename T, typename = void> struct frame_have_form : std::false_type {};
		template<typename T> struct frame_have_form<T, 
			std::void_t<decltype(&T::form::is_avalible)>
		> :std::true_type {};

		template<typename T, typename = void> struct frame_have_view_outside : std::false_type {};
		template<typename T> struct frame_have_view_outside
			<
				T,
				std::enable_if_t< 
					std::is_constructible<typename T::view_outside, typename T::form&>::value &&
					std::is_constructible<typename T::view_outside, typename T::view_outside& >::value
				>
			> :std::true_type {};

		template<typename T, typename = void> struct frame_have_view_inside : std::false_type {};
		template<typename T> struct frame_have_view_inside
			<
				T,
				std::enable_if_t<
					std::is_constructible<typename T::view_inside, typename T::form&>::value &&
					std::is_constructible<typename T::view_inside, typename T::view_inside& >::value
				>
			> :std::true_type {};

		template<typename T, typename = void> struct frame_form_have_check_tick : std::false_type {};
		template<typename T> struct frame_form_have_check_tick<T, 
			std::void_t<std::integral_constant<bool (T::*)(duration), &T::check_tick> >
		> :std::true_type {};

		template<typename T, typename = void> struct frame_form_have_tick : std::false_type {};
		template<typename T> struct frame_form_have_tick<T,
			std::void_t<std::integral_constant<void (T::*)(duration), &T::tick> >
		> :std::true_type {};

	}

	template<typename T> using frame_legal =
		std::integral_constant<bool,
			Assistant::frame_have_form<T>::value 
		>;

	template<typename T> struct frame_static_assert
	{
		static_assert(Assistant::frame_have_form<T>::value, "frame need form");
	};

	namespace Assistant
	{

		template<bool, template<typename...> class picker, typename input> class frame_type_picker : public picker<input>
		{
			using type = picker<input>;
		public:
			using type::type;
		};

		template<template<typename...> class picker, typename input> struct frame_type_picker<false, picker, input> 
		{
			template<typename ...AK> frame_type_picker(AK&& ...ak) {}
		};

		template<typename T> using frame_view_inside = typename T::view_inside;
		template<typename T> using frame_view_outside = typename T::view_outside;
	}

	template<typename T> struct frame : frame_static_assert<T>
	{
		using form = typename T::form;
		static constexpr bool check_tick = Assistant::frame_form_have_check_tick<form>::value;
		static constexpr bool tick = Assistant::frame_form_have_tick<form>::value;

		using view_inside = Assistant::frame_type_picker<Assistant::frame_have_view_inside<T>::value, Assistant::frame_view_inside, T>;
		using view_outside = Assistant::frame_type_picker<Assistant::frame_have_view_outside<T>::value, Assistant::frame_view_outside, T>;
	};

	

	/* extern class */
	namespace Assistant
	{
		template<typename T> struct plugin_ptr;
		template<typename T, typename K> class plugin_implement;
		struct form_ptr;
		template<typename T> struct form_append;
	}
	template<typename T> struct pre_inital;
	template<typename T> class inital;
	template<typename T> class ticker;
	template<typename T> class viewer;



	/* define class */
	namespace Assistant
	{
		template<typename T> struct plugin_ptr
		{
			bool use_tick = true;
			virtual void tick_implement(ticker<T>&) = 0;
			bool avalible = true;
			operator bool() const { return avalible; }
			void tick(ticker<T>& ti)
			{
				if (use_tick)
					tick_implement(ti);
			}
			virtual ~plugin_ptr() {}
		};

		template<typename K, typename T, typename = void> struct able_to_call_tick :std::false_type {};
		template<typename K, typename T> struct able_to_call_tick<K, T, std::void_t<decltype(std::declval<T>().tick(std::declval<ticker<K>>()))>> :std::true_type {};

		template<typename T, typename K> class plugin_implement : public plugin_ptr<T>, public K
		{
			template<typename ...AK> plugin_implement(std::true_type, AK&& ...ak) :K(std::forward<AK>(ak)...) {}
			template<typename IO, typename ...AK> plugin_implement(std::false_type, IO&&, AK&& ...ak) : K(std::forward<AK>(ak)...) {}
		public:
			template<typename ...AK>
			plugin_implement(const Tool::completeness_ref& cpr, pre_inital<T>&, AK&& ...ak);
			void tick_implement(ticker<T>& t) override
			{
				Tool::statement_if<Assistant::able_to_call_tick<T, K>::value>
					(
						[&](auto&& plugin) { plugin.tick(t); },
						[this](auto&& plugin) {this->use_tick = false; },
						static_cast<K&>(*this)
						);
			}
		};

		template<typename T, typename K> using plugin_final = Tool::completeness<plugin_implement<T, K>>;

		struct form_ptr
		{
			bool avalible;
			duration frame_duration = duration(10);
			std::thread logic_form_thread;
			std::atomic_bool force_exist_form;
			Tool::completeness_ref ref;
			form_ptr(const Tool::completeness_ref& cr):ref(cr) {}
			virtual ~form_ptr();
			template<typename T, typename ...AK> auto create_window(AK&& ...);
			void force_stop() { force_exist_form = true; }
		};

		template<typename T> struct form_append : public frame<T>::form
		{
			pre_inital<T> pe;

			std::mutex pim;
			std::list<std::unique_ptr<plugin_ptr<T>>> inilizered_plugin_list;

			std::mutex pifm;
			std::condition_variable cv;
			std::function<std::unique_ptr<plugin_ptr<T>>(void)> pif;

			std::list<std::unique_ptr<plugin_ptr<T>>> plugin_list;


			using form_type = typename frame<T>::form;
			template<typename ...AK> form_append(form_ptr& fi, std::true_type, AK&& ...ak) :
				form_type(std::forward<AK>(ak)...), pe{ *this, fi } {}
			template<typename ...AK> form_append(form_ptr& fi, std::false_type, Tool::completeness_ref&& cpr, AK&& ...ak) :
				form_type(std::forward<AK>(ak)...) , pe{ *this, fi } {}
		public:
			template<typename ...AK> form_append(Tool::completeness_ref&& ref, form_ptr& fi, AK&&... ak):
				form_append(fi,
					std::integral_constant<bool, std::is_constructible<form_type, Tool::completeness_ref&&, AK...>::value>(),
					std::move(ref), std::forward<AK>(ak)...)
			{}

			template<typename K, typename ...AK> decltype(auto) create_plugin_inside(AK&& ...ak);
			template<typename K, typename ...AK> decltype(auto) create_plugin_outside(AK&& ...ak);
			void run_tick(ticker<T>&);
		};

		template<typename T> using form_packet = Tool::completeness<form_append<T>>;

		using form_instance = Tool::completeness<form_ptr>;
	}


	template<typename T> struct pre_inital
	{
		Assistant::form_append<T>& append;
		Assistant::form_ptr& instance;
	};

	template<typename T> class inital : public frame<T>::view_inside
	{
		using view = typename frame<T>::view_inside;
		Tool::completeness_ref self_ref;
		pre_inital<T> pre;
	public:
		operator Tool::completeness_ref& () { return self_ref; }
		inital(const Tool::completeness_ref& ref, pre_inital<T>& pi) :view( pre.append ), self_ref(ref), pre(pi) {}
		inital(const inital& i) : frame<T>::view_inside(i), self_ref(i.self_ref), pre(i.pre) {  }
		template<typename K, typename ...AK>
		decltype(auto) create_plugin(AK&& ...ak)
		{
			
			pre.append.template create_plugin_inside<K>(std::forward<AK>(ak)...);
		}
	};

	template<typename T> class ticker : public frame<T>::view_inside
	{
		duration dua;
		Assistant::form_ptr& ptr;
		Assistant::form_append<T>& append;
		void set_time(duration d) { dua = d; }
		friend struct Assistant::form_ptr;
	public:
		duration get_time() const { return dua; }
		ticker(const ticker&) = default;
		ticker(Assistant::form_ptr& fp, Assistant::form_append<T>& fa) :frame<T>::view_inside(fa), ptr(fp), append(fa) {}
	};

	template<typename T> class viewer : public frame<T>::view_outside
	{
		Tool::completeness_ref ptr_ref;
		Assistant::form_ptr& ptr;
		Tool::completeness_ptr<Assistant::form_append<T>> append_ref;
	public:
		viewer(const Tool::completeness_ref& cr, Assistant::form_ptr& fp, Tool::completeness<Assistant::form_append<T>>& cp ):
			frame<T>::view_outside(cp), ptr_ref(cr), ptr(fp), append_ref(cp) {}
		viewer(const viewer&) = default;
		template<typename K,typename ...AK>
		decltype(auto) create_plugin(AK&& ...ak)
		{
			append_ref.lock_if(
				[&,this]()
			{
				auto data( append_ref.ptr );
				data->template create_plugin_outside<K>(std::forward<AK>(ak)...);
			}
			);
		}
	};


	/*
	namespace Assistant
	{

		struct form_ptr;
		template<typename T> struct form_plugin;

		template<typename T> struct base_view_data
		{
			Assistant::form_ptr* ptr = nullptr;
			typename frame<T>::form* form = nullptr;
			Tool::completeness_ref plugin_cpr;
			Assistant::form_plugin<T>* plugin = nullptr;
		public:
			base_view_data(Assistant::form_ptr* p, typename frame<T>::form* f, Tool::completeness_ref c, Assistant::form_plugin<T>* fp) : ptr(p), form(f), plugin_cpr(c), plugin(fp) {}
			base_view_data(const base_view_data&) = default;
			base_view_data() = default;
			base_view_data& operator= (const base_view_data&) = default;

		};

		

		template<typename T> struct plugin_ptr
		{
			bool use_tick = true;
			virtual void tick_implement(base_view_data<T>& du, duration dua) = 0;
			bool avalible = true;
			operator bool() const { return avalible; }
			void tick(base_view_data<T>& tv, duration dua)
			{
				if (use_tick)
					tick_implement(tv,dua);
			}
			virtual ~plugin_ptr() {}
		};

		

		template<typename T, typename K> using plugin_final = Tool::completeness<plugin_implement<T, K>>;

		template<typename T, typename K> struct base_plugin_view_data
		{
			Tool::completeness_ref plugin_ref;
			plugin_implement<T, K>* ptr;
		};

		template<typename T> struct form_plugin
		{

			std::mutex pifm;
			std::condition_variable cv;
			std::function<std::unique_ptr<plugin_ptr<T>>(void)> pif;

			std::list<std::unique_ptr<plugin_ptr<T>>> inilizered_plugin_list;
			std::list<std::unique_ptr<plugin_ptr<T>>> plugin_list;

			std::thread::id ID;

			form_plugin() :ID(std::this_thread::get_id()) {}

			template<typename K, typename ...AK>
			decltype(auto) create_plugin(AK&& ...ak)
			{
				if (std::this_thread::get_id() == ID)
				{
					auto ptr = std::make_unique<plugin_final<T, K>>(std::forward<AK>(ak)...);
					plugin_final<T, K>* pi = ptr.get();
					std::lock_guard<decltype(pim)> ld(pim);
					inilizered_plugin_list.push_back(std::move(ptr));
					return base_plugin_view_data<T, K>{ *pi, pi };
				}
				else {
					std::promise<base_plugin_view_data<T, K>> pro;
					auto fur = pro.get_future();
					std::unique_lock<decltype(pim)> ld(pim);
					cv.wait(ld, [this]() {return !static_cast<bool>(pif); });
					pif = [&]() ->std::unique_ptr<plugin_ptr<T>>
					{
						auto ptr = std::make_unique<plugin_final<T, K>>(std::forward<AK>(ak)...);
						plugin_final<T, K>* pi = ptr.get();
						pro.set_value(base_plugin_view_data<T, K>{*pi, pi});
						return std::move(ptr);
					};
					ld.unlock();
					fur.wait();
					return fur.get();
				}
			}

			void run_tick(base_view_data<T>& tv, duration dua)
			{
				std::unique_lock<decltype(pifm)> ld(pifm);
				while (pif)
				{
					std::unique_lock<decltype(pim)> ld(pim);
					inilizered_plugin_list.push_back(pif());
					pif = std::function<std::unique_ptr<plugin_ptr<T>>(void)>();
					ld.unlock();
					cv.notify_one();
					std::this_thread::yield();
					ld.lock();
				}
				ld.unlock();

				{
					std::lock_guard<decltype(pim)> ld(pim);
					if (!inilizered_plugin_list.empty())
					{
						std::lock_guard<decltype(rm)> ld(rm);
						plugin_list.splice(plugin_list.end(), std::move(inilizered_plugin_list));
					}
				}

				std::lock_guard<decltype(rm)> ld2(rm);
				for (auto ptr = plugin_list.begin(); ptr != plugin_list.end();)
				{
					if ((*ptr))
					{
						if (**ptr)
						{
							(*ptr++)->tick(tv, dua);
							continue;
						}
					}
					plugin_list.erase(ptr++);
				}
			}
		};

		template<typename T> struct form_instance : form_ptr
		{

			using form_ptr::form_ptr;

			template<typename ...AK>
			form_instance(std::promise<base_view_data<T>>& promise, AK&& ... ak)
			{
				this->form_ptr::force_exist_form = false;
				logic_form_thread = std::thread(
					[&ak..., this, &promise]()
				{
					typename frame<T>::form render{ std::forward<AK>(ak)... };
					Tool::completeness<form_plugin<T>> plugin_list;
					base_view_data<T> bvd{ this, &render, plugin_list, &plugin_list };
					promise.set_value(bvd);
					typename frame<T>::form_control fc(render);
					std::chrono::time_point<std::chrono::system_clock> start_loop = std::chrono::system_clock::now();
					std::this_thread::sleep_until(start_loop + this->frame_duration);
					while (!this->form_ptr::force_exist_form  && fc.is_avalible())
					{
						duration da = std::chrono::duration_cast<duration>(std::chrono::system_clock::now() - start_loop);
						plugin_list.run_tick(bvd, da);
						fc.tick(da);
						start_loop = std::chrono::system_clock::now();
						std::this_thread::sleep_until(start_loop + this->frame_duration);
					}
				}
				);
			}

			~form_instance()
			{
				if (logic_form_thread.joinable())
					logic_form_thread.join();
			}

		};

	}

	template<typename T,typename K> class plugin_view : Assistant::base_plugin_view_data<T,K>
	{
	public:
		plugin_view(const Assistant::base_plugin_view_data<T, K>& b) :Assistant::base_plugin_view_data<T, K>(b) {}
		plugin_view(const plugin_view&) = default;
	};

	template<typename T> class form_view : Assistant::base_view_data<T>, public frame<T>::form_view
	{

	public:

		form_view(const Assistant::base_view_data<T>& p) :Assistant::base_view_data<T>{ p }, frame<T>::form_view(p.form) {}
		form_view(const form_view&) = default;

		template<typename K, typename ...AK>
		decltype(auto) create_plugin(AK&&... ak)
		{
			Assistant::base_plugin_view_data<T, K> pv;
			Assistant::base_view_data<T>::plugin_cpr.lock_if(
				[&, this]()
			{
				pv = Assistant::base_view_data<T>::plugin->create_plugin<K>(static_cast<Assistant::base_view_data<T>&>(*this), std::forward<AK>(ak)...);
			}
			);
			return plugin_view<T,K>(pv);

		}

	};

	template<typename T> class tick_view : Assistant::base_view_data<T>, public frame<T>::tick_view
	{
		duration time;
		Assistant::plugin_ptr<T>* this_plugin_ptr;
	public:
		decltype(auto) get_time() const { return time; }
		tick_view(const  Assistant::base_view_data<T>& b, duration d, Assistant::plugin_ptr<T>* p) :Assistant::base_view_data<T>(b), frame<T>::tick_view(b.form), time(d), this_plugin_ptr(p){}
		tick_view(const tick_view&) = default;
		void close_this_plugin()
		{
			this_plugin_ptr->avalible = false;
		}
		template<typename K, typename ...AK>
		decltype(auto) create_plugin(AK&&... ak)
		{
			Assistant::base_plugin_view_data<T, K> pv;
			Assistant::base_view_data<T>::plugin_cpr.lock_if(
				[&, this]()
			{
				pv = Assistant::base_view_data<T>::plugin->create_plugin<K>(static_cast<Assistant::base_view_data<T>&>(*this), std::forward<AK>(ak)...);
			}
			);
			return plugin_view<T, K>(pv);
		}
	};

	template<typename T> class plugin_initializer : Assistant::base_view_data<T>, public frame<T>::plugin_initializer
	{
		Tool::completeness_ref plugin_self_ref;
	public:
		operator Tool::completeness_ref& () { return plugin_self_ref; }
		operator const Tool::completeness_ref& () const { return plugin_self_ref; }
		plugin_initializer(const Tool::completeness_ref& cpr, const Assistant::base_view_data<T>& p) :plugin_self_ref(cpr), Assistant::base_view_data<T>(p), frame<T>::plugin_initializer(p.form) {}
		plugin_initializer(const plugin_initializer&) = default;
		template<typename K, typename ...AK>
		decltype(auto) create_plugin(AK&&... ak)
		{
			Assistant::base_plugin_view_data<T, K> pv;
			Assistant::base_view_data<T>::plugin_cpr.lock_if(
				[&, this]()
			{
				pv = Assistant::base_view_data<T>::plugin->create_plugin<K>(static_cast<Assistant::base_view_data<T>&>(*this), std::forward<AK>(ak)...);
			}
			);
			return plugin_view<T, K>(pv);

		}
	};

	namespace Assistant
	{
		template<typename K, typename T, typename = void> struct able_to_call_tick :std::false_type {};
		template<typename K, typename T> struct able_to_call_tick<K, T, std::void_t<decltype(std::declval<T>().tick(std::declval<tick_view<K>>()))>> :std::true_type {};

		template<typename T, typename K> void plugin_implement<T, K>::tick_implement(base_view_data<T>& du, duration dua)
		{
			Tool::statement_if<Assistant::able_to_call_tick<T, K>::value>
				(
					[&](auto&& plugin) { plugin.tick(tick_view<T>(du,dua,this)); },
					[this](auto&& plugin) {this->use_tick = false; },
					static_cast<K&>(*this)
					);
		}

		template<typename T, typename K> template<typename ...AK> plugin_implement<T, K>::plugin_implement(const Tool::completeness_ref& cpr, const base_view_data<T>& bvd, AK&&... ak)
			:plugin_implement(std::integral_constant<bool, std::is_constructible<K, plugin_initializer<T>, AK... >::value>(), plugin_initializer<T>(cpr,bvd),std::forward<AK>(ak)...)
		{

		}
	
	}
	*/

}

namespace PO
{

	class context
	{

		std::mutex this_form_mutex;
		std::deque<std::unique_ptr<PO::Assistant::form_ptr>> this_form;

	public:

		void wait_all_form_close();
		
		context();
		~context();
		void set_form(std::unique_ptr<Assistant::form_ptr>&& fp)
		{
			std::lock_guard<decltype(this_form_mutex)> ld(this_form_mutex);
			this_form.push_back(std::move(fp));
		}
		void detach();
		template<typename T, typename ...AK> auto create_window(AK&& ...ak)
		{
			frame_static_assert<T>{};
			std::unique_ptr<Assistant::form_instance> tem = std::make_unique<Assistant::form_instance>();
			Assistant::form_ptr& ptr = *tem;
			set_form(std::move(tem));
			return *(ptr.create_window<T>(std::forward<AK>(ak)...));
		}
	};
	
}

#include "po.hpp"