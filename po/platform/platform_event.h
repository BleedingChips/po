#pragma once
#include <Windows.h>
#include <string>
namespace PO
{
	namespace Platform
	{
		std::u16string translate_event_to_u16string(UINT type) noexcept;
		std::string translate_event_to_string(UINT type) noexcept;

		struct window_event
		{
			HWND hWnd;
			UINT msg;
			WPARAM wParam;
			LPARAM lParam;
		};

		namespace Event_Key
		{
			inline bool is_key_up(const window_event& we) { return we.msg == WM_KEYUP; }
			inline bool is_key_down(const window_event& we) { return we.msg == WM_KEYUP; }
		}
		
	}
}
