#pragma once
#include "win32_define.h"
#include "../../form.h"
#include <atomic>
#include <deque>
#include <mutex>
#include <set>
namespace PO
{
	namespace Win32
	{
		struct win32_form_style
		{
			DWORD window_style = WS_VISIBLE | WS_OVERLAPPEDWINDOW;
			DWORD ex_window_style = WS_EX_CLIENTEDGE;
		};

		struct win32_initial
		{
			std::u16string title = u"PO default title :>";
			int shitf_x = (GetSystemMetrics(SM_CXSCREEN) - 1024) / 2;
			int shift_y = (GetSystemMetrics(SM_CYSCREEN) - 768) / 2;
			int width = 1024;
			int height = 768;
			win32_form_style style = win32_form_style();
		};

		struct win32_form : PO::form_constraint
		{
			HWND raw_handle;
			std::atomic_bool quit;
			using tank = std::vector<event>;
			Tool::scope_lock<tank> capture_event_tank;
			tank event_tank;
			win32_form(const win32_initial& = win32_initial{});
			~win32_form();
			value_table mapping();
			bool available() const { return !quit; }
			Respond handle_event(event& e);
			void pre_tick(duration da);
		};
	}
}