#pragma once
#include <Windows.h>
#include "../../po/frame/define.h"
#include <atomic>
#include <deque>
#include <mutex>
#include <set>
namespace PO
{

	namespace Error
	{
		class win32_exception : public std::exception
		{
			void *pMsgBuf;
		public:
			win32_exception(HRESULT re)
			{
				FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, re, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&pMsgBuf, 0, NULL);
			}
			const char* what() const override { return static_cast<const char*>(pMsgBuf); }
			win32_exception(const win32_exception&) = default;
			win32_exception(win32_exception&&) = default;
			~win32_exception() { LocalFree(pMsgBuf); }
		};
		template<typename T>
		void fail_throw(HRESULT re, T&& t)
		{
			if (!SUCCEEDED(re))
			{
				t();
				throw win32_exception(re);
			}
		}

		inline void fail_throw(HRESULT re)
		{
			if (!SUCCEEDED(re))
			{
				throw win32_exception(re);
			}
		}
	}

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