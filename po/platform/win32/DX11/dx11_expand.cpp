#include "dx11_expand.h"
#include <iostream>
namespace PO
{
	namespace DX11
	{
		renderer::renderer(Tool::completeness_ref&& cpf, const initializer& it) :win32_form(it)
		{
			initializer tem = it;
			D3D_FEATURE_LEVEL lel[] = { D3D_FEATURE_LEVEL_11_0 };
			D3D_FEATURE_LEVEL lel2 = D3D_FEATURE_LEVEL_11_0;


			DXGI_SWAP_CHAIN_DESC DSCD =
			{
				DXGI_MODE_DESC
			{
				1024,
				768,
				DXGI_RATIONAL{ 60,1 },
				DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM,
				DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,
				DXGI_MODE_SCALING_UNSPECIFIED
			},
				DXGI_SAMPLE_DESC
			{
				1,
				0
			},
				DXGI_USAGE_RENDER_TARGET_OUTPUT,
				1,
				nullptr,
				true,
				DXGI_SWAP_EFFECT_DISCARD,
				0
			};

			PO::Platform::Dxgi::swap_chain_desc swc(raw());

			DSCD.OutputWindow = raw();
			/*
			HRESULT re = D3D11CreateDevice(
				NULL,
				D3D_DRIVER_TYPE_HARDWARE,
				NULL,
				D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DISABLE_GPU_TIMEOUT,
				lel,
				1,
				D3D11_SDK_VERSION,
				&dev,
				&lel2,
				&dc
			);*/
			//tem.DSCD.Windowed = false;
			HRESULT re = D3D11CreateDeviceAndSwapChain(
				nullptr,
				D3D_DRIVER_TYPE_HARDWARE,
				nullptr,
				D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DISABLE_GPU_TIMEOUT,
				lel,
				1,
				D3D11_SDK_VERSION,
				&swc,
				&swap,
				&dev,
				&lel2,
				&dc
			);
			
			DWORD ui = GetLastError();
			if(re!=S_OK)
			{
				if (re == DXGI_ERROR_NOT_CURRENTLY_AVAILABLE)
				{
					std::cout << "A" << std::endl;
				}
				else if (re == DXGI_ERROR_SDK_COMPONENT_MISSING)
				{
					std::cout << "M" << std::endl;
				}
				std::cout << "fail!! :" <<std::hex<<re<< std::endl;
				throw std::exception{};
			}
			re = swap->GetBuffer(0,
				__uuidof(ID3D11Texture2D),
				(void **)&main_buffer);
			if (re != S_OK)
			{
				__debugbreak();
			}
			else {
				D3D11_TEXTURE2D_DESC DTD;
				main_buffer->GetDesc(&DTD);
				std::cout << DTD.Width << "," << DTD.Height << std::endl;
			}

		}

		renderer::~renderer()
		{
			
		}
	}
}