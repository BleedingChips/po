#pragma once
#include <exception>
#include <string>
#include "define.h"
#include <iostream>
#include "../tool/tool.h"
#include <tuple>
#include "../tool/utf_support.h"
namespace PO
{
	namespace Error
	{
		using std::cout;
		class po_error
		{
			std::u16string scr;
		public:
			po_error(std::u16string str) : scr(std::move(str)){}
			operator std::string() const { return utf16_to_asc(scr); }
			operator std::u16string&() { return scr; }
		};

		inline std::ostream& operator<<(std::ostream& o, const po_error& pe)
		{
			return o << static_cast<std::string>(pe);
		}

		//using result = Tool::optional<std::tuple<std::string>>

		template<typename T>
		void unavalible_throw(T&& t)
		{
			if (!static_cast<bool>(t))
			{
				throw po_error(u"po<object unavalible>:");
			}
		}

		template<typename T, typename K>
		void unavalible_throw(const T& t, K&& d)
		{
			if (!static_cast<bool>(t))
			{
				Tool::statement_if<std::is_same<decltype(d()), void>>
					(
						[](auto&& y)
				{
					y();
					throw PO::Error::po_error(u"po<object unavalible>:");
				},
						[](auto&& y)
				{
					throw PO::Error::po_error(u"po<object unavalible>:" + y());
				},
					std::forward<T>(d)
					);
			}
		}
	}
}