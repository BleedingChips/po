#pragma once
#include "tool\thread_tool.h"
#include <thread>
#include <iostream>
#include <future>
#include <atomic>
#include <deque>
#include <memory>
#include <list>
#include "frame\frame.h"

namespace PO
{
	template<typename frame_t> using frame = Tmp::itself<frame_t>;


	class context
	{

		std::mutex this_form_mutex;
		std::deque<std::unique_ptr<Implement::form_ptr>> this_form;

	public:

		void wait_all_form_close();

		context();
		~context();
		void set_form(std::unique_ptr<Implement::form_ptr>&& fp)
		{
			std::lock_guard<decltype(this_form_mutex)> ld(this_form_mutex);
			this_form.push_back(std::move(fp));
		}
		void detach();
		template<typename frame, typename ...AK> auto create_frame(Tmp::itself<frame> i, AK&& ...ak)
		{
			std::unique_ptr<Implement::form_ptr> tem = std::make_unique<Implement::form_ptr>();
			Implement::form_ptr& ptr = *tem;
			set_form(std::move(tem));
			return ptr.create_frame(i, std::forward<AK>(ak)...);
		}
	};
}
/*
namespace PO
{
	using self_control = Assistant::self_control;
	template<typename frame_type> using viewer = Assistant::viewer<frame_type>;
	template<typename frame_type> using initial = Assistant::initial<frame_type>;
	template<typename frame_type> using ticker = Assistant::ticker<frame_type>;
	template<typename frame_type> using responder = Assistant::responder<frame_type>;

	class ticker_self
	{
		self_control ps;
		duration da;
	public:
		template<typename frame_type>
		ticker_self(ticker<frame_type>& t) : ps(t), da(t) {}
		decltype(auto) get_self() { return ps; }
		decltype(auto) time() { return da; }
	};

	class responder_self
	{
		self_control ps;
		event& ev;
	public:
	};

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
		template<typename frame_type, typename ...AK> auto create_window(AK&& ...ak)
		{
			std::unique_ptr<Assistant::form_ptr> tem = std::make_unique<Assistant::form_ptr>();
			Assistant::form_ptr& ptr = *tem;
			set_form(std::move(tem));
			return ptr.create_window<frame_type>(std::forward<AK>(ak)...);
		}
	};
	
}*/