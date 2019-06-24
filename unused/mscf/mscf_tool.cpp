#include "mscf_tool.h"
#include <array>
using namespace std;
namespace PO::Mscf
{
	namespace Error
	{
		array<std::string, static_cast<size_t>(Type::AllType) + 1> type_mapping =
		{
			"none",
			"operator unsupported type",
			"equal to const value",
			"all type"
		};

		complie_error::complie_error(Type type, const std::string& path, Syntax::line_record line, const std::string& message)
			: m_type(type), m_path(std::move(path)), m_record(line)
		{
			m_message = "error [" + type_mapping[static_cast<size_t>(type)] + "] in <" + path + "> (" + Implement::to_string(line) + ") :" + message;
		}

		const char* complie_error::what() const noexcept
		{
			return m_message.c_str();
		}
	}

	namespace Implement
	{
		std::string to_string(const Syntax::line_record& line)
		{
			return "";
			//return to_string(line.m_line) + " - " + to_string(line.m_charactor_count);
		}
	}
}