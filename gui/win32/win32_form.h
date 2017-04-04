#pragma once
#include "win32_define.h"
#include "../../po.h"
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

		struct win32_form
		{
			HWND raw_handle;
			using tank = std::vector<event>;
			Tool::scope_lock<tank> input_event;
			Tool::scope_lock<tank> output_event;
			win32_form(form_self&, const win32_initial& = win32_initial{});
			win32_form(const win32_initial& = win32_initial{});
			~win32_form();
			void tick(form_ticker& ft);
		};
	}
}