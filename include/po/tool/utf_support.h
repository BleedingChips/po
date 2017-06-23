#pragma once
#include <string>
#ifdef _WIN32
#include <Windows.h>
namespace PO
{
	std::string utf16_to_asc(const std::u16string& data);
	std::u16string asc_to_utf16(const std::string& data);
}

inline wchar_t const* operator"" _wc
(
	std::conditional_t<sizeof(wchar_t) == sizeof(char16_t), char16_t const *, char32_t const *> ca,
	size_t l
)
{
	return reinterpret_cast<wchar_t const*>(ca);
}
#endif


