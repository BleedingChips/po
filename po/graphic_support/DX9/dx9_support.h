#pragma once
#include <iostream>
#include <d3d9.h>
#include <vector>
#include "../../platform/platform_window.h"
#pragma comment(lib,"d3d9.lib")
#include "dx3_debug.h"
namespace Graphic
{
	namespace DX9
	{
		/*
		class adapter
		{
			UINT index;
			D3DCAPS9 cap;
			friend class device;
			adapter(UINT i, const D3DCAPS9& c) :index(i), cap(c) {}
			adapter() {}
			friend class context;
		public:
			adapter(const adapter&) = default;
			adapter& operator= (const adapter& op) = default;
		};

		class context
		{
			static IDirect3D9* single_device_ptr;
			static size_t init_count;
			IDirect3D9* ptr;
		public:
			IDirect3D9* operator->() { return ptr; }
			context()
			{ 
				if (single_device_ptr == NULL)
				{
					single_device_ptr = Direct3DCreate9(D3D_SDK_VERSION);
				}
				++init_count;
				ptr = single_device_ptr;
			}
			~context() 
			{
				--init_count;
				if (init_count == 0 && single_device_ptr != nullptr)
				{
					single_device_ptr->Release();
					single_device_ptr = nullptr;
				}
			}
			UINT get_adapter_count() { return ptr->GetAdapterCount(); }
		};

		class device_type
		{
			D3DPRESENT_PARAMETERS dp;
			friend class device;
		public:
			device_type(Platform::window& win) :
				dp({800,600,D3DFMT_A8R8G8B8,1,D3DMULTISAMPLE_NONE,0,D3DSWAPEFFECT_DISCARD,win,true,true,D3DFMT_D24S8,0,D3DPRESENT_RATE_DEFAULT,D3DPRESENT_INTERVAL_IMMEDIATE})
			{
				//GetWindowLong(win,GWL_STYLE)
			}
		};

		class device
		{
			IDirect3DDevice9* device_;
		public:
			device(context& cot, device_type& dt)
			{
				HRESULT re = cot->CreateDevice(
					0,
					D3DDEVTYPE_HAL,
					NULL,
					D3DCREATE_HARDWARE_VERTEXPROCESSING,
					&(dt.dp),
					&device_
					);
				if (FAILED(re))
					throw re;
			}
			operator IDirect3DDevice9* () { return device_; }
			IDirect3DDevice9* operator->() { return device_; }
			~device()
			{
				device_->Release();
			}
		};
		*/
	}
}
