#include <Windows.h>
#include "../../frame/define.h"
namespace
{

}

namespace PO
{
	namespace Interface
	{
		struct win32_form_implement
		{
		};
		struct win32_initial_implement
		{
		};
		bool init_win32_initial_implement(win32_initial_implement*&) noexcept
		{
			return true;
		}
		bool dest_win32_initial_implement(win32_initial_implement*&) noexcept
		{
			return true;
		}
		bool init_win32_form_implement(win32_form_implement*&, win32_initial_implement*) noexcept
		{
			return true;
		}
		bool dest_win32_form_implement(win32_form_implement*&) noexcept
		{
			return true;
		}
		bool is_win32_form_available(const win32_form_implement*) noexcept
		{
			return true;
		}
		bool pick_event(event&) noexcept
		{
			return true;
		}
		void throw_event(event&) noexcept
		{
			//return true;
		}
	}
}