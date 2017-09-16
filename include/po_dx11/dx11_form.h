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

		struct swap_chain
		{
			Win32::com_ptr<IDXGISwapChain> ptr;
		};

		struct dx11_renderer_initializer
		{
			creator dev;
			stage_context dc;
			Implement::tex2_interface back_buffer;
		};

		namespace Implement
		{
			class form_pre_construction : public Win32::win32_form
			{
			protected:
				Win32::com_ptr<ID3D11Device> dev;
				Win32::com_ptr<ID3D11DeviceContext> dc;
				swap_chain swap;
				form_pre_construction(const initializer_form_default& = initializer_form_default{});
			};
		}

		class form_default : public Implement::form_pre_construction
		{
			stage_context pipe;
			creator creat;
			Implement::tex2_interface back_buffer;
		public:

			operator dx11_renderer_initializer() const { return dx11_renderer_initializer{ creat, pipe, back_buffer }; }

			form_default(const initializer_form_default& = initializer_form_default{});
			~form_default() { };
			void pre_tick(duration da) {
				(swap.ptr)->Present(0, 0);
			}
		};
	}
}
