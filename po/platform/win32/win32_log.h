#pragma once
#include "win32_form.h"
#include <sstream>
namespace PO
{
	namespace Platform
	{
		namespace Win32
		{
			namespace Log
			{
				std::string translate_error_code_to_string(DWORD code) noexcept;
				std::string translate_event_type_to_string(UINT type) noexcept;
			}
		}
	}
}
