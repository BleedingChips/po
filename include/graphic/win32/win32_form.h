#pragma once
#include "win32_event.h"
#include <future>
#include <variant>
namespace PO
{
	namespace Win32
	{

		struct form_property
		{
			std::u16string title = u"PO default title :>";
			int shitf_x = (GetSystemMetrics(SM_CXSCREEN) - 1024) / 2;
			int shift_y = (GetSystemMetrics(SM_CYSCREEN) - 768) / 2;
			int width = 1024;
			int height = 768;
			DWORD window_style = WS_VISIBLE | WS_OVERLAPPEDWINDOW;
			DWORD ex_window_style = WS_EX_CLIENTEDGE;
		};

		class form
		{
			HWND handle;
		public:
			form(HWND h) : handle(h) {}
			form() : handle(nullptr){}
			HWND raw_handle() const noexcept { return handle; }
			void close_window() { PostMessage(handle, WM_CLOSE, 0, 0); handle = nullptr; }
			operator bool() const noexcept { return handle != nullptr; }
			bool operator == (form f) const noexcept { return handle == f.handle; }
		};

		extern const char16_t* default_form_style;
		form create_form(const form_property&, const char16_t* style = default_form_style, void* user_data = nullptr) noexcept;
		bool respond_event() noexcept;
	}
}