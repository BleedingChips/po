#pragma once
#include <d3d11.h>
#pragma comment(lib,"d3d11.lib")
#include "../win32_form.h"
#include <iostream>
#include "../../../po.h"
#include "../dxgi/dxgi.h"

namespace PO
{
	namespace DX11
	{

		struct initializer :public Platform::Win32::win32_initializer
		{
			
		};

		
		struct renderer:public PO::Platform::Win32::win32_form
		{
			CComPtr<ID3D11Device> dev;
			CComPtr<ID3D11DeviceContext> dc;
			CComPtr<IDXGISwapChain> swap;
			CComPtr<ID3D11Texture2D> main_buffer;
			//event_receipt receipt;
			renderer(Tool::completeness_ref&& cpf, const initializer& it = initializer());
			~renderer();
			bool is_avalible() { return *this; }
		};
		
		class renderer_view
		{
			renderer* red;
		public:
			renderer_view(renderer* data) :red(data) {}
			renderer_view(const renderer_view& rv) :red(rv.red) {}
			void text() { std::cout << "No!!!!" << std::endl; }
		};
		
	}

	struct DX11_expand
	{
		using form = DX11::renderer;
		//using renderer_view = DX11::renderer_view;


		//inline static bool  is_renderer_avalible(Tool::completeness_ref& cpr, form& r) { return r; }
	};
}