#include "win32_implement.h"
#include <future>
#include <functional>
#include <map>
#include <unordered_map>
#include <iostream>
namespace
{
	const char16_t static_class_name[] = u"po_frame_window_class2";

	std::unordered_map<WPARAM, PO::KeyValue> virtual_key_map = 
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
		{ VK_RETURN, PO::KeyValue::K_ENTER},
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

	PO::KeyValue translate_key_para_to_key_value(WPARAM w , LPARAM l)
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

	auto default_hande_event = [](WPARAM wParam, LPARAM lParam) { return 0; };

	struct event_handle_struct
	{
		std::function<PO::event(WPARAM wParam, LPARAM lParam)> translate_event;
		std::function<HRESULT(WPARAM wParam, LPARAM lParam)> responded_event;
		event_handle_struct(std::function<PO::event(WPARAM wParam, LPARAM lParam)>&& t) : translate_event(std::move(t)), responded_event(default_hande_event){}
		event_handle_struct(std::function<PO::event(WPARAM wParam, LPARAM lParam)>&& t, std::function<HRESULT(WPARAM wParam, LPARAM lParam)>&& p) : translate_event(std::move(t)), responded_event(std::move(p)) {}
		event_handle_struct(const event_handle_struct&) = default;
		event_handle_struct(event_handle_struct&&) = default;
	};

	bool key_state_upper()
	{
		return (GetKeyState(VK_SHIFT) >= 0) ? ((GetKeyState(VK_CAPITAL) & 0x1) == 0x1) : !((GetKeyState(VK_CAPITAL) & 0x1) == 0x1);
	}

	const std::unordered_map<UINT, event_handle_struct > handled_event_filter =
	{
		{ WM_CLOSE, 
		event_handle_struct{ [](WPARAM wParam, LPARAM lParam) {
		return PO::event{PO::EventType::E_CLOSE};
	}}
		},
		{ WM_MOUSEMOVE, 
		event_handle_struct{ [](WPARAM wParam, LPARAM lParam) {
		return PO::event{PO::move_type {PO::EventType::E_MOVE, static_cast<int16_t>(LOWORD(lParam)), static_cast<int16_t>(HIWORD(lParam))}};
	}}
		},
		{ WM_LBUTTONUP, 
		event_handle_struct{ [](WPARAM wParam, LPARAM lParam) {
		return PO::event{PO::click_type {PO::EventType::E_CLICK, PO::ButtonState::BS_UP,  static_cast<PO::KeyState>((wParam & (MK_CONTROL | MK_SHIFT)) >> 2), PO::KeyValue::K_LBUTTON, static_cast<int16_t>(LOWORD(lParam)), static_cast<int16_t>(HIWORD(lParam))}};
	}}
		},
		{ WM_MBUTTONUP, 
		event_handle_struct{ [](WPARAM wParam, LPARAM lParam) {
		return PO::event{PO::click_type { PO::EventType::E_CLICK, PO::ButtonState::BS_UP,  static_cast<PO::KeyState>((wParam & (MK_CONTROL | MK_SHIFT)) >> 2), PO::KeyValue::K_MBUTTON, static_cast<int16_t>(LOWORD(lParam)), static_cast<int16_t>(HIWORD(lParam)) }};
	}} 
		},
		{ WM_RBUTTONUP, 
		event_handle_struct{ [](WPARAM wParam, LPARAM lParam) { 
		return PO::event{PO::click_type{ PO::EventType::E_CLICK, PO::ButtonState::BS_UP,  static_cast<PO::KeyState>((wParam & (MK_CONTROL | MK_SHIFT)) >> 2), PO::KeyValue::K_RBUTTON, static_cast<int16_t>(LOWORD(lParam)), static_cast<int16_t>(HIWORD(lParam)) }};
	}} 
		},
		{ WM_LBUTTONDOWN, 
		event_handle_struct{ [](WPARAM wParam, LPARAM lParam) { 
		return PO::event{PO::click_type{ PO::EventType::E_CLICK, PO::ButtonState::BS_DOWN,  static_cast<PO::KeyState>((wParam & (MK_CONTROL | MK_SHIFT)) >> 2), PO::KeyValue::K_LBUTTON, static_cast<int16_t>(LOWORD(lParam)), static_cast<int16_t>(HIWORD(lParam)) }};
	}}
		},
		{ WM_MBUTTONDOWN, 
		event_handle_struct{ [](WPARAM wParam, LPARAM lParam) { 
		return PO::event{PO::click_type{ PO::EventType::E_CLICK, PO::ButtonState::BS_DOWN,  static_cast<PO::KeyState>((wParam & (MK_CONTROL | MK_SHIFT)) >> 2), PO::KeyValue::K_MBUTTON, static_cast<int16_t>(LOWORD(lParam)), static_cast<int16_t>(HIWORD(lParam)) }};
	}}
		},
		{ WM_RBUTTONDOWN, 
		event_handle_struct{ [](WPARAM wParam, LPARAM lParam) {
		return PO::event{PO::click_type { PO::EventType::E_CLICK, PO::ButtonState::BS_DOWN,  static_cast<PO::KeyState>((wParam & (MK_CONTROL | MK_SHIFT)) >> 2), PO::KeyValue::K_RBUTTON, static_cast<int16_t>(LOWORD(lParam)), static_cast<int16_t>(HIWORD(lParam)) }};
	}}
		},
		{ WM_KEYDOWN, 
		event_handle_struct{ [](WPARAM wParam, LPARAM lParam) { 
		auto kv = translate_key_para_to_key_value(wParam, lParam);
		return PO::event{PO::key_type{ PO::EventType::E_KEY, PO::ButtonState::BS_DOWN,  kv, translate_key_value_to_asc_char_pair(kv, key_state_upper()) }};
	}}
		},
		{ WM_KEYUP, 
		event_handle_struct{ [](WPARAM wParam, LPARAM lParam) { 
		auto kv = translate_key_para_to_key_value(wParam, lParam);
		return PO::event{PO::key_type{ PO::EventType::E_KEY, PO::ButtonState::BS_UP,  kv, translate_key_value_to_asc_char_pair(kv, key_state_upper()) }};
	}}
		},
		{ WM_SYSKEYDOWN,
		event_handle_struct{ [](WPARAM wParam, LPARAM lParam) {
		auto kv = translate_key_para_to_key_value(wParam, lParam);
		return PO::event{PO::key_type{ PO::EventType::E_KEY, PO::ButtonState::BS_DOWN,  kv, translate_key_value_to_asc_char_pair(kv, key_state_upper()) }};
	} }
		},
		{ WM_SYSKEYUP,
		event_handle_struct{ [](WPARAM wParam, LPARAM lParam) {
		auto kv = translate_key_para_to_key_value(wParam, lParam);
		return PO::event{PO::key_type{ PO::EventType::E_KEY, PO::ButtonState::BS_UP,  kv, translate_key_value_to_asc_char_pair(kv, key_state_upper()) }};
	} }
		}
	};

	bool translate_event_to_massage(PO::event& ev, UINT& msg, WPARAM& wParam, LPARAM& lParam)
	{
		return false;
	}

	LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		if (msg >= WM_USER)
		{
			return DefWindowProcW(hWnd, msg - WM_USER, wParam, lParam);
		}
		PO::Interface::win32_form_implement* ptr = reinterpret_cast<PO::Interface::win32_form_implement*> (GetWindowLongW(hWnd, GWL_USERDATA));
		if (ptr != nullptr)
		{
			auto ite = handled_event_filter.find(msg);
			if (ite != handled_event_filter.end() && ite->second.translate_event && ite->second.responded_event)
			{
				std::lock_guard<decltype(ptr->input_mutex)> lg(ptr->input_mutex);
				ptr->input_event.push_back(ite->second.translate_event(wParam, lParam));
				return ite->second.responded_event(wParam, lParam);
			}
		}
		return DefWindowProcW(hWnd, msg, wParam, lParam);
	}

	const WNDCLASSEXW static_class = { sizeof(WNDCLASSEXW), CS_HREDRAW | CS_VREDRAW , WndProc, 0, 0, GetModuleHandle(0), NULL,NULL, 0, NULL, (const wchar_t*)static_class_name, NULL };

	const struct static_class_init_struct
	{
		static_class_init_struct()
		{
			HRESULT res = RegisterClassExW(&static_class);
			if (!SUCCEEDED(res))
			{
				__debugbreak();
			}
		}

		~static_class_init_struct()
		{
			UnregisterClassW((const wchar_t*)static_class_name, GetModuleHandleW(0));
		}

	};

	HRESULT create_window(
		const PO::Interface::win32_initial_implement& wi,
		PO::Interface::win32_form_implement* ptr
	)
	{
		static static_class_init_struct scis;
		ptr->raw_handle = CreateWindowExW(
			wi.style.ex_window_style,
			(wchar_t*)(static_class_name),
			(wchar_t*)(wi.title.c_str()),
			wi.style.window_style,
			wi.shitf_x, wi.shift_y, wi.width, wi.height,
			NULL,
			NULL,
			GetModuleHandle(0),
			NULL
		);
		if (ptr->raw_handle == nullptr)
		{
			HRESULT ret = GetLastError();
			return ret;
		}
		SetWindowLongW(ptr->raw_handle, GWL_USERDATA, reinterpret_cast<LONG>(ptr));
		return S_OK;
	}

	void destory_window(HWND& handle)
	{
		SetWindowLongW(handle, GWL_USERDATA, reinterpret_cast<LONG>(nullptr));
		DestroyWindow(handle);
		handle = nullptr;
	}

	class form_event_thread_manager
	{
		std::mutex thread_mutex;
		std::thread event_thread;
		std::mutex ref_mutex;
		size_t ref = 0;
		std::function<void(void)> delegate_function;
		std::condition_variable cv;

	public:

		void main_thread()
		{
			while (true)
			{
				std::unique_lock<decltype(ref_mutex)> ul(ref_mutex);
				if (ref == 0) break;
				while (delegate_function)
				{
					delegate_function();
					delegate_function = std::function<void()>{};
					ul.unlock();
					cv.notify_one();
					std::this_thread::yield();
					ul.lock();
				}
				ul.unlock();
				MSG msg;
				while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
				{
					DispatchMessageW(&msg);
				}
				std::this_thread::yield();
			}
		}

		HRESULT create(
			const PO::Interface::win32_initial_implement& wi,
			PO::Interface::win32_form_implement* ptr)
		{
			std::unique_lock<decltype(ref_mutex)> ul(ref_mutex);
			++ref;
			if (ref == 1)
			{
				std::lock_guard<decltype(thread_mutex)> ld(thread_mutex);
				if (event_thread.joinable())
					event_thread.join();
				event_thread = std::thread([this]() {this->main_thread(); });
			}
			
			cv.wait(ul, [this]() {return !static_cast<bool>(delegate_function); });
			std::promise<HRESULT> pro;
			auto fur = pro.get_future();
			delegate_function = [&]()
			{
				pro.set_value(create_window(wi, ptr));
			};
			ul.unlock();
			fur.wait();
			return fur.get();
		}

		void destory(HWND& handle)
		{
			std::unique_lock<decltype(ref_mutex)> ul(ref_mutex);
			if (std::this_thread::get_id() == event_thread.get_id())
			{
				destory_window(handle);
				--ref;
				return;
			}
			else {
				cv.wait(ul, [this]() {return !static_cast<bool>(delegate_function); });
				std::promise<void> pro;
				auto fur = pro.get_future();
				delegate_function = [&]()
				{
					destory_window(handle);
					pro.set_value();
				};
				ul.unlock();
				fur.wait();
				ul.lock();
				--ref;
				if (ref == 0)
				{
					ul.unlock();
					std::lock_guard<decltype(thread_mutex)> ld(thread_mutex);
					if (event_thread.joinable())
						event_thread.join();
				}
			}
		}

		~form_event_thread_manager()
		{
			std::lock_guard<decltype(thread_mutex)> ld(thread_mutex);
			if (event_thread.joinable())
			{
				{
					std::lock_guard<decltype(ref_mutex)> ld(ref_mutex);
					ref = 0;
				}
				if (event_thread.joinable()) event_thread.join();
			}
		}

	} manager;
}

namespace PO
{

	namespace Interface
	{
		Tool::optional<std::string> init_win32_initial_implement(win32_initial_implement*& wfi) noexcept
		{
			try {
				wfi = new win32_initial_implement;
				return{};
			}
			catch (const std::exception& ec)
			{
				return{std::string(__FUNCTION__) + " : " + ec.what()};
			}
			catch (...)
			{
				return{ "unknow exception" };
			}
		}
		void dest_win32_initial_implement(win32_initial_implement*& wfi) noexcept
		{
			delete wfi;
			wfi = nullptr;
		}
		Tool::optional<std::string> init_win32_form_implement(win32_form_implement*& wfi, win32_initial_implement* wii) noexcept
		{
			try
			{
				wfi = new win32_form_implement;
				Error::fail_throw(manager.create(*wii, wfi), [&]() {delete wfi; });
				return{};
			}
			catch (std::exception& ec)
			{
				return{ std::string(__FUNCTION__) + " : " + ec.what() };
			}
			//return SUCCEEDED(manager.create(*wii, wfi));
		}
		void dest_win32_form_implement(win32_form_implement*& wfi) noexcept
		{
			manager.destory(wfi->raw_handle);
		}
		Tool::optional<event> pick_win32_form_event(win32_form_implement* wfi) noexcept
		{
			std::lock_guard<decltype(wfi->input_mutex)> lg(wfi->input_mutex);
			if (!wfi->input_event.empty())
			{
				event ev = *(wfi->input_event.begin());
				wfi->input_event.pop_front();
				return{ev};
			}
			for (auto& po : wfi->output_event)
			{
				UINT msg; WPARAM wParam; LPARAM lParam;
				if (translate_event_to_massage(po, msg, wParam, lParam))
				{
					SendMessageW(wfi->raw_handle, msg + WM_USER, wParam, lParam);
				}
			}
			wfi->output_event.clear();
			return{};
		}
		void throw_win32_form_event(win32_form_implement* wfi, const event& ev) noexcept
		{
			wfi->output_event.push_back(ev);
		}
	}
}