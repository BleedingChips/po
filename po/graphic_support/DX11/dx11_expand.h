#pragma once
#include <d3d11.h>
#pragma comment(lib,"d3d11.lib")
#include "../../platform/win32/win32_form.h"
#include <iostream>
namespace PO
{
	namespace DX11
	{

		struct initializer :public Platform::Win32::win32_initializer
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
				DXGI_SAMPLE_DESC
			{
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

		
		class renderer:public PO::Platform::Win32::win32_form
		{
			ID3D11Device* dev;
			ID3D11DeviceContext* dc;
			IDXGISwapChain* swap;
			//event_receipt receipt;
			typename win32_form::event_receipt er;
		public:
			renderer(Tool::completeness_protector_ref&& cpf, const initializer& it = initializer());
		};
		
		class renderer_view
		{
			renderer& red;
		public:
			renderer_view(renderer* data) :red(*data) {}
			renderer_view(const renderer_view& rv) :red(rv.red) {}
			void text() { std::cout << "No!!!!" << std::endl; }
		};
		
	}

	struct DX11_expand
	{
		using form = Tool::completeness_protector<DX11::renderer>;
		//using renderer_view = DX11::renderer_view;
		static inline bool form_avalible(form& r)
		{
			return r;
		};
		static inline void form_close(form& r)
		{
			r.close_window();
		};
		//inline static bool  is_renderer_avalible(Tool::completeness_protector_ref& cpr, form& r) { return r; }
	};
}