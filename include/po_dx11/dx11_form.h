#pragma once
#include "../po_win32/win32_form.h"
#include "dx11_frame.h"
#include "dx11_vertex.h"
#include "../po_dxgi/dxgi_define.h"
#include <DirectXMath.h>
#include <DirectXMathVector.inl>
#include <d3d11.h>

namespace PO
{
	namespace Dx11
	{

		class initializer_form_default
		{
			Win32::win32_initial initial_win32;
		public:
			operator const Win32::win32_initial& () const { return initial_win32; }
		};

		namespace Implement
		{
			class form_pre_construction : public Win32::win32_form
			{
			protected:
				Dx11_initializer initializer;
				form_pre_construction(const initializer_form_default& = initializer_form_default{});
			};
		}

		class form_default : public Implement::form_pre_construction
		{
			Dx11_frame_initializer frame_initializer;
		public:
			value_table mapping();
			form_default(const initializer_form_default& = initializer_form_default{});
			~form_default() {  };
		};
	}
}
