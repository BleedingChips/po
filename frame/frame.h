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
		template<typename T> class plugin_interface;
		template<typename plugin_t, typename ticker_t> class plugin_implement;
		template<typename frame> struct form_packet;
		struct form_ptr;
		class thread_task_runer;
		struct ticker_init_type {};
		template<typename frame> struct viewer_init_type {};
		struct constor_init_type {};
	}

	class thread_task
	{
		enum class State :uint8_t
		{
			READY,
			WAITING,
			FINISH
		};

		Tool::completeness_ref ref;
		std::mutex task_mutex;
		std::condition_variable cv;

		std::atomic_bool bad;
		std::atomic<State> task_state;

		friend class Implement::thread_task_runer;
	protected:
		void set_bad() { bad = true; }
	public:
		thread_task(Tool::completeness_ref r) : ref(r), bad(false) , task_state(State::READY) {}
		virtual bool operator()() = 0;
		bool is_bad() const { return bad; }
		bool is_finish() { return task_state == State::FINISH; task_state = State::READY; }
		void wait_finish()
		{
			std::unique_lock<std::mutex> lk(task_mutex);
			cv.wait(lk, [this]() { return task_state == State::FINISH; });
			task_state = State::READY;
		}
	};

	namespace Implement
	{
		class thread_task_runer
		{
			using set_type = std::vector<std::weak_ptr<thread_task>>;
			std::mutex input_mutex;
			set_type input;
			set_type calling;
			std::atomic_bool exit;
			std::thread main;
			void thread_funtion();
		public:
			thread_task_runer();
			~thread_task_runer();
			bool push_task(std::weak_ptr<thread_task> task);
		};
	}

	class form_self
	{
		std::mutex record_mutex;
		time_calculator record;
		Tool::completeness_ref cr;
		std::atomic_bool available;
		Implement::thread_task_runer ttr;
		template<typename frame> friend struct Implement::form_packet;
		Tool::optional<duration> tick(time_point tp)
		{
			std::lock_guard<decltype(record_mutex)> lg(record_mutex);
			duration da;
			if (record.tick(tp, da))
				return da;
			return{};
		}

		form_self(Tool::completeness_ref c) : cr(std::move(c)), available(true) {}
		friend struct Implement::form_ptr;
		template<typename T> friend struct Implement::form_packet;

	public:

		bool push_task(std::weak_ptr<thread_task> task) { return ttr.push_task(std::move(task)); }
		operator bool() const noexcept { return available; }
		operator const Tool::completeness_ref&() const { return cr; }
		void close() { available = false; }
	};

	class plugin_self
	{
		Tool::completeness_ref cr;
		plugin_self(const Tool::completeness_ref& c) :cr(c), avalible(true) {}
		template<typename T> friend class Implement::plugin_interface;
		template<typename plugin_t, typename ticker_t> friend class Implement::plugin_implement;
	public:
		virtual ~plugin_self() {}
		bool use_tick = true;
		bool use_event = true;
		std::atomic_bool avalible;
		operator bool() const { return avalible; }
		operator Tool::completeness_ref() const { return cr; };
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

		struct default_viewer_or_ticker
		{
			template<typename T> default_viewer_or_ticker(T&& t) {}
		};

		template<typename frame> using frame_viewer= typename std::conditional_t<frame_have_viewer<frame>::value, Tmp::instant<picker_viewer>, Tmp::instant<Tmp::itself<default_viewer_or_ticker>::template in_t>>::template in_t<frame>::type;
		template<typename frame> using frame_ticker = typename std::conditional_t<frame_have_ticker<frame>::value, Tmp::instant<picker_ticker>, Tmp::instant<Tmp::itself<default_viewer_or_ticker>::template in_t>>::template in_t<frame>::type;
		template<typename frame> using frame_form = typename frame::form;

		template<typename ticker_t> class plugin_append;
	}

	class constor;
	class ticker_tl;

	namespace Implement
	{
		struct plugin_tl_interface : public plugin_self
		{
			virtual void tick(ticker_tl& tt) = 0;
			virtual void init(ticker_tl& tt) = 0;
			virtual ~plugin_tl_interface() {}
		};

		template<typename plugin_t> class plugin_have_tick_tl
		{
			template<typename T, void (T::*)(ticker_tl&)> struct del;
			template<typename T> static std::true_type fun(del<T, &T::tick>*);
			template<typename T> static std::false_type fun(...);
		public:
			static constexpr bool value = decltype(fun<plugin_t>(nullptr))::value;
		};

		template<typename plugin_t> class plugin_have_init_tl
		{
			template<typename T, void (T::*)(ticker_tl&)> struct del;
			template<typename T> static std::true_type fun(del<T, &T::init>*);
			template<typename T> static std::false_type fun(...);
		public:
			static constexpr bool value = decltype(fun<plugin_t>(nullptr))::value;
		};

		template<typename plugin_t>
		class plugin_t1_implement
		{
			plugin_t plugin_data;
			template<typename ...AK> plugin_t1_implement(std::true_type, constor& oi, AK&& ...ak) :
				plugin_data(oi, std::forward<AK>(ak)...) {}
			template<typename ...AK> plugin_t1_implement(std::false_type, constor& oi, AK&& ...ak) :
				plugin_data(std::forward<AK>(ak)...) {}
		public:
			virtual void tick(ticker_tl& tt)
			{
				Tool::statement_if<plugin_have_tick_tl<plugin_t>::value>
					(
						[](auto& plu, ticker_tl& t) {plu.tick(t); },
						[](auto& plu, ticker_tl& t) {},
						plugin_data, tt
						);
			}
			virtual void init(ticker_tl& tt)
			{
				Tool::statement_if<plugin_have_init_tl<plugin_t>::value>
					(
						[](auto& plu, ticker_tl& t) {plu.init(t); },
						[](auto& plu, ticker_tl& t) {},
						plugin_data, tt
						);
			}

			template<typename ...AK> plugin_t1_implement(constor& oi, AK&& ...ak) :
				plugin_t1_implement(
					std::integral_constant<bool, std::is_constructible<plugin_t, constor&, AK&&...>::value>{},
					oi, std::forward<AK>(ak)...
				) {}

		};

		class plugin_tl_holder
		{
			std::unique_ptr<plugin_tl_interface> inter;
		public:
			plugin_tl_holder(constor& cr, std::function<std::unique_ptr<Implement::plugin_tl_interface>(constor&)>&& up) : inter(up(cr)) {}
			void tick(ticker_tl& tt) { inter->tick(tt); }
			void init(ticker_tl& tt) { inter->init(tt); }
		};

		class plugin_append_tl
		{
			template<typename ticker_t> friend class plugin_append;
			virtual void create_plugin_tl_execute(std::function<std::unique_ptr<Implement::plugin_tl_interface>(constor&)>&& up, form_self& fs) = 0;
		public:
			template<typename plugin_t, typename ...AK>
			void create_plugin_tl(plugin_t t, form_self& fs, AK&& ...ak)
			{
				constor tem(form());
				create_plugin_tl_execute(
					[&](constor& c) {return std::make_unique<Implement::plugin_t1_implement<typename plugin_t::type>>(c, std::forward<AK>(ak)...); },
					fs
				);
			}
		};
	}

	class form_ticker
	{
		form_self& form_ref;
		duration da;
		form_ticker(form_self& fs, duration d) : form_ref(fs), da(d) {}
		template<typename frame> friend struct Implement::form_packet;
	public:
		operator form_self& () { return form_ref; }
		operator const form_self& () const { return form_ref; }
		operator duration() const { return da; }
		duration time() const { return da; }
		form_self& self() { return form_ref; }
	};

	class constor
	{
		plugin_self& plugin_ref;
		form_self& form_ref;
		Implement::plugin_append_tl& plugin_append_tl_ref;
		constor(plugin_self& p, form_self& fs, Implement::plugin_append_tl& pat) : form_ref(fs), plugin_ref(p), plugin_append_tl_ref(pat) {}
		template<typename plugin_t, typename ticker_t> friend class Implement::plugin_implement;
		friend class ticker_tl;
		friend class form_tick;
		template<typename frame> friend class constor_outside;
		template<typename frame> friend class constor_inside;
	public:
		form_self& form() { return form_ref; }
		plugin_self& self() { return plugin_ref; }
		template<typename plugin_t, typename ...AK>
		void create_plugin_tl(plugin_t t, AK&& ...ak)
		{
			plugin_append_tl_ref.create_plugin_tl(t, form_ref, std::forward<AK>(ak)...);
		}
	};

	template<typename ticker_t> class constor_inside : public constor
	{
		Implement::plugin_append<ticker_t>& plugin_append_ref;
		ticker_t& ticker_ref;
		constor_inside(plugin_self& ps, form_self& fs, Implement::plugin_append<ticker_t>& ft, ticker_t& fv) : constor(ps, fs, ft), plugin_append_ref(ft), ticker_ref(fv) {}
		template<typename plugin_t, typename ticker_t> friend class Implement::plugin_implement;
	public:
		template<typename plugin_t, typename ...AK>
		void create_plugin(plugin_t t, AK&& ...ak)
		{
			plugin.create_plugin(t, ticker_init_type{}, form(), ticker_, std::forward<AK>(ak)...);
		}
	};

	template<typename frame> class constor_outside: public constor
	{
		Implement::plugin_append<Implement::frame_ticker<frame>>& plugin_append_ref;
		Implement::frame_viewer<frame>& viewer_ref;
		constor_outside(plugin_self& ps, form_self& fs, Implement::plugin_append<Implement::frame_ticker<frame>>& ft, Implement::frame_viewer<frame>& fv) : constor(ps, fs, ft), plugin_append_ref(ft), viewer_ref(fv) {}
		template<typename plugin_t, typename ticker_t> friend class Implement::plugin_implement;
	public:
		Implement::frame_viewer<frame>& viewer() { return viewer_ref; }
		template<typename plugin_t, typename ...AK>
		void create_plugin(plugin_t t, AK&& ...ak)
		{
			plugin.create_plugin(t, Implement::viewer_init_type<frame>{}, form(), view_ref, std::forward<AK>(ak)...);
		}
	};

	class ticker_tl : public constor
	{
		duration time_data;
		ticker_tl(plugin_self& s,form_self& f, Implement::plugin_append_tl& pat, duration d) :constor(s, f, pat), time_data(d) {}
		template<typename ticker_t> friend class ticker;
		template<typename plugin_t, typename ticker_t> friend class Implement::plugin_implement;
	public:
		duration time() const { return time_data; }
	};

	template<typename ticker_t> class ticker : public ticker_tl
	{
		Implement::plugin_append<ticker_t>& plugin;
		ticker_t& ticker_;
		ticker(plugin_self& ps, form_self& fs, Implement::plugin_append<ticker_t>& ft, duration da, ticker_t& fv)
			: ticker_tl(ps, fs, ft, da), plugin(ft), ticker_(fv) {}
		template<typename plugin_t, typename ticker_t> friend class Implement::plugin_implement;
	public:
		ticker_t& tick() { return ticker_; }
		template<typename plugin_t, typename ...AK>
		void create_plugin(plugin_t t, AK&& ...ak)
		{
			plugin.create_plugin(t, ticker_init_type{}, form(), ticker_, std::forward<AK>(ak)...);
		}
	};

	template<typename frame> class viewer
	{
		form_self& form_ref;
		Implement::plugin_append<Implement::frame_ticker<frame>>& plugin;
		Implement::frame_viewer<frame> view_ref;
		friend struct Implement::form_ptr;
		template<typename T> friend class viewer_packet;
		template<typename plugin_t, typename ticker_t> friend class Implement::plugin_implement;
	public:
		Implement::frame_viewer<frame>& view() { return view_ref; }
		viewer(form_self& fs, Implement::plugin_append<Implement::frame_ticker<frame>>& ft, Implement::frame_form<frame>& fv)
			: form_ref(fs), plugin(ft), view_ref(fv) {}
		viewer(const viewer&) = default;
		template<typename plugin_t, typename ...AK>
		void create_plugin(plugin_t t, AK&& ...ak)
		{
			plugin.create_plugin(t, Implement::viewer_init_type<frame>{}, form_ref, view_ref, std::forward<AK>(ak)...);
		}
	};

	template<typename frame> class viewer_packet
	{
		Tool::completeness_ref ref;
		viewer<frame> view;
	public:
		viewer_packet(const Tool::completeness_ref& cr, form_self& fs, Implement::plugin_append<Implement::frame_ticker<frame>>& pa, Implement::frame_form<frame>& ff) 
			: ref(cr), view(fs, pa, ff) {}
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

	namespace Implement
	{

		template<typename ticker_t>
		class plugin_interface : public plugin_self
		{
			template<typename ticker_t> friend  class Implement::plugin_append;
			template<typename plugin_t, typename ticker_t> friend class Implement::plugin_implement;
			virtual void plug_init(form_self& fs, plugin_append<ticker_t>& pa, ticker_t& t, duration da) = 0;
			virtual void plug_tick(form_self& fs, plugin_append<ticker_t>& pa, ticker_t& t, duration da) = 0;
			using plugin_self::plugin_self;
		public:
		};
		
		template<typename plugin_t, typename ticker_t> struct plugin_have_init
		{
			template<typename T, void (T::*)(ticker<ticker_t>&)> struct del;
			template<typename T>
			static std::true_type func(del<T, &T::init>*);
			template<typename T>
			static std::false_type func(...);
		public:
			static constexpr bool value = decltype(func<plugin_t>(nullptr))::value;
		};

		template<typename plugin_t, typename ticker_t> struct plugin_have_tick
		{
			template<typename T, void (T::*)(ticker<ticker_t>&)> struct del;
			template<typename T>
			static std::true_type func(del<T, &T::tick>*);
			template<typename T>
			static std::false_type func(...);
		public:
			static constexpr bool value = decltype(func<plugin_t>(nullptr))::value;
		};
		

		template<typename plugin_t, typename ticker_t> class plugin_implement : public plugin_interface<ticker_t>
		{
			typename std::aligned_union<1, plugin_t>::type data;
			
			template<typename ...AK>
			plugin_implement(std::true_type, constor_init_type, const Tool::completeness_ref& cpr, form_self& f, plugin_append_tl& pa, AK&& ...ak) :
				plugin_interface<ticker_t>(cpr)
			{
				constor tem(*this, f, pa);
				new (&data) plugin_t(tem, std::forward<AK>(ak)...);
			}

			template<typename ...AK>
			plugin_implement( std::false_type, constor_init_type, const Tool::completeness_ref& cpr, form_self& f, plugin_append_tl& pa, AK&& ...ak) :
				plugin_interface<ticker_t>(cpr)
			{
				new (&data) plugin_t(std::forward<AK>(ak)...);
			}

			template<typename ...AK>
			plugin_implement(std::true_type, ticker_init_type, const Tool::completeness_ref& cpr, form_self& fs, plugin_append<ticker_t>& pa, ticker_t& t, AK&& ...ak) :
				plugin_interface<ticker_t>(cpr)
			{
				constor_inside<ticker_t> tem(*this, fs, pa, t);
				new (&data) plugin_t(tem, std::forward<AK>(ak)...);
			}
			template<typename ...AK>
			plugin_implement(std::false_type, ticker_init_type, const Tool::completeness_ref& cpr, form_self& fs, plugin_append<ticker_t>& pa, ticker_t& t, AK&& ...ak) :
				plugin_interface<ticker_t>(cpr)
			{
				new (&data) plugin_t(std::forward<AK>(ak)...);
			}

			template<typename frame, typename ...AK>
			plugin_implement(std::true_type, viewer_init_type<frame>, const Tool::completeness_ref& cpr, form_self& fs, plugin_append<ticker_t>& pa, frame_viewer<frame>& t, AK&& ...ak) :
				plugin_interface<ticker_t>(cpr)
			{
				constor_outside<frame> tem(*this, fs, pa, t);
				new (&data) plugin_t(tem,std::forward<AK>(ak)...);
			}
			template<typename frame, typename ...AK>
			plugin_implement(std::false_type, viewer_init_type<frame>, const Tool::completeness_ref& cpr, form_self& fs, plugin_append<ticker_t>& pa, frame_viewer<frame>& t, AK&& ...ak) :
				plugin_interface<ticker_t>(cpr)
			{
				new (&data) plugin_t(std::forward<AK>(ak)...);
			}
			

		public:

			
			template<typename ...AK>
			plugin_implement(const Tool::completeness_ref& cpr, constor_init_type it, form_self& fs, plugin_append_tl& pa, AK&& ...ak) :
				plugin_implement(
					std::integral_constant<bool, std::is_constructible<plugin_t, constor&, AK&&... >::value>{},
					it,
					cpr, fs, pa, std::forward<AK>(ak)...
				) {}
			
			template< typename ...AK>
			plugin_implement(const Tool::completeness_ref& cpr, ticker_init_type it, form_self& fs, plugin_append<ticker_t>& pa, ticker_t& t, AK&& ...ak) :
				plugin_implement(
					std::integral_constant<bool, std::is_constructible<plugin_t, constor_inside<ticker_t>&, AK&&... >::value>{},
					it,
					cpr, fs, pa, t, std::forward<AK>(ak)...
				) {}

			template<typename frame, typename ...AK>
			plugin_implement(const Tool::completeness_ref& cpr, viewer_init_type<frame> vit, form_self& fs, plugin_append<ticker_t>& pa, frame_viewer<frame>& t, AK&& ...ak) :
				plugin_implement(
					std::integral_constant<bool, std::is_constructible<plugin_t, constor_outside<frame>&, AK&&... >::value>{},
					vit,
					cpr, fs, pa, t, std::forward<AK>(ak)...
				) {}

			~plugin_implement()
			{
				reinterpret_cast<plugin_t*>(&data) -> ~plugin_t();
			}

			virtual void plug_init(form_self& fs, plugin_append<ticker_t>& pa, ticker_t& t, duration da) override
			{
				ticker<ticker_t> tem(*this, fs, pa, da, t);
				Tool::statement_if<plugin_have_init<plugin_t, ticker_t>::value>
					(
						[](auto& a, ticker<ticker_t>& t) { a.init(t); },
						[](auto& a, ticker<ticker_t>& t) 
				{
					Tool::statement_if<plugin_have_init_tl<plugin_t>::value>
						(
							[](auto& a, ticker<ticker_t>& t) { a.init(t); },
							[](auto& a, ticker<ticker_t>& t) {},
							a, t
							);
				},
						*(reinterpret_cast<plugin_t*>(&data)), tem
						);
			}
			virtual void plug_tick(form_self& fs, plugin_append<ticker_t>& pa, ticker_t& t, duration da) override
			{
				ticker<ticker_t> tem(*this, fs, pa, da, t);
				Tool::statement_if<plugin_have_tick<plugin_t, ticker_t>::value>
					(
						[](auto& a, ticker<ticker_t>& t) { a.tick(t); },
						[](auto& a, ticker<ticker_t>& t) 
				{
					Tool::statement_if<plugin_have_tick_tl<plugin_t>::value>
						(
							[](auto& a, ticker<ticker_t>& t) { a.tick(t); },
							[](auto& a, ticker<ticker_t>& t) {},
							a, t
							);
				},
						*(reinterpret_cast<plugin_t*>(&data)), tem
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

		template<typename ticker_t> class plugin_append : public plugin_append_tl
		{
			std::mutex pim;
			std::list<std::unique_ptr<plugin_interface<ticker_t>>> inilizered_plugin_list;
			std::list<std::unique_ptr<plugin_interface<ticker_t>>> plugin_list;
			ticker_t tick;

			virtual void create_plugin_tl_execute(std::function<std::unique_ptr<Implement::plugin_tl_interface>(constor&)>&& up, form_self& fs)
			{
				this->create_plugin(Tmp::itself<plugin_tl_holder>{}, constor_init_type{}, fs, std::move(up));
			}

		public:

			template<typename plugin_t, typename init_type, typename ...AK> auto create_plugin(plugin_t t, init_type it, form_self& fs, AK&&... ak)
			{
				auto ptr = std::make_unique<plugin_final<typename plugin_t::type, ticker_t>>(it, fs, *this, std::forward<AK>(ak)...);
				{
					std::lock_guard<decltype(pim)> lg(pim);
					inilizered_plugin_list.push_back(std::move(ptr));
				}
			}
			template<typename form>
			plugin_append(form& fv) : tick(fv) {}
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
						(ptr)->plug_init(fs, *this, tick , da);
					plugin_list.splice(plugin_list.end(), std::move(temporary_list), temporary_list.begin(), temporary_list.end());
				}
				for (auto ptr = plugin_list.begin(); ptr != plugin_list.end();)
				{
					if ((*ptr) && (**ptr))
					{
						(*ptr++)->plug_tick(fs, *this, tick, da);
						continue;
					}
					plugin_list.erase(ptr++);
				}
			}
			
		};

		template<typename form, typename = void> struct form_call_pre_tick
		{
			void operator() (form& f, form_self& fs, duration da){}
		};
		template<typename form> struct form_call_pre_tick < form, std::void_t<decltype(Tmp::itself<form>{}().pre_tick(Tmp::itself<form_tick&>{}())) >>
		{
			void operator() (form& f, form_self& fs, duration da) 
			{
				form_ticker tem(fs, da);
				f.pre_tick(tem);
			}
		};

		template<typename form> class form_have_tick
		{
			template<typename T, void (T::*)(form_ticker&)> struct del;
			template<typename T> static std::true_type fun(del<T, &T::tick>*);
			template<typename T> static std::false_type fun(...);
		public:
			static constexpr bool value = decltype(fun<form>(nullptr))::value;
		};

		template<typename frame>
		struct form_packet
		{
			form_self self;
			Implement::frame_form<frame> form_data;
			plugin_append<Implement::frame_ticker<frame>> plugin_data;

			template<typename ...AT> form_packet(std::true_type, const Tool::completeness_ref& cr, AT&& ...at) :
				self(cr), form_data(self, std::forward<AT>(at)...), plugin_data(form_data)
			{
			}

			template<typename ...AT> form_packet(std::false_type, const Tool::completeness_ref& cr, AT&& ...at) :
				self(cr), form_data(std::forward<AT>(at)...), plugin_data(form_data)
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
					//form_call_pre_tick<Implement::frame_form<frame>>{}(form_data, self, *dua);
					plugin_data.plug_tick(*dua, self);
					Tool::statement_if<form_have_tick<Implement::frame_form<frame>>::value>
						(
							[](auto& p, duration da, form_self& fs)
					{
						form_ticker ft(fs, da);
						p.tick(ft);
					},
							[](auto& p, duration da, form_self&) {},
						form_data, *dua, self
						);
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
					p.set_value(std::make_unique<viewer_packet<frame>>(packet, packet.self, packet.plugin_data, packet.form_data));
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

	template<typename plugin_t> using plugin_type = Tmp::itself<plugin_t>;

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
