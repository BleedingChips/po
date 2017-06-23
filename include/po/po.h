#pragma once
#include "plugin.h"
#include "renderer.h"
#include "form.h"
#include <future>
namespace PO
{
	namespace Implement {

		template<typename form_t> struct have_end_construction
		{
			template<typename P> static std::true_type func(decltype(((form_t*)(nullptr))->end_construction())*);
			template<typename P> static std::false_type func(...);
			void operator()(form_t& f) {
				Tool::statement_if<decltype(func<form_t>(nullptr))::value>(
					[](auto& f) {f.end_construction(); },
					[](auto& f) {},
					f
					);
			}
		};

		template<typename form_t> struct have_start_destruction
		{
			template<typename P> static std::true_type func(decltype(((form_t*)(nullptr))->start_destruction())*);
			template<typename P> static std::false_type func(...);
			void operator()(form_t& f) {
				Tool::statement_if<decltype(func<form_t>(nullptr))::value>(
					[](auto& f) {f.start_destruction(); },
					[](auto& f) {},
					f
					);
			}
		};

		struct form_interface {
			virtual ~form_interface() = default;
			virtual void tick(duration da) = 0;
			operator bool() const { return is_available(); }
			virtual bool is_available() const = 0;
		};

		template<typename form_t>
		struct form_packet : form_interface, form_t
		{
			viewer v;
			plugins plugin_packet;

			template<typename ...AK>
			form_packet(Tool::completeness_ref cr, AK&& ...ak) : form_t(std::forward<AK>(ak)...), plugin_packet(form_t::mapping()) {
				have_end_construction<form_t>{}(*this);
			}

			virtual bool is_available() const {
				return form_t::available();
			}

			~form_packet()
			{
				have_start_destruction<form_t>{}(*this);
			}

			virtual Respond ask_for_respond_mt(event& e) { return Respond::Pass; };
			virtual Respond ask_for_respond(event& e) {
				return plugin_packet.respond(e, v);
			};
			virtual void tick(duration da)
			{
				have_pre_tick<form_t>{}(*this, da);
				plugin_packet.tick(v, da);
				have_pos_tick<form_t>{}(*this, da);
			}
		};

		template<typename form_t> using form_final = Tool::completeness<form_packet<form_t>>;
	}

	class out_viewer
	{
		viewer v;
		plugins& plu;
	public:
		viewer& get_viewer() { return v; }
		out_viewer(viewer i, plugins& p) : v(i), plu(p) {}
		out_viewer(const out_viewer&) = default;
		out_viewer(out_viewer&&) = default;
		template<typename renderer_t, typename ...AK> void create(renderer<renderer_t> i, AK&& ...ak) { plu.create(i, std::forward<AK>(ak)...); }
		template<typename plugin_t, typename ...AK> void create(plugin<plugin_t> i, AK&& ...ak) { plu.create(i, std::forward<AK>(ak)...); }
	};

	class out_viewer_packet 
	{
		out_viewer ov;
		Tool::completeness_ref ref;
	public:
		using type = out_viewer;
		out_viewer_packet(out_viewer o, Tool::completeness_ref r) : ov(std::move(o)), ref(std::move(r)) {}
		out_viewer_packet(const out_viewer_packet&) = default;
		out_viewer_packet(out_viewer_packet&&) = default;
		template<typename func> decltype(auto) lock(func&& fn) {
			return ref.lock_if([fn, this]() {
				return fn(ov);
			});
		}
	};

		
	namespace Implement
	{
		struct form_ptr
		{
			bool avalible;
			std::thread logic_form_thread;
			std::atomic_bool force_exist_form;
			form_ptr() : force_exist_form(false) {}
			~form_ptr();
			void push_function(std::function<std::unique_ptr<form_interface>(void)> f);
			template<typename form_t, typename ...AK> auto create_form(form<form_t> i, AK&& ...ak)
			{
				std::promise<Tool::optional<out_viewer_packet>> pro;
				auto fur = pro.get_future();
				push_function([&]() -> std::unique_ptr<form_interface> {
					std::unique_ptr<form_final<form_t>> po = std::make_unique<form_final<form_t>>(std::forward<AK>(ak)...);
					pro.set_value(out_viewer_packet{ out_viewer{po->v, po->plugin_packet}, *po });
					return std::move(po);
				});
				fur.wait();
				return *fur.get();
			}
			void force_stop() { force_exist_form = true; }
		};


	}



	class context
	{
		Tool::scope_lock<std::vector<std::unique_ptr<Implement::form_ptr>>> this_form;
		void set_form(std::unique_ptr<Implement::form_ptr> fp);
	public:

		void wait_all_form_close();

		context();
		~context();
		
		void detach();
		template<typename form_t, typename ...AK> auto create(form<form_t> i, AK&& ...ak)
		{
			std::unique_ptr<Implement::form_ptr> tem = std::make_unique<Implement::form_ptr>();
			Implement::form_ptr& ptr = *tem;
			set_form(std::move(tem));
			return ptr.create_form(i, std::forward<AK>(ak)...);
		}
	};


}
