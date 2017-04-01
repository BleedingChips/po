#pragma once
#include "../win32/win32_form.h"
#include "dx11_define.h"
#include "dx11_vertex.h"
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
			Implement::resource_ptr dev;
			Implement::context_ptr dc;
			Implement::chain_ptr swap;

			CComPtr<ID3D11Texture2D> main_buffer;
			CComPtr<ID3D11RenderTargetView> pView;
			CComPtr<ID3D11Texture2D> pDepthStencilBuffer;
			CComPtr<ID3D11DepthStencilView> pDepthView;
			Dx11_form(const Dx11_initial& = Dx11_initial{});
			~Dx11_form() {};
			void tick(form_ticker& ft);
		};

		struct Dx11_ticker
		{
			Implement::resource_ptr dev;
			Implement::context_ptr dc;
			Implement::chain_ptr swap;
			CComPtr<ID3D11RenderTargetView> pView;
			CComPtr<ID3D11DepthStencilView> pDepthView;
			std::array<float, 4> clear_color = { 0.0, 0.0, 0.0, 1.0 };
			std::pair<float, UINT> depth_sten_clear = {1.0f, 0};
			Dx11_ticker(Dx11_form& Df) : dev(Df.dev), dc(Df.dc), pView(Df.pView), pDepthView(Df.pDepthView), swap(Df.swap){}
			operator Implement::resource_ptr& () { return dev; }
			operator Implement::context_ptr& () { return dc; }
		};

		class shader_loader
		{
			struct shader_loader_execute : thread_task
			{
				static std::map<std::u16string, binary::weak_ref> all_shader;
				std::u16string path;
				binary info;
				shader_loader_execute(Tool::completeness_ref cr, std::u16string p) :thread_task(std::move(cr)), path(std::move(p)) {}
				virtual bool operator()() override;
			};
			std::shared_ptr<Tool::completeness<shader_loader_execute>> request;
		public:
			operator bool() const { return request && request->is_finish(); }
			void load(form_self& fs, std::u16string p) 
			{
				if (!request)
					request = std::make_shared<Tool::completeness<shader_loader_execute>>(std::move(p));
				else
					request->path = std::move(p);
				fs.push_task(request);
			}
			Tool::optional<binary> get()
			{
				if (*this)
					return{ std::move(request->info) };
				else
					return{};
			}
			Tool::optional<binary> wait_get()
			{
				if (request)
				{
					request->wait_finish();
					if (!request->is_bad() && request->info)
						return{ std::move(request->info) };
				}
				return{};
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
