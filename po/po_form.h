#pragma once
#include "po_plugin.h"
#include <memory>
#include <list>
namespace PO
{
	namespace Assistant
	{

		struct form_ptr
		{
			bool avalible;
			duration frame_duration = duration(10);
			std::thread logic_form_thread;
			std::atomic_bool force_exist_form;
			virtual ~form_ptr() {}
		};

		template<typename T> struct form_plugin
		{

			std::mutex pim;
			std::list<std::unique_ptr<plugin_ptr<T>>> inilizered_plugin_list;

			std::mutex pifm;
			std::function<std::unique_ptr<plugin_ptr<T>>(void)> pif;

			std::mutex rm;
			std::list<std::unique_ptr<plugin_ptr<T>>> plugin_list;

			std::thread::id ID;

			form_plugin() :ID(std::this_thread::get_id()) {}

			template<typename K,typename ...AK>
			decltype(auto) create_plugin(typename frame<T>::form& f,AK&& ...ak)
			{
				if (std::this_thread::get_id() == ID)
				{
					auto ptr = std::make_unique<plugin_final<T, K>>(std::forward<AK>(ak)...);
					plugin_implement<T, K>* pi = ptr.get();
					std::lock_guard<decltype(pim)> ld(pim);
					inilizered_plugin_list.push_back(std::move(ptr));
					return plugin_view<T, K>(pi);
				}
				else {
					std::promise<plugin_view<T, K>> pro;
					auto fur = pro.get_future();
					{
						std::lock_guard<decltype(pim)> ld(pim);
						while (pif)
						{
							pim.unlock();
							std::this_thread::yield();
							pim.lock();
						}
						pif = [&]() ->std::unique_ptr<plugin_ptr<T>>
						{
							auto ptr = std::make_unique<plugin_final<T, K>>(std::forward<AK>(ak)...);
							plugin_implement<T, K>* pi = ptr.get();
							pro.set_value(plugin_view<T, K>(pi));
							return std::move(ptr);
						};
					}
					fur.wait();
					return fur.get();
				}
			}

			void run_tick(tick_view<T>& tv)
			{

				{
					std::lock_guard<decltype(pifm)> ld(pifm);
					if (pif)
					{
						std::lock_guard<decltype(pim)> ld(pim);
						inilizered_plugin_list.push_back(pif());
						pif = std::function<std::unique_ptr<plugin_ptr<T>>(void)>();
					}
				}

				{
					std::lock_guard<decltype(pim)> ld(pim);
					if (!inilizered_plugin_list.empty())
					{
						std::lock_guard<decltype(rm)> ld(rm);
						plugin_list.splice(plugin_list.end(), std::move(inilizered_plugin_list));
					}
				}

				std::lock_guard<decltype(rm)> ld(rm);
				for (auto ptr = plugin_list.begin(); ptr != plugin_list.end();)
				{
					if ((*ptr))
					{
						if (**ptr)
						{
							(*ptr++)->tick(tv);
							continue;
						}
					}
					plugin_list.erase(ptr++);
				}
			}
		};
	}

	template<typename T> class tick_view : frame<T>::tick_view
	{
		Assistant::form_plugin<T>* plugin;
		duration time;
	public:
		tick_view(typename T::form* f, duration d, Assistant::form_plugin<T>* fp) :frame<T>::tick_view(f), plugin(fp), time(d) {}
		tick_view(const tick_view&) = default;
		decltype(auto) get_time() const { return time; }
	};

	namespace Assistant
	{
		template<typename T> struct form_instance: form_ptr
		{

			using form_ptr::form_ptr;

			struct deliver_data
			{
				Tool::completeness_protector_ref plugin_cpr;
				typename form_plugin<T>& fp;
				typename frame<T>::form& render;
			};

			template<typename ...AK>
			form_instance(std::promise<std::unique_ptr<deliver_data>>& promise, AK&& ... ak)
			{
				this->form_ptr::force_exist_form = false;
				logic_form_thread = std::thread(
					[&ak..., this, &promise]()
				{
					typename frame<T>::form render{std::forward<AK>(ak)... };
					Tool::completeness_protector<form_plugin<T>> plugin_list;
					promise.set_value(std::make_unique<deliver_data>(deliver_data{ plugin_list, plugin_list , render }));
					std::chrono::time_point<std::chrono::system_clock> start_loop = std::chrono::system_clock::now();
					std::this_thread::sleep_until(start_loop + this->frame_duration);
					while (!this->form_ptr::force_exist_form  && render)
					{
						PO::tick_view<T> tem(&render, std::chrono::duration_cast<duration>(std::chrono::system_clock::now() - start_loop), &plugin_list);
						plugin_list.run_tick(tem);

						start_loop = std::chrono::system_clock::now();
						std::this_thread::sleep_until(start_loop + this->frame_duration);
					}
				}
				);
			};

			~form_instance()
			{
				if (logic_form_thread.joinable())
					logic_form_thread.join();
			}

		};

	}

	template<typename T> class form_view : frame<T>::form_view
	{
		Tool::completeness_protector_ref cpf;
		typename Assistant::form_plugin<T>* fp;
	public:

		class view
		{
			form_view* fv;
		public:
		};

		form_view(typename frame<T>::form* f, Tool::completeness_protector_ref&& c, typename Assistant::form_plugin<T>* fpp) :
			frame<T>::form_view(f),
			cpf(c),
			fp(fpp)
		{

		}

		template<typename K, typename ...AK>
		decltype(auto) create_plugin(AK&&... ak)
		{
			plugin_view<T, K> pv;
			cpf.lock_if(
					[&,this]() 
			{
				pv = fp->create_plugin<K>(*this->form, std::forward<AK>(ak)...);
			}
			);
			return pv;

		}

	};
}