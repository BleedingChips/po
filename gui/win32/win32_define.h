#pragma once
#include <Windows.h>
#include <string>
#include "../../frame/error.h"
//#define LINE_INFO ( _FILENAME#_LINE_ )
namespace PO
{
	namespace Win32
	{
		inline std::u16string HRESULT_to_u16string(HRESULT re)
		{
			static thread_local char16_t buffer[1024 * 2];
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, re, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<wchar_t*>(buffer), 0, NULL);
			return std::u16string(buffer);
		}

		namespace Error
		{
			inline void fail_throw(HRESULT re)
			{
				if (!SUCCEEDED(re))
				{
					throw PO::Error::po_error(HRESULT_to_u16string(re));
				}
			}

			template<typename T>
			void fail_throw(HRESULT re, T&& t)
			{
				if (!SUCCEEDED(re))
				{
					Tool::statement_if<std::is_same<decltype(t()), void>>
						(
							[re](auto&& y)
					{
						y();
						throw PO::Error::po_error(HRESULT_to_u16string(re));
					},
							[re](auto&& y)
					{
						throw PO::Error::po_error(HRESULT_to_u16string(re) + y());
					},
						std::forward<T>(t)
						);
				}
			}
		}
	}

}
