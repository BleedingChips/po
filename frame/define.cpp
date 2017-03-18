#include "define.h"
#include <unordered_map>
#include <fstream>
namespace
{
	std::unordered_map<PO::KeyValue, std::pair<char, char>> key_value_map =
	{
		{ PO::KeyValue::K_1,{ '1','!' } }, { PO::KeyValue::K_2,{ '2','@' } }, { PO::KeyValue::K_3,{ '3','#' } }, { PO::KeyValue::K_4,{ '4','$' } },
		{ PO::KeyValue::K_5,{ '5','%' } }, { PO::KeyValue::K_6,{ '6','^' } }, { PO::KeyValue::K_7,{ '7','&' } }, { PO::KeyValue::K_8,{ '8','*' } },
		{ PO::KeyValue::K_9,{ '9','(' } }, { PO::KeyValue::K_0,{ '9',')' } },

		{ PO::KeyValue::K_A,{ 'a','A' } }, { PO::KeyValue::K_B,{ 'b','B' } }, { PO::KeyValue::K_C,{ 'c','C' } }, { PO::KeyValue::K_C,{ 'c','C' } }, { PO::KeyValue::K_D,{ 'd','D' } },
		{ PO::KeyValue::K_E,{ 'e','E' } }, { PO::KeyValue::K_F,{ 'f','F' } }, { PO::KeyValue::K_G,{ 'g','G' } }, { PO::KeyValue::K_H,{ 'h','H' } }, { PO::KeyValue::K_I,{ 'i','I' } },
		{ PO::KeyValue::K_J,{ 'j','J' } }, { PO::KeyValue::K_K,{ 'k','K' } }, { PO::KeyValue::K_L,{ 'l','L' } }, { PO::KeyValue::K_M,{ 'm','M' } }, { PO::KeyValue::K_O,{ 'o','O' } },
		{ PO::KeyValue::K_P,{ 'p','P' } }, { PO::KeyValue::K_Q,{ 'q','Q' } }, { PO::KeyValue::K_R,{ 'r','R' } }, { PO::KeyValue::K_S,{ 's','S' } }, { PO::KeyValue::K_T,{ 't','T' } },
		{ PO::KeyValue::K_U,{ 'u','U' } }, { PO::KeyValue::K_V,{ 'v','V' } }, { PO::KeyValue::K_W,{ 'w','W' } }, { PO::KeyValue::K_X,{ 'x','X' } }, { PO::KeyValue::K_Y,{ 'y','Y' } },
		{ PO::KeyValue::K_Z,{ 'z','Z' } },

		{ PO::KeyValue::K_SPACE, { ' ', ' ' } },
		{ PO::KeyValue::K_COMMA, { ',', '<' } },
		{ PO::KeyValue::K_PERIOD,{ '.', '>' } },
		{ PO::KeyValue::K_SLASH , {'/','?'}},
		{ PO::KeyValue::K_BACKSLASH, { '\\', '|' }},
		{ PO::KeyValue::K_L_BRACKET, {'[', '{'} },
		{ PO::KeyValue::K_R_BRACKET,{ ']', '}' } },
		{ PO::KeyValue::K_SEMICOLONS,{ ';', ':' } },
		{ PO::KeyValue::K_QUOTE,{ '\'', '\"' } },
		{ PO::KeyValue::K_MINUS,{ '-', '_' } },
		{ PO::KeyValue::K_EQUAL,{ '=', '+' } },
		{ PO::KeyValue::K_MINUTE,{ '`', '~' } },
	};
}



namespace PO
{
	char translate_key_value_to_asc_char_pair(KeyValue kv, bool upper)
	{
		auto ite = key_value_map.find(kv);
		if (ite != key_value_map.end())
			return upper ? ite->second.second : ite->second.first;
		return 0;
	}

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

	bool binary::load_file(std::u16string path)
	{
		ptr.reset();
		std::ifstream file(utf16_to_asc(path), std::ios::binary | std::ios::in);
		if (file.good())
		{
			file.seekg(0, std::ios::end);
			auto end_poi = file.tellg();
			file.seekg(0, std::ios::beg);
			auto sta_poi = file.tellg();
			alloc(static_cast<size_t>(end_poi - sta_poi));
			file.read(*this, end_poi - sta_poi);
			update();
			return true;
		}
		return false;
	}

}