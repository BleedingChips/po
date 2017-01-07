#include "translate.h"
#include <Windows.h>
namespace
{
	thread_local std::vector<char>  translate_char_buffer;
}
namespace PO
{
	namespace Platform
	{
		namespace Language
		{
			std::string utf16_to_local(const std::u16string& data)
			{
				if (data.size() * 2 + 1 > translate_char_buffer.size())
					translate_char_buffer.resize(data.size() * 2 + 1);
				WideCharToMultiByte(CP_ACP, 0, (wchar_t*)(data.c_str()), -1, &translate_char_buffer[0], translate_char_buffer.size(), NULL, NULL);
				return std::string(&translate_char_buffer[0]);
			}
		}
	}
}