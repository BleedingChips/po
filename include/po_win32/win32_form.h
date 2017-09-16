#pragma once
#include "win32_define.h"
#include "../po/form.h"
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

		struct win32_renderer_initializer
		{
			HWND handle;
		};

		class win32_form
		{
			using tank = std::vector<event>;
			std::atomic_bool quit;
			Tool::scope_lock<tank> capture_event_tank;
			tank event_tank;
		protected:
			HWND raw_handle;
			virtual Respond form_implement_respond_event(event& e) = 0;
		public:
			operator win32_renderer_initializer() const { return win32_renderer_initializer{ raw_handle }; }
			win32_form(const win32_initial& = win32_initial{});
			~win32_form();
			void WndProcInputEvent(const event& e);
			bool available() const { return !quit; }
			const tank& generate_event_tank();
			Respond pre_respond(const event& ev) { return Respond::Pass; }
			Respond pos_respond(const event& ev) { return Respond::Pass; }
		};
	}
}