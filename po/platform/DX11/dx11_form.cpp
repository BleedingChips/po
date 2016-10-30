#include "dx11_form.h"
#include <iostream>
namespace PO
{
	namespace Platform
	{
		namespace Dx11
		{
			dx11_form::dx11_form(Tool::completeness_ref cpf, const dx11_init& it) :win32_form(it), self_ref(cpf)
			{
				PO::Platform::Dxgi::swap_chain_desc swc(raw());

				dx11_init tem = it;
				D3D_FEATURE_LEVEL lel[] = { D3D_FEATURE_LEVEL_11_0 };
				D3D_FEATURE_LEVEL lel2 = D3D_FEATURE_LEVEL_11_0;

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
				if (re != S_OK)
				{
					if (re == DXGI_ERROR_NOT_CURRENTLY_AVAILABLE)
					{
						std::cout << "A" << std::endl;
					}
					else if (re == DXGI_ERROR_SDK_COMPONENT_MISSING)
					{
						std::cout << "M" << std::endl;
					}
					std::cout << "fail!! :" << std::hex << re << std::endl;
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

				D3D11_TEXTURE2D_DESC desc;
				desc.Width = 1024;
				desc.Height = 768;
				desc.MipLevels = 1;
				desc.ArraySize = 1;
				desc.Format = DXGI_FORMAT_D32_FLOAT;
				desc.SampleDesc.Count = 1;
				desc.SampleDesc.Quality = 0;
				desc.Usage = D3D11_USAGE_DEFAULT;
				desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
				desc.CPUAccessFlags = 0;
				desc.MiscFlags = 0;
				HRESULT hr = dev->CreateTexture2D(&desc,
					0, &pDepthStencilBuffer);
				 hr = dev->CreateDepthStencilView(pDepthStencilBuffer,
					0, &pDepthView);
				dev->CreateRenderTargetView(main_buffer, 0, &pView);
				rec = event_mail.bind(self_ref, [](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) -> bool { return false; });
				
				ID3D11RenderTargetView* RenderTargetViews[1] = { pView };
				ID3D11DepthStencilView* DepthTargetView = pDepthView;

				dc->OMSetRenderTargets(1, RenderTargetViews, pDepthView);
				//clean_chain();
			}

			dx11_form::~dx11_form()
			{
				dc->ClearState();
			}
		}
	}
}