#pragma once
#include "define.h"
#include "../tool/thread_tool.h"
#include <atomic>
#include <list>
#include <future>
namespace PO
{
	template<typename mod_type, typename interface_type, typename viewer_type> class mod_pair;

	namespace Assistant
	{
		struct default_viewer_or_interface
		{
			template<typename mod>
			default_viewer_or_interface(mod& m){}
		};

		template<typename T> struct is_mod_pair :std::false_type {};
		template<typename mod_type, typename interface_type, typename viewer_type> struct is_mod_pair<mod_pair<mod_type, interface_type, viewer_type>> :std::true_type {};

		template<typename mod_pair, typename append_pair> struct can_mod_pair_append
		{
			static_assert(is_mod_pair<mod_pair>::value, "can only append to mod pair");
			static_assert(is_mod_pair<append_pair>::value, "can only append to mod pair");
			static constexpr bool value = std::is_constructible<typename mod_pair::mod, typename append_pair::mod_interface&>::value;
		};
	}

	template<typename mod_type, typename interface_type = Assistant::default_viewer_or_interface, typename viewer_type = Assistant::default_viewer_or_interface > class mod_pair
	{
		using pure_mod = std::remove_const_t<std::remove_reference_t<mod_type>>;
		using pure_interface = std::remove_const_t<std::remove_reference_t<interface_type>>;
		using pure_viewer = std::remove_const_t<std::remove_reference_t<viewer_type>>;
	public:
		using mod = pure_mod;
		using mod_interface = pure_interface;
		using mod_viewer = pure_viewer;
		static_assert(std::is_constructible<mod_interface, mod&>::value, "mod have an interface but can't construct form mod&");
		static_assert(std::is_constructible<mod_viewer, mod&>::value, "mod have an interface but can't construct form mod&");
	};

	namespace Assistant
	{
		template<typename form_type, typename = void> struct form_have_is_available : std::false_type {};
		template<typename form_type> struct form_have_is_available < form_type, std::void_t<decltype(std::declval<form_type>().is_available())>> : std::true_type {};

		template<typename frame_type, typename = void > struct frame_have_form : std::false_type 
		{
			using viewer = default_viewer_or_interface;
		};
		template<typename frame_type> struct frame_have_form<frame_type, std::void_t<typename frame_type::form> > : std::true_type
		{
			static_assert(is_mod_pair<typename frame_type::form>::value, "frame::form should define as mod_pair");
			static_assert(form_have_is_available<typename frame_type::form::mod>::value, "form should have member function bool is_available()");
			using viewer = typename frame_type::form::mod_viewer;
		};

		template<typename frame_type, typename = void > struct frame_have_renderer : std::false_type 
		{
			using viewer = default_viewer_or_interface;
		};
		template<typename frame_type> struct frame_have_renderer<frame_type, std::void_t<typename frame_type::renderer> > : std::true_type
		{
			static_assert(is_mod_pair<typename frame_type::renderer>::value, "frame::renderer should define as mod_pair");
			static_assert(can_mod_pair_append<typename frame_type::renderer, typename frame_type::form>::value, "renderer can not construct from form interface");
			using viewer = typename frame_type::renderer::mod_viewer;
		};

		template<typename frame_type, typename = void > struct frame_have_scene : std::false_type 
		{
			using viewer = default_viewer_or_interface;
		};
		template<typename frame_type> struct frame_have_scene<frame_type, std::void_t<typename frame_type::scene> > : std::true_type
		{
			static_assert(is_mod_pair<typename frame_type::scene>::value, "frame::scene should define as mod_pair");
			static_assert(frame_have_renderer<frame_type>::value, "frame should define renderer before scene");
			static_assert(can_mod_pair_append<typename frame_type::scene, typename frame_type::renderer>::value, "scene can not construct from renderer interface");
			using viewer = typename frame_type::scene::mod_viewer;
		};

		template<typename T, typename = void> struct able_operator_to_call_tick : std::false_type {};
		template<typename T> struct able_operator_to_call_tick < T, std::void_t<decltype(std::declval<T>().tick(time_point{})) >> : std::true_type {};

		template<typename frame_type> struct frame_form_implement
		{
			static_assert(frame_have_form<frame_type>::value, "frame should have form");
			typename frame_type::form::mod form_mod;
			operator typename frame_type::form::mod&(){ return form_mod; }
			template<typename ...any_parameter> frame_form_implement(any_parameter&& ...ap) :form_mod(std::forward<any_parameter>(ap)...) {}
			bool is_available() { return form_mod.is_available(); }
			void tick(time_point da)
			{
				Tool::statement_if<able_operator_to_call_tick<decltype(form_mod)>::value>
					([&da](auto& f) { f.tick(da); })
					(form_mod);
			}
		};

		template<bool, bool, typename frame_type> struct frame_implement : frame_form_implement<frame_type>
		{
			using frame_form_implement<frame_type>::frame_form_implement;
			using channel = typename frame_type::form::mod_interface;
		};

		template<bool s, typename frame_type> struct frame_implement<true, s, frame_type> : frame_form_implement<frame_type>
		{
			using frame_form_implement<frame_type>::frame_form_implement;
			typename frame_type::renderer::mod renderer_mod;
			typename frame_type::scene::mod scene_mod;
			using channel = typename frame_type::scene::mod_interface;
			operator typename frame_type::renderer::mod&(){ return renderer_mod; }
			operator typename frame_type::scene::mod&(){ return scene_mod; }

			template<typename ...any_parameter> frame_implement(any_parameter&& ...ap) :
				frame_form_implement<frame_type>(std::forward<any_parameter>(ap)...),
				renderer_mod(typename frame_type::form::mod_interface(static_cast<frame_form_implement<frame_type>&>(*this))),
				scene_mod(typename frame_type::renderer::append)
			{
			}
			
			void tick(time_point da)
			{
				Tool::statement_if<able_operator_to_call_tick<decltype(scene_mod)>::value>
					([&da](auto& f) { f.tick(da); })
					(scene_mod);
				Tool::statement_if<able_operator_to_call_tick<decltype(renderer_mod)>::value>
					([&da](auto& f) { f.tick(da); })
					(renderer_mod);
				frame_form_implement<frame_type>::tick(da);
			}
		};

		template<typename frame_type> struct frame_implement<false, true, frame_type> : frame_form_implement<frame_type>
		{
			using frame_form_implement<frame_type>::frame_form_implement;
			using channel = typename frame_type::renderer::mod_interface;
			typename frame_type::renderer::mod renderer_mod;
			operator typename frame_type::renderer::mod&(){ return renderer_mod; }
			template<typename ...any_parameter> frame_implement(any_parameter&& ...ap) :
				frame_form_implement<frame_type>(std::forward<any_parameter>(ap)...),
				renderer_mod(typename frame_type::form::mod_interface(static_cast<frame_form_implement<frame_type>&>(*this)))
			{
			}
			void tick(time_point da)
			{
				Tool::statement_if<able_operator_to_call_tick<decltype(renderer_mod)>::value>
					([&da](auto& f) { f.tick(da); })
					(renderer_mod);
				frame_form_implement<frame_type>::tick(da);
			}
		};

		template<typename frame_type> using frame = frame_implement< frame_have_scene<frame_type>::value, frame_have_renderer<frame_type>::value, frame_type>;
		template<typename frame_type> using frame_channel = typename frame<frame_type>::channel;
		
		template<typename frame_type> class frame_viewer
		{
			typename frame_have_form<frame_type>::viewer form_viewer;
			typename frame_have_renderer<frame_type>::viewer renderer_viewer;
			typename frame_have_scene<frame_type>::viewer scene_viewer;
		public:
			decltype(auto) get_form() { return form_viewer; }
			decltype(auto) get_renderer() { return renderer_viewer; }
			decltype(auto) get_scene() { return scene_viewer; }
			frame_viewer(frame<frame_type>& f) :form_viewer(f), renderer_viewer(f), scene_viewer(f) {}
		};

	}

	namespace Assistant
	{

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
			viewer(form_final<frame_type>& ff) : form_ref(ff), form_ptr(&ff), frame_viewer<frame_type>(ff) {}
			viewer(const Tool::completeness_ref& cr, form_packet<frame_type>& ff) : form_ref(cr), form_ptr(&ff), frame_viewer<frame_type>(ff) {}
			viewer() {}
			viewer(const viewer&) = default;
			template<typename plugin_type, typename ...AK> decltype(auto) create_plugin(AK&&...ak);
		};

		template<typename frame_type>
		class initial : public viewer<frame_type>
		{
			plugin_self plugin_data;
			frame_channel<frame_type> channel_data;
		public:
			operator plugin_self& () { return plugin_data; }
			operator frame_channel<frame_type>& () { return channel_data; }
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
					(
						[&](auto& plugin) { plugin.init(ticker_data); },
						[this](auto& plugin) {},
						(plugin_data)
						);
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
		};

		template<typename frame_type> class form_packet : public frame<frame_type>
		{
			plugin_append all_plugin;

			template<typename ...AK> form_packet(std::true_type, const Tool::completeness_ref& cr, AK&&... ak) : 
				frame<frame_type>(cr, std::forward<AK>(ak)...)
				{}
			template<typename ...AK> form_packet(std::false_type, const Tool::completeness_ref& cr, AK&&... ak) : 
				frame<frame_type>(std::forward<AK>(ak)...)
				{}

			friend class viewer<frame_type>;

		public:

			template<typename ...AK> form_packet(const Tool::completeness_ref& cr, AK&&... ak) :
				form_packet(
					std::integral_constant<bool, std::is_constructible<frame<frame_type>, const Tool::completeness_ref&, AK... >::value>(),
					cr, std::forward<AK>(ak)...
				) {}

			void tick(time_point da)
			{
				all_plugin.tick(da);
				frame<frame_type>::tick(da);
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
