#pragma once
#include "dx9_configure.h"
#include <string>
namespace Graphic
{
	namespace DX9
	{
		inline std::string translate_DX_HRESULT_to_string(HRESULT re)
		{
			switch (re)
			{
			case D3DERR_DEVICELOST:
				return "D3DERR_DEVICELOST";
			case D3DERR_INVALIDCALL:
				return "D3DERR_INVALIDCALL";
			case D3DERR_NOTAVAILABLE:
				return "D3DERR_NOTAVAILABLE";
			case D3DERR_OUTOFVIDEOMEMORY:
				return "D3DERR_OUTOFVIDEOMEMORY";
			case D3D_OK:
				return "D3D_OK";
			default:
				return std::string("unknow");
			}
		}
	}
}