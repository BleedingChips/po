#include "po.h"
namespace PO
{

	const char* value_table::value_not_exist::what() const { return "value not exist!"; }

	namespace Implement 
	{
		void form_ptr::push_function(std::function<std::unique_ptr<form_interface>(void)> f) {
			if (logic_form_thread.joinable())
			{
				force_exist_form = true;
				logic_form_thread.join();
			}
			force_exist_form = false;
			logic_form_thread = std::thread([f = std::move(f), this](){
				auto ptr = f();
				time_calculator tc;
				tc.record_point = std::chrono::system_clock::now();
				while (!force_exist_form && *ptr)
				{
					duration da;
					auto now = std::chrono::system_clock::now();
					if(tc.tick(now, da))
						ptr->tick(da);
					std::this_thread::sleep_until(now + duration(1));
				}
			});
		}
	}
}

namespace
{
	PO::Tool::scope_lock<size_t> init_count = 0;
	PO::Tool::scope_lock<std::vector<std::unique_ptr<PO::Implement::form_ptr>>> all_form;
}

namespace PO
{

	namespace Implement
	{
		form_ptr::~form_ptr()
		{
			if (logic_form_thread.joinable())
				logic_form_thread.join();
		}
	}

	void context::set_form(std::unique_ptr<Implement::form_ptr> fp)
	{
		this_form.lock([&](decltype(this_form)::type& i) {
			i.push_back(std::move(fp));
		});
	}

	context::context()
	{
		init_count.lock([](size_t& i) {++i; });
	}

	context::~context()
	{
		this_form.lock([this](decltype(this_form)::type& t) {
			for (auto& ptr : t)
				ptr->force_exist_form = true;
			t.clear();
		});

		init_count.lock([this](size_t& i) {
			if (--i == 0)
			{
				all_form.lock([](decltype(all_form)::type& at) {
					for (auto& ptr : at)
						ptr->force_exist_form = true;
					at.clear();
				});
			}
		});
	}

	void context::detach()
	{
		Tool::lock_scope_look(all_form, this_form, [](decltype(all_form)::type& i, decltype(this_form)::type& t) {
			i.insert(i.end(), std::make_move_iterator(t.begin()), std::make_move_iterator(t.end()));
			t.clear();
		});
	}

	void context::wait_all_form_close()
	{
		this_form.lock([this](decltype(this_form)::type& t) {
			t.clear();
		});
	}
}