#pragma once
#include "../win32/win32_form.h"
#include "dx11_frame.h"
#include "dx11_vertex.h"
#include "../dxgi/dxgi_define.h"
#include <DirectXMath.h>
#include <DirectXMathVector.inl>
#include <d3d11.h>

namespace PO
{
	namespace Dx11
	{

		struct Dx11_initial
		{

		};

		class Dx11_form : public Win32::win32_form
		{
			Win32::com_ptr<ID3D11Device> dev;
			Win32::com_ptr<ID3D11DeviceContext> dc;
			Win32::com_ptr<IDXGISwapChain> swap;
		public:
			decltype(auto) get_Dx11_device() { return dev; }
			decltype(auto) get_Dx11_context() { return dc; }
			decltype(auto) get_swap_chain() { return swap; }
			value_table mapping();
			Dx11_form(const Dx11_initial& = Dx11_initial{});
			~Dx11_form() { std::cout << "exit" << std::endl; };
			void pre_tick(duration da) {
				Win32::win32_form::pre_tick(da);
				swap->Present(0, 0);
			}
		};

		struct Dx11_viewer
		{
			Dx11_viewer(Dx11_form&) {}
		};

		/*
		struct Dx11_ticker
		{
			Win32::com_ptr<IDXGISwapChain> swap;
			creator res;
			pipeline pipe;
			tex2 back_buffer;
			viewports vp;
			//Dx11_ticker(renderer_control& rc, Dx11_form& Df);
			void update_screen() { swap->Present(0, 0); }
			operator const tex2& () const { return back_buffer; }
			operator const viewports& () const { return vp; }
			Dx11_ticker(Dx11_form&);
			
			void tick(renderer_control& rc) {
				update_screen();
			}
		};*/

		/*
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
		};*/


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
