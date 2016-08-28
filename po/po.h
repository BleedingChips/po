#pragma once
#include "platform\platform_window.h"
#include "tool\type_tool.h"
#include <thread>
#include <unordered_map>
#include <iostream>
#include "event\event.h"

namespace PO
{

	namespace Event
	{
		struct key : PO::Platform::window_event 
		{
			key(const PO::Platform::window_event& we) :PO::Platform::window_event(we){}
			key(const key&) = default;
			key& operator=(const key&) = default;
		};

		struct mouse : PO::Platform::window_event 
		{
			mouse(const PO::Platform::window_event& we) :PO::Platform::window_event(we) {}
			mouse(const mouse&) = default;
			mouse& operator=(const mouse&) = default;
		};

		struct form : PO::Platform::window_event 
		{
			form(const PO::Platform::window_event& we) :PO::Platform::window_event(we) {}
			form(const form&) = default;
			form& operator=(const form&) = default;
		};

		struct system : PO::Platform::window_event 
		{
			system(const PO::Platform::window_event& we) :PO::Platform::window_event(we) {}
			system(const system&) = default;
			system& operator=(const system&) = default;
		};

		struct event_box
		{
			Tool::Mail::box<bool(const key&)> key_box;
			Tool::Mail::box<bool(const mouse&)> mouse_box;
			Tool::Mail::box<bool(const form&)> form_box;
			Tool::Mail::box<bool(const system&)> system_box;
			bool respond_event(const PO::Platform::window_event&);
		};

		struct event_receipt
		{
			Tool::Mail::receipt<bool(const key&)> key_receipt;
			//Tool::Mail::receipt<bool(const key&)> key_receipt;
		};
	}


	using duration = std::chrono::duration<long long, std::ratio<1, 1000>>;

	struct default_expand
	{
		using initializer = PO::Platform::window_style;
		using renderer = PO::Platform::window_instance;
	};
	
	template<typename T = default_expand > using initializer = typename T::initializer;
	template<typename T > class ticker;
	template<typename T > class renderer;
	template<typename T > class plugin_initializer;

	namespace Assistant
	{
		template<typename T, typename K> class have_tick_function
		{
			template<typename plugin, typename render, void (plugin::*)(ticker<render>)> struct tick_function_1;
			template<typename plugin, typename render, void (plugin::*)(const ticker<render>&)> struct tick_function_2;

			template<typename plugin, typename render> static std::true_type func1(tick_function_1<plugin, render, &plugin::tick>*);
			template<typename plugin, typename render> static std::true_type func2(tick_function_2<plugin, render, &plugin::tick>*);
			template<typename plugin, typename render> static std::false_type func1(...);
			template<typename plugin, typename render> static std::false_type func2(...);
		public:
			static constexpr bool value = decltype(func1<T, K>(nullptr))::value || decltype(func2<T, K>(nullptr))::value;
		};
	}

	template<typename T = default_expand > struct plugin_define
	{
		using ticker = ticker<T>;
		using renderer = renderer<T>;
		using plugin_initializer = plugin_initializer<T>;
	};

	template<typename T>
	class plugin_ptr
	{
		std::atomic_bool exist = true;
	public:
		bool is_exist() const { return exist; }
		virtual void tick(ticker<T>) = 0;
		virtual ~plugin_ptr() {}
	};

	template<typename T,typename K> class plugin_substance: public plugin_ptr<K>
	{
		T real_plugin;
		template<typename T> friend class window_substance;
		T& get_real_plugin() { return real_plugin; }
		
	public:
		template<typename ...AT> plugin_substance(AT&&... at) :real_plugin(std::forward<AT>(at)...) {}
		void tick( ticker<K> t)
		{
			Tool::statement_if<Assistant::have_tick_function<T, K>::value>
				(
					[&t](auto& i) {  i.tick(t); },
					[&t](auto& i) {},
					real_plugin
					);
		}
		
	};
	
	template<typename T = default_expand > class window_substance : public T::renderer
	{

		std::thread tick_loop;

		std::recursive_mutex plugin_list_mutex;
		std::list<std::unique_ptr<plugin_ptr<T>>> plugin_list;

		std::chrono::system_clock::time_point last_time_point;

		PO::Event::event_box all_event;

		template<typename T > friend class ticker;
		template<typename T > friend class renderer;
		template<typename T > friend class plugin_initializer;

	public:

		using init_type = Platform::window_style;

		T& get_renderer() { return *this; }

		virtual bool respond_event(const PO::Platform::window_event& we) override
		{
			return all_event.respond_event(we);//Platform::window_instance::pre_respond_event(ev);
		}

		window_substance()
		{
			last_time_point = std::chrono::system_clock::now();
			tick_loop = std::thread(
				[this]() {
				last_time_point = std::chrono::system_clock::now();
				std::this_thread::sleep_for(duration(10));
				while (Platform::window_instance::is_exist())
				{
					auto now = std::chrono::system_clock::now();
					auto target = now + duration(10);
					duration dur = std::chrono::duration_cast<duration>(now - last_time_point);
					last_time_point = now;

					plugin_list_mutex.lock();
					for (auto ptr = plugin_list.begin(); ptr != plugin_list.end();)
					{
						if (*ptr && (*ptr)->is_exist())
						{
							(*ptr)->tick(ticker<T>(*this, dur));
							++ptr;
						}
						else
							plugin_list.erase(ptr++);
					}
					plugin_list_mutex.unlock();
					std::this_thread::sleep_until(target);
					
				}
			}
			);
		}

		template<typename K,typename ...AT> K& create_plugin(AT&&... at)
		{

			std::unique_ptr<plugin_substance<K, T>> ptr = Tool::statement_if<std::is_constructible<K, plugin_initializer<T>, AT&&...>::value>
				(
					[&, this](auto* ptr) { return std::make_unique<plugin_substance<std::remove_pointer_t<decltype(ptr)>, T>>(plugin_initializer<T>{*this}, std::forward<AT>(at)...); },
					[&, this](auto* ptr) { return std::make_unique<plugin_substance<std::remove_pointer_t<decltype(ptr)>, T>>(std::forward<AT>(at)...); },
					static_cast<K*>(nullptr)
					);
			K& tem = ptr->get_real_plugin();
			plugin_list_mutex.lock();
			plugin_list.push_back(std::move(ptr));
			plugin_list_mutex.unlock();
			return tem;
		}

		~window_substance()
		{
			//if(tick_loop.joinable())
			tick_loop.join();
			cout << "call this!" << endl;
		}

	};

	template<typename T = default_expand > class ticker
	{
		window_substance<T>& win_sub;
		duration dura_time;
	public:
		ticker(window_substance<T>& ws, duration d) :win_sub(ws), dura_time(d) {}
		ticker(const ticker& ws) = default;
		duration get_time() const { return dura_time; }
	};

	template<typename T = default_expand > class renderer
	{
		window_substance<T>& win_sub;
	public:
		renderer(window_substance<T>& ws) :win_sub(ws) {}
		renderer(const renderer& ws) = default;
	};

	template<typename T = default_expand > class plugin_initializer
	{
		window_substance<T>& win_sub;
	public:
		plugin_initializer(window_substance<T>& ws) :win_sub(ws) {}
		plugin_initializer(const plugin_initializer& ws) = default;
		Event::event_box& get_event_box()
		{
			return win_sub.all_event;
		}
		
	};

	class gui_context;

	class context
	{
		gui_context& gui;
		friend class gui_context;
	public:

		context(gui_context& gc) :gui(gc) {}
		context(const context& gc) = default;

		template<typename Y = default_expand>
		window_substance<Y>& create_winodw(const initializer<Y>& t)
		{
			std::unique_ptr<window_substance<Y>> win = std::make_unique<window_substance<Y>>();
			window_substance<Y>& tem = *win;
			gui.regedit_window(std::move(win));
			return tem;
		}
	};


	class gui_context
	{

		void regedit_window(std::unique_ptr<PO::Platform::window_instance>&& ptr);
		friend class context;
	public:

		template<typename T> void operator() (T&& t) { context tem{ *this }; t(tem); }

		static void loop();

		gui_context();
		~gui_context();

	};

	/*
	class window_ptr
	{
		std::shared_ptr<>
	};

	template<typename T = default_window_expand ,typename ...AP>
	std::shared_ptr<window<T>> create_window(AP&&... ap)
	{
		std::shared_ptr<>
	}
	*/
	
}