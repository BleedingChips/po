#include "po.h"

namespace
{
	std::mutex init_count_mutex;
	size_t init_count = 0;

	std::recursive_mutex all_form_mutex;
	std::deque<std::unique_ptr<PO::Implement::form_ptr>> all_form;
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

	context::context()
	{
		init_count_mutex.lock();
		++init_count;
		init_count_mutex.unlock();
	}

	context::~context()
	{

		{
			std::lock_guard<decltype(this_form_mutex)> ld(this_form_mutex);
			for (auto& ptr : this_form)
			{
				ptr->force_exist_form = true;
			}
			for (auto& ptr : this_form)
			{
				ptr.reset();
			}
			this_form.clear();
		}

		{
			std::lock_guard<decltype(init_count_mutex)> ld(init_count_mutex);
			if (--init_count == 0)
			{
				std::lock_guard<decltype(all_form_mutex)> ld(all_form_mutex);
				for (auto& ptr : all_form)
				{
					ptr->force_exist_form = true;
				}
				for (auto& ptr : all_form)
				{
					ptr.reset();
				}
				all_form.clear();
			}
		}
	}

	void context::detach()
	{
		std::lock_guard<decltype(all_form_mutex)> ld(all_form_mutex);
		std::lock_guard<decltype(this_form_mutex)> ld2(this_form_mutex);
		for (auto& io : this_form)
			all_form.push_back(std::move(io));
		this_form.clear();
	}

	void context::wait_all_form_close()
	{
		{
			std::lock_guard<decltype(this_form_mutex)> ld(this_form_mutex);
			for (auto& ptr : this_form)
			{
				ptr.reset();
			}
			this_form.clear();
		}

		{
			std::lock_guard<decltype(init_count_mutex)> ld(init_count_mutex);
			if (--init_count == 0)
			{
				std::lock_guard<decltype(all_form_mutex)> ld(all_form_mutex);
				for (auto& ptr : all_form)
				{
					ptr->force_exist_form = true;
				}
				for (auto& ptr : all_form)
				{
					ptr.reset();
				}
				all_form.clear();
			}
		}
	}
}