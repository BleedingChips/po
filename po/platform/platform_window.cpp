#include "platform_window.h"
#include "platform_event.h"
#include <iostream>
#include <atomic>
#include <sstream>
#include <unordered_map>
#include "../tool/mail.h"
#include "platform_event.h"
namespace {
	std::mutex translate_char_buffer_mutex;
	std::vector<char> translate_char_buffer;
}

std::ostream& operator<<(std::ostream& os, const char16_t* st) noexcept
{
	translate_char_buffer_mutex.lock();
	size_t string_size = std::char_traits<char16_t>::length(st);
	if (string_size * 2 + 1 > translate_char_buffer.size())
		translate_char_buffer.resize(string_size * 2 + 1);
	WideCharToMultiByte(CP_ACP, 0, (wchar_t*)st, -1, &translate_char_buffer[0], translate_char_buffer.size(), NULL, NULL);
	os << &translate_char_buffer[0];
	translate_char_buffer_mutex.unlock();
	return os;
}

std::ostream& operator<<(std::ostream& os, const std::u16string& st) noexcept
{
	translate_char_buffer_mutex.lock();
	size_t string_size = st.size();
	if (string_size * 2 + 1 > translate_char_buffer.size())
		translate_char_buffer.resize(string_size * 2 + 1);
	WideCharToMultiByte(CP_ACP, 0, (wchar_t*)st.c_str(), -1, &translate_char_buffer[0], translate_char_buffer.size(), NULL, NULL);
	os << &translate_char_buffer[0];
	translate_char_buffer_mutex.unlock();
	return os;
}


namespace
{
	std::string translate_error_code_to_string(DWORD code) noexcept
	{
		static std::mutex error_code_mutex;
		static char max_string_buffer[256] = "";
		error_code_mutex.lock();
		DWORD resault = FormatMessageA(
			FORMAT_MESSAGE_FROM_SYSTEM,
			nullptr,
			code,
			0, // language identifier
			max_string_buffer,
			255,
			nullptr
		);
		error_code_mutex.unlock();
		if (resault == 0)
		{
			error_code_mutex.lock();
			static std::stringstream ss;
			ss.clear();
			ss << code;
			std::string Tem;
			ss >> Tem;
			error_code_mutex.unlock();
			return "unknow error code - " + Tem;
		}
		return std::string(max_string_buffer);
	}
}


namespace
{

	const char32_t static_class_name[] = U"po_frame_window_class";

	LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) 
	{
		PO::Platform::window_instance* ptr = reinterpret_cast<PO::Platform::window_instance*> (GetWindowLongW(hWnd, GWL_USERDATA));
		if (ptr != nullptr)
		{
			return ptr->main_event_respond(hWnd,msg,wParam,lParam);
		}else
			return DefWindowProcW(hWnd, msg, wParam, lParam);
	}

	class
	{
		std::mutex init_mutex;
		size_t init_count = 0;
		WNDCLASSEX static_class = { sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW , WndProc, 0, 0, GetModuleHandle(0), NULL,NULL, 0, NULL, (const wchar_t*)static_class_name, NULL };
	public:
		void init()
		{
			init_mutex.lock();
			if (init_count == 0)
			{
				HRESULT res = RegisterClassExW(&static_class);
				if (!SUCCEEDED(res))
				{
					init_mutex.unlock();
					throw PO::Platform::Error::win32_init_error("window_instance::window_instance ");
				}
			}
			++init_count;
			init_mutex.unlock();
		}
		void destory()
		{
			init_mutex.lock();
			if (init_count == 0)
			{
				init_mutex.unlock();
				throw std::overflow_error("window_class_initer::destory meet init_count of zero");
			}
			if (init_count == 1)
				UnregisterClassW((const wchar_t*)static_class_name, GetModuleHandleW(0));
			--init_count;
			init_mutex.unlock();
		}
		const wchar_t* const get_class_name() const { return (const wchar_t* const)static_class_name; }
	}ws_init;

}

namespace PO
{
	namespace Platform
	{

		window_style::window_style():	title(u"po_default_title"),
										window_shift_x((GetSystemMetrics(SM_CXSCREEN) - 1024) / 2),
										window_shift_y((GetSystemMetrics(SM_CYSCREEN) - 768) / 2),
										window_width(1024),
										window_height(768),
										ex_style(0),
										style(WS_OVERLAPPEDWINDOW | WS_VISIBLE)
		{}

		namespace Error
		{
			win32_init_error::win32_init_error(const std::string& scr) noexcept : scription(scr + translate_error_code_to_string(GetLastError())) {}
		}

		window_instance::window_instance() :window_instance(window_style()) {}
		window_instance::window_instance(const window_style& ws) 
		{
			ws_init.init();

			RECT win_rect = { ws.window_shift_x,ws.window_shift_y,ws.window_shift_x + ws.window_width, ws.window_shift_y + ws.window_height };

			AdjustWindowRectEx(&win_rect,ws.style,false,ws.ex_style);

			RECT avalibleWindow;
			SystemParametersInfo(SPI_GETWORKAREA, 0, (PVOID)&avalibleWindow, 0);

			if (win_rect.top < avalibleWindow.top)
			{
				win_rect.bottom -= (win_rect.top - avalibleWindow.top);
				win_rect.top = avalibleWindow.top;
			}

			win = CreateWindowExW(
				ws.ex_style,
				ws_init.get_class_name(),
				(const wchar_t*)ws.title.c_str(),
				ws.style,
				win_rect.left,
				win_rect.top,
				win_rect.right - win_rect.left,
				win_rect.bottom - win_rect.top,
				nullptr,nullptr,
				GetModuleHandleW(nullptr),
				nullptr
			);

			if (win == nullptr)
			{
				ws_init.destory();
				throw Error::win32_init_error("window_instance::window_instance ");
			}

			SetWindowLongW(win, GWL_USERDATA, reinterpret_cast<LONG>(this));
		}

		window_instance::~window_instance()
		{
			if (win != nullptr)
			{
				DestroyWindow(win);
				win = nullptr;
			}
			ws_init.destory();
		}

		LRESULT window_instance::main_event_respond(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
		{
			window_event tem_event{ hWnd, msg, wParam, lParam };

			bool finish = false;
			if (!finish && respond_event(tem_event))
			{
				switch (msg)
				{
				case WM_CLOSE:
					window_exist = false;
					break;
				}
				return DefWindowProcW(hWnd, msg, wParam, lParam);
			}else
				return DefWindowProcW(hWnd, msg, wParam, lParam);
		}

		void window_instance::one_frame_loop()
		{
			MSG msg;
			while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
			{
				DispatchMessageW(&msg);
			}
		}
	}
}







/*
namespace Platform 
{
	//const std::string window_data::class_name = "TEST";
	bool running = true;

	

	bool app_running = true;

	bool is_app_still_running() { return app_running; }
	void close_app() { app_running = false; }

	LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		//std::cout << "Get Event:" << Event::translate_event_to_string(msg) << std::endl;
		switch (msg)
		{
		case WM_CREATE:
			//std::wcout << (wchar_t*)u"你莫斯科放寒假快乐圣诞节反抗拉萨大家风口浪尖圣诞快乐福建阿斯科利~~" << std::endl;
			break;
		case WM_CLOSE:
			app_running = false;
			//return false;
			break;
		case WM_PAINT:
			//std::wcout << (wchar_t*)u"你莫斯科放寒假快乐圣诞节反抗拉萨大家风口浪尖圣诞快乐福建阿斯科利~~" << std::endl;
			break;
			//return true;
		case WM_CHAR:
			//app_running = false;
			break;
		}
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	window::window(app_context& appc, const char16_t* str, area ar, style ws)
	{
		RECT client = { ar.left,ar.top,ar.left + ar.width, ar.top + ar.height };
		AdjustWindowRectEx(&client, ws.ex_style, false, ws.nor_style);
		win = CreateWindowEx(
			ws.ex_style,
			(LPCWSTR)appc.index,
			(const wchar_t*)str,
			ws.nor_style,
			client.left, client.top,
			client.right - client.left, client.bottom - client.top,
			NULL, NULL,
			GetModuleHandle(0),
			NULL);
		if (win != NULL)
		{
			BringWindowToTop(win);
			ShowWindow(win, SW_SHOW);
			UpdateWindow(win);
		}
	}

	window::window(app_context& appc, const char16_t* str, area ar, style ws, PO::Tool::auto_adapter<bool(area&, style&)> fun)
	{
		RECT client = { ar.left,ar.top,ar.left + ar.width, ar.top + ar.height };
		AdjustWindowRectEx(&client, ws.ex_style, false, ws.nor_style);
		ar = { client.left,client.top,client.right - client.left,client.bottom - client.top };
		if (fun(ar, ws))
		{
			client = { ar.left,ar.top,ar.left + ar.width, ar.top + ar.height };
			AdjustWindowRectEx(&client, ws.ex_style, false, ws.nor_style);
		}
		win = CreateWindowEx(
			ws.ex_style,
			(LPCWSTR)appc.index,
			(const wchar_t*)str,
			ws.nor_style,
			client.left, client.top,
			client.right - client.left, client.bottom - client.top,
			NULL, NULL,
			GetModuleHandle(0),
			NULL);
		if (win != NULL)
		{
			BringWindowToTop(win);
			ShowWindow(win, SW_SHOW);
			UpdateWindow(win);
		}
	}

	const style style::default_{0, WS_OVERLAPPEDWINDOW };
	const style style::full_window{0, WS_POPUP & ~WS_SYSMENU };

}




*/