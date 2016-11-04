#pragma once
#include "../../frame/define.h"
#include "dx11_define.h"
#include "../win32/win32_form.h"


#define DX11_THROW_IF_FAIL(s) do{ if(s!=S_OK) throw std::exception(); }while(0)

namespace PO
{
	namespace Platform
	{
		namespace Dx11
		{

			namespace Error
			{

			}



			struct dx11_init :public Platform::Win32::win32_initializer
			{

			};

			struct dx11_form : Win32::win32_form
			{
				CComPtr<ID3D11Device> dev;
				CComPtr<ID3D11DeviceContext> dc;
				CComPtr<IDXGISwapChain> swap;
				CComPtr<ID3D11Texture2D> main_buffer;
				CComPtr<ID3D11RenderTargetView> pView;
				Tool::receiption rec;
				Tool::completeness_ref self_ref;
				CComPtr<ID3D11Texture2D> pDepthStencilBuffer = 0;
				CComPtr<ID3D11DepthStencilView> pDepthView;
				Tool::mail_ts<bool(event)> event_function;
				template<typename T> void bind_event(T&& t)
				{

				}
				dx11_form(Tool::completeness_ref cpf, const dx11_init& it = dx11_init());
				~dx11_form();
			};

			struct dx11_form_interface
			{
				CComPtr<ID3D11Device> dev;
				CComPtr<ID3D11DeviceContext> dc;
				CComPtr<IDXGISwapChain> swap;
				CComPtr<ID3D11RenderTargetView> pView;
				CComPtr<ID3D11DepthStencilView> pDepthView;
			public:
				void swap_chain() 
				{
					swap->Present(0, 0);
				}
				void clean_chain(float R = 0.0f, float G = 0.0f, float B = 0.0f, float A = 1.0f) {
					float color[4] = { R,G,B,A };
					dc->ClearRenderTargetView(pView, color);
				}
				dx11_form_interface(dx11_form& df)  : dev(df.dev), dc(df.dc), swap(df.swap), pView(df.pView), pDepthView(df.pDepthView)
				{

				}
				dx11_form_interface(const dx11_form_interface&) = default;
			};

			class dx11_form_view
			{
				dx11_form& df;
			public:
				dx11_form_view(dx11_form& data) :df(data) {}
				dx11_form_view(const dx11_form_view& rv) = default;
			};
		}
	}
}