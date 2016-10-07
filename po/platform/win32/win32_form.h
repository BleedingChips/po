#pragma once
#include <Windows.h>
#include <string>
#include <mutex>
#include <atomic>
#include <future>
#include <thread>
#include <deque>
namespace PO
{
	namespace Platform
	{
		namespace Win32
		{
			struct win32_init_error :std::exception
			{
				virtual char const* what() const override
				{
					return "unable to create win32 form";
				}
			};

			namespace Assistant
			{
				struct win32_style
				{
					DWORD window_style = WS_VISIBLE | WS_OVERLAPPEDWINDOW;
					DWORD ex_window_style = WS_EX_CLIENTEDGE;
				};
			}

			struct simple_event
			{
				HWND        hwnd;
				UINT        message;
				WPARAM      wParam;
				LPARAM      lParam;
			};

			struct win32_initializer
			{
				std::u16string title = u"PO default title :>";
				int shitf_x = (GetSystemMetrics(SM_CXSCREEN) - 1024) / 2;
				int shift_y = (GetSystemMetrics(SM_CYSCREEN) - 768) / 2;
				int width = 1024;
				int height = 768;
				Assistant::win32_style style = Assistant::win32_style();
			};
			

			class win32_form
			{
				HWND raw_handle = nullptr;
				std::atomic_bool avalible;
				std::mutex mut;
				bool delegate_event = false;
				std::deque<simple_event> input_message;
				std::function<bool(simple_event)> func;
			public:
				bool input_event(simple_event msg);
				void respond_event();
				bool bind_event_function(std::function<bool(simple_event)>&& f) { func = std::move(f); }
				bool bind_event_function(const std::function<bool(simple_event)>& f) { func = f; }
				bool window_close = false;
				HWND raw() const { return raw_handle; }
				operator bool() const 
				{
					return avalible; 
				}
				win32_form(
					const win32_initializer& = win32_initializer()
					);
				~win32_form();
				void close_window() { avalible = false; }
			};

		}
	}
}
