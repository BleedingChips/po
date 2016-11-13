#include "dx11_implement.h"
namespace PO
{
	namespace Interface
	{
		bool init_dx11_initial_form_win32_initial(dx11_initial_implement*& dii, win32_initial_implement* wii) noexcept
		{
			dii = new dx11_initial_implement;
			return dii != nullptr;
		}

		void dest_dx11_initial(dx11_initial_implement*& dii) noexcept
		{
			delete dii;
			dii = nullptr;
		}

		bool init_dx11_form_form_win32_form(dx11_form_implement*& dfi, dx11_initial_implement* dii, win32_form_implement* wfi) noexcept
		{
			dfi = new dx11_form_implement();
			if (dfi == nullptr) return false;
			D3D_FEATURE_LEVEL lel[] = { D3D_FEATURE_LEVEL_11_0 };
			D3D_FEATURE_LEVEL lel2 = D3D_FEATURE_LEVEL_11_0;
			PO::Platform::Dxgi::swap_chain_desc swc(wfi->raw_handle);
			HRESULT re = D3D11CreateDeviceAndSwapChain(
				nullptr,
				D3D_DRIVER_TYPE_HARDWARE,
				nullptr,
				D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DISABLE_GPU_TIMEOUT,
				lel,
				1,
				D3D11_SDK_VERSION,
				&swc,
				&dfi->swap,
				&dfi->dev,
				&lel2,
				&dfi->dc
			);
			if (!SUCCEEDED(re)) { delete dfi; dfi = nullptr; return false; }
			re = dfi->swap->GetBuffer(0,
				__uuidof(ID3D11Texture2D),
				(void **)&dfi->main_buffer);

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
			HRESULT hr = dfi->dev->CreateTexture2D(&desc,
				0, &dfi->pDepthStencilBuffer);
			hr = dfi->dev->CreateDepthStencilView(dfi->pDepthStencilBuffer,
				0, &dfi->pDepthView);
			dfi->dev->CreateRenderTargetView(dfi->main_buffer, 0, &dfi->pView);

			ID3D11RenderTargetView* RenderTargetViews[1] = { dfi->pView };
			ID3D11DepthStencilView* DepthTargetView = dfi->pDepthView;

			dfi->dc->OMSetRenderTargets(1, RenderTargetViews, dfi->pDepthView);
			return true;
		}
		
		void dest_dx11_form(dx11_form_implement*& dfi) noexcept
		{
			dfi->dc->ClearState();
			delete dfi;
			dfi = nullptr;
		}

		void clean_dx11_form(dx11_form_implement* dfi, float R,float G,float B,float A) noexcept
		{
			float color[4] = { R,G,B,A };
			dfi->dc->ClearRenderTargetView(dfi->pView, color);
		}
		void swap_dx11_form(dx11_form_implement* dfi) noexcept
		{
			dfi->swap->Present(0, 0);
		}
	}
}