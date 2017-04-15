#pragma once
#include "define.h"
#include "../tool/thread_tool.h"
#include "../tool/auto_adapter.h"
#include <atomic>
#include <list>
#include <future>
#include <typeindex>
#include <map>
#include <memory>
#include <fstream>
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
		template<typename ticker_t> class plugin_append;
	}

	class io_method;

	struct io_block
	{
		io_method& metho;
		const std::u16string& path;
		const std::u16string& name;
		std::fstream& stream;
		const Tool::any& parameter;
	};

	class io_method
	{
	public:

		struct block
		{

			using analyze_t = std::function<Tool::any(io_block)>;
			using filter_t = std::function<std::fstream(const std::u16string&, const std::u16string&)>;
			using pair_t = std::pair<filter_t, analyze_t>;
			using d_filter_t = std::function<Tool::any(io_method& ,const std::u16string&, const std::u16string&, const Tool::any&)>;

			Tool::variant<pair_t, analyze_t, d_filter_t> method;

			static std::fstream default_filter(const std::u16string& path, const std::u16string& name);

			Tool::optional<Tool::any> operator()(io_method& iom, const std::u16string& path, const std::u16string& name, const Tool::any& a);
		};

		struct path_list
		{
			std::vector<std::u16string> top_path;
			std::map<std::type_index, std::vector<std::u16string>> type_path;
		};

	private:

		Tool::scope_lock<std::map<std::type_index, block>, std::recursive_mutex> fun_map;
		Tool::scope_lock<path_list, std::recursive_mutex> paths;

	public:
		Tool::optional<Tool::any> calling_specified_path_execute(std::type_index ti, const std::u16string&, const std::u16string&, const Tool::any& a);
		Tool::optional<Tool::any> calling_execute(std::type_index ti, const std::u16string&, const Tool::any& a);
		void set_function(std::type_index ti, block b);
		void set_function(std::type_index ti, block::analyze_t analyze) { set_function(ti, block{ std::move(analyze) }); }
		void set_function(std::type_index ti, block::analyze_t analyze, block::filter_t filter) { set_function(ti, block{ std::make_pair(std::move(filter), std::move(analyze)) }); }
		void set_function(std::type_index ti, block::d_filter_t d) { set_function(ti, block{ d }); }
		void add_path(std::type_index ti, std::u16string pa);
		void add_path(std::u16string pa);
		void add_path(std::initializer_list<std::u16string> pa);
		void add_path(std::initializer_list<std::pair<std::type_index, std::u16string>> pa);
		void add_path(std::type_index ti, std::initializer_list<std::u16string> pa);
	};


	namespace Implement
	{
		class io_task_implement
		{
			Tool::thread_task_operator ope;
			io_method method;
			Tool::completeness_ref cr;
		public:
			void set_function(std::type_index ti, io_method::block b) { method.set_function(ti, std::move(b)); }
			void set_function(std::type_index ti, io_method::block::analyze_t analyze) { method.set_function(ti, io_method::block{ std::move(analyze) }); }
			void set_function(std::type_index ti, io_method::block::analyze_t analyze, io_method::block::filter_t filter) { method.set_function(std::move(ti), io_method::block{ std::make_pair(std::move(filter), std::move(analyze)) }); }
			void set_function(std::type_index ti, io_method::block::d_filter_t filter) { method.set_function(std::move(ti), io_method::block{  std::move(filter) }); }
			void add_path(std::type_index ti, std::u16string pa) { method.add_path(ti, std::move(pa)); }
			void add_path(std::u16string pa) { method.add_path(std::move(pa)); }
			void add_path(std::initializer_list<std::u16string> pa) { method.add_path(std::move(pa)); }
			void add_path(std::initializer_list<std::pair<std::type_index, std::u16string>> pa) { method.add_path(std::move(pa)); }
			void add_path(std::type_index ti, std::initializer_list<std::u16string> pa) { method.add_path(ti, std::move(pa)); }
			std::future<Tool::optional<Tool::any>> add_request(std::type_index ti, std::u16string pa, Tool::any a);
			decltype(auto) request(std::type_index ti, const std::u16string& pa, const Tool::any& a) { return method.calling_execute(ti, pa, a); }
			io_task_implement(Tool::completeness_ref rf) :cr(rf) {}
		};
	}
	
	using io_task = Tool::completeness<Implement::io_task_implement>;
	io_task& io_task_instance();

	class raw_scene
	{
		struct request
		{
			std::type_index ti;
			std::u16string path;
			bool save_raw_data;
		};

		// make Tool::any(Tool::any&)
		using store_type = Tool::variant<Tool::any, std::future<Tool::optional<Tool::any>>>;
		Tool::scope_lock<std::map<std::type_index, std::map<std::u16string, store_type>>> store_map;
	public:
		Tool::optional<Tool::any> find(std::type_index, const std::u16string&, const Tool::any& , bool save_data = true);
		Tool::optional<Tool::any> find(std::type_index ti, const std::u16string& name, bool save_data = true) { return find(ti, name, Tool::any{}, save_data); }
		void pre_load(std::type_index, const std::u16string& path, Tool::any a);
		void pre_load(std::type_index ti, const std::u16string& path) { pre_load(ti, path, Tool::any{}); }
		void pre_load(std::type_index, std::initializer_list<std::pair<std::u16string, Tool::any>> path);
		void pre_load(std::type_index, std::initializer_list<std::u16string> path);
	};


	/*
	class thread_task
	{
		enum class State :uint8_t
		{
			READY,
			WAITING,
			RUNNING,
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
		bool is_finish() const { return task_state == State::FINISH; }
		bool is_ready() const { return task_state == State::READY; }
		bool set_ready()
		{
			if (task_state == State::FINISH || task_state == State::READY)
			{
				task_state = State::READY;
				return true;
			}
			return false;
		}
		void wait_finish()
		{
			std::unique_lock<std::mutex> lk(task_mutex);
			if (task_state == State::WAITING || task_state == State::RUNNING)
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

		class file_io_task_runner : Implement::thread_task_runer
		{
			std::mutex handle_type_mutex; 
			std::unordered_map<std::u16string, std::tuple<std::function<bool(std::u16string)> >> handle_type;
			std::vector<std::u16string> search_patch;
			friend class raw_scene_data;
		public:
			void add_search_pacth(std::u16string patch);
			//void set_special_loader(std::u16string, std::function<std::u16string>,std::function<>)
		};

		struct raw_scene_request_t
		{
			std::type_index type;
			std::u16string path;
		};

		enum class raw_scene_data_state
		{
			BAD,
			LOADING,
			WAITINT,
			FINISH
		};

		struct raw_scene_data_t
		{
			Tool::any data;
			std::u16string path;
			bool is_bad;
		};

		class raw_scene_data
		{
			using request_t = raw_scene_request_t;
			using request_list_t = std::vector<request_t>;
			using store_t = raw_scene_data_t;
			//type - name - (data, path)
			using store_map_t = std::unordered_map<std::type_index, std::unordered_map<std::u16string, store_t>>;

			Tool::completeness_ref ref;

			Tool::scope_lock<store_map_t> store_mapping;
			Tool::scope_lock<request_list_t> request_list;

			class task_operator :public thread_task
			{
				raw_scene_data* rsd;
				request_list_t request_list;
			public:
				task_operator(Tool::completeness_ref c, raw_scene_data* r) : thread_task(std::move(c)), rsd(r) {}
				bool operator()();
			};

			std::shared_ptr<task_operator> task_ptr;

		public:
			void load(std::initializer_list<request_t> il);
			void load(request_t type);
			//Tool::any get_
			void start_task();
			raw_scene_data(Tool::completeness_ref cr) : ref(std::move(cr)) {}
		};
	}

	using raw_scena = Tool::completeness<Implement::raw_scene_data>;
	*/
	class form_self
	{
		std::mutex record_mutex;
		time_calculator record;
		Tool::completeness_ref cr;
		std::atomic_bool available;
		//Implement::thread_task_runer ttr;

		template<typename frame> friend struct Implement::form_packet;
		template<typename ticker_t> friend class Implement::plugin_append;

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

		virtual Respond respond_event(event& f) = 0;
		//bool push_task(std::weak_ptr<thread_task> task) { return ttr.push_task(std::move(task)); }
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
		friend class conveyer_tl;
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

	class conveyer_tl : public constor
	{
		event& ev;
		void set_event(const event& e) { ev = e; }
		template<typename T> friend class conveyer;
		conveyer_tl(plugin_self& s, form_self& f, Implement::plugin_append_tl& pat, event& e) :constor(s, f, pat), ev(e) {}
	public:
		event& get_event() const { return ev; }
	};

	template<typename ticker_t> class conveyer : public conveyer_tl
	{
		Implement::plugin_append<ticker_t>& plugin;
		ticker_t& ticker_;
		conveyer(plugin_self& ps, form_self& fs, Implement::plugin_append<ticker_t>& ft, event& e, ticker_t& fv)
			: conveyer_tl(ps, fs, ft, e), plugin(ft), ticker_(fv) {}
		template<typename plugin_t, typename ticker_t> friend class Implement::plugin_implement;
		operator conveyer_tl& () { return *this; }
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
			virtual Respond plug_respond(form_self& fs, plugin_append<ticker_t>& pa, ticker_t& t, event& e) = 0;
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

		template<typename plugin_t, typename ticker_t> struct plugin_have_respond_tl
		{
			template<typename T, Respond(T::*)(conveyer_tl&)> struct del;
			template<typename T>
			static std::true_type func(del<T, &T::respond>*);
			template<typename T>
			static std::false_type func(...);
		public:
			static constexpr bool value = decltype(func<plugin_t>(nullptr))::value;
		};

		template<typename plugin_t, typename ticker_t> struct plugin_have_respond
		{
			template<typename T, Respond (T::*)(conveyer<ticker_t>&)> struct del;
			template<typename T>
			static std::true_type func(del<T, &T::respond>*);
			template<typename T>
			static auto func(...)->plugin_have_respond_tl<plugin_t, ticker_t>;
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

			virtual Respond plug_respond(form_self& fs, plugin_append<ticker_t>& pa, ticker_t& t, event& e)
			{
				conveyer<ticker_t> tem_con{ *this, fs, pa, e, t };
				return Tool::statement_if<plugin_have_respond<plugin_t, ticker_t>::value>
					(
						[](auto& p, conveyer<ticker_t>& e) { return p.respond(e); },
						[](auto& p, conveyer<ticker_t>& e) {return Respond::Pass; },
						*(reinterpret_cast<plugin_t*>(&data)), tem_con
						);
			}

		};

		template<typename plugin_t, typename ticker_t> using plugin_final = Tool::completeness<plugin_implement<plugin_t, ticker_t>>;

		template<typename ticker_t> class plugin_append : public plugin_append_tl
		{
			std::mutex pim;
			using tank = std::vector<std::unique_ptr<plugin_interface<ticker_t>>>;
			
			Tool::scope_lock<tank> inilizered_plugin_list;
			tank plugin_list;
			ticker_t tick;

			virtual void create_plugin_tl_execute(std::function<std::unique_ptr<Implement::plugin_tl_interface>(constor&)>&& up, form_self& fs)
			{
				this->create_plugin(Tmp::itself<plugin_tl_holder>{}, constor_init_type{}, fs, std::move(up));
			}

		public:

			template<typename plugin_t, typename init_type, typename ...AK> auto create_plugin(plugin_t t, init_type it, form_self& fs, AK&&... ak)
			{
				auto ptr = std::make_unique<plugin_final<typename plugin_t::type, ticker_t>>(it, fs, *this, std::forward<AK>(ak)...);
				inilizered_plugin_list.lock(
					[&ptr](tank& io) {io.push_back(std::move(ptr)); }
				);
			}
			template<typename form>
			plugin_append(form& fv) : tick(fv) {}


			void plug_init(duration da, form_self& fs)
			{
				plugin_list.erase(std::remove_if(plugin_list.begin(), plugin_list.end(), [](auto& i) {return !(i && (*i)); }), plugin_list.end());
				auto start = inilizered_plugin_list.lock(
					[this](tank& in)
				{
					in.erase(std::remove_if(in.begin(), in.end(), [](auto& i) {return !(i && (*i)); }), in.end());
					auto ite = plugin_list.insert(plugin_list.end(), std::make_move_iterator(in.begin()), std::make_move_iterator(in.end()));
					in.clear();
					return ite;
				}
				);

				auto pre_start = start;
				for (; pre_start != plugin_list.end(); ++pre_start)
					(*pre_start)->plug_init(fs, *this, tick, da);

				plugin_list.erase(std::remove_if(start, plugin_list.end(), [](auto& i) {return !(i && (*i)); }), plugin_list.end());

			}

			Respond plug_respond(event& e, form_self& fs)
			{
				Respond re = Respond::Pass;
				for (auto& ptr : plugin_list)
				{
					re = (ptr)->plug_respond(fs, *this, tick, e);
					if (re == Respond::Truncation || re == Respond::Return)
						break;
				}
				return re;
			}

			void plug_tick(duration da, form_self& fs)
			{
				for (auto& ptr : plugin_list)
					ptr->plug_tick(fs, *this, tick, da);
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

		template<typename form> class form_have_pos_tick
		{
			template<typename T, void (T::*)(form_ticker&)> struct del;
			template<typename T> static std::true_type fun(del<T, &T::pos_tick>*);
			template<typename T> static std::false_type fun(...);
		public:
			static constexpr bool value = decltype(fun<form>(nullptr))::value;
		};

		template<typename frame>
		struct form_packet : public form_self
		{
			//form_self self;
			Implement::frame_form<frame> form_data;
			plugin_append<Implement::frame_ticker<frame>> plugin_data;

			template<typename ...AT> form_packet(std::true_type, const Tool::completeness_ref& cr, AT&& ...at) :
				form_self(cr), form_data(*this, std::forward<AT>(at)...), plugin_data(form_data)
			{
			}

			template<typename ...AT> form_packet(std::false_type, const Tool::completeness_ref& cr, AT&& ...at) :
				form_self(cr), form_data(std::forward<AT>(at)...), plugin_data(form_data)
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

			virtual Respond respond_event(event& f) override
			{
				return plugin_data.plug_respond(f, *this);
			}

			void tick(time_point tp)
			{
				auto dua = form_self::tick(tp);
				if (dua)
				{
					plugin_data.plug_init(*dua, *this);
					Tool::statement_if<form_have_tick<Implement::frame_form<frame>>::value>
						(
							[](auto& p, duration da, form_self& fs)
					{
						form_ticker ft(fs, da);
						p.tick(ft);
					},
							[](auto& p, duration da, form_self&) {},
						form_data, *dua, *this
						);
					plugin_data.plug_tick(*dua, *this);
					Tool::statement_if<form_have_pos_tick<Implement::frame_form<frame>>::value>
						(
							[](auto& p, duration da, form_self& fs)
					{
						form_ticker ft(fs, da);
						p.pos_tick(ft);
					},
							[](auto& p, duration da, form_self&) {},
						form_data, *dua, *this
						);
				}
			}
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
					p.set_value(std::make_unique<viewer_packet<frame>>(packet, packet, packet.plugin_data, packet.form_data));
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