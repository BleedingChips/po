#pragma once
#include "../win32/define_win32.h"
#include "../po/tool/tool.h"
#include "../po/tool/intrusive_ptr.h"
#include "../graphic/context.h"
#include <future>
#include <variant>
#include <deque>
#include <string>

namespace PO::Win32
{
	struct form_property
	{
		std::u16string title = u"PO default title :>";
		DWORD window_style = WS_VISIBLE | WS_OVERLAPPEDWINDOW;
		DWORD ex_window_style = WS_EX_CLIENTEDGE;
		Graphic::int2 shift = { (GetSystemMetrics(SM_CXSCREEN) - 1024) / 2 , (GetSystemMetrics(SM_CYSCREEN) - 768) / 2 };
		Graphic::uint2 size = { 1024, 768 };
		Graphic::FormatRT format = Graphic::FormatRT::RGBA_F16;
		bool handle_close_event = true;
	};

	namespace Error
	{
		struct create_window_fauit : std::exception
		{
			DWORD m_result;
			const char* what() const noexcept override;
			create_window_fauit(DWORD result) : m_result(result) {}
		};
	}

	struct form
	{
		struct control : Tool::intrusive_object_base
		{
			std::mutex m_mutex;
			std::deque<event> m_pool;
			void release() noexcept { delete this; }
		};

		form(Graphic::graphic_context& context, const form_property& setting = form_property{}) { create(context, setting); }
		form() = default;
		form(form&& f) = default;
		form& operator=(form&&) noexcept = default;
		~form();
		
		bool pop_event(event& e);
		bool is_available() const noexcept { return form_context; }
		Graphic::form_context& as_form_context() noexcept { return *form_context; }
		const Graphic::form_context& as_form_context() const noexcept { return *form_context; }
		void create(Graphic::graphic_context& context, const form_property& fp);
		void close() noexcept;
	private:
		bool m_handle_quit_event = false;
		Tool::integer_moveonly<HWND, nullptr> m_handle;
		Graphic::form_context_ptr form_context;
		std::thread m_event_loop;
		Tool::intrusive_ptr<control> m_control;
		std::deque<event> m_events;
	};

	
}