#pragma once
#include "dxgi_define.h"
#include "../po_win32/win32_log.h"
#include <string>
namespace PO
{
	namespace Platform
	{
		namespace Dxgi
		{
			namespace Log
			{
				std::string DXGI_FORMAT_to_s(DXGI_FORMAT);
				std::string DXGI_ADAPTER_FLAG_to_s(DXGI_ADAPTER_FLAG);
				inline std::string DXGI_ADAPTER_FLAG_to_s(UINT u) { return DXGI_ADAPTER_FLAG_to_s(DXGI_ADAPTER_FLAG(u)); }
			}
		}
	}
	
}