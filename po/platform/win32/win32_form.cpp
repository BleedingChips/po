#include "win32_form.h"
#include <vector>
#include <sstream>
#include <exception>
#include <iostream>
#include <thread>
#include <future>
#include <map>
#include <chrono>
#include <set>
namespace
{
	const char16_t static_class_name[] = u"po_frame_window_class2";


	const std::set<UINT> handled_event_filter= 
	{
		WM_CLOSE,
		WM_MOUSEMOVE
	};

	HRESULT default_handled_event(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		// WM_CLOSE
		return 0;
	}



	LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		if (handled_event_filter.find(msg) != handled_event_filter.end())
		{
			PO::Platform::Win32::win32_form* ptr = reinterpret_cast<PO::Platform::Win32::win32_form*> (GetWindowLongW(hWnd, GWL_USERDATA));
			if (ptr != nullptr && ptr->respond_event( hWnd,msg , wParam, lParam ))
				return default_handled_event(hWnd,msg,wParam, lParam);
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
				throw PO::Platform::Win32::win32_init_error{};
			}
		}

		~static_class_init_struct()
		{
			UnregisterClassW((const wchar_t*)static_class_name, GetModuleHandleW(0));
		}

	};


	HWND create_window(
		const PO::Platform::Win32::win32_initializer& wi,
		PO::Platform::Win32::win32_form* ptr
		)
	{
		HWND handle = CreateWindowExW(
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
		HRESULT re = GetLastError();
		if (handle == nullptr)
		{
			throw PO::Platform::Win32::win32_init_error{};
		}
		SetWindowLongW(handle, GWL_USERDATA, reinterpret_cast<LONG>(ptr));
		return handle;
	}

	void destory_window(HWND handle)
	{
		SetWindowLongW(handle, GWL_USERDATA, reinterpret_cast<LONG>(nullptr));
		DestroyWindow(handle);
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

		HWND create(
			const PO::Platform::Win32::win32_initializer& wi,
			PO::Platform::Win32::win32_form* wf)
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

			if (std::this_thread::get_id() == event_thread.get_id())
			{
				return create_window(wi, wf);
			}
			else {
				cv.wait(ul, [this]() {return !static_cast<bool>(delegate_function); });
				std::promise<HWND> pro;
				auto fur = pro.get_future();
				delegate_function = [&]()
				{
					pro.set_value(create_window(wi, wf));
				};
				ul.unlock();
				fur.wait();
				return fur.get();
			}
		}

		void destory(HWND handle)
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
			if(event_thread.joinable())
			{
				{
					std::lock_guard<decltype(ref_mutex)> ld(ref_mutex);
					ref = 0;
				}
				if(event_thread.joinable()) event_thread.join();
			}
		}

	} manager;

}


namespace PO
{
	namespace Platform
	{
		namespace Win32
		{
			win32_form::win32_form(const win32_initializer& wi) : avalible(true)
			{
				static static_class_init_struct static_class_init;
				raw_handle = manager.create(wi, this);
			}

			win32_form::~win32_form()
			{
				manager.destory(raw_handle);
			}

			bool win32_form::respond_event(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
			{
				bool result = false;

				if (event_mail.lock_if_capture(
					[&result](bool re) { result = re; },
					hWnd, msg, wParam, lParam
				) && result)
					return true;

				switch (msg)
				{
				case WM_CLOSE:
					avalible = false;
					return true;
				}
				return false;
			}
		}
	}
}