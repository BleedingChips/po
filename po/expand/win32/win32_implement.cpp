#include "win32_implement.h"
#include <future>
namespace
{
	const char16_t static_class_name[] = u"po_frame_window_class2";

	const std::set<UINT> handled_event_filter =
	{
		WM_CLOSE,
		WM_MOUSEMOVE,
		WM_LBUTTONUP,
		WM_MBUTTONUP,
		WM_RBUTTONUP,
		WM_LBUTTONDOWN,
		WM_MBUTTONDOWN,
		WM_RBUTTONDOWN
	};

	bool translate_event(PO::event& ev, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_CLOSE:
			ev.type = PO::EventType::E_CLOSE;
			return true;

		case WM_LBUTTONUP:
			ev.type = PO::EventType::E_CLICK_L;
			ev.click.button_state = PO::ButtonState::BS_UP;
			ev.click.location_x = LOWORD(lParam);
			ev.click.location_y = HIWORD(lParam);
			ev.click.key_state = static_cast<PO::KeyState>((wParam & (MK_CONTROL | MK_SHIFT)) >> 2);
			return true;

		case WM_LBUTTONDOWN:
			ev.type = PO::EventType::E_CLICK_L;
			ev.click.button_state = PO::ButtonState::BS_DOWN;
			ev.click.location_x = LOWORD(lParam);
			ev.click.location_y = HIWORD(lParam);
			ev.click.key_state = static_cast<PO::KeyState>((wParam & (MK_CONTROL | MK_SHIFT)) >> 2);
			return true;

		case WM_MBUTTONUP:
			ev.type = PO::EventType::E_CLICK_M;
			ev.click.button_state = PO::ButtonState::BS_UP;
			ev.click.location_x = LOWORD(lParam);
			ev.click.location_y = HIWORD(lParam);
			ev.click.key_state = static_cast<PO::KeyState>((wParam & (MK_CONTROL | MK_SHIFT)) >> 2);
			return true;

		case WM_MBUTTONDOWN:
			ev.type = PO::EventType::E_CLICK_M;
			ev.click.button_state = PO::ButtonState::BS_DOWN;
			ev.click.location_x = LOWORD(lParam);
			ev.click.location_y = HIWORD(lParam);
			ev.click.key_state = static_cast<PO::KeyState>((wParam & (MK_CONTROL | MK_SHIFT)) >> 2);


		case WM_RBUTTONUP:
			ev.type = PO::EventType::E_CLICK_R;
			ev.click.button_state = PO::ButtonState::BS_UP;
			ev.click.location_x = LOWORD(lParam);
			ev.click.location_y = HIWORD(lParam);
			ev.click.key_state = static_cast<PO::KeyState>((wParam & (MK_CONTROL | MK_SHIFT)) >> 2);
			return true;

		case WM_RBUTTONDOWN:
			ev.type = PO::EventType::E_CLICK_R;
			ev.click.button_state = PO::ButtonState::BS_DOWN;
			ev.click.location_x = LOWORD(lParam);
			ev.click.location_y = HIWORD(lParam);
			ev.click.key_state = static_cast<PO::KeyState>((wParam & (MK_CONTROL | MK_SHIFT)) >> 2);
			return true;

		case WM_MOUSEMOVE:
			ev.type = PO::EventType::E_MOVE;
			ev.move.location_x = LOWORD(lParam);
			ev.move.location_y = HIWORD(lParam);
			return true;

		}
		return false;
	}

	bool distranslate_event(PO::event& ev, UINT& msg, WPARAM& wParam, LPARAM& lParam)
	{
		return false;
	}

	

	HRESULT default_handled_event(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		// WM_CLOSE
		return 0;
	}

	LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		if (msg >= WM_USER)
		{
			return DefWindowProcW(hWnd, msg - WM_USER, wParam, lParam);
		}
		PO::Interface::win32_form_implement* ptr = reinterpret_cast<PO::Interface::win32_form_implement*> (GetWindowLongW(hWnd, GWL_USERDATA));
		PO::event ev;
		if (ptr != nullptr && handled_event_filter.find(msg) != handled_event_filter.end() && translate_event(ev, msg, wParam, lParam))
		{
			std::lock_guard<decltype(ptr->input_mutex)> lg(ptr->input_mutex);
			ptr->input_event.push_back(ev);
			return default_handled_event(hWnd, msg, wParam, lParam);
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
		bool init_win32_initial_implement(win32_initial_implement*& wfi) noexcept
		{
			wfi = new win32_initial_implement;
			return wfi != nullptr;
		}
		void dest_win32_initial_implement(win32_initial_implement*& wfi) noexcept
		{
			delete wfi;
			wfi = nullptr;
		}
		bool init_win32_form_implement(win32_form_implement*& wfi, win32_initial_implement* wii) noexcept
		{
			wfi = new win32_form_implement;
			return SUCCEEDED(manager.create(*wii, wfi));
		}
		void dest_win32_form_implement(win32_form_implement*& wfi) noexcept
		{
			manager.destory(wfi->raw_handle);
		}
		bool pick_win32_form_event(win32_form_implement* wfi, event& ev) noexcept
		{
			std::lock_guard<decltype(wfi->input_mutex)> lg(wfi->input_mutex);
			if (!wfi->input_event.empty())
			{
				ev = *(wfi->input_event.begin());
				wfi->input_event.pop_front();
				return true;
			}
			for (auto& po : wfi->output_event)
			{
				UINT msg; WPARAM wParam; LPARAM lParam;
				if (distranslate_event(po, msg, wParam, lParam))
				{
					SendMessageW(wfi->raw_handle, msg + WM_USER, wParam, lParam);
				}
			}
			wfi->output_event.clear();
			return false;
		}
		void throw_win32_form_event(win32_form_implement* wfi, event& ev) noexcept
		{
			wfi->output_event.push_back(ev);
		}
	}
}