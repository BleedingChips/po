#pragma once
#include "frame/adapter.h"
namespace PO
{

	struct proxy {
		std::function<void(self&, plugins&, viewer&)> init;
		std::function<void(self&, plugins&, viewer&, duration)> tick;
	};

	template<typename renderer_t, typename input_type> static proxy make_proxy(adapter_interface& ai, input_type& f) {
		Implement::adapter_store_t<renderer_t>& adt = static_cast<Implement::adapter_store_t<renderer_t>&>(ai);
		std::reference_wrapper<renderer_t> ref(f);
		return {
			adt.init ? ([init = adt.init, ref](self& s, plugins& p, viewer& v) {init(s, p, v, ref); }) : (std::function<void(self&, plugins&, viewer&)>{}),
			adt.tick ? ([tick = adt.tick, ref](self& s, plugins& p, viewer& v, duration da) {tick(s, p, v, ref, da); }) : (std::function<void(self&, plugins&, viewer&, duration)>{})
		};
	}

	namespace Implement {

		struct tick_proxy{
			std::shared_ptr<self> self_ptr;
			std::function<void(self&, plugins&, viewer&, duration)> ptr;
		};

		struct init_proxy {
			std::shared_ptr<self> self_ptr;
			std::function<void(self&, plugins&, viewer&)> ptr;
		};

		template<typename renderer_t> struct have_pre_tick {
			template<typename P> static std::true_type func(decltype(std::declval<P>().pre_tick(duration()))*);
			template<typename P> static std::false_type func(...);
			void operator()(renderer_t& r, duration da) {
				Tool::statement_if<decltype(func<renderer_t>(nullptr))::value>(
					[da](auto& r) {r.pre_tick(da); },
					[](auto& r) {},
					r
					);
			}
		};

		template<typename renderer_t> struct have_pos_tick {
			template<typename P> static std::true_type func(decltype(std::declval<P>().pos_tick(duration()))*);
			template<typename P> static std::false_type func(...);
			void operator()(renderer_t& r, duration da) {
				Tool::statement_if<decltype(func<renderer_t>(nullptr))::value>(
					[da](auto& r) {r.pos_tick(da); },
					[](auto& r) {},
					r
					);
			}
		};

		struct renderer_interface {
			std::type_index ti;
			std::vector<tick_proxy> tick_proxy_list;
			std::vector<init_proxy> init_proxy_list;

			renderer_interface(std::type_index t) : ti(t) {}
			virtual void plugin_register(std::shared_ptr<self> self, adapter_map& am) = 0;
			void tick(plugins&, viewer&, duration da);
			virtual void pre_tick(duration da) = 0;
			virtual void pos_tick(duration da) = 0;
			virtual void init(value_table& om) = 0;
			virtual ~renderer_interface() = default;
		};

		template<typename renderer_t> struct renderer_expand_t : renderer_interface, renderer_t
		{
			template<typename ...AK>
			renderer_expand_t(AK&& ...ak) : renderer_interface(typeid(renderer_t)), renderer_t(std::forward<AK>(ak)...) {}
			void plugin_register(std::shared_ptr<self> self, adapter_map& am) {
				if (!self) return;
				for (auto& po : am) {
					proxy return_proxy = renderer_t::mapping(po->index(), *po);
					if (return_proxy.init)
						init_proxy_list.push_back(init_proxy{ self, std::move(return_proxy.init) });
					if (return_proxy.tick)
						tick_proxy_list.push_back(tick_proxy{ std::move(self), std::move(return_proxy.tick) });
				}
			}
			void pre_tick(duration da) {
				have_pre_tick<renderer_t>{}(*this, da);
			}
			void pos_tick(duration da) {
				have_pos_tick<renderer_t>{}(*this, da);
			}
			virtual void init(value_table& om) {
				renderer_t::init(om);
			}
		};
		template<typename renderer_t> using renderer_expand = renderer_expand_t<std::decay_t<renderer_t>>;
	}

	namespace Implement {
		template<typename renderer_t> struct have_make_proxy
		{
			template<typename P> static std::true_type func(std::enable_if_t<std::is_same<
				decltype(Tmp::itself<renderer_t>{}().mapping(Tmp::itself<std::type_index>{}(), Tmp::itself<adapter_interface&>{}()))
				, proxy>::value>*);
			template<typename P> static std::false_type func(...);
			static constexpr bool value = decltype(func<renderer_t>(nullptr))::value;
		};
		template<typename renderer_t> struct have_init
		{
			template<typename P> static std::true_type func(decltype(Tmp::itself<renderer_t>{}().init(Tmp::itself<value_table&>{}()))*);
			template<typename P> static std::false_type func(...);
			static constexpr bool value = decltype(func<renderer_t>(nullptr))::value;
		};
	}


	template<typename renderer_t> struct renderer {
		static_assert(Implement::have_make_proxy<renderer_t>::value, "renderer need have a memeber function \'proxy mapping(std::type_index, adapter_interface&)\'");
		static_assert(Implement::have_init<renderer_t>::value, "renderer need have a memeber function \'void init(value_table&)\'");
	};
	
}
