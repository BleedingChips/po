#pragma once
#include <Windows.h>
#include <string>
//#define LINE_INFO ( _FILENAME#_LINE_ )
namespace PO::Win32
{
	size_t HRESULT_to_utf16(HRESULT, char16_t*, size_t avalible_buffer) noexcept;
	size_t HRESULT_to_utf8(HRESULT, char*, size_t avalible_buffer) noexcept;
	std::u16string HRESULT_to_utf16(HRESULT) noexcept;
	const char* message_type_to_utf8(UINT type) noexcept;
	//std::string HRESULT_to_utf8(HRESULT) noexcept;
	//std::string HRESULT_to_asc(HRESULT) noexcept;
	//std::string translate_error_code_to_string(DWORD code) noexcept;
	//std::string translate_event_type_to_string(UINT type) noexcept;

}
