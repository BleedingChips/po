#pragma once
#include <d3d11.h>
#pragma comment(lib,"d3d11.lib")
#include "../../platform/platform_window.h"
namespace PO
{
	namespace DX11
	{

		struct initializer :public PO::Platform::window_style
		{
			DXGI_SWAP_CHAIN_DESC DSCD =
			{
				DXGI_MODE_DESC
			{
				1024,
				768,
				DXGI_RATIONAL{ 60,1 },
				DXGI_FORMAT_R8G8B8A8_UNORM,
				DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,
				DXGI_MODE_SCALING_UNSPECIFIED
			},
				DXGI_SAMPLE_DESC{
				1,
				0
			},
				DXGI_USAGE_RENDER_TARGET_OUTPUT,
				1,
				nullptr,
				false,
				DXGI_SWAP_EFFECT_DISCARD,
				0
			};
		};

		struct renderer:public PO::Platform::window_instance
		{
			ID3D11Device* dev;
			ID3D11DeviceContext* dc;
			IDXGISwapChain* swap;
			renderer(const initializer& it = initializer())
			{
				initializer tem(it);

				tem.DSCD.OutputWindow = get_index();
				HRESULT res;
				//res = 
			}
		};

		

		
	}

	struct DX11_expand
	{
		using initializer = DX11::initializer;
		using renderer = DX11::renderer;
	};
}