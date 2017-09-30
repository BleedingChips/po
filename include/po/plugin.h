#pragma once
#include <functional>
#include "frame/viewer.h"
#include "tool/auto_adapter.h"
#include "extension.h"
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
		std::function<Respond(const event&, self&, plugins&, viewer& v)> respond_f;

		friend struct Implement::plugin_interface;

	public:

		self(const self&) = delete;
		self(Tool::completeness_ref r) :ref(std::move(r)) {}

		operator bool() const { return avalible; }
		void killmyself() { avalible = false; }
		
		template<typename T, typename ...AT> void auto_bind_tick(T&& t, AT&& ...at) { tick_f = Tool::auto_bind_function<void(self&, plugins&, viewer& v, duration da), Tool::unorder_adapt>(std::forward<T>(t), std::forward<AT>(at)...); }
		template<typename T, typename ...AT> void auto_bind_respond(T&& t, AT&& ...at) { respond_f = Tool::auto_bind_function<Respond(const event&, self&, plugins&, viewer& v), Tool::unorder_adapt>(std::forward<T>(t), std::forward<AT>(at)...); }

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
			Respond respond(const event& e, plugins& p, viewer& v) { return self_ptr->respond_f ? self_ptr->respond_f(e, *self_ptr, p, v) : Respond::Pass; }
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

		template<typename T>
		struct check
		{
			using funtype = Tmp::pick_func<typename Tmp::degenerate_func<Tmp::extract_func_t<T>>::type>;
			static_assert(funtype::size == 1, "only receive one parameter");
			using true_type = std::decay_t<typename funtype::template out<Tmp::itself>::type>;
		};

	protected:
		value_table om;

		using plugin_tank_t = std::vector<std::unique_ptr<Implement::plugin_interface>>;

		Tool::scope_lock<plugin_tank_t> depute_plugin_tank;
		plugin_tank_t raw_plugin_tank;
		plugin_tank_t plugin_tank;

		std::unique_ptr<Implement::renderer_interface> renderer_ptr;
		using renderer_depute_f = std::function<std::unique_ptr<Implement::renderer_interface>(value_table&)>;
		Tool::scope_lock<renderer_depute_f> depute_renderer_function;

		using extension_ptr = std::unique_ptr<Implement::extension_interface>;
		using extension_f = std::function<extension_ptr(value_table&)>;

		Tool::scope_lock<std::vector<extension_f>> extension_delegate_function;
		std::vector<extension_f> inside_extension_delegate_function;

		std::map<std::type_index, extension_ptr> extension_map;

	public:

		plugins(value_table o) : om(std::move(o)){}
		~plugins();

		template<typename function_t> bool find_extension(function_t&& t)
		{
			using type = typename check<function_t>::true_type;
			auto ite = extension_map.find(typeid(type));
			if (ite != extension_map.end())
				return ite->second->cast(t);
			return false;
		}
		
	};

	class plugins_implement : protected plugins
	{
	public:

		template<typename plugin_t, typename ...AP>
		void create(plugin<plugin_t> p, AP&& ...ap) {
			std::unique_ptr<Implement::plugin_interface> ptr = std::make_unique<Implement::plugin_final<plugin_t>>(std::forward<AP>(ap)...);
			depute_plugin_tank.lock([&ptr](plugin_tank_t& tank) {
				tank.push_back(std::move(ptr));
			});
		}

		template<typename extension_t, typename ...AP>
		void create(extension<extension_t>, AP&& ...ap) {
			extension_delegate_function.lock([&](decltype(extension_delegate_function)::type& tabk) {
				tabk.push_back([=](value_table& vt) -> std::unique_ptr<Implement::extension_interface> {
					return std::make_unique<Implement::extension_implement<extension_t>>(vt, std::forward<AP>(ap)...);
				});
			});
		}

		template<typename renderer_t, typename ...AP>
		void create(renderer<renderer_t>, AP&& ...ap) {
			depute_renderer_function.lock([&](renderer_depute_f& tabk) {
				tabk = [=](value_table& vt) -> std::unique_ptr<Implement::renderer_interface> {
					return std::make_unique<Implement::renderer_expand_t<renderer_t>>(vt, std::forward<AP>(ap)...);
				};
			});
		}

		plugins_implement(value_table o) : plugins(std::move(o)) {}
		void tick(viewer& v, duration da);
		Respond respond(const event& ev, viewer& v);
		operator plugins& () { return *this; }
	};
}
