#include "utf_support.h"
#include <vector>
#ifdef _WIN32
namespace PO 
{
	std::string utf16_to_asc(const std::u16string& data)
	{
		static thread_local std::vector<char>  translate_char_buffer;
		if (data.size() * 2 + 1 > translate_char_buffer.size())
			translate_char_buffer.resize(data.size() * 2 + 1);
		HRESULT re = WideCharToMultiByte(CP_ACP, 0, (wchar_t*)(data.c_str()), -1, &translate_char_buffer[0], static_cast<int>(translate_char_buffer.size()), NULL, NULL);
		if (!SUCCEEDED(re))
		{
			__debugbreak();
		}
		std::string tem(&translate_char_buffer[0]);
		return tem;
	}

	inline std::u16string asc_to_utf16(const std::string& data)
	{
		static thread_local std::vector<wchar_t>  translate_char_buffer;
		if (data.size() > translate_char_buffer.size())
			translate_char_buffer.resize(data.size());
		MultiByteToWideChar(CP_ACP, 0, data.c_str(), -1, &translate_char_buffer[0], static_cast<int>(translate_char_buffer.size()));
		return std::u16string(reinterpret_cast<char16_t*>(&translate_char_buffer[0]));
	}
}
#endif