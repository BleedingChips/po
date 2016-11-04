#pragma once
#include "../win32/win32_implement.h"
#include "../dxgi/dxgi.h"
#include <d3d11.h>
#pragma comment(lib,"d3d11.lib")
namespace PO
{
	namespace Interface
	{
		struct dx11_initial_implement
		{

		};

		struct dx11_form_implement
		{
			CComPtr<ID3D11Device> dev;
			CComPtr<ID3D11DeviceContext> dc;
			CComPtr<IDXGISwapChain> swap;

			CComPtr<ID3D11Texture2D> main_buffer;
			CComPtr<ID3D11RenderTargetView> pView;
			CComPtr<ID3D11Texture2D> pDepthStencilBuffer;
			CComPtr<ID3D11DepthStencilView> pDepthView;
		};
	}
}
