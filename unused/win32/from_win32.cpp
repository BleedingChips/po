#include "from_win32.h"
#include <future>
#include <functional>
#include <map>
#include <unordered_map>
#include <iostream>

#include <Windows.h>
#include <assert.h>
#include <iostream>
namespace
{

	const std::unordered_map<WPARAM, PO::KeyValue> virtual_key_map =
	{
	{ VK_SPACE, PO::KeyValue::K_SPACE },
	{ VK_F1, PO::KeyValue::K_F1 },
	{ VK_F2, PO::KeyValue::K_F2 },
	{ VK_F3, PO::KeyValue::K_F3 },
	{ VK_F4, PO::KeyValue::K_F4 },
	{ VK_F5, PO::KeyValue::K_F5 },
	{ VK_F6, PO::KeyValue::K_F6 },
	{ VK_F7, PO::KeyValue::K_F7 },
	{ VK_F8, PO::KeyValue::K_F8 },
	{ VK_F9, PO::KeyValue::K_F9 },
	{ VK_F10, PO::KeyValue::K_F10 },
	{ VK_F11, PO::KeyValue::K_F11 },
	{ VK_F12, PO::KeyValue::K_F12 },
	{ VK_BACK, PO::KeyValue::K_BACKSPACE },
	{ VK_RETURN, PO::KeyValue::K_ENTER },
	{ VK_TAB, PO::KeyValue::K_TAP },
	{ VK_LEFT, PO::KeyValue::K_ARROW_LEFT },
	{ VK_UP, PO::KeyValue::K_ARROW_UP },
	{ VK_DOWN, PO::KeyValue::K_ARROW_DOWN },
	{ VK_RIGHT, PO::KeyValue::K_ARROW_RIGHT },
	{ VK_OEM_1, PO::KeyValue::K_SEMICOLONS },
	{ VK_OEM_COMMA, PO::KeyValue::K_COMMA },
	{ VK_OEM_MINUS, PO::KeyValue::K_MINUS },
	{ VK_OEM_PERIOD, PO::KeyValue::K_PERIOD },
	{ VK_OEM_2, PO::KeyValue::K_SLASH },
	{ VK_OEM_3, PO::KeyValue::K_MINUTE },
	{ VK_OEM_4, PO::KeyValue::K_L_BRACKET },
	{ VK_OEM_5, PO::KeyValue::K_BACKSLASH },
	{ VK_OEM_6, PO::KeyValue::K_R_BRACKET },
	{ VK_OEM_7, PO::KeyValue::K_QUOTE },
#define VK_RO_KV(x) {0x30 + x, PO::KeyValue::K_##x },
	VK_RO_KV(0) VK_RO_KV(1) VK_RO_KV(2)
	VK_RO_KV(3) VK_RO_KV(4) VK_RO_KV(5)
		VK_RO_KV(6) VK_RO_KV(7) VK_RO_KV(8)
		VK_RO_KV(9)
#undef VK_RO_KV
#define VK_RO_KV(x) {0x41 + ( #x[0] - 'A' ), PO::KeyValue::K_##x },
		VK_RO_KV(A) VK_RO_KV(B) VK_RO_KV(C) VK_RO_KV(D) VK_RO_KV(E) VK_RO_KV(F) VK_RO_KV(G)
		VK_RO_KV(H) VK_RO_KV(I) VK_RO_KV(J) VK_RO_KV(K) VK_RO_KV(L) VK_RO_KV(M) VK_RO_KV(N)
		VK_RO_KV(O) VK_RO_KV(P) VK_RO_KV(Q) VK_RO_KV(R) VK_RO_KV(S) VK_RO_KV(T) VK_RO_KV(U)
		VK_RO_KV(V) VK_RO_KV(W) VK_RO_KV(X) VK_RO_KV(Y) VK_RO_KV(Z)
#undef VK_RO_KV
	};

	PO::KeyValue translate_key_para_to_key_value(WPARAM w, LPARAM l) noexcept
	{
		auto ptr = virtual_key_map.find(w);
		if (ptr != virtual_key_map.end())
			return ptr->second;
		else {
			switch (w)
			{
			case VK_SHIFT:
			{
				UINT  ocm = ((l & 0xff0000) >> (4 * 4));
				UINT  vir_code = MapVirtualKey(ocm, MAPVK_VSC_TO_VK_EX);
				switch (vir_code)
				{
				case VK_LSHIFT:
					return PO::KeyValue::K_L_SHIFT;
				case VK_RSHIFT:
					return PO::KeyValue::K_R_SHIFT;
				default:
					return PO::KeyValue::K_UNKNOW;
				}
			}
			case VK_CONTROL:
				return ((l & 0x1000000) != 0x1000000) ? PO::KeyValue::K_L_CON : PO::KeyValue::K_R_CON;
			case VK_MENU:
				return ((l & 0x1000000) != 0x1000000) ? PO::KeyValue::K_L_ALT : PO::KeyValue::K_R_ALT;
			}
		}
		return PO::KeyValue::K_UNKNOW;
	}

	bool key_state_upper() noexcept
	{
		return (GetKeyState(VK_SHIFT) >= 0) ? ((GetKeyState(VK_CAPITAL) & 0x1) == 0x1) : !((GetKeyState(VK_CAPITAL) & 0x1) == 0x1);
	}

	std::optional<PO::event> translate_message_to_event(const MSG& msg) noexcept
	{
		auto lParam = msg.lParam; auto wParam = msg.wParam;
		switch (msg.message)
		{
		case WM_SYSCOMMAND:
			if (wParam == SC_CLOSE)
				return PO::event {PO::EventType::E_CLOSE};
			return {};
		case WM_MOUSEMOVE:
			return PO::event {PO::move_type{ PO::EventType::E_MOVE, static_cast<int16_t>(LOWORD(lParam)), static_cast<int16_t>(HIWORD(lParam)) }};
		case WM_LBUTTONUP:
			return PO::event {PO::click_type{ PO::EventType::E_CLICK, PO::ButtonState::BS_UP,  static_cast<PO::KeyState>((wParam & (MK_CONTROL | MK_SHIFT)) >> 2), PO::KeyValue::K_LBUTTON, static_cast<int16_t>(LOWORD(lParam)), static_cast<int16_t>(HIWORD(lParam)) }};
		case WM_MBUTTONUP:
			return PO::event {PO::click_type{ PO::EventType::E_CLICK, PO::ButtonState::BS_UP,  static_cast<PO::KeyState>((wParam & (MK_CONTROL | MK_SHIFT)) >> 2), PO::KeyValue::K_MBUTTON, static_cast<int16_t>(LOWORD(lParam)), static_cast<int16_t>(HIWORD(lParam)) }};
		case WM_RBUTTONUP:
			return PO::event {PO::click_type{ PO::EventType::E_CLICK, PO::ButtonState::BS_UP,  static_cast<PO::KeyState>((wParam & (MK_CONTROL | MK_SHIFT)) >> 2), PO::KeyValue::K_RBUTTON, static_cast<int16_t>(LOWORD(lParam)), static_cast<int16_t>(HIWORD(lParam)) }};
		case WM_LBUTTONDOWN:
			return PO::event {PO::click_type{ PO::EventType::E_CLICK, PO::ButtonState::BS_DOWN,  static_cast<PO::KeyState>((wParam & (MK_CONTROL | MK_SHIFT)) >> 2), PO::KeyValue::K_LBUTTON, static_cast<int16_t>(LOWORD(lParam)), static_cast<int16_t>(HIWORD(lParam)) }};
		case WM_MBUTTONDOWN:
			return PO::event {PO::click_type{ PO::EventType::E_CLICK, PO::ButtonState::BS_DOWN,  static_cast<PO::KeyState>((wParam & (MK_CONTROL | MK_SHIFT)) >> 2), PO::KeyValue::K_MBUTTON, static_cast<int16_t>(LOWORD(lParam)), static_cast<int16_t>(HIWORD(lParam)) }};
		case WM_RBUTTONDOWN:
			return PO::event {PO::click_type{ PO::EventType::E_CLICK, PO::ButtonState::BS_DOWN,  static_cast<PO::KeyState>((wParam & (MK_CONTROL | MK_SHIFT)) >> 2), PO::KeyValue::K_RBUTTON, static_cast<int16_t>(LOWORD(lParam)), static_cast<int16_t>(HIWORD(lParam)) }};
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		{
			auto kv = translate_key_para_to_key_value(wParam, lParam);
			return PO::event {PO::key_type{ PO::EventType::E_KEY, PO::ButtonState::BS_DOWN,  kv, translate_key_value_to_asc_char_pair(kv, key_state_upper()) }};
		}
		case WM_SYSKEYUP:
		case WM_KEYUP:
		{
			auto kv = translate_key_para_to_key_value(wParam, lParam);
			return PO::event {PO::key_type{ PO::EventType::E_KEY, PO::ButtonState::BS_UP,  kv, translate_key_value_to_asc_char_pair(kv, key_state_upper()) }};
		}
		default:
			return {};
		}
	}

	bool translate_message_to_event(const MSG& msg, PO::event& e) noexcept
	{
		auto ev = translate_message_to_event(msg);
		if (ev.has_value())
		{
			e = ev.value();
			return true;
		}
		else
			return false;
	}

	const char16_t static_class_name[] = u"po_frame_default_win32_class";

	LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_DESTROY:
		{
			std::cout << "receive destory" << std::endl;
			LONG_PTR data = GetWindowLongPtr(hWnd, GWLP_USERDATA);
			PO::Win32::form::control* ptr = reinterpret_cast<PO::Win32::form::control*>(data);
			if(ptr != nullptr)
				ptr->sub_ref();
			SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
			PostMessage(hWnd, WM_QUIT, 0, 0);
			return DefWindowProcW(hWnd, msg, wParam, lParam);
		}
		default:
		{
			MSG message{ hWnd, msg, wParam, lParam };
			PO::event ev;
			if (::translate_message_to_event(message, ev))
			{
				if (ev.is_quit())
					std::cout << "send quit" << std::endl;
				LONG_PTR data = GetWindowLongPtr(hWnd, GWLP_USERDATA);
				PO::Win32::form::control* ptr = reinterpret_cast<PO::Win32::form::control*>(data);
				if (ptr != nullptr)
				{
				
					std::lock_guard<decltype(ptr->m_mutex)> lg(ptr->m_mutex);
					ptr->m_pool.push_back(ev);
					return 0;
				}
			}
			return DefWindowProcW(hWnd, msg, wParam, lParam);
		}
		}
	}

	const WNDCLASSEXW static_class = { sizeof(WNDCLASSEXW), CS_HREDRAW | CS_VREDRAW , WndProc, 0, 0, GetModuleHandle(0), NULL,NULL, 0, NULL, (const wchar_t*)static_class_name, NULL };

	const struct static_class_init_struct
	{
		static_class_init_struct()
		{
			HRESULT res = RegisterClassExW(&static_class);
			assert(SUCCEEDED(res));
		}

		~static_class_init_struct()
		{
			UnregisterClassW((const wchar_t*)static_class_name, GetModuleHandleW(0));
		}
	}init;

}

namespace PO::Win32
{
	namespace Error
	{
		const char* create_window_fauit::what() const noexcept
		{
			return "unable to create window";
		}
	}


	int event_loop(const form_property& setting, form::control* ptr, std::promise<HWND> pro)
	{
		try {
			HWND handle = CreateWindowExW(
				setting.ex_window_style,
				(wchar_t*)(static_class_name),
				(wchar_t*)(setting.title.c_str()),
				setting.window_style,
				setting.shift.x, setting.shift.y, setting.size.x, setting.size.y,
				NULL,
				NULL,
				GetModuleHandle(0),
				NULL
			);
			if (handle != nullptr)
			{
				ptr->add_ref();
				SetWindowLongPtr(handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(ptr));
				pro.set_value(handle);
			}
			else throw Error::create_window_fauit{ GetLastError() };
		}
		catch (...)
		{
			pro.set_exception(std::current_exception());
			return -1;
		}
		MSG msg;
		bool m_avalible = true;
		while (m_avalible)
		{
			while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
			{
				if (msg.message != WM_QUIT)
				{
					TranslateMessage(&msg);
					DispatchMessageW(&msg);
				}
				else
					m_avalible = false;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds{ 1 });
		}
		return 0;
	}

	void form::create(Graphic::graphic_context& context, const form_property& setting)
	{
		if (m_handle != nullptr)
			close();
		Tool::intrusive_ptr<control> con_ptr = new control{};
		std::promise<HWND> con_promise;
		auto fur = con_promise.get_future();
		m_event_loop = std::thread(event_loop, setting, con_ptr, std::move(con_promise));
		fur.wait();
		m_handle = fur.get();
		m_control = std::move(con_ptr);
		form_context = context.create_form_context(m_handle, setting.size, setting.format);
		m_handle_quit_event = setting.handle_close_event;
	}

	void form::close() noexcept
	{
		if (m_handle != nullptr)
		{
			std::cout << "send DESTORY" << std::endl;
			m_events.clear();
			m_control.reset();
			form_context.reset();
			PostMessage(m_handle, WM_DESTROY, 0, 0);
			m_handle = nullptr;
			std::cout << "wait thread exits" << std::endl;
			m_event_loop.join();
			std::cout << "thread exist" << std::endl;
		}
	}

	form::~form()
	{
		close();
	}

	bool form::pop_event(event& e)
	{
		do
		{
			if (!m_events.empty())
			{
				e = *m_events.begin();
				if (e.is_quit() && m_handle_quit_event)
				{
					close();
					return false;
				}
				else {
					m_events.pop_front();
					return true;
				}
			}
			else {
				std::lock_guard<std::mutex> lg(m_control->m_mutex);
				if (m_control->m_pool.empty())
					return false;
				else {
					std::swap(m_control->m_pool, m_events);
				}
			}
		} while (true);
	}

	/*
	form::form(form&& f) noexcept
		: m_handle(f.m_handle), m_events(std::move(f.m_events)), m_avalible(f.m_avalible), 
		m_event_loop(std::move(f.m_event_loop)), m_control(std::move(f.m_control)), m_handle_quit_event(f.m_handle_quit_event)
	{
		f.m_handle = nullptr;
	}
	*/

	/*
	form& form::operator=(form&& f) noexcept
	{
		form tem(std::move(f));
		m_handle = tem.m_handle;
		m_events = std::move(tem.m_events);
		m_avalible = tem.m_avalible;
		m_event_loop = std::move(tem.m_event_loop);
		m_control = std::move(tem.m_control);
		tem.m_handle = nullptr;
		return *this;
	}
	*/
}