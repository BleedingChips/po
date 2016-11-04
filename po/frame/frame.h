#pragma once
#include "define.h"
#include "../tool/thread_tool.h"
#include <atomic>
#include <list>
#include <future>
namespace PO
{

	namespace Assistant
	{
		struct void_interface_or_viewer
		{
			template<typename T> void_interface_or_viewer(T&& t) {}
		};
	}

	template<typename mod_type, typename interface_type = Assistant::void_interface_or_viewer, typename viewer_type = Assistant::void_interface_or_viewer> struct mod_pair
	{
		using mod = std::remove_const_t<std::remove_reference_t<mod_type>>;
		using mod_interface = std::remove_const_t<std::remove_reference_t<interface_type>>;
		using mod_viewer = std::remove_const_t<std::remove_reference_t<viewer_type>>;
		static_assert(std::is_constructible<mod_interface, mod&>::value, "interface need to be able to construct form mod");
		static_assert(std::is_constructible<mod_viewer, mod&>::value, "viewer need to be able to construct form mod");
	};

	namespace Assistant
	{

		template<typename T> struct is_mod_pair : std::false_type {};
		template<typename mod_type, typename interface_type, typename viewer_type> struct is_mod_pair<mod_pair<mod_type, interface_type, viewer_type>> : std::true_type {};

		template<typename pre, typename append> struct append_pair_assert
		{
			static_assert(is_mod_pair<pre>::value, "can only mod_pair append");
			static_assert(is_mod_pair<append>::value, "can only mod_pair append");
			static constexpr bool value = std::is_constructible<typename pre::mod, typename append::mod_interface&>::value;
		};

		template<typename T, typename = void> struct form_have_is_available : std::false_type {};
		template<typename T> struct form_have_is_available<T,std::void_t<decltype(std::declval<T>().is_available())>> : std::true_type {};

		template<typename T, typename = void> struct form_have_bind_event : std::false_type {};
		template<typename T> struct form_have_bind_event < T, std::void_t<decltype(std::declval<T>().bind_event(std::function<bool(event)> {} )) >> : std::true_type {};

		template<typename frame, typename = void> struct frame_have_form : std::false_type { using viewer = void_interface_or_viewer; };
		template<typename frame> struct frame_have_form<frame, std::void_t<typename frame::form>> : std::true_type 
		{
			static_assert(is_mod_pair<typename frame::form>::value, "form should be define as a mod_pair");
			static_assert(form_have_is_available<typename frame::form::mod>::value, "form need provide member function bool is_available()");
			static_assert(form_have_bind_event<typename frame::form::mod>::value, "form need provide member function bool bind_event(std::function<bool(event)>)");
			using viewer = typename frame::form::mod_viewer; 
		};

		template<typename frame, typename = void> struct frame_have_renderer : std::false_type { using viewer = void_interface_or_viewer; };
		template<typename frame> struct frame_have_renderer<frame, std::void_t<typename frame::renderer>> : std::true_type
		{ 
			static_assert(is_mod_pair<typename frame::renderer>::value, "renderer should be define as a mod_pair");
			static_assert(frame_have_form<frame>::value, "renderer need define after form");
			static_assert(append_pair_assert<typename frame::renderer, typename frame::form>::value, "renderer can not append to form");
			using viewer = typename frame::renderer::mod_viewer; 
		};

		template<typename frame, typename = void> struct frame_have_scene : std::false_type { using viewer = void_interface_or_viewer; };
		template<typename frame> struct frame_have_renderer<frame, std::void_t<typename frame::scene>> : std::true_type
		{
			static_assert(is_mod_pair<typename frame::scene>::value, "scene should be define as a mod_pair");
			static_assert(frame_have_renderer<frame>::value, "scene need define after renderer");
			static_assert(append_pair_assert<typename frame::scene, typename frame::renderer>::value, "scene can not append to renderer");
			using viewer = typename frame::scene::mod_viewer;
		};

		template<typename T, typename = void> struct able_operator_to_call_tick : std::false_type {};
		template<typename T> struct able_operator_to_call_tick < T, std::void_t<decltype(std::declval<T>().tick(time_point{})) >> : std::true_type {};

		template<typename frame> struct frame_form_implement
		{
			typename frame::form::mod form_data;
			operator typename frame::form::mod&() { return form_data; }
			template<typename ...any_parameter> frame_form_implement(const Tool::completeness_ref& cr, any_parameter&&... ap) :
				frame_form_implement(
					std::integral_constant<bool, std::is_constructible<typename frame::form::mod, const Tool::completeness_ref&, any_parameter&&...>::value>(),
					cr, std::forward<any_parameter>(ap)...
				) {}
			using channel = typename frame::form::mod_interface;
			bool is_available() { return form_data.is_available(); }
			void tick(time_point da) 
			{
				Tool::statement_if<able_operator_to_call_tick<typename frame::form::mod>::value>
					([&da](auto& f) { f.tick(da); })
					(form_data);
			}
			template<typename func> decltype(auto) bind_event(func&& f)
			{
				form_data.bind_event(std::forward<func>(f));
			}
		private:
			template<typename ...any_parameter> frame_form_implement(std::true_type, const Tool::completeness_ref& cr, any_parameter&&... ap) :
				form_data(cr, std::forward<any_parameter>(ap)...) {}
			template<typename ...any_parameter> frame_form_implement(std::false_type, const Tool::completeness_ref& cr, any_parameter&&... ap) :
				form_data(std::forward<any_parameter>(ap)...) {}
		};

		template<bool, typename frame> struct frame_renderer_implement : frame_form_implement<frame>
		{
			using frame_form_implement<frame>::frame_form_implement;
		};
		template<typename frame> struct frame_renderer_implement<true, frame> : frame_form_implement<frame>
		{
			typename frame::renderer::mod renderer_data;
			operator typename frame::renderer::mod&() { return renderer_data; }
			using channel = typename frame::renderer::mod_interface;
			template<typename ...any_parameter> frame_renderer_implement(any_parameter&&... ap) :
				frame_form_implement<frame>(std::forward<any_parameter>(ap)...),
				renderer_data(typename frame::form::mod_interface(static_cast<typename frame::form::mod&>(*this))) {}
			void tick(time_point da)
			{
				Tool::statement_if<able_operator_to_call_tick<typename frame::renderer::mod>::value>
					([&da](auto& f) { f.tick(da); })
					(renderer_data);
				frame_form_implement<frame>::tick(da);
			}
		};

		template<bool, typename frame> struct frame_scene_implement : frame_renderer_implement<frame_have_renderer<frame>::value, frame>
		{
			using frame_renderer_implement<frame_have_renderer<frame>::value, frame>::frame_renderer_implement;
		};
		template<typename frame> struct frame_scene_implement<true, frame> : frame_renderer_implement<true, frame>
		{
			typename frame::scene::mod scene_data;
			operator typename frame::scene::mod&() { return scene_data; }
			using channel = typename frame::scene::mod_interface;
			template<typename ...any_parameter> frame_scene_implement(any_parameter&&... ap) :
				frame_renderer_implement<true, frame>(std::forward<any_parameter>(ap)...),
				scene_data(typename frame::renderer::mod_interface(static_cast<typename frame::renderer::mod&>(*this))) {}
			void tick(time_point da)
			{
				Tool::statement_if<able_operator_to_call_tick<typename frame::scene::mod>::value>
					([&da](auto& f) { f.tick(da); })
					(scene_data);
				frame_renderer_implement<true, frame>::tick(da);
			}
		};

		template<typename frame_type> using frame = frame_scene_implement<frame_have_scene<frame_type>::value, frame_type>;

		template<typename frame_type> class frame_viewer
		{
			typename frame_have_form<frame_type>::viewer form_viewer;
			typename frame_have_renderer<frame_type>::viewer renderer_viewer;
			typename frame_have_scene<frame_type>::viewer scene_viewer;
		public:
			template<typename T>
			frame_viewer(T&& t) : form_viewer(std::forward<T>(t)), renderer_viewer(std::forward<T>(t)), scene_viewer(std::forward<T>(t)) {}
			decltype(auto) get_form() { return form_viewer; }
			decltype(auto) get_renderer() { return renderer_viewer; }
			decltype(auto) get_scene() { return scene_viewer; }
		};

		struct plugin_control
		{
			Tool::completeness_ref cr;
			bool use_tick = true;
			std::atomic_bool avalible;
			time_calculator record;

			operator bool() const { return avalible; }
			virtual ~plugin_control() {}
			plugin_control(const Tool::completeness_ref& c) :cr(c), avalible(true) {}
			virtual void tick_implementation(duration ti) = 0;
			virtual void init() = 0;
			void tick(time_point& da)
			{
				if(use_tick)
					record.tick(
						da, [this](duration da) {tick_implementation(da); }
					);
			}
		};

		struct plugin_self
		{
			Tool::completeness_ref plugin_ref;
			plugin_control* plugin_ptr = nullptr;
		public:
			plugin_self() {}
			plugin_self(plugin_control& pc) : plugin_ref(pc.cr), plugin_ptr(&pc) {}
			bool destory_plugin() 
			{ 
				return plugin_ref.lock_if([this]() {plugin_ptr->avalible = false; });
			}
			bool set_duration(duration da)
			{
				return plugin_ref.lock_if([this,&da]() {plugin_ptr->record.set_duration(da); });
			}
		};

		template<typename frame_type> struct plugin_ptr;
		template<typename frame_type, typename plugin_type> class plugin_implement;
		template<typename frame_type, typename plugin_type> using plugin_final = Tool::completeness<plugin_implement<frame_type, plugin_type>>;
		struct plugin_append;

		// template<bool s, typename frame_type> class form_packet_implement; 
		template<typename frame_type> class form_packet;
		template<typename frame_type> using form_final = Tool::completeness<form_packet<frame_type>>;
		struct form_ptr;

		template<typename frame_type> class initial;
		template<typename frame_type> class viewer : public frame_viewer<frame_type>
		{
			Tool::completeness_ref form_ref;
			form_packet<frame_type>* form_ptr = nullptr;
			friend class initial<frame_type>;
		public:
			viewer(form_final<frame_type>& ff) : frame_viewer<frame_type>(ff), form_ref(ff), form_ptr(&ff) {}
			viewer(const Tool::completeness_ref& cr, form_packet<frame_type>& ff) : frame_viewer<frame_type>(ff), form_ref(cr), form_ptr(&ff) {}
			viewer() {}
			viewer(const viewer&) = default;
			template<typename plugin_type, typename ...AK> decltype(auto) create_plugin(AK&&...ak);
		};

		template<typename frame_type>
		class initial : public viewer<frame_type>
		{
			plugin_self plugin_data;
			typename frame<frame_type>::channel channel_data;
		public:
			operator plugin_self& () { return plugin_data; }
			operator typename frame<frame_type>::channel& () { return channel_data; }
			decltype(auto) get_self() { return plugin_data; }
			decltype(auto) get_channel() { return channel_data; }
			initial(form_final<frame_type>& ff, plugin_control& pp) : viewer<frame_type>(ff), plugin_data(pp), channel_data(ff){}
			initial(viewer<frame_type>& v, plugin_control& pp) : viewer<frame_type>(v), plugin_data(pp), channel_data(*v.form_ptr) {}
			initial(const initial&) = default;
			initial() {}
		};

		template<typename frame_type> class ticker : public initial<frame_type>
		{
			duration duration_time;
		public:
			operator duration() const { return duration_time; }
			void set_time(duration da) { duration_time = da; }
			auto get_time() const { return duration_time; }
			using initial<frame_type>::initial;
		};

		template<typename frame_type, typename plugin_type, typename = void> struct able_to_call_tick :std::false_type {};
		template<typename frame_type, typename plugin_type> struct able_to_call_tick<frame_type, plugin_type, std::void_t<decltype(std::declval<plugin_type>().tick(std::declval<ticker<frame_type>&>()))>> :std::true_type {};
		template<typename frame_type, typename plugin_type, typename = void> struct able_to_call_init :std::false_type {};
		template<typename frame_type, typename plugin_type> struct able_to_call_init<frame_type, plugin_type, std::void_t<decltype(std::declval<plugin_type>().init(std::declval<initial<frame_type>&>()))>> :std::true_type {};

		template<typename frame_type, typename plugin_type> class plugin_implement : public plugin_control
		{
			plugin_type plugin_data;
			using form_type = typename frame_type::form;

			ticker<frame_type> ticker_data;

			template<typename ...AK>
			plugin_implement(std::true_type, const Tool::completeness_ref& cpr, viewer<frame_type>& ff, AK&& ...ak) :
				plugin_control(cpr), plugin_data(initial<frame_type>(ff, *this), std::forward<AK>(ak)...), ticker_data(ff, *this) {}
			template<typename ...AK>
			plugin_implement(std::false_type, const Tool::completeness_ref& cpr, viewer<frame_type>& ff, AK&& ...ak) :
				plugin_control(cpr), plugin_data(std::forward<AK>(ak)...), ticker_data(ff, *this) {}

		public:

			template<typename ...AK>
			plugin_implement(const Tool::completeness_ref& cpr, viewer<frame_type>& ff, AK&& ...ak) :
				plugin_implement(
					std::integral_constant<bool, std::is_constructible<plugin_type, initial<frame_type>, AK... >::value>(),
					cpr, ff, std::forward<AK>(ak)...
				)
			{
				
			}

			virtual void init()
			{
				Tool::statement_if<able_to_call_init<frame_type, plugin_type>::value>
					([&](auto& plugin) { plugin.init(ticker_data); })
					(plugin_data);
			}

			void tick_implementation(duration ti) override
			{
				ticker_data.set_time(ti);
				Tool::statement_if<able_to_call_tick<frame_type, plugin_type>::value>
					(
						[&](auto& plugin) { plugin.tick(ticker_data); },
						[this](auto& plugin) {use_tick = false; },
						(plugin_data)
						);
			}
		};

		struct plugin_append
		{

			std::mutex pim;
			std::list<std::unique_ptr<plugin_control>> inilizered_plugin_list;

			std::list<std::unique_ptr<plugin_control>> plugin_list;

		public:

			template<typename frame_type, typename plugin_type, typename ...AK> decltype(auto) create_plugin(viewer<frame_type>& ff, AK&& ...ak)
			{
				auto ptr = std::make_unique<plugin_final<frame_type, plugin_type>>(ff, std::forward<AK>(ak)...);
				std::lock_guard<decltype(pim)> lg(pim);
				inilizered_plugin_list.push_back(std::move(ptr));
			}
			void tick(time_point da)
			{
				decltype(inilizered_plugin_list) temporary_list;
				{
					std::lock_guard<decltype(pim)> lg(pim);
					if (!inilizered_plugin_list.empty())
					{
						temporary_list = std::move(inilizered_plugin_list);
						inilizered_plugin_list.clear();
					}
				}
				if (!temporary_list.empty())
				{
					for (auto& ptr : temporary_list)
						(ptr)->init();
					plugin_list.splice(plugin_list.end(), std::move(temporary_list), temporary_list.begin(), temporary_list.end());
				}
				for (auto ptr = plugin_list.begin(); ptr != plugin_list.end();)
				{
					if ((*ptr) && (**ptr) )
					{
						(*ptr++)->tick(da);
						continue;
					}
					plugin_list.erase(ptr++);
				} 
			}
			bool event_function(event ev)
			{
				return false;
			}
		};

		

		template<typename frame_type> class form_packet : public frame<frame_type>
		{
			plugin_append all_plugin;

			friend class viewer<frame_type>;
		public:
			template<typename ...AK> form_packet(const Tool::completeness_ref& cr, AK&&... ak) :
				frame<frame_type>(cr, std::forward<AK>(ak)...) 
			{
				frame<frame_type>::template bind_event([this](event ev) -> bool {return all_plugin.event_function(ev); });
			}
			void tick(time_point& da)
			{
				frame<frame_type>::tick(da);
				all_plugin.tick(da);
			}
		};

		struct form_ptr
		{
			bool avalible;
			std::thread logic_form_thread;
			std::atomic_bool force_exist_form;
			//Tool::completeness_ref ref;
			//form_ptr(const Tool::completeness_ref& cr) :ref(cr) {}
			virtual ~form_ptr();
			template<typename frame_type, typename ...AK> auto create_window(AK&& ...ak)
			{
				if (logic_form_thread.joinable())
				{
					force_exist_form = true;
					logic_form_thread.join();
				}
				force_exist_form = false;
				std::promise<std::pair<Tool::completeness_ref, form_packet<frame_type>*>> p;
				auto fur = p.get_future();
				force_exist_form = false;
				logic_form_thread = std::thread(
					[&, this]()
				{
					form_final<frame_type> packet(std::forward<AK>(ak)...);
					p.set_value({ static_cast<Tool::completeness_ref>(packet), &packet });
					time_point start_loop = std::chrono::system_clock::now();
					while (!force_exist_form  && packet.is_available())
					{
						start_loop = std::chrono::system_clock::now();
						packet.tick(start_loop);
						std::this_thread::sleep_until(start_loop + duration(1));
					}
				}
				);
				fur.wait();
				std::pair<Tool::completeness_ref, form_packet<frame_type>*> tem = fur.get();
				return viewer<frame_type>(tem.first,*tem.second);
			}
			void force_stop() { force_exist_form = true; }
		};

		template<typename frame_type> template<typename plugin_type, typename ...AK> decltype(auto) viewer<frame_type>::create_plugin(AK&&...ak)
		{
			return form_ref.lock_if(
				[&, this]() {  (*form_ptr).all_plugin.template create_plugin<frame_type, plugin_type>(*this,std::forward<AK>(ak)... ); }
			);
		}
	}
}
