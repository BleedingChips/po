#pragma once
#include "po.h"
namespace PO
{
	namespace Assistant
	{
		template<typename T, typename K> template<typename ...AK> plugin_implement<T, K>::plugin_implement(
			const Tool::completeness_ref& cpr, pre_inital<T>& pi, AK&& ...ak
		):plugin_implement(std::integral_constant<bool, std::is_constructible<K, inital<T>, AK...>::value>(), 
			inital<T>(cpr, pi), std::forward<AK>(ak)...)
		{}

		template<typename T> template<typename K, typename ...AK> decltype(auto)  form_append<T>::create_plugin_inside(AK&& ...ak)
		{
			auto ptr = std::make_unique<plugin_final<T, K>>(pe, std::forward<AK>(ak)...);
			inilizered_plugin_list.push_back(std::move(ptr));
		}

		template<typename T> template<typename K, typename ...AK> decltype(auto)  form_append<T>::create_plugin_outside(AK&& ...ak)
		{
			std::unique_lock<decltype(pifm)> ul(pifm);
			cv.wait(ul, [this]() {return !static_cast<bool>(pif); });
			pif = [&ak...,this]() ->std::unique_ptr<plugin_ptr<T>>
			{
				auto ptr = std::make_unique<plugin_final<T, K>>(pe,std::forward<AK>(ak)...);
				return std::move(ptr);
			};
		}

		template<typename T> void form_append<T>::run_tick(ticker<T>& ti)
		{
			{
				std::unique_lock<decltype(pifm)> ld(pifm);
				while (pif)
				{
					inilizered_plugin_list.push_back(pif());
					pif = std::function<std::unique_ptr<plugin_ptr<T>>(void)>();
					ld.unlock();
					cv.notify_one();
					std::this_thread::yield();
					ld.lock();
				}
			}

			if (inilizered_plugin_list.empty())
			{
				plugin_list.splice(plugin_list.end(), std::move(inilizered_plugin_list));
			}

			if (Tool::statement_if<frame<T>::check_tick>(
				[&ti](auto& t) { return t.check_tick(ti.get_time());  },
				[](auto& t) {return true; },
				*this
				))
			{
				for (auto ptr = plugin_list.begin(); ptr != plugin_list.end();)
				{
					if ((*ptr))
					{
						if (**ptr)
						{
							(*ptr++)->tick(ti);
							continue;
						}
					}
					plugin_list.erase(ptr++);
				}
				Tool::statement_if<frame<T>::tick>([&ti](auto& i) {return i.tick(ti.get_time()); })(*this);
			}
		}


		template<typename T, typename ...AK> auto form_ptr::create_window(AK&& ...ak)
		{
			if (logic_form_thread.joinable())
			{
				force_exist_form = true;
				logic_form_thread.join();
			}
			force_exist_form = false;
			std::promise<std::shared_ptr<viewer<T>>> p;
			auto fur = p.get_future();
			force_exist_form = false;
			logic_form_thread = std::thread(
				[&,this]()
			{
				form_packet<T> packet(*this, std::forward<AK>(ak)...);
				p.set_value(std::make_shared<viewer<T>>(ref, *this, packet));
				ticker<T> ti(*this, packet);
				std::chrono::time_point<std::chrono::system_clock> start_loop = std::chrono::system_clock::now();
				std::this_thread::sleep_until(start_loop + this->frame_duration);
				while (!force_exist_form  && packet.is_avalible())
				{
					duration da = std::chrono::duration_cast<duration>(std::chrono::system_clock::now() - start_loop);
					ti.set_time(da);					
					packet.run_tick(ti);
					start_loop = std::chrono::system_clock::now();
					std::this_thread::sleep_until(start_loop + this->frame_duration);
				}
			}
			);
			fur.wait();
			return std::move(fur.get());
		}


	}
}
