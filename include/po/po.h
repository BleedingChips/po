#pragma once
#include "plugin.h"
#include <future>
namespace PO
{
	namespace Implement {

		struct form_interface {
			virtual ~form_interface();
			virtual void tick(duration da) = 0;
			operator bool() const { return is_available(); }
			virtual bool is_available() const = 0;
		};

		template<typename form_t>
		struct form_packet : form_interface, viewer_interface, form_t
		{
			viewer v;
			plugins_implement plugin_packet;

			template<typename ...AK>
			form_packet(Tool::completeness_ref cr, AK&& ...ak) : form_t(std::forward<AK>(ak)...), plugin_packet(form_t::mapping()), v(*this)
			{}

			virtual bool is_available() const { return form_t::available(); }

			virtual void tick(duration da)
			{
				auto& ref = form_t::generate_event_tank();
				for (auto& ite : ref)
				{
					[&, this]() {
						if constexpr (Tmp::able_instance_v<mf_pre_respond, form_t>)
							return form_t::pre_respond(ite) != Respond::Pass;
						else
							return false; 
					}() || plugin_packet.respond(ite, v) != Respond::Pass ||
						[&, this]() {
						if constexpr (Tmp::able_instance_v<mf_pos_respond, form_t>)
							return form_t::pos_respond(ite) != Respond::Pass;
						else
							return false;
					}();
				}
				if constexpr(Tmp::able_instance_v<mf_pre_tick, form_t>)
					form_t::pre_tick(da);
				plugin_packet.tick(v, da);
				if constexpr(Tmp::able_instance_v<mf_pos_tick, form_t>)
					form_t::pos_tick(da);
			}
		};

		template<typename form_t> using form_final = Tool::completeness<form_packet<form_t>>;
	}

	class out_viewer
	{
		viewer v;
		plugins_implement& plu;
	public:
		viewer& get_viewer() { return v; }
		out_viewer(viewer i, plugins_implement& p) : v(i), plu(p) {}
		out_viewer(const out_viewer&) = default;
		out_viewer(out_viewer&&) = default;
		template<typename renderer_t, typename ...AK> void create(renderer<renderer_t> i, AK&& ...ak) { plu.create(i, std::forward<AK>(ak)...); }
		template<typename plugin_t, typename ...AK> void create(plugin<plugin_t> i, AK&& ...ak) { plu.create(i, std::forward<AK>(ak)...); }
		template<typename extension_t, typename ...AK> void create(extension<extension_t> i, AK&& ...ak) { plu.create(i, std::forward<AK>(ak)...); }
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
