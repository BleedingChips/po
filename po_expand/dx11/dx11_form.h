#pragma once
#include "../win32/win32_form.h"
#include "../dxgi/dxgi.h"
#include <DirectXMath.h>
#include <DirectXMathVector.inl>
#include <d3d11.h>
#pragma comment(lib,"d3d11.lib")

namespace PO
{
	namespace Dx11
	{

		struct Dx11_initial
		{

		};

		struct Dx11_form
		{
			Win32::win32_form form;
			CComPtr<ID3D11Device> dev;
			CComPtr<ID3D11DeviceContext> dc;
			CComPtr<IDXGISwapChain> swap;

			CComPtr<ID3D11Texture2D> main_buffer;
			CComPtr<ID3D11RenderTargetView> pView;
			CComPtr<ID3D11Texture2D> pDepthStencilBuffer;
			CComPtr<ID3D11DepthStencilView> pDepthView;
			Dx11_form(const Dx11_initial& = Dx11_initial{});
			~Dx11_form() {};
			void form_tick_implement(duration da, form_self& fs);
			template<typename ...AT> void form_tick(AT&& ...at)
			{
				Tool::auto_adapter<Tool::unorder_adapt>(&Dx11_form::form_tick_implement, this, at...);
			}

		};
	}







	/*
	namespace Interface
	{
		struct dx11_initial_implement
		{

		};

		struct dx11_form_implement
		{
			
		};

		struct dx11_renderer_implement
		{
			CComPtr<ID3D11Device> dev;
			CComPtr<ID3D11DeviceContext> dc;
			CComPtr<IDXGISwapChain> swap;
			CComPtr<ID3D11RenderTargetView> pView;
		};

		struct dx11_scene_implement
		{

		};
		

	}
	*/
}
