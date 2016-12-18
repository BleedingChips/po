#pragma once
#include "../../po/frame/define.h"
#include "../po_expand.h"
#include <atomic>
namespace PO
{
	namespace Interface
	{
		struct win32_form_implement;
		struct win32_initial_implement;
		Tool::optional<std::string> init_win32_initial_implement(win32_initial_implement*&) noexcept;
		void dest_win32_initial_implement(win32_initial_implement*&) noexcept;
		Tool::optional<std::string> init_win32_form_implement(win32_form_implement*&, win32_initial_implement*) noexcept;
		void dest_win32_form_implement(win32_form_implement*&) noexcept;
		Tool::optional<event> pick_win32_form_event(win32_form_implement*) noexcept;
		void throw_win32_form_event(win32_form_implement*, const event&) noexcept;
	}

	namespace Win32
	{

		namespace Error
		{
			class win32_interface_init_error : public std::exception
			{
				std::string statement;
			public:
				win32_interface_init_error(std::string a) : statement(std::move(a)) {}
				const char* what() const override { return statement.c_str(); }
			};
		}

		class win32_initial
		{
		protected:
			Interface::win32_initial_implement* wii;
			friend class win32_form;
		public:
			win32_initial() 
			{ 
				if (auto poi = Interface::init_win32_initial_implement(wii))
					throw Error::win32_interface_init_error(*poi);
			}
			~win32_initial() { Interface::dest_win32_initial_implement(wii); }
		};

		class win32_form
		{
		protected:
			Interface::win32_form_implement* wfi;
			friend class win32_form_viewer;
			std::atomic<bool> available;
			std::function<bool(event)> event_function;
			Tool::completeness_ref self;
		public:
			operator const Tool::completeness_ref&() const { return self; }
			win32_form(Tool::completeness_ref cr, const win32_initial& wi = win32_initial()):available(true), self(std::move(cr)) 
			{ 
				if (auto pi = Interface::init_win32_form_implement(wfi, wi.wii))
					throw Error::win32_interface_init_error(*pi);
			}
			~win32_form() { Interface::dest_win32_form_implement(wfi); }
			bool is_available() const noexcept { return available; }
			void bind_event(std::function<bool(event)> t)
			{
				event_function = std::move(t);
			}
			void tick(time_point tp)
			{
				event_implement([](event&) { return false; });
			}
			template<typename T> void event_implement(T&& t)
			{
				while (auto ev = pick_win32_form_event(wfi))
				{
					if (!( (event_function && event_function(*ev)) || t(*ev)))
					{
						if ((*ev).is_quit())
							available = false;
						else
							throw_win32_form_event(wfi, *ev);
					}
				}
			}
		};

		class win32_form_viewer
		{
			win32_form* form;
			Tool::completeness_ref self;
		public:
			win32_form_viewer(win32_form& wf) :form(&wf), self(wf.self) {}
			bool close_form() { return self.lock_if([this]() {form->available = false; }); }
		};

		class win32_form_interface
		{
			win32_form* form;
		public:
			win32_form_interface(win32_form& wf) :form(&wf) {}
		};

	}
}