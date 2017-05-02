#include "dx11_form.h"
#include <fstream>
namespace PO
{
	namespace Dx11
	{
		Dx11_form::Dx11_form(form_control& fc, const Dx11_initial& di) : form()
		{
			D3D_FEATURE_LEVEL lel[] = { D3D_FEATURE_LEVEL_11_0 };
			D3D_FEATURE_LEVEL lel2 = D3D_FEATURE_LEVEL_11_0;
			PO::Platform::Dxgi::swap_chain_desc swc(form.raw_handle);
			HRESULT re = D3D11CreateDeviceAndSwapChain(
				nullptr,
				D3D_DRIVER_TYPE_HARDWARE,
				nullptr,
				D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DISABLE_GPU_TIMEOUT | D3D11_CREATE_DEVICE_DEBUG,
				lel,
				1,
				D3D11_SDK_VERSION,
				&swc,
				&swap,
				&dev,
				&lel2,
				&dc
			);
			Win32::Error::fail_throw(re);
			fc.pre_tick = [this](form_control& fc, duration da) {this->form.tick(fc, da); };
			/*
			Win32::Error::fail_throw(swap->GetBuffer(0,
				__uuidof(ID3D11Texture2D),
				(void **)&main_buffer));

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

			ID3D11RenderTargetView* RenderTargetViews[1] = { pView };
			ID3D11DepthStencilView* DepthTargetView = pDepthView;

			dc->OMSetRenderTargets(1, RenderTargetViews, pDepthView);

			D3D11_VIEWPORT viewport;     
			viewport.Width = static_cast<float>(desc.Width);
			viewport.Height = static_cast<float>(desc.Height);     
			viewport.MinDepth = 0.0f;
			viewport.MaxDepth = 1.0f;
			viewport.TopLeftX = 0.0f;     
			viewport.TopLeftY = 0.0f;

			dc->RSSetViewports(1, &viewport);
			*/
		}

		Dx11_ticker::Dx11_ticker(renderer_control& rc, Dx11_form& Df) : swap(Df.swap), res(Df.dev), pipe(Df.dc)
		{
			if (!SUCCEEDED(swap->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&back_buffer))) __debugbreak();
			rc.auto_bind_pos_tick([this]() {
				swap->Present(0, 0);
			});
		}
		
		/*
		bool shader_loader::shader_loader_execute::operator()()
		{
			std::ifstream file(utf16_to_asc(path), std::ios::binary | std::ios::in);
			if (file.good())
			{
				file.seekg(0, std::ios::end);
				auto end_poi = file.tellg();
				file.seekg(0, std::ios::beg);
				auto sta_poi = file.tellg();
				info = binary{ static_cast<size_t>(end_poi - sta_poi) };
				file.read(info, end_poi - sta_poi);
				info.update();
			}
			else
				set_bad();
			return false;
		}
		*/
		

	}
}







/*
namespace PO
{
	namespace Interface
	{
		Tool::optional<std::string> init_dx11_initial_form_win32_initial(dx11_initial_implement*& dii, win32_initial_implement* wii) noexcept
		{
			try {
				dii = new dx11_initial_implement;
				return{};
			}
			catch (const std::exception& ex)
			{
				return{ std::string(__FUNCDNAME__) + " : " + ex.what() };
			}
			catch (...)
			{
				return{ std::string(__FUNCDNAME__) + " : Unknow exeception"};
			}
		}

		void dest_dx11_initial(dx11_initial_implement*& dii) noexcept
		{
			delete dii;
			dii = nullptr;
		}

		Tool::optional<std::string> init_dx11_form_form_win32_form(dx11_form_implement*& dfi, dx11_initial_implement* dii, win32_form_implement* wfi) noexcept
		{
			try {
				dfi = new dx11_form_implement();
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
				Error::fail_throw(re, [&]() {delete dfi; });

				Error::fail_throw(dfi->swap->GetBuffer(0,
					__uuidof(ID3D11Texture2D),
					(void **)&dfi->main_buffer));

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
				return{};
			}
			catch (std::exception& ex)
			{
				return{ std::string(__FUNCDNAME__) + " : " + ex.what() };
			}
			catch (...)
			{
				return{ std::string(__FUNCDNAME__) + " Unknow " };
			}
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

		Tool::optional<std::string> init_dx11_renderer_from_from(dx11_renderer_implement*& dri, dx11_form_implement* dfi) noexcept
		{
			try
			{
				dri = new dx11_renderer_implement();
				dri->dc = dfi->dc;
				dri->dev = dfi->dev;
				dri->swap = dfi->swap;
				dri->pView = dfi->pView;
				return{};
			}
			catch (std::exception& ex)
			{
				return{ std::string(__FUNCDNAME__) + " : " + ex.what() };
			}
			catch (...)
			{
				return{ std::string(__FUNCDNAME__) + " Unknow " };
			}
		}

		void dest_dx11_renderer(dx11_renderer_implement*& dri) noexcept
		{
			delete dri;
			dri = nullptr;
		}

		void tick_dx11_renderer(dx11_renderer_implement* dri, duration da) noexcept
		{
			static float all_time = 0.0f;
			all_time += da.count();
			float a = abs(sin(all_time * 0.001f));
			float a2 = abs(sin(all_time * 0.001f * 2));
			float a3 = abs(sin(all_time * 0.001f * 3));
			float color[4] = { a,a2,a3,1.0f };
			dri->dc->ClearRenderTargetView(dri->pView, color);
			dri->swap->Present(0, 0);
		}
	}
}
*/