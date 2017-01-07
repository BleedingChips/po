#pragma once
#include "define.h"
#include "../tool/thread_tool.h"
#include "../tool/auto_adapter.h"
#include <atomic>
#include <list>
#include <future>
namespace PO
{
	namespace Implement
	{
		template<typename frame> struct form_packet;
	}

	class form_self
	{
		std::mutex record_mutex;
		time_calculator record;
		Tool::completeness_ref cr;
		std::atomic_bool available;
		template<typename frame> friend struct Implement::form_packet;
		Tool::optional<duration> tick(time_point tp)
		{
			std::lock_guard<decltype(record_mutex)> lg(record_mutex);
			duration da;
			if (record.tick(tp, da))
				return da;
			return{};
		}

	public:
		operator bool() const noexcept { return available; }
		form_self(Tool::completeness_ref c) : cr(std::move(c)), available(true) {}
		void close() { available = false; }
	};

	class plugin_self
	{
		Tool::completeness_ref cr;
	protected:
		plugin_self(const Tool::completeness_ref& c) :cr(c), avalible(true) {}
	public:
		virtual ~plugin_self() {}
		bool use_tick = true;
		bool use_event = true;
		std::atomic_bool avalible;
		operator bool() const { return avalible; }
	};

	struct default_viewer_or_ticker
	{
		template<typename T> default_viewer_or_ticker(T&& t) {}
	};

	namespace Implement
	{
		template<typename frame, typename = void> struct frame_have_form :std::false_type {};
		template<typename frame> struct frame_have_form<frame, std::void_t<typename frame::form>> :std::true_type {};
		
		template<typename frame, typename = void> struct frame_have_viewer : std::false_type {};
		template<typename frame> struct frame_have_viewer <frame, std::void_t<typename frame::viewer>> : std::true_type
		{
			static_assert(std::is_constructible<typename frame::viewer, typename frame::form&>::value, "");
			static_assert(std::is_constructible<typename frame::viewer, const typename frame::viewer&>::value, "");
		};

		template<typename frame, typename = void> struct frame_have_ticker : std::false_type {};
		template<typename frame> struct frame_have_ticker <frame, std::void_t<typename frame::ticker>> : std::true_type 
		{
			static_assert(std::is_constructible<typename frame::ticker, typename frame::form&>::value, "");
		};

		template<typename frame> struct frame_assert
		{
			static_assert(frame_have_form<frame>::value, "");
			using viewer = frame_have_viewer<frame>;
			using ticker = frame_have_ticker<frame>;
		};

		template<typename frame> struct picker_viewer { using type = typename frame::viewer; };
		template<typename frame> struct picker_ticker { using type = typename frame::ticker; };
		//template<typename frame> struct picker_default { using type = default_viewer_or_ticker; };

		template<typename frame> using frame_viewer= typename std::conditional_t<frame_have_viewer<frame>::value, Tmp::instant<picker_viewer>, Tmp::instant<Tmp::itself<default_viewer_or_ticker>::template in_t>>::template in_t<frame>::type;
		template<typename frame> using frame_ticker = typename std::conditional_t<frame_have_ticker<frame>::value, Tmp::instant<picker_ticker>, Tmp::instant<Tmp::itself<default_viewer_or_ticker>::template in_t>>::template in_t<frame>::type;
		template<typename frame> using frame_form = typename frame::form;

		template<typename ticker_t> class plugin_append;
	}

	template<typename ticker_t> class ticker : public ticker_t
	{
		form_self* form;
		Implement::plugin_append<ticker_t>* data;
	public:
		template<typename form>
		ticker(form& fv, form_self& fs, Implement::plugin_append<ticker_t>& pa) : ticker_t(fv), form(&fs), data(&pa) {}
		ticker(ticker&& v) = default;
		ticker(const ticker& v) = default;
		template<typename plugin_t, typename ...AK> auto create_plugin(AK&& ...al)
		{
			return data->template create_plugin<plugin_t>(*this, std::forward<AK>(al)...);
		}
	};

	template<typename frame> class viewer : public Implement::frame_viewer<frame>
	{
		form_self* form;
		Implement::plugin_append<Implement::frame_ticker<frame>>* plugin;
	public:
		viewer(Implement::frame_form<frame>& ff, form_self& fs, Implement::plugin_append<Implement::frame_ticker<frame>>& pa) : Implement::frame_viewer<frame>(ff), form(&fs), plugin(&pa) {}
		viewer(viewer&& v) = default;
		viewer(const viewer& v) = default;
		template<typename plugin_t, typename ...AK> auto create_plugin(AK&& ...al)
		{
			return plugin->template create_plugin<plugin_t>(*this, std::forward<AK>(al)...);
		}
	};

	template<typename frame> class viewer_packet
	{
		Tool::completeness_ref ref;
		viewer<frame> view;
	public:
		viewer_packet(const Tool::completeness_ref& cr, Implement::frame_form<frame>& ff, form_self& fs, Implement::plugin_append<Implement::frame_ticker<frame>>& pa) : ref(cr), view(ff, fs, pa) {}
		viewer_packet(const viewer_packet&) = default;
		viewer_packet(viewer_packet&&) = default;
		template<typename T>
		decltype(auto) lock_if(T&& t)
		{
			return ref.lock_if(
				[&, this]() 
			{
				t(view);
			}
			);
		}
	};

	template<typename ticker_t>
	class plugin_interface : public plugin_self
	{
		template<typename ticker_t> friend  class Implement::plugin_append;
		virtual void plug_init(form_self& fs, ticker<ticker_t>& t) = 0;
		virtual void plug_tick(duration tp, form_self& fs, ticker<ticker_t>& t) = 0;
	protected:
		using plugin_self::plugin_self;
	public:
	};

	namespace Implement
	{

		template<typename plugin_t, typename ticker_t> struct plugin_have_plug_init
		{
			template<typename T>
			static std::true_type func(decltype(Tmp::itself<T>{}().plug_init(Tmp::itself<form_self&>{}(), Tmp::itself<plugin_interface<ticker_t>&>{}(), Tmp::itself<ticker<ticker_t>&>{}()))*);
			template<typename T>
			static std::false_type func(...);
		public:
			static constexpr bool value = decltype(func<plugin_t>(nullptr))::value;
		};

		template<typename plugin_t, typename ticker_t> struct plugin_have_plug_tick
		{
			template<typename T>
			static std::true_type func(decltype(Tmp::itself<T>{}().plug_tick(Tmp::itself<duration>{}(), Tmp::itself<form_self&>{}(), Tmp::itself<plugin_interface<ticker_t>&>{}(), Tmp::itself<ticker<ticker_t>&>{}()))*);
			template<typename T>
			static std::false_type func(...);
		public:
			static constexpr bool value = decltype(func<plugin_t>(nullptr))::value;
		};




		template<typename plugin_t, typename ticker_t> class plugin_implement : public plugin_interface<ticker_t>
		{
			plugin_t data;

			template<typename view_or_tick, typename ...AK>
			plugin_implement(std::true_type, const Tool::completeness_ref& cpr, view_or_tick& vt, AK&& ...ak) :
				plugin_interface<ticker_t>(cpr), data(static_cast<plugin_interface<ticker_t>&>(*this), vt, std::forward<AK>(ak)...) {}

			template<typename view_or_tick, typename ...AK>
			plugin_implement(std::false_type, std::true_type, const Tool::completeness_ref& cpr, view_or_tick& t, AK&& ...ak) :
				plugin_interface<ticker_t>(cpr), data(static_cast<plugin_interface<ticker_t>&>(*this), std::forward<AK>(ak)...)
			{

			}

			template<typename view_or_tick, typename ...AK>
			plugin_implement(std::false_type, std::false_type, const Tool::completeness_ref& cpr, view_or_tick& t, AK&& ...ak) :
				plugin_interface<ticker_t>(cpr), data(std::forward<AK>(ak)...)
			{
				static_assert(std::is_constructible<plugin_t, AK...>::value, "can not construct plugin form this");
			}

			template<typename view_or_tick, typename ...AK>
			plugin_implement(std::false_type ft, const Tool::completeness_ref& cpr, view_or_tick& t, AK&& ...ak) :
				plugin_implement(ft, std::integral_constant<bool, std::is_constructible<plugin_t, plugin_interface<ticker_t>&, AK...>::value>{}, cpr, t, std::forward<AK>(ak)...)
			{
				
			}

		public:

			template<typename view_or_tick, typename ...AK>
			plugin_implement(const Tool::completeness_ref& cpr, view_or_tick& t, AK&& ...ak) :
				plugin_implement(
					std::integral_constant<bool, std::is_constructible<plugin_t, plugin_self&, view_or_tick&, AK... >::value>{},
					cpr, t, std::forward<AK>(ak)...
				)
			{
			}

			virtual void plug_init(form_self& fs, ticker<ticker_t>& t) override
			{
				Tool::statement_if<plugin_have_plug_init<plugin_t, ticker_t>::value>
					(
						[&, this](auto& a) { a.plug_init(fs, *this, t); },
						[](auto& a) {},
						data
						);
			}
			virtual void plug_tick(duration da, form_self& fs, ticker<ticker_t>& t) override
			{
				Tool::statement_if<plugin_have_plug_tick<plugin_t, ticker_t>::value>
					(
						[&, this](auto& a) { a.plug_tick(da, fs, *this, t); },
						[](auto& a) {},
						data
						);
			}

			/*
			bool event_respond_implement(event& ev) override
			{
				all_data.message = ev;
				return Tool::statement_if<able_to_call_event_respond<frame_type, plugin_type>::value>
				(
				[&](auto& plugin) { return plugin.event_respond(responder<frame_type>(all_data)); },
				[this](auto& plugin) {use_tick = false; return false; },
				(plugin_data)
				);
			}*/
		};

		template<typename plugin_t, typename ticker_t> using plugin_final = Tool::completeness<plugin_implement<plugin_t, ticker_t>>;

		template<typename ticker_t> class plugin_append
		{
			std::mutex pim;
			std::list<std::unique_ptr<plugin_interface<ticker_t>>> inilizered_plugin_list;
			std::list<std::unique_ptr<plugin_interface<ticker_t>>> plugin_list;
			ticker<ticker_t> tick;
		public:
			template<typename form>
			plugin_append(form& fv, form_self& fs) : tick(fv, fs, *this) {}
			void plug_tick(duration da, form_self& fs)
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
						(ptr)->plug_init(fs, tick);
					plugin_list.splice(plugin_list.end(), std::move(temporary_list), temporary_list.begin(), temporary_list.end());
				}
				for (auto ptr = plugin_list.begin(); ptr != plugin_list.end();)
				{
					if ((*ptr) && (**ptr))
					{
						(*ptr++)->plug_tick(da, fs, tick);
						continue;
					}
					plugin_list.erase(ptr++);
				}
			}
			template<typename plugin_t, typename view_or_ticker, typename ...AK> auto create_plugin(view_or_ticker& vt, AK&&... ak)
			{
				auto ptr = std::make_unique<plugin_final<plugin_t, ticker_t>>(vt, std::forward<AK>(ak)...);
				{
					std::lock_guard<decltype(pim)> lg(pim);
					inilizered_plugin_list.push_back(std::move(ptr));
				}
			}
		};

		template<typename form> struct form_have_form_tick
		{
			template<typename T>
			static std::true_type func(decltype(Tmp::itself<T>{}().form_tick(Tmp::itself<duration>{}(), Tmp::itself<form_self&>{}()))*);
			template<typename T>
			static std::false_type func(...);
		public:
			static constexpr bool value = decltype(func<form>(nullptr))::value;
		};

		template<typename frame>
		struct form_packet
		{
			form_self self;
			Implement::frame_form<frame> form_data;
			plugin_append<Implement::frame_ticker<frame>> plugin_data;

			template<typename ...AT> form_packet(std::true_type, const Tool::completeness_ref& cr, AT&& ...at) :
				self(cr), form_data(self, std::forward<AT>(at)...), plugin_data(form_data, self)
			{
			}

			template<typename ...AT> form_packet(std::false_type, const Tool::completeness_ref& cr, AT&& ...at) :
				self(cr), form_data(std::forward<AT>(at)...), plugin_data(form_data, self)
			{
			}

		public:
			template<typename ...AT> form_packet(const Tool::completeness_ref& cr, AT&& ...at) :
				form_packet(
					std::integral_constant<bool, std::is_constructible<Implement::frame_form<frame>, form_self&, AT...>::value>{},
					std::move(cr), std::forward<AT>(at)...
				)
			{
			}

			operator bool() { return self; }
			/*
			template<typename plugin, typename view_or_tick, typename ...AK> decltype(auto) create_plugin(view_or_tick& ff, AK&& ...ak)
			{
				auto ptr = std::make_unique<plugin_final<plugin, form>>(ff, std::forward<AK>(ak)...);
				std::lock_guard<decltype(pim)> lg(pim);
				inilizered_plugin_list.push_back(std::move(ptr));
			}
			*/
			void tick(time_point tp)
			{
				auto dua = self.tick(tp);
				if (dua)
				{
					Tool::statement_if<form_have_form_tick<Implement::frame_form<frame>>::value>
						(
							[](auto& a, auto& fs, auto da) mutable {a.form_tick(da, fs); },
							[](auto& a, auto& fs, auto da) {},
							form_data, self, *dua
							);
					plugin_data.plug_tick(*dua, self);
				}
				/*
				self.reored.tick(da,
					[&, this](duration dua) 
				{
					Tool::statement_if<form_have_func_tick<form>::value>
						(
							[dua](auto& t) {t.tick(dua); }
					)(form_data);
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
							(ptr)->init(ticker);
						plugin_list.splice(plugin_list.end(), std::move(temporary_list), temporary_list.begin(), temporary_list.end());
					}
					for (auto ptr = plugin_list.begin(); ptr != plugin_list.end();)
					{
						if ((*ptr) && (**ptr))
						{
							(*ptr++)->tick(dua, ticker);
							continue;
						}
						plugin_list.erase(ptr++);
					}
				}
					);
				*/
				
			}

			/*
			bool event_function(event ev)
			{
				for (auto ptr = plugin_list.begin(); ptr != plugin_list.end();)
				{
					if ((*ptr) && (**ptr))
					{
						if ((*ptr++)->event_respond(ev))
							return true;
						continue;
					}
					plugin_list.erase(ptr++);
				}
				return false;
			}
			*/
		};

		template<typename frame> using form_final = Tool::completeness<form_packet<frame>>;

		struct form_ptr
		{
			bool avalible;
			std::thread logic_form_thread;
			std::atomic_bool force_exist_form;
			virtual ~form_ptr();
			template<typename frame, typename ...AK> auto create_window(AK&& ...ak)
			{
				if (logic_form_thread.joinable())
				{
					force_exist_form = true;
					logic_form_thread.join();
				}
				force_exist_form = false;
				std::promise<std::unique_ptr<viewer_packet<frame>>> p;
				auto fur = p.get_future();
				force_exist_form = false;
				logic_form_thread = std::thread(
					[&, this]()
				{
					form_final<frame> packet(std::forward<AK>(ak)...);
					//viewer_packer(const Tool::completeness_ref& cr, frame_form<frame>& ff, form_self& fs, Implement::plugin_append<frame_ticker<frame>>& pa) : ref(cr), view(ff, fs, pa) {}
					p.set_value(std::make_unique<viewer_packet<frame>>(packet,packet.form_data,packet.self, packet.plugin_data));
					time_point start_loop = std::chrono::system_clock::now();
					while (!force_exist_form  && packet)
					{
						start_loop = std::chrono::system_clock::now();
						packet.tick(start_loop);
						std::this_thread::sleep_until(start_loop + duration(1));
					}
				}
				);
				fur.wait();
				auto tem = std::move(fur.get());
				return *tem;
			}
			void force_stop() { force_exist_form = true; }
		};

	}

}






/*
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
			static constexpr bool use_interface = std::is_constructible<typename pre::mod, typename append::mod_interface&>::value;
			static constexpr bool use_mod = std::is_constructible<typename pre::mod, typename append::mod&>::value;
			static constexpr bool value = use_interface || use_mod;
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
		template<typename T> struct able_operator_to_call_tick < T, std::void_t<decltype((*((T*)(nullptr))).tick(time_point{})) >> : std::true_type {};

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
				frame_renderer_implement(std::integral_constant<bool, append_pair_assert<typename frame::renderer, typename frame::form>::use_interface>(), std::forward<any_parameter>(ap)...) {}
			void tick(time_point da)
			{
				Tool::statement_if<able_operator_to_call_tick<typename frame::renderer::mod>::value>
					([&da](auto& f) { f.tick(da); })
					(renderer_data);
				frame_form_implement<frame>::tick(da);
			}
		private:
			template<typename ...any_parameter> frame_renderer_implement(std::true_type, any_parameter&&... ap) :frame_form_implement<frame>(std::forward<any_parameter>(ap)...),
				renderer_data(typename frame::form::mod_interface(static_cast<typename frame::form::mod&>(*this))) {}
			template<typename ...any_parameter> frame_renderer_implement(std::false_type, any_parameter&&... ap) : frame_form_implement<frame>(std::forward<any_parameter>(ap)...),
				renderer_data(static_cast<typename frame::form::mod&>(*this)) {}
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
				frame_scene_implement(std::integral_constant<bool, append_pair_assert<typename frame::scene, typename frame::renderer>::use_interface>(), std::forward<any_parameter>(ap)...) {}
			void tick(time_point da)
			{
				Tool::statement_if<able_operator_to_call_tick<typename frame::scene::mod>::value>
					([&da](auto& f) { f.tick(da); })
					(scene_data);
				frame_renderer_implement<true, frame>::tick(da);
			}
		private:
			template<typename ...any_parameter> frame_scene_implement(std::true_type, any_parameter&&... ap) :frame_renderer_implement<true, frame>(std::forward<any_parameter>(ap)...),
				scene_data(typename frame::renderer::mod_interface(static_cast<typename frame::form::mod&>(*this))) {}
			template<typename ...any_parameter> frame_scene_implement(std::false_type, any_parameter&&... ap) : frame_renderer_implement<true, frame>(std::forward<any_parameter>(ap)...),
				scene_data(static_cast<typename frame::renderer::mod&>(*this)) {}
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
			bool use_event = true;
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
			bool event_respond(event& da)
			{
				return use_event &&  event_respond_implement(da);
			}
			virtual bool event_respond_implement(event& da) = 0;
		};

		struct self_control
		{
			Tool::completeness_ref plugin_ref;
			plugin_control* plugin_ptr = nullptr;
		public:
			operator bool() const { return plugin_ptr != nullptr; }
			operator const Tool::completeness_ref& () { return plugin_ref; }
			self_control() {}
			self_control(plugin_control& pc) : plugin_ref(pc.cr), plugin_ptr(&pc) {}
			void destory_self()
			{
				plugin_ptr->avalible = false;
			}
			void set_duration(duration da)
			{
				plugin_ptr->record.set_duration(da);
			}
			void tick_state(bool s = true)
			{
				plugin_ptr->use_tick = s;
			}
			void event_state(bool s = true)
			{
				plugin_ptr->use_event = s;
			}
			template<typename T>
			bool lock_if(T&& t) {
				return plugin_ref.lock_if(std::forward<T>(t));
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

		template<typename frame_type> struct carrier;
		template<typename frame_type> class viewer : public frame_viewer<frame_type>
		{
			Tool::completeness_ref form_ref;
			form_packet<frame_type>* form_ptr = nullptr;
			friend struct carrier<frame_type>;
		public:
			viewer(form_final<frame_type>& ff) : frame_viewer<frame_type>(ff), form_ref(ff), form_ptr(&ff) {}
			viewer(const Tool::completeness_ref& cr, form_packet<frame_type>& ff) : frame_viewer<frame_type>(ff), form_ref(cr), form_ptr(&ff) {}
			viewer() {}
			viewer(const viewer&) = default;
			template<typename plugin_type, typename ...AK> decltype(auto) create_plugin(AK&&...ak);
		};

		template<typename frame_type>
		struct carrier
		{
			viewer<frame_type> viewer_data;
			self_control plugin_data;
			typename frame<frame_type>::channel channel_data;
			duration duration_data;
			event message;
			carrier(viewer<frame_type>& v, plugin_control& pp) : viewer_data(v), plugin_data(pp), channel_data(*v.form_ptr) {}
		};

		template<typename frame_type>
		class initial
		{
			carrier<frame_type>& carr;
		public:
			initial(carrier<frame_type>& c) :carr(c) {}
			initial(const initial&) = default;
			operator self_control& () { return carr.plugin_data; }
			decltype(auto) get_form() { return carr.viewer_data.get_form(); }
			decltype(auto) get_renderer() { return carr.viewer_data.get_renderer(); }
			decltype(auto) get_scene() { return carr.viewer_data.get_scene(); }
			decltype(auto) get_self() { return carr.plugin_data; }
			decltype(auto) get_channel() { return carr.channel_data; }
		};

		class ticker_self
		{
			duration da;
			self_control plu;
		public:
			ticker_self(duration d, self_control p) : da(d), plu(p) {}
			ticker_self(const ticker_self&) = default;
			decltype(auto) get_self() { return plu; }
			decltype(auto) get_time() { return da; }
		};

		class responder_self
		{
			event& da;
			self_control plu;
		public:
			responder_self(event& d, self_control p) : da(d), plu(p) {}
			responder_self(const responder_self&) = default;
			decltype(auto) get_self() { return plu; }
			decltype(auto) get_event() { return da; }
		};

		template<typename frame_type>
		class ticker
		{
			carrier<frame_type>& carr;
		public:
			operator ticker_self() {
				return ticker_self{get_time(),get_self()};
			}
			operator self_control () { return carr.plugin_data; }
			operator duration() { return carr.duration_data; }
			ticker(carrier<frame_type>& c) :carr(c) {}
			ticker(const ticker&) = default;
			decltype(auto) get_form() { return carr.viewer_data.get_form(); }
			decltype(auto) get_renderer() { return carr.viewer_data.get_renderer(); }
			decltype(auto) get_scene() { return carr.viewer_data.get_scene(); }
			decltype(auto) get_self() { return carr.plugin_data; }
			decltype(auto) get_channel() { return carr.channel_data; }
			decltype(auto) get_time() { return carr.duration_data; }
		};

		template<typename frame_type>
		class responder
		{
			carrier<frame_type>& carr;
		public:
			operator responder_self() {
				return responder_self{ get_event(),get_self() };
			}
			operator self_control () { return carr.plugin_data; }
			operator event& () { return carr.message; }
			responder(carrier<frame_type>& c) :carr(c) {}
			responder(const responder&) = default;
			decltype(auto) get_form() { return carr.viewer_data.get_form(); }
			decltype(auto) get_renderer() { return carr.viewer_data.get_renderer(); }
			decltype(auto) get_scene() { return carr.viewer_data.get_scene(); }
			decltype(auto) get_self() { return carr.plugin_data; }
			decltype(auto) get_channel() { return carr.channel_data; }
			decltype(auto) get_event() { return carr.message; }
		};

		template<typename frame_type, typename plugin_type, typename = void> struct able_to_call_tick :std::false_type {};
		template<typename frame_type, typename plugin_type> struct able_to_call_tick<frame_type, plugin_type, std::void_t<decltype(std::declval<plugin_type>().tick(std::declval<ticker<frame_type>>()))>> :std::true_type {};
		template<typename frame_type, typename plugin_type, typename = void> struct able_to_call_init :std::false_type {};
		template<typename frame_type, typename plugin_type> struct able_to_call_init<frame_type, plugin_type, std::void_t<decltype(std::declval<plugin_type>().init(std::declval<initial<frame_type>>()))>> :std::true_type {};
		template<typename frame_type, typename plugin_type, typename = void> struct able_to_call_event_respond :std::false_type {};
		template<typename frame_type, typename plugin_type> struct able_to_call_event_respond<frame_type, plugin_type, std::void_t<decltype(std::declval<plugin_type>().event_respond(std::declval<responder<frame_type>>()))>> :std::true_type {};

		template<typename frame_type, typename plugin_type> class plugin_implement : public plugin_control
		{
			plugin_type plugin_data;
			using form_type = typename frame_type::form;

			carrier<frame_type> all_data;

			template<typename ...AK>
			plugin_implement(std::true_type, const Tool::completeness_ref& cpr, viewer<frame_type>& ff, AK&& ...ak) :
				plugin_control(cpr), plugin_data(ff, std::forward<AK>(ak)...), all_data(ff, *this) {}
			template<typename ...AK>
			plugin_implement(std::false_type, const Tool::completeness_ref& cpr, viewer<frame_type>& ff, AK&& ...ak) :
				plugin_control(cpr), plugin_data(std::forward<AK>(ak)...), all_data(ff, *this) {}

		public:

			template<typename ...AK>
			plugin_implement(const Tool::completeness_ref& cpr, viewer<frame_type>& ff, AK&& ...ak) :
				plugin_implement(
					std::integral_constant<bool, std::is_constructible<plugin_type, viewer<frame_type>, AK... >::value>(),
					cpr, ff, std::forward<AK>(ak)...
				)
			{
				
			}

			virtual void init()
			{
				Tool::statement_if<able_to_call_init<frame_type, plugin_type>::value>
					([&](auto& plugin) { plugin.init(initial<frame_type>(all_data)); })
					(plugin_data);
			}

			void tick_implementation(duration ti) override
			{
				all_data.duration_data = ti;
				Tool::statement_if<able_to_call_tick<frame_type, plugin_type>::value>
					(
						[&](auto& plugin) { plugin.tick(ticker<frame_type>(all_data)); },
						[this](auto& plugin) {use_tick = false; },
						(plugin_data)
						);
			}

			bool event_respond_implement(event& ev) override
			{
				all_data.message = ev;
				return Tool::statement_if<able_to_call_event_respond<frame_type, plugin_type>::value>
					(
						[&](auto& plugin) { return plugin.event_respond(responder<frame_type>(all_data)); },
						[this](auto& plugin) {use_tick = false; return false; },
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
				for (auto ptr = plugin_list.begin(); ptr != plugin_list.end();)
				{
					if ((*ptr) && (**ptr))
					{
						if ((*ptr++)->event_respond(ev))
							return true;
						continue;
					}
					plugin_list.erase(ptr++);
				}
				return false;
			}
		};

		template<typename frame_type> class alignas(std::alignment_of<frame<frame_type>>::value) form_packet : public frame<frame_type>
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
*/
