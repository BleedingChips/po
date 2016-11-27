#pragma once
#include "../win32/win32_interface.h"
namespace PO
{
	namespace Interface
	{
		struct dx11_form_implement;
		struct dx11_initial_implement;
		struct dx11_renderer_implement;

		Tool::optional<std::string> init_dx11_initial_form_win32_initial(dx11_initial_implement*&, win32_initial_implement*) noexcept;
		void dest_dx11_initial(dx11_initial_implement*&) noexcept;
		Tool::optional<std::string> init_dx11_form_form_win32_form(dx11_form_implement*&, dx11_initial_implement*, win32_form_implement*) noexcept;
		void dest_dx11_form(dx11_form_implement*&) noexcept;

		Tool::optional<std::string> init_dx11_renderer_from_from(dx11_renderer_implement*&, dx11_form_implement*) noexcept;
		void dest_dx11_renderer(dx11_renderer_implement*&) noexcept;

		void tick_dx11_renderer(dx11_renderer_implement*, duration da) noexcept;
		
		void clean_dx11_form(dx11_form_implement*, float, float, float, float) noexcept;
		void swap_dx11_form(dx11_form_implement*) noexcept;
	}
	namespace DX11
	{
		class dx11_initial : public Win32::win32_initial
		{
			Interface::dx11_initial_implement* initial;
			friend class dx11_form;
		public:
			dx11_initial() { init_dx11_initial_form_win32_initial(initial, Win32::win32_initial::wii); }
			~dx11_initial() { dest_dx11_initial(initial); }
		};

		class dx11_form : public Win32::win32_form
		{
			Interface::dx11_form_implement* form;
			friend class dx11_interface;
			friend class dx11_viewer;
			friend class dx11_renderer;
		public:
			dx11_form(Tool::completeness_ref cr, const dx11_initial& di = dx11_initial()) :Win32::win32_form(std::move(cr), di){ init_dx11_form_form_win32_form(form, di.initial, Win32::win32_form::wfi); }
			~dx11_form() { dest_dx11_form(form); }
			void tick(time_point tp)
			{
				Win32::win32_form::template event_implement([](event&) { return false; });
			}
		};

		class dx11_interface
		{
		private:
			Interface::dx11_form_implement* form;
			friend class dx11_renderer;
		public:
			dx11_interface(dx11_form& df) : form(df.form) {}
			void clean_chain(float R, float G, float B, float A)
			{
				Interface::clean_dx11_form(form, R, G, B, A);
			}
			void swap()
			{
				Interface::swap_dx11_form(form);
			}
		};

		class dx11_viewer : public Win32::win32_form_viewer
		{
			Tool::completeness_ref cr;
			Interface::dx11_form_implement* form;
		public:
			dx11_viewer(dx11_form& df) : Win32::win32_form_viewer(df) {}
		};
		
		class dx11_renderer
		{
			Interface::dx11_renderer_implement* dri;
			time_calculator tc;
			float all_time;
		public:
			dx11_renderer(dx11_form& di) ///: inter(di)
			{
				Interface::init_dx11_renderer_from_from(dri, di.form);
			}
			void tick(time_point to)
			{

				tc.tick(
					to,
					[this](duration da)
				{
					Interface::tick_dx11_renderer(dri, da);
				}
				);

				//Interface::tick_dx11_renderer(dri, );
				/*
				tc.tick(to,
					[this](duration da) 
				{
					all_time += da.count();
					float a = abs(sin(all_time * 0.001f));
					float a2 = abs(sin(all_time * 0.001f * 2));
					float a3 = abs(sin(all_time * 0.001f * 3));
					inter.clean_chain(a, a2, a3, 1.0f);
					inter.swap();
				}
				);
				*/
			}
			~dx11_renderer()
			{
				Interface::dest_dx11_renderer(dri);
			}
		};

	}
}