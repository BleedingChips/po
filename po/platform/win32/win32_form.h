#pragma once
#include <string>
#include <mutex>
#include <atomic>
#include <future>
#include <thread>
#include <deque>
#include "../../tool/thread_tool.h"
#include "win32_define.h"
#include "../../frame/define.h"
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
				Tool::mail_ts<bool(event)> event_mail;
			public:
				template<typename T>
				decltype(auto) bind_event(const Tool::completeness_ref& cr, T&& t)
				{
					return event_mail.bind(cr, std::forward<T>(t));
				}
				bool respond_event(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
				bool window_close = false;
				HWND raw() const { return raw_handle; }
				bool is_available() const { return avalible; }
				win32_form( const win32_initializer& = win32_initializer() );
				~win32_form();
				void close_window() { avalible = false; }
			};

			class win32_interface
			{
				HWND raw_handle = nullptr;
			public:
				win32_interface(win32_form& wf) : raw_handle(wf.raw()) {}
			};

			class win32_viewer
			{
				HWND raw_handle = nullptr;
			public:
				win32_viewer(win32_form& wf) : raw_handle(wf.raw()) {}
			};

		}
	}
}
