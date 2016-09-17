#include "win32_form.h"
#include <vector>
#include <sstream>
#include <exception>
#include <iostream>
#include <thread>
#include <future>
#include <map>
#include <chrono>
namespace
{

	bool event_translate(PO::event& ev, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_CLOSE:
			ev.msg = WM_CLOSE;
			return true; 
		}
		return false;
	}

	const char16_t static_class_name[] = u"po_frame_window_class2";

	LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_CREATE:
		case WM_PAINT:
		case WM_DESTROY:
			return DefWindowProcW(hWnd, msg, wParam, lParam);
		default:
		{
			PO::Platform::Win32::win32_form* ptr = reinterpret_cast<PO::Platform::Win32::win32_form*> (GetWindowLongW(hWnd, GWL_USERDATA));
			PO::event eve;
			if (ptr != nullptr && event_translate(eve, hWnd, msg, wParam, lParam))
			{
				if (ptr->reapond_window_evnet(eve))
					return 0;
			}
			switch (msg)
			{
			case WM_CLOSE:
				ptr->close_window();
				return 0;
			default:
				break;
			}
			return DefWindowProcW(hWnd, msg, wParam, lParam);
			break;
		}
		}
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

	}static_class_init;


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
		std::thread event_thread;
		std::recursive_mutex ref_mutex;
		size_t ref = 0;
		std::function<void(void)> delegate_function;
	public:
		HWND create(
			const PO::Platform::Win32::win32_initializer& wi,
			PO::Platform::Win32::win32_form* wf)
		{
			std::promise<HWND> pro;
			std::future<HWND> fur = pro.get_future();
			{
				std::lock_guard<decltype(ref_mutex)> ld(ref_mutex);
				++ref;
				if (ref == 1)
				{
					if(event_thread.joinable())
						event_thread.join();
					event_thread = std::thread(
						[this]()
						{
							while(true)
							{
								{
									std::lock_guard<decltype(ref_mutex)> ld(ref_mutex);
									if(ref == 0) break;
									if(delegate_function)
									{
										delegate_function();
										delegate_function = std::function<void()>{};
									}
								}
								MSG msg;
								while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
								{
									DispatchMessageW(&msg);
								}
								std::this_thread::yield();
							}
						}
					);
				}
				if(std::this_thread::get_id()==event_thread.get_id())
				{
					return create_window(wi,wf);
				}else{
					while(delegate_function)
					{
						ref_mutex.unlock();
						std::this_thread::yield();
						ref_mutex.lock();
					}
					delegate_function = [&]()
					{
						pro.set_value(create_window(wi,wf));
					};
				}
			}
			fur.wait();
			return fur.get();
		}

		void destory(HWND handle)
		{
			std::promise<void> pro;
			std::future<void> fur = pro.get_future();
			{
				std::lock_guard<decltype(ref_mutex)> ld(ref_mutex);
				if(std::this_thread::get_id() == event_thread.get_id())
				{
					destory_window(handle);
					--ref;
					return;
				}else{
					while(delegate_function)
					{
						ref_mutex.unlock();
						std::this_thread::yield();
						ref_mutex.lock();
					}
					delegate_function = [&]()
					{
						destory_window(handle);
						pro.set_value();
					};
				}
			}
			fur.wait();
			fur.get();
			{
				std::lock_guard<decltype(ref_mutex)> ld(ref_mutex);
				--ref;
				if(ref !=0 ) return;
			}
			if (event_thread.joinable())
				event_thread.join();
		}

		~form_event_thread_manager()
		{
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
				raw_handle = manager.create(wi, this);
			}

			win32_form::~win32_form()
			{
				manager.destory(raw_handle);
			}

			
			bool win32_form::reapond_window_evnet(const event& ev)
			{
				bool res = false;
				return respond_call_back.capture(
					[&res](bool r) 
				{
					res = r;
					return r;
				}
				, ev) && res;
			}
			

		}
	}
}