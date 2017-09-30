#pragma once
#include "form.h"

namespace PO
{
	class plugins; //store all plugin and renderer
	class self; //message
	

	namespace Implement {

		struct renderer_interface;

		struct adapter_store_t {
			using init_t = std::function<void(self&, plugins&, viewer&, renderer_interface&)>;
			using tick_t = std::function<void(self&, plugins&, viewer&, renderer_interface&, duration)>;
			init_t init;
			tick_t tick;
		};
	}

	using adapter_map = std::map<std::type_index, Implement::adapter_store_t>;

	namespace Implement {

		struct tick_proxy {
			std::weak_ptr<self> self_ptr;
			adapter_store_t::tick_t tick;
			bool operator()(plugins& p, viewer& v, renderer_interface& r, duration d);
			tick_proxy(std::weak_ptr<self>&& w, adapter_store_t::tick_t&& t) : self_ptr(std::move(w)), tick(std::move(t)) {}
			tick_proxy(tick_proxy&&) = default;
			tick_proxy& operator=(tick_proxy&&) = default;
		};

		template<typename T> using mf_pre_tick = decltype(std::declval<T>().pre_tick(std::declval<duration>()));
		template<typename T> using mf_pos_tick = decltype(std::declval<T>().pos_tick(std::declval<duration>()));
		template<typename T, typename ...AT> using mf_pre_respond = std::enable_if<std::is_same_v<decltype(std::declval<T>().pre_respond(std::declval<const event&>(), std::declval<AT>()...)),Respond>>;
		template<typename T, typename ...AT> using mf_pos_respond = std::enable_if<std::is_same_v<decltype(std::declval<T>().pos_respond(std::declval<const event&>(), std::declval<AT>()...)), Respond>>;

		struct renderer_interface {
			std::type_index ti;
			std::vector<tick_proxy> tick_proxy_list;
			renderer_interface(std::type_index t) : ti(t) {}

			const std::type_index& id() const { return ti; }

			bool insert(const std::type_index& ti, std::weak_ptr<self> ptr, adapter_store_t::init_t& it, adapter_store_t::tick_t tf, plugins&, viewer& v);
			void tick(plugins&, viewer&, duration da);
			virtual void pre_tick(duration da) = 0;
			virtual void pos_tick(duration da) = 0;
			virtual ~renderer_interface();
		};

		template<typename renderer_t> class renderer_expand_t : public renderer_interface, public renderer_t
		{

			template<typename ...AT>
			renderer_expand_t(std::true_type, value_table& vt, AT&&... at) : renderer_interface(typeid(renderer_t)), renderer_t(vt, std::forward<AT>(at)...) {}

			template<typename ...AT>
			renderer_expand_t(std::false_type, value_table& vt, AT&&... at) : renderer_interface(typeid(renderer_t)), renderer_t(std::forward<AT>(at)...) {}

		public:

			template<typename ...AK>
			renderer_expand_t(value_table& v, AK&& ...ak) :
				renderer_expand_t(std::integral_constant<bool, std::is_constructible<renderer_t, value_table&, AK...>::value || true>{}, v, std::forward<AK>(ak)...) {}
			void pre_tick(duration da) {
				if constexpr(Tmp::able_instance_v<mf_pre_tick, renderer_t>)
					renderer_t::pre_tick(da);
			}
			void pos_tick(duration da) {
				if constexpr(Tmp::able_instance_v<mf_pos_tick, renderer_t>)
					renderer_t::pos_tick(da);
			}
		};
		template<typename renderer_t> using renderer_expand = renderer_expand_t<std::decay_t<renderer_t>>;
	}

	template<typename renderer_t> struct renderer {
		//static_assert(Implement::have_make_proxy<renderer_t>::value, "renderer need have a memeber function \'proxy mapping(std::type_index, adapter_interface&)\'");
		//static_assert(Implement::have_init<renderer_t>::value, "renderer need have a memeber function \'void init(value_table&)\'");
	};

	template<typename renderer_t, typename ptr, typename init_t, typename tick_t> std::pair<std::type_index, Implement::adapter_store_t> make_member_adapter(ptr* p, init_t&& i, tick_t&& t) {
		return
			std::pair<std::type_index, Implement::adapter_store_t>{
			typeid(renderer_t),
			Implement::adapter_store_t{
				[p, i](self& s, plugins& pl, viewer& v, Implement::renderer_interface& ri) {
				Tool::auto_adapter<Tool::unorder_adapt>(i, p, s, pl, v, static_cast<renderer_t&>(static_cast<Implement::renderer_expand_t<renderer_t>&>(ri)));
			},
				[p, t](self& s, plugins& pl, viewer& v, Implement::renderer_interface& ri, duration da) {
				Tool::auto_adapter<Tool::unorder_adapt>(t, p, s, pl, v, static_cast<renderer_t&>(static_cast<Implement::renderer_expand_t<renderer_t>&>(ri)), da);
			}
			}
		};
	}
	
}
