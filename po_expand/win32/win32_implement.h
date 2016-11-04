#pragma once
#include <Windows.h>
#include "../../po/frame/define.h"
#include <atomic>
#include <deque>
#include <mutex>
#include <set>
namespace PO
{
	namespace Interface
	{
		struct win32_style
		{
			DWORD window_style = WS_VISIBLE | WS_OVERLAPPEDWINDOW;
			DWORD ex_window_style = WS_EX_CLIENTEDGE;
		};

		struct win32_initial_implement
		{
			std::u16string title = u"PO default title :>";
			int shitf_x = (GetSystemMetrics(SM_CXSCREEN) - 1024) / 2;
			int shift_y = (GetSystemMetrics(SM_CYSCREEN) - 768) / 2;
			int width = 1024;
			int height = 768;
			win32_style style = win32_style();
		};

		struct win32_form_implement
		{
			HWND raw_handle;
			std::mutex input_mutex;
			std::deque<event> input_event;
			std::deque<event> output_event;
		};
		
	}
}