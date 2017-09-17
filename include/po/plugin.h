#pragma once
#include <functional>
#include "frame/viewer.h"
#include "tool/auto_adapter.h"
#include "renderer.h"
#include <future>
namespace PO
{

	namespace Implement {
		struct plugin_interface;
	}

	class self {

		bool avalible = true;
		Tool::completeness_ref ref;
		std::function<void(self&, plugins&, viewer& v, duration da)> tick_f;
		std::function<Respond(event&, self&, plugins&, viewer& v)> respond_f;

		friend struct Implement::plugin_interface;

	public:

		self(const self&) = delete;
		self(Tool::completeness_ref r) :ref(std::move(r)) {}

		operator bool() const { return avalible; }
		void killmyself() { avalible = false; }
		
		template<typename T, typename ...AT> void auto_bind_tick(T&& t, AT&& ...at) { tick_f = Tool::auto_bind_function<void(self&, plugins&, viewer& v, duration da), Tool::unorder_adapt>(std::forward<T>(t), std::forward<AT>(at)...); }
		template<typename T, typename ...AT> void auto_bind_respond(T&& t, AT&& ...at) { respond_f = Tool::auto_bind_function<Respond(event&, self&, plugins&, viewer& v), Tool::unorder_adapt>(std::forward<T>(t), std::forward<AT>(at)...); }

	};

	namespace Implement 
	{
		struct plugin_interface {
			std::shared_ptr<self> self_ptr;
			adapter_map mapping;
			operator bool() const { return *self_ptr; }
			operator self&() { return *self_ptr; }
			plugin_interface(Tool::completeness_ref r) : self_ptr(std::make_shared<self>(std::move(r))) {}

			void tick(plugins& p, viewer& v, duration da) { if (self_ptr->tick_f) self_ptr->tick_f(*self_ptr, p, v, da); }
			Respond respond(event& e, plugins& p, viewer& v) { return self_ptr->respond_f ? self_ptr->respond_f(e, *self_ptr, p, v) : Respond::Pass; }
			virtual ~plugin_interface() = default;
		};

		template<typename plugin_t>
		class plugin_packet : public plugin_interface, public plugin_t {

		public:
			template<typename ...AT>
			plugin_packet(Tool::completeness_ref r, AT&& ...at) : plugin_interface(std::move(r)), plugin_t(std::forward<AT>(at)...) {
				plugin_interface::mapping = plugin_t::mapping(*this);
			}
		};

		template<typename plugin_t> using plugin_final = Tool::completeness<plugin_packet<std::decay_t<plugin_t>>>;
	}

	namespace Implement {
		template<typename plugin_t> struct have_make_adapter_map
		{
			template<typename P> static std::true_type func(
				std::enable_if_t<
					std::is_same<
						decltype(Tmp::itself<plugin_t>{}().mapping(Tmp::itself<self&>{}())),
						adapter_map
					>::value
				>*);
			template<typename P> static std::false_type func(...);
			static constexpr bool value = decltype(func<plugin_t>(nullptr))::value;
		};
	}

	template<typename plugin_t> struct plugin {
		static_assert(Implement::have_make_adapter_map<plugin_t>::value, "plugin should need an memeber function \'adapter_map mapping(self&)\'");
	};

	class plugins 
	{
		value_table om;

		using plugin_tank_t = std::vector<std::unique_ptr<Implement::plugin_interface>>;

		Tool::scope_lock<plugin_tank_t> depute_plugin_tank;
		plugin_tank_t raw_plugin_tank;
		plugin_tank_t plugin_tank;

		using renderer_tank_t = std::vector<std::unique_ptr<Implement::renderer_interface>>;

		renderer_tank_t renderer_tank;

		using renderer_depute_f = std::function<std::unique_ptr<Implement::renderer_interface>(value_table&)>;
		using renderer_depute_tank_t = std::vector<renderer_depute_f>;

		Tool::scope_lock<renderer_depute_tank_t> depute_renderer_f_tank;

	public:

		plugins(value_table o) : om(std::move(o)){}
		~plugins();

		template<typename plugin_t, typename ...AP>
		void create(plugin<plugin_t> p, AP&& ...ap) {
			std::unique_ptr<Implement::plugin_interface> ptr = std::make_unique<Implement::plugin_final<plugin_t>>(std::forward<AP>(ap)...);
			depute_plugin_tank.lock([&ptr](plugin_tank_t& tank) {
				tank.push_back(std::move(ptr));
			});
		}

		/*
		template<typename renderer_t, typename ...AP>
		void create(renderer<renderer_t>, AP&& ...ap) {
			std::unique_ptr<Implement::renderer_interface> ptr = std::make_unique<Implement::renderer_expand_t<renderer_t>>(om, std::forward<AP>(ap)...);
			depute_renderer_tank.lock([&ptr](renderer_tank_t& tank) {
				tank.push_back(std::move(ptr));
			});
		}*/

		template<typename renderer_t, typename ...AP>
		void create(renderer<renderer_t>, AP&& ...ap) {
			depute_renderer_f_tank.lock([=](renderer_depute_tank_t& tabk) {
				tabk.push_back([&ap...](value_table& vt) -> std::unique_ptr<Implement::renderer_interface> {  
					return std::make_unique<Implement::renderer_expand_t<renderer_t>>(vt, std::forward<AP>(ap)...);
				});
			});
		}

		void tick(viewer& v, duration da);
		Respond respond(event& ev, viewer& v);
	};
}
