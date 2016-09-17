#pragma once
#include "po_form.h"
#include "tool\tool.h"
#include <thread>
#include <iostream>
#include "event\event.h"
#include <future>
#include <atomic>
#include <deque>
#include <memory>
#include <list>
namespace PO
{

	class context
	{

		std::mutex this_form_mutex;
		std::deque<std::unique_ptr<PO::Assistant::form_ptr>> this_form;

	public:

		void wait_all_form_close();
		
		context();
		~context();
		void set_form(std::unique_ptr<Assistant::form_ptr>&& fp)
		{
			std::lock_guard<decltype(this_form_mutex)> ld(this_form_mutex);
			this_form.push_back(std::move(fp));
		}
		void detach();
		template<typename T, typename ...AK> decltype(auto) create_window(AK&& ...ak)
		{
			frame_static_assert<T>{};
			using fd = std::unique_ptr<typename Assistant::form_instance<T>::deliver_data>;
			std::promise<fd> promise;
			auto future = promise.get_future();
			std::unique_ptr<Assistant::form_ptr> tem = std::make_unique<Assistant::form_instance<T>>(promise,std::forward<AK>(ak)...);
			set_form(std::move(tem));
			future.wait();
			fd result = future.get();
			return form_view<T>{ &result->render, std::move(result->plugin_cpr), &result->fp };
		}
	};
	
}