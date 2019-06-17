#pragma once
#include "mscf_syntax.h"
#include <sstream>
namespace PO::Mscf
{
	namespace Error
	{
		enum class Type
		{
			None = 0,
			OperatorUnSupportFormat,
			UnFoundId,
			AllType,
		};

		struct complie_error : std::exception
		{
			complie_error(Type type, const std::string& path, Syntax::line_record line, const std::string& message);
			Type type() const noexcept { return m_type; }
			Syntax::line_record record() const noexcept { return m_record; }
			const char* what() const noexcept;
		private:
			Type m_type;
			Syntax::line_record m_record;
			std::string m_path;
			std::string m_message;
		};
	}

	namespace Implement
	{

		template<typename Value>
		std::string to_string(const Value& value)
		{
			std::stringstream ss;
			ss << value;
			std::string result;
			ss >> result;
			return std::move(result);
		}

		std::string to_string(const Syntax::line_record& line);

		template<typename Type> Type form_string(const std::string& string)
		{
			std::stringstream ss;
			ss << string;
			Type result;
			ss >> result;
			return result;
		}
	}
}