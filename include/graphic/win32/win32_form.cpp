#include "win32_form.h"
#include <future>
#include <functional>
#include <map>
#include <unordered_map>
#include <iostream>
#include "win32_log.h"
#include <Windows.h>
#include <assert.h>
namespace
{
	const char16_t static_class_name[] = u"po_frame_default_win32_class";

	LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		return DefWindowProcW(hWnd, msg, wParam, lParam);
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

	/*
	HRESULT create_window(
		const PO::Win32::win32_initial& wi,
		HWND& handle,
		PO::Win32::win32_form* ptr
	)
	{
		static static_class_init_struct scis;
		handle = CreateWindowExW(
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
			HRESULT ret = GetLastError();
			return ret;
		}
		SetWindowLongPtrW(handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(ptr));
		return S_OK;
	}

	void destory_window(HWND& handle)
	{
		SetWindowLongPtrW(handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(nullptr));
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
			const PO::Win32::win32_initial& wi,
			HWND& handle,
			PO::Win32::win32_form* ptr)
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
				pro.set_value(create_window(wi, handle, ptr));
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
	*/
}

namespace PO::Win32
{
	const char16_t* default_form_style = static_class_name;

	form create_form(const form_property& setting, const char16_t* style, void* user_data) noexcept
	{
		HWND handle = CreateWindowExW(
			setting.ex_window_style,
			(wchar_t*)(style),
			(wchar_t*)(setting.title.c_str()),
			setting.window_style,
			setting.shitf_x, setting.shift_y, setting.width, setting.height,
			NULL,
			NULL,
			GetModuleHandle(0),
			NULL
		);
		SetWindowLongPtr(handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(user_data));
		return form{ handle };
	}

	bool respond_event() noexcept
	{
		MSG msg;
		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			if (msg.message != WM_QUIT)
			{
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
			}
			else
				return false;
		}
		return true;
	}




	/*
	std::optional<event> form::peek_event(bool& out) noexcept
	{
		MSG msg;
		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			std::cout << PO::Platform::Win32::Log::translate_event_type_to_string(msg.message) << std::endl;
			switch (msg.message)
			{
			case WM_NCLBUTTONDOWN:
				if (msg.wParam == HTCLOSE)
				{
					event ev;
					ev.type = decltype(ev.type)::E_CLOSE;
					return ev;
				}
				break;
			case WM_CLOSE:
				out = false;
				break;
			}
			auto po = translate_message_to_event(msg);
			if (po.has_value())
				return po.value();
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		return {};
	}
	*/








	/*
	namespace Win32
	{
		form::form(const form_property& ft)
		{
			call_back = std::make_shared<control>();
			call_back->setting = ft;
			call_back->create_result = std::promise<HWND>{};
			auto furture = call_back->create_result.value().get_future();
			main_thread = std::thread(
				&form::main_thread_execute, call_back
			);
			furture.wait();
			raw_handle = furture.get();
			assert(raw_handle != nullptr);
		}

		form::~form()
		{
			assert(raw_handle != nullptr);
			PostMessage(raw_handle, WM_CLOSE, 0, 0);
			if (main_thread.joinable())
				main_thread.join();
		}

		void form::set_function_wait(std::function<void()> renderer, std::function<Respond(event&)> event_respond)
		{
			std::promise<void> pro;
			auto fur = pro.get_future();
			{
				std::lock_guard<decltype(call_back->mutex)> ld(call_back->mutex);
				call_back->function = std::make_pair(std::move(renderer), std::move(event_respond));
				call_back->function_replace_promise = std::move(pro);
			}
			fur.wait();
			fur.get();
		}

		void form::set_function(std::function<void()> renderer, std::function<Respond(event&)> event_respond)
		{
			std::lock_guard<decltype(call_back->mutex)> ld(call_back->mutex);
			call_back->function = std::make_pair(std::move(renderer), std::move(event_respond));
		}

		void form::main_thread_execute(std::shared_ptr<control> sp)
		{
			regeister();
			HWND handle;
			{
				std::lock_guard<decltype(sp->mutex)> ld(sp->mutex);
				auto& setting = sp->setting;
				handle = CreateWindowExW(
					sp->setting.ex_window_style,
					(wchar_t*)(static_class_name),
					(wchar_t*)(sp->setting.title.c_str()),
					sp->setting.window_style,
					setting.shitf_x, setting.shift_y, setting.width, setting.height,
					NULL,
					NULL,
					GetModuleHandle(0),
					NULL
				);
				ShowWindow(handle, SW_HIDE);
				if (sp->create_result.has_value())
				{
					sp->create_result.value().set_value(handle);
					sp->create_result.reset();
				}
			}

			
			
			if (handle != nullptr)
			{

				std::function<void()> render_function;
				std::function<Respond(event&)> respond_function;

				MSG msg;
				bool avalible = true;
				while (avalible)
				{
					{
						std::lock_guard<decltype(sp->mutex)> ld(sp->mutex);
						if (sp->function.has_value())
						{
							auto& at = sp->function.value();
							render_function = std::move(at.first);
							respond_function = std::move(at.second);
							sp->function.reset();
							if (sp->function_replace_promise.has_value())
							{
								sp->function_replace_promise.value().set_value();
								sp->function_replace_promise.reset();
							}
						}
					}
					

					while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
					{
						//TranslateMessage(&msg);
						//std::cout << PO::Platform::Win32::Log::translate_event_type_to_string(msg.message) << std::endl;
						//TranslateMessage(&msg);
						switch (msg.message)
						{
						case WM_NCLBUTTONDOWN:
							if (msg.wParam == HTCLOSE)
							{
								event ev;
								ev.type = decltype(ev.type)::E_CLOSE;
								if (respond_function)
									respond_function(ev);
								continue;
							}
							break;
						case WM_CLOSE:
							DestroyWindow(handle);
							PostQuitMessage(0);
							avalible = false;
							continue;
							break;
						default:
						{
							if (respond_function)
							{
								auto ite = handled_event_filter.find(msg.message);
								if (ite != handled_event_filter.end())
								{
									event ev = ite->second.translate_event(msg.wParam, msg.lParam);
									respond_function(ev);
									continue;
								}
							}
							TranslateMessage(&msg);
							DispatchMessageW(&msg);
							break;
						}
							
						}
						
						//TranslateMessage(&msg);
						
					}
					if (render_function)
						render_function();
				}
			}
		}

		/*
		void form_entity_mt::rander_frame(duration da) {}

		void form_entity_mt::init(PO::entity_self& sef, PO::context& c)
		{
			finish_construction_p.set_value();
			auto en = c.create_componenet<event_provider>();
			c.create_system<PO::event_transmision_system>();
			sef.insert(en);
		}


		void form_entity_mt::window_loop(std::promise<void> p, std::future<void> f, const form_type& wi)
		{
			static size_t count = 0;
			assert(SUCCEEDED(RegisterClassExW(&static_class)));

			raw_handle = CreateWindowExW(
				wi.ex_window_style,
				(wchar_t*)(static_class_name),
				(wchar_t*)(wi.title.c_str()),
				wi.window_style,
				wi.shitf_x, wi.shift_y, wi.width, wi.height,
				NULL,
				NULL,
				GetModuleHandle(0),
				NULL
			);

			p.set_value();
			f.wait();

			assert(raw_handle != nullptr);

			ShowWindow(raw_handle, SW_SHOW);
			UpdateWindow(raw_handle);

			//ÏûÏ¢Ñ­»·
			MSG msg;
			bool wait_quit = false;
			bool quit = false;

			time_point start_time = Frame::get_time_now();

			while (!quit)
			{
				while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
				{
					//std::cout << PO::Platform::Win32::Log::translate_event_type_to_string(msg.message) << std::endl;
					if (msg.message == WM_NCLBUTTONDOWN)
					{
						if (msg.wParam == HTCLOSE)
						{
							event tem;
							tem.type = PO::EventType::E_CLOSE;
							capture_event_tank.lock([&](decltype(capture_event_tank)::type& t) {
								t.push_back(tem);
							});
							continue;
						}
					}
					else if (msg.message == WM_CLOSE)
					{
						quit = true;
					}
					else {
						auto ite = handled_event_filter.find(msg.message);
						if (ite != handled_event_filter.end() && ite->second.translate_event)
						{
							if (msg.message == WM_CLOSE)
								volatile int i = 0;
							auto ev = ite->second.translate_event(msg.wParam, msg.lParam);
							capture_event_tank.lock([&](decltype(capture_event_tank)::type& t) {
								t.push_back(ev);
							});
							continue;
						}
					}
					DispatchMessageW(&msg);
				}
				time_point cur_time = Frame::get_time_now();
				duration da = std::chrono::duration_cast<duration>(cur_time - start_time);
				rander_frame(da);
				start_time = cur_time;
				std::this_thread::sleep_for(duration(1));
			}
			assert(UnregisterClassW((const wchar_t*)static_class_name, GetModuleHandleW(0)));
		}


		void form_entity_mt::tick(PO::entity_self& self, PO::context& c, duration da)
		{
			capture_event_tank.lock([this](decltype(capture_event_tank)::type& t) {
				std::swap(t, event_tank);
			});

			self.find_component([&](PO::event_provider& ep) {
				for (auto& ite : event_tank)
				{
					if (ite.is_quit())
						form_entity_mt::close_window();
					ep.insert(ite);
				}
				return true;
			});
			event_tank.clear();
		}

		void form_entity_mt::close_window()
		{
			PostMessage(raw_handle, WM_CLOSE, 0, 0);
			destory();
		}

		void form_entity_mt::wait_thread_close()
		{
			if (window_thread.joinable())
				window_thread.join();
		}

		form_entity_mt::form_entity_mt(const form_type& wi)
		{
			RegisterClassExW(&static_class);
			std::promise<void> handle_init_p;
			auto handle_init_f = handle_init_p.get_future();
			auto finish_construction_f = finish_construction_p.get_future();
			window_thread = std::thread([&, this]() {
				window_loop(std::move(handle_init_p), std::move(finish_construction_f), wi);
			});
			handle_init_f.wait();
		}

		form_entity_mt::~form_entity_mt()
		{
			PostMessage(raw_handle, WM_CLOSE, 0, 0);
			wait_thread_close();
		}
		*/
}