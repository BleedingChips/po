#pragma once
#include "po_frame.h"
namespace PO
{
	template<typename T> struct plugin_define
	{
		//using init_view = T::init_view;
		//using render_view = T::render_view;
	};

	template<typename T> class tick_view;

	namespace Assistant
	{
		

		template<typename T> struct plugin_ptr
		{
			bool use_tick = true;
			virtual void tick_implement(tick_view<T>& du) = 0;
			virtual bool avalible_implemenet() const = 0;
			operator bool() const { return avalible_implemenet(); }
			void tick(tick_view<T>& tv)
			{
				if (use_tick)
					tick_implement(tv);
			}
			virtual ~plugin_ptr() {}
		};

		template<typename K, typename T, typename = void> struct able_to_call_tick :std::false_type {};
		template<typename K, typename T> struct able_to_call_tick<K, T, std::void_t<decltype(std::declval<T>().tick(std::declval<tick_view<K>&>()))>> :std::true_type {};

		template<typename T, typename K> struct plugin_implement : plugin_ptr<T>, K
		{
			void tick_implement(tick_view<T>& du) override
			{
				Tool::statement_if<Assistant::able_to_call_tick<T, K>::value>
					(
						[&du](auto&& plugin) { plugin.tick(du); },
						[this](auto&& plugin) {this->use_tick = false; },
						static_cast<K&>(*this)
						);
			}

			virtual bool avalible_implemenet() const override
			{
				return true;
			}

			template<typename ...AK>
			plugin_implement(AK&& ...ak) :K(std::forward<AK>(ak)...) {}
		};

		template<typename T,typename K> using plugin_final = Tool::completeness_protector<plugin_implement<T, K>>;

	}

	template<typename T, typename K> struct plugin_view
	{
		Assistant::plugin_implement<T, K>* ref = nullptr;
		plugin_view(Assistant::plugin_implement<T, K>* pi) :ref(pi)
		{
		}
		plugin_view(const plugin_view& pv) = default;
		plugin_view() = default;
		plugin_view& operator = (const plugin_view& pv) = default;
	};
}