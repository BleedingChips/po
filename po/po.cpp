#include "po.h"

/*
namespace 
{
	std::mutex window_list_mutex;
	std::list < std::unique_ptr<PO::Platform::window_instance> > window_list;

	std::mutex init_count_mutex;
	size_t init_count = 0;
}

namespace PO
{

	gui_context::gui_context()
	{
		init_count_mutex.lock();
		++init_count;
		init_count_mutex.unlock();
	}

	gui_context::~gui_context()
	{
		init_count_mutex.lock();
		--init_count;
		if (init_count == 0)
		{
			window_list_mutex.lock();
			window_list.clear();
			window_list_mutex.unlock();
		}
		init_count_mutex.unlock();
	}

	void gui_context::regedit_window(std::unique_ptr<PO::Platform::window_instance>&& ptr)
	{
		window_list_mutex.lock();
		window_list.push_back(std::move(ptr));
		window_list_mutex.unlock();
	}

	void gui_context::loop()
	{
		init_count_mutex.lock();
		if (init_count == 1)
		{
			init_count_mutex.unlock();
			while (true)
			{
				window_list_mutex.lock();
				if (window_list.size() > 0)
				{
					for (auto ptr = window_list.begin(); ptr != window_list.end(); )
					{
						if ((*ptr) && (*ptr)->is_exist())
							++ptr;
						else
							window_list.erase(ptr++);
					}
				}
				if (window_list.empty())
				{
					window_list_mutex.unlock();
					break;
				}
				window_list_mutex.unlock();
				PO::Platform::window_instance::one_frame_loop();
				std::this_thread::sleep_for(std::chrono::microseconds(1));
			}
		}
		else {
			init_count_mutex.unlock();
		}
	}












	namespace Event
	{
		bool event_box::respond_event(const PO::Platform::window_event& we)
		{
			
			bool respond = false;
			auto next_judge = [&respond](bool co) { respond = co; return !co; };
			switch (we.msg)
			{
			case WM_KEYUP:
			case WM_KEYDOWN:
				key_box(next_judge, key(we));
				break;
			case WM_MOUSEMOVE:
			case WM_MOUSEWHEEL:
				mouse_box(next_judge, mouse(we));
				break;
			default:
				break;
			}
			return respond;
			
			return true;
		}
	}
}*/