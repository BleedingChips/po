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
			using d_filter_t = std::function<Tool::any(io_method&, const std::u16string&, const std::u16string&, const Tool::any&)>;

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
			void set_function(std::type_index ti, io_method::block::d_filter_t filter) { method.set_function(std::move(ti), io_method::block{ std::move(filter) }); }
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
		Tool::optional<Tool::any> find(std::type_index, const std::u16string&, const Tool::any&, bool save_data = true);
		Tool::optional<Tool::any> find(std::type_index ti, const std::u16string& name, bool save_data = true) { return find(ti, name, Tool::any{}, save_data); }
		void pre_load(std::type_index, const std::u16string& path, Tool::any a);
		void pre_load(std::type_index ti, const std::u16string& path) { pre_load(ti, path, Tool::any{}); }
		void pre_load(std::type_index, std::initializer_list<std::pair<std::u16string, Tool::any>> path);
		void pre_load(std::type_index, std::initializer_list<std::u16string> path);
	};


	/************************************************************************************************/

	struct form_depute;
	template<typename renderer_t, typename viewer_t> struct self_depute;

	template<typename renderer_t, typename viewer_t> struct peek;
	

	namespace Implement
	{
		template<typename renderer_t, typename viewer_t> struct plugins;

		template<typename renderer_t = void, typename viewer_t = void>
		struct plugin_self : virtual plugin_self<void, void>, plugin_self<renderer_t, void>, plugin_self<void, viewer_t>
		{
			plugin_self(const Tool::completeness_ref& c) : plugin_self<void, void>(c), plugin_self<renderer_t, void>(c), plugin_self<void, viewer_t>(c) {}

			std::function<void(self_depute<renderer_t, viewer_t>)> init_function;
			std::function<void(self_depute<renderer_t, viewer_t>, duration da)> tick_function;

			void bind_init(std::function<void(self_depute<renderer_t, viewer_t>)> ift) { init_function = std::move(ift); }
			void bind_tick(std::function<void(self_depute<renderer_t, viewer_t>, duration da)> tft) { tick_function = std::move(tft); }
			
			virtual void bind_init(std::function<void(self_depute<void, void>)> ift) override { init_function = ift; }
			virtual void bind_tick(std::function<void(self_depute<void, void>, duration)> tft) override { tick_function = tft; }

			virtual void bind_init(std::function<void(self_depute<renderer_t, void>)> ift) override { init_function = ift; }
			virtual void bind_tick(std::function<void(self_depute<renderer_t, void>, duration)> tft) override { tick_function = tft; }

			virtual void bind_init(std::function<void(self_depute<void, viewer_t>)> ift) override { init_function = ift; }
			virtual void bind_tick(std::function<void(self_depute<void, viewer_t>, duration)> tft) override { tick_function = tft; }

			std::function<Respond(event&, viewer_t&)> respond_function;
			virtual void bind_respond(std::function<Respond(event&, viewer_t&)> rft) override { respond_function = std::move(rft); }
			virtual void bind_respond(std::function<Respond(event&)> rft) override { respond_function = [rft = std::move(rft)](event& ev, viewer_t&) {return rft(ev); }; }

			template<typename T, typename ...AT> void auto_bind_tick(T&& t, AT&&... at) { bind_tick(Tool::auto_bind_function<void(self_depute<renderer_t, viewer_t>, duration), Tool::unorder_adapt>(std::forward<T>(t), std::forward<AT>(at)...)); }
			template<typename T, typename ...AT> void auto_bind_init(T&& t, AT&&... at) { bind_init(Tool::auto_bind_function<void(self_depute<renderer_t, viewer_t>), Tool::unorder_adapt>(std::forward<T>(t), std::forward<AT>(at)...)); }
			template<typename T, typename ...AT> void auto_bind_respond(T&& t, AT&&... at) { bind_respond(Tool::auto_bind_function<Respond(event&, viewer_t&), Tool::unorder_adapt>(std::forward<T>(t), std::forward<AT>(at)...)); }
		};

		template<> struct plugin_self<void, void>
		{
			bool avalible = true;
			Tool::completeness_ref cr;
			virtual void bind_init(std::function<void(self_depute<void, void>)> ift) = 0;
			virtual void bind_tick(std::function<void(self_depute<void, void>, duration)> tft) = 0;
			virtual void bind_respond(std::function<Respond(event&)> rft) = 0;
			plugin_self(const Tool::completeness_ref& c) :cr(c) {}
			operator bool() const { return avalible; }
			template<typename T, typename ...AT> void auto_bind_tick(T&& t, AT&&... at) { bind_tick(Tool::auto_bind_function<void(self_depute<void, void>, duration), Tool::unorder_adapt>(std::forward<T>(t), std::forward<AT>(at)...)); }
			template<typename T, typename ...AT> void auto_bind_init(T&& t, AT&&... at) { bind_init(Tool::auto_bind_function<void(self_depute<void, void>), Tool::unorder_adapt>(std::forward<T>(t), std::forward<AT>(at)...)); }
			template<typename T, typename ...AT> void auto_bind_respond(T&& t, AT&&... at) { bind_respond(Tool::auto_bind_function<Respond(event&), Tool::unorder_adapt>(std::forward<T>(t), std::forward<AT>(at)...)); }
		};

		template<typename renderer_t> struct plugin_self<renderer_t, void> : virtual plugin_self<void, void>
		{
			virtual void bind_init(std::function<void(self_depute<renderer_t, void>)> ift) = 0;
			virtual void bind_tick(std::function<void(self_depute<renderer_t, void>, duration)> tft) = 0;
			plugin_self(const Tool::completeness_ref& c) : plugin_self<void, void>(c) {}
			template<typename T, typename ...AT> void auto_bind_tick(T&& t, AT&&... at) { bind_tick(Tool::auto_bind_function<void(self_depute<renderer_t, void>, duration), Tool::unorder_adapt>(std::forward<T>(t), std::forward<AT>(at)...)); }
			template<typename T, typename ...AT> void auto_bind_init(T&& t, AT&&... at) { bind_init(Tool::auto_bind_function<void(self_depute<renderer_t, void>), Tool::unorder_adapt>(std::forward<T>(t), std::forward<AT>(at)...)); }
			template<typename T, typename ...AT> void auto_bind_respond(T&& t, AT&&... at) { bind_respond(Tool::auto_bind_function<Respond(event&), Tool::unorder_adapt>(std::forward<T>(t), std::forward<AT>(at)...)); }
		};

		template<typename viewer_t> struct plugin_self<void, viewer_t> : virtual plugin_self<void, void>
		{
			virtual void bind_init(std::function<void(self_depute<void, viewer_t>)> ift) = 0;
			virtual void bind_tick(std::function<void(self_depute<void, viewer_t>, duration)> tft) = 0;
			virtual void bind_respond(std::function<Respond(event&, viewer_t&)> rft) = 0;
			plugin_self(const Tool::completeness_ref& c) : plugin_self<void, void>(c) {}
			template<typename T, typename ...AT> void auto_bind_tick(T&& t, AT&&... at) { bind_tick(Tool::auto_bind_function<void(self_depute<void, viewer_t>, duration), Tool::unorder_adapt>(std::forward<T>(t), std::forward<AT>(at)...)); }
			template<typename T, typename ...AT> void auto_bind_init(T&& t, AT&&... at) { bind_init(Tool::auto_bind_function<void(self_depute<void, viewer_t>), Tool::unorder_adapt>(std::forward<T>(t), std::forward<AT>(at)...)); }
			template<typename T, typename ...AT> void auto_bind_respond(T&& t, AT&&... at) { bind_respond(Tool::auto_bind_function<Respond(event&, viewer_t&), Tool::unorder_adapt>(std::forward<T>(t), std::forward<AT>(at)...)); }
		};

		struct holder_ptr
		{
			virtual ~holder_ptr() {}
		};

		template<typename plugin_t> struct plugin_holder : holder_ptr
		{
			Tool::variant<plugin_t> data;
			template<typename ...AK> plugin_holder(AK&&... ak) : data(std::forward<AK>(ak)...) {}
		};

		template<typename render_t, typename viewer_t> struct plugin_packet
		{

			void init(render_t& r, viewer_t& v, plugins<render_t, viewer_t>& p) {
				if (self.init_function) self.init_function(self_depute<render_t, viewer_t>{ {self, p, v}, r});
			}
			void tick(render_t& r, viewer_t& v, plugins<render_t, viewer_t>& p, duration da) {
				if (self.tick_function) self.tick_function(self_depute<render_t, viewer_t>{ {self, p, v}, r}, da);
			}
			Respond respond(event& ev, viewer_t& v){
				if (self.respond_function) return self.respond_function(ev, v);
				return Respond::Pass;
			}
			operator bool() const { return self; }



			plugin_self<render_t, viewer_t> self;
			std::unique_ptr<holder_ptr> ptr;
			plugin_packet(const Tool::completeness_ref& c, std::function<std::unique_ptr<holder_ptr>(plugin_self<render_t, viewer_t>&)> fu) : self(c), ptr(fu(self)){}
		};

		template<typename render_t, typename viewer_t> using plugin_packet_t = Tool::completeness<plugin_packet<render_t, viewer_t>>;

		template<typename T,typename R, typename ...AT> std::unique_ptr<holder_ptr> create_plugin_implement(Tmp::itself<T> i, R r, AT&&... at)
		{
			using type = std::decay_t<T>;
			return std::make_unique<plugin_holder<type>>(r, std::forward<AT>(at)...);
			/*
			return Tool::statement_if<std::is_constructible<type, R, AT...>::value>
				(
					[&](auto i, auto& r) { return Tool::any{ i, r, std::forward<AT>(at)... }; },
					[&](auto i, auto& r) { return Tool::any{ i, std::forward<AT>(at)... }; },
					Tmp::itself<type>{}, r
					);
					*/
		}

		template<typename render_t = void, typename viewer_t = void> struct plugins : virtual plugins<void, void>, plugins<render_t, void>, plugins<void, viewer_t>
		{
			Tool::scope_lock<std::vector<std::function<std::unique_ptr<holder_ptr>(plugin_self<render_t, viewer_t>&, render_t&, viewer_t&)>>> depute_function;
			std::vector<std::function<std::unique_ptr<holder_ptr>(plugin_self<render_t, viewer_t>&, render_t&, viewer_t&)>> all_depute_function;

			Tool::scope_lock<std::vector<std::unique_ptr<plugin_packet_t<render_t, viewer_t>>>> init_plugin;
			std::vector<std::unique_ptr<plugin_packet_t<render_t, viewer_t>>> all_plugin;

			void create_execute(std::function<std::unique_ptr<holder_ptr>(plugin_self<render_t, viewer_t>&)> f)
			{
				std::unique_ptr<plugin_packet_t<render_t, viewer_t>> plu = std::make_unique<plugin_packet_t<render_t, viewer_t>>(f);
				init_plugin.lock([&plu](typename decltype(init_plugin)::type& b) {
					b.push_back(std::move(plu));
				});
			}

			template<typename T, typename ...AT> void create(Tmp::itself<T> i, render_t& rt, viewer_t& vt, AT&& ... at)
			{
				auto fun = [&](plugin_self<render_t, viewer_t>& sd) { 
					return create_plugin_implement(i, self_depute<render_t, viewer_t>{ {sd, *this, vt}, rt}, std::forward<AT>(at)...);
				};
				create_execute(fun);
			}

			virtual void create_implement( std::function<std::unique_ptr<holder_ptr>(plugin_self<void, void>&)> cft) override { create_execute(cft); }
			virtual void create_implement( std::function<std::unique_ptr<holder_ptr>(plugin_self<render_t, void>&)> cft) override { create_execute(cft); }
			virtual void create_implement( std::function<std::unique_ptr<holder_ptr>(plugin_self<void, viewer_t>&)> cft) override { create_execute(cft); }

			template<typename T, typename ...AT>
			void outside_create(Tmp::itself<T> i, viewer_t& v, AT&&... at)
			{
				auto fun = [&, this](plugin_self<render_t, viewer_t>& sd) {
					return create_plugin_implement(i, peek<render_t, viewer_t>{sd, *this, v}, std::forward<AT>(at)...);
				};
				create_execute(fun);
			}

			template<typename T, typename ...AT>
			void depute_create(Tmp::itself<T> i, AT&&... at)
			{
				auto fun = [=, this](plugin_self<render_t, viewer_t>& ps, render_t& r, viewer_t& v) {
					return create_plugin_implement(i, self_depute<render_t, viewer_t>{ {ps, *this, v}, r}, std::forward<AT>(at)...);
				};
				depute_function.lock([&](decltype(depute_function)::type& b) {
					b.push_back(std::move(fun));
				});
			}

			void tick(render_t& r, viewer_t& v, duration da)
			{
				all_plugin.erase(std::remove_if(all_plugin.begin(), all_plugin.end(), [](auto& i) {return !(i && (*i)); }), all_plugin.end());

				depute_function.lock([this](decltype(depute_function)::type& b) {
					std::swap(all_depute_function, b);
				});

				all_plugin.reserve(all_plugin.size() + all_depute_function.size());
				for (auto& fun : all_depute_function)
				{
					if (fun)
					{
						std::unique_ptr<plugin_packet_t<render_t, viewer_t>> plu = std::make_unique<plugin_packet_t<render_t, viewer_t>>([&](plugin_self<render_t, viewer_t>& ps) {
							return fun(ps, r, v);
						});
						plu->init(r, v, *this);
						all_plugin.push_back(std::move(plu));
					}
				}
				all_depute_function.clear();
				
				auto start = init_plugin.lock(
					[this](decltype(init_plugin)::type& in)
				{
					in.erase(std::remove_if(in.begin(), in.end(), [](auto& i) {return !(i && (*i)); }), in.end());
					auto ite = all_plugin.insert(all_plugin.end(), std::make_move_iterator(in.begin()), std::make_move_iterator(in.end()));
					in.clear();
					return ite;
				}
				);
				auto pre_start = start;
				for (; pre_start != all_plugin.end(); ++pre_start)
					(*pre_start)->init(r, v, *this);

				for (auto& po : all_plugin)
					po->tick(r, v, *this, da);
			}

			Respond respond(viewer_t& v, event& e)
			{
				Respond re = Respond::Pass;
				for (auto& ite : all_plugin)
				{
					re = ite->respond(e, v);
					if (re != Respond::Pass) return re;
				}
				return Respond::Pass;
			}
		};

		template<> struct plugins<void, void>
		{
			virtual void create_implement(std::function<std::unique_ptr<holder_ptr>(plugin_self<void, void>&)> cft) = 0;
			template<typename T, typename ...AT> void create(Tmp::itself<T> i, AT&& ... at)
			{
				using type = std::decay_t<T>;
				create_implement([&](plugin_self<void, void>& sd) { return create_plugin_implement(Tmp::itself<type>{}, self_depute<void, void>{ {sd, *this} }, std::forward<AT>(at)... ); });
			}
		};

		template<typename render_t> struct plugins<render_t, void>
		{
			virtual void create_implement(std::function<std::unique_ptr<holder_ptr>(plugin_self<render_t, void>&)> cft) = 0;
			template<typename T, typename ...AT> void create(Tmp::itself<T> i, render_t& re, AT&& ... at)
			{
				using type = std::decay_t<T>;
				create_implement([&](plugin_self<render_t, void>& sd) { return create_plugin_implement(Tmp::itself<type>{}, self_depute<render_t, void>{ {sd, *this}, re }, std::forward<AT>(at)... ); });
			}
		};

		template<typename viewer_t> struct plugins<void, viewer_t>
		{
			virtual void create_implement(std::function<std::unique_ptr<holder_ptr>(plugin_self<void, viewer_t>&)> cft) = 0;
			template<typename T, typename ...AT> void create(Tmp::itself<T> i, viewer_t& vt, AT&& ... at)
			{
				using type = std::decay_t<T>;
				create_implement([&](plugin_self<void, viewer_t>& sd) { return create_plugin_implement(Tmp::itself<type>{}, self_depute<void, viewer_t>{ {sd, *this, vt} }, std::forward<AT>(at)... ); });
			}
		};
	}

	template<typename plu> using plugin = Tmp::itself<plu>;

	template<typename renderer_t = void, typename viewer_t = void> struct peek
	{
		Implement::plugin_self<renderer_t, viewer_t>& self;
		Implement::plugins<renderer_t, viewer_t>& plugin;
		viewer_t& vt;
		operator peek<void, void>() { return peek<void, void>{self, plugin}; }
		operator peek<renderer_t, void>() { return peek<renderer_t, void>{self, plugin}; }
		operator peek<void, viewer_t>() { return peek<void, viewer_t>{self, plugin, vt}; }
	};

	template<> struct peek<void, void>
	{
		Implement::plugin_self<void, void>& self;
		Implement::plugins<void, void>& plugin;
	};

	template<typename renderer_t> struct peek<renderer_t, void>
	{
		Implement::plugin_self<renderer_t, void>& self;
		Implement::plugins<renderer_t, void>& plugin;
		operator peek<void, void>() { return peek<void, void>{self, plugin}; }
	};

	template<typename viewer_t> struct peek<void, viewer_t>
	{
		Implement::plugin_self<void, viewer_t>& self;
		Implement::plugins<void, viewer_t>& plugin;
		viewer_t& vt;
		operator peek<void, void>() { return peek<void, void>{self, plugin}; }
	};

	template<typename renderer_t = void, typename viewer_t = void> struct self_depute : peek<renderer_t, viewer_t>
	{
		renderer_t& rt;
		operator self_depute<void, void>() { return self_depute<void, void>(this->operator PO::peek<void, void>()); }
		operator self_depute<renderer_t, void>() { return self_depute<renderer_t, void>(this->operator PO::peek<renderer_t, void>(), rt); }
		operator self_depute<void, viewer_t>() { return self_depute<void, viewer_t>(this->operator PO::peek<void, viewer_t>()); }
		self_depute(peek<renderer_t, viewer_t> c, renderer_t& r) : peek<renderer_t, viewer_t>(c), rt(r){}
	};

	template<> struct self_depute<void, void> : peek<void, void>
	{
		self_depute(peek<void, void> c) : peek<void, void>(c){}
	};

	template<typename renderer_t> struct self_depute<renderer_t, void> : peek<renderer_t, void>
	{
		renderer_t& rt;
		operator self_depute<void, void>() { return self_depute<void, void>{*this}; }
		self_depute(peek<renderer_t, void> c, renderer_t& r) : peek<renderer_t, void>{ c }, rt(r){}
	};

	template<typename viewer_t> struct self_depute<void, viewer_t> : peek<void, viewer_t>
	{
		operator self_depute<void, void>() { return self_depute<void, void>{*this}; }
		self_depute(peek<void, viewer_t> c) : peek<void, viewer_t>(c) {}
	};

	/************************************************************************************************/
	
	struct form_control
	{
		Tool::scope_lock<time_calculator> record;
		Tool::completeness_ref cr;
		std::atomic_bool available;
		std::function<void(form_control&, duration)> pre_tick;
		std::function<void(form_control&, duration)> pos_tick;
		Tool::optional<duration> tick(time_point& tp)
		{
			return record.lock([&](decltype(record)::type& re) -> Tool::optional<duration> {
				duration da;
				if (re.tick(tp, da))
					return da;
				return{};
			});
		}
		operator bool() const { return available; }
		form_control(const Tool::completeness_ref& r) : cr(r), available(true) {}
		virtual Respond respond_event(event& ev) = 0;
	};

	template<typename frame> struct observe
	{
		typename frame::viewer view;
		Implement::plugins<typename frame::renderer, typename frame::viewer>& plugins_ref;
		template<typename T, typename ...AT> void create_plugin(Tmp::itself<T> i, AT&&... at)
		{
			plugins_ref.outside_create(i, view, std::forward<AT>(at)...);
		}
		template<typename T, typename ...AT> void depute_create_plugin(Tmp::itself<T> i, AT&&... at)
		{
			plugins_ref.depute_create(i, std::forward<AT>(at)...);
		}
		observe(const observe&) = default;
		observe(const typename frame::viewer& v, Implement::plugins<typename frame::renderer, typename frame::viewer>& p) : view(v), plugins_ref(p) {}
	};

	template<typename frame> struct observe_packet
	{
		Tool::completeness_ref cr;
		observe<frame> ober;
		template<typename T>
		auto lock(T&& t) { return cr.lock_if([&t, this]() {return t(ober); }); }
		observe_packet(const observe_packet&) = default;
		observe_packet(const Tool::completeness_ref& c, const observe<frame>& o) : cr(c), ober(o) {}
	};

	struct renderer_control
	{
		std::function<void(renderer_control&, duration)> pre_tick;
		std::function<void(renderer_control&, duration)> pos_tick;
		std::function<Respond(renderer_control&, event&)> pre_respond;
		std::function<Respond(renderer_control&, event&)> pos_respond;
		template<typename T, typename ...AK> void auto_bind_pre_tick(T&& t, AK&&... at) {
			pre_tick = Tool::auto_bind_function<void(renderer_control& rc, duration), Tool::unorder_adapt>(std::forward<T>(t), std::forward<AK>(at)...);
		}
		template<typename T, typename ...AK> void auto_bind_pos_tick(T&& t, AK&&... at) {
			pos_tick = Tool::auto_bind_function<void(renderer_control& rc, duration), Tool::unorder_adapt>(std::forward<T>(t), std::forward<AK>(at)...);
		}
		template<typename T, typename ...AK> void auto_bind_pre_respond(T&& t, AK&&... at) {
			pre_respond = Tool::auto_bind_function<Respond(renderer_control&, event&), Tool::unorder_adapt>(std::forward<T>(t), std::forward<AK>(at)...);
		}
		template<typename T, typename ...AK> void auto_bind_pos_respond(T&& t, AK&&... at) {
			pos_tick = Tool::auto_bind_function<Respond(renderer_control&, event&), Tool::unorder_adapt>(std::forward<T>(t), std::forward<AK>(at)...);
		}
	};

	namespace Implement
	{
		

		template<typename frame> struct form_packet : form_control
		{
			using form_t = typename frame::form;
			using viewer_t = typename frame::viewer;
			using renderer_t = typename frame::renderer;
			form_t form;
			viewer_t viewer;
			renderer_control ren_con;
			renderer_t renderer;

			plugins<renderer_t, viewer_t> plugins;

			template<typename ...AT>
			form_packet(const Tool::completeness_ref& c, AT&&... at) : form_control(c), form(*this, std::forward<AT>(at)...), viewer(form), renderer(ren_con, form){}

			virtual Respond respond_event(event& ev) override
			{
				Respond re = Respond::Pass; 
				if (ren_con.pre_respond)
				{
					re = ren_con.pre_respond(ren_con, ev);
					if (re != Respond::Pass) return re;
				}
				re = plugins.respond(viewer, ev);
				if (re != Respond::Pass) return re;
				if (ren_con.pos_respond) re = ren_con.pos_respond(ren_con, ev);
				return re;
			}

			void tick(time_point& tp)
			{
				auto du = form_control::tick(tp);
				if (du)
				{
					if (form_control::pre_tick) form_control::pre_tick(*this, *du);
					if (ren_con.pre_tick)ren_con.pre_tick(ren_con, *du);
					plugins.tick(renderer, viewer, *du);
					if (ren_con.pos_tick)ren_con.pos_tick(ren_con, *du);
					if (form_control::pos_tick) form_control::pos_tick(*this, *du);
				}
			}
		};

		template<typename frame> using form_t = Tool::completeness<form_packet<frame>>;

		struct form_ptr
		{
			bool avalible;
			std::thread logic_form_thread;
			std::atomic_bool force_exist_form;
			~form_ptr();
			template<typename frame_t, typename ...AK> auto create_frame(Tmp::itself<frame_t> i, AK&& ...ak)
			{
				using frame = std::decay_t<frame_t>;
				if (logic_form_thread.joinable())
				{
					force_exist_form = true;
					logic_form_thread.join();
				}
				force_exist_form = false;
				std::promise<Tool::optional<observe_packet<frame>>> p;
				auto fur = p.get_future();
				force_exist_form = false;
				logic_form_thread = std::thread(
					[&, this]()
				{
					form_t<frame> packet(std::forward<AK>(ak)...);
					p.set_value(observe_packet<frame>{packet.cr, observe<frame>{packet.viewer, packet.plugins}});
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
				return *(fur.get());
			}
			void force_stop() { force_exist_form = true; }
		};
	}


	
	/*
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

	class form_self
	{
		std::mutex record_mutex;
		time_calculator record;
		Tool::completeness_ref cr;
		std::atomic_bool available;

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
	*/

}