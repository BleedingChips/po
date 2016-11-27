#pragma once
#ifdef _DEBUG
	#pragma comment(lib,"po_d.lib")
#else
	#pragma comment(lib,"po.lib")
#endif
#include "tool\thread_tool.h"
#include <thread>
#include <iostream>
#include <future>
#include <atomic>
#include <deque>
#include <memory>
#include <list>
#include "frame\frame.h"



/*

struct event
[must
carry the event infomation from form or input

should provide event().
should provide opertor=(const event&).
]



struct form;
[must

should provide bool is_avalible() to show whether this form should close, call in same thread.
should provide void bind_event(const PO::Tool::completeness_ref&, std::function<bool(event&)>&&) or bind_event(std::function<bool(event&)>&&)to get the event function from form_append.

optional provide bool check_tick(duration) to return weather enter plugin tick and form tick function, call in same thread.
optional provide void tick(duration) call in each tick, call in same thread.

each form will be constructed in difference thread.
should be close form in destruction.
]



struct view_outside;
[optional

should provide view_outside::view_outside(form&), this will call in the same thread of form.
should provide view_outside::view_outside(const view_outside&) or view_outside::view_outside(view_outside&), this will call in diference thread, may call many time.

viewer's life time is usually longer then form.
usually call in diference thread.
]

struct view_inside;
[optional

should provide view_inside::view_inside(form&).
should provide view_inside::view_inside(const view_inside&) or view_inside::view_inside(view_inside&).

construct and use in the same thread of form.
viewer's life time is same as form.
]

*/

namespace PO
{

}

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
	
}