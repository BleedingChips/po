#pragma once
#include <Atlbase.h>
#include <vector>
#include <map>
#include <functional>
#include "dxgi_define.h"

namespace PO
{
	namespace Platform
	{
		namespace Dxgi
		{
			namespace Error
			{
				struct dxgi_error :std::exception
				{
					const char* what() const override { return "dxgi init error"; }
				};
			}

			struct mode_desc : DXGI_MODE_DESC
			{
				mode_desc() :DXGI_MODE_DESC{ 1024, 768, DXGI_RATIONAL {60,1}, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED, DXGI_MODE_SCALING_UNSPECIFIED } {}
				mode_desc(const DXGI_MODE_DESC& DMD) :DXGI_MODE_DESC(DMD) {}
			};

			struct sample_desc : DXGI_SAMPLE_DESC
			{
				sample_desc() : DXGI_SAMPLE_DESC{ 1, 0 } {}
			};

			//struct  

			struct swap_chain_desc : DXGI_SWAP_CHAIN_DESC
			{
				swap_chain_desc(HWND handle = nullptr) :DXGI_SWAP_CHAIN_DESC{ mode_desc(), sample_desc() , DXGI_USAGE_RENDER_TARGET_OUTPUT , 1, handle, true, DXGI_SWAP_EFFECT_DISCARD, 0 } {}
				swap_chain_desc(const DXGI_SWAP_CHAIN_DESC& dsc) : DXGI_SWAP_CHAIN_DESC(dsc) {}
				swap_chain_desc(const swap_chain_desc&) = default;
				swap_chain_desc& operator=(const swap_chain_desc&) = default;
			};

			struct output
			{
				CComPtr<IDXGIOutput> pOutput;
			public:
				output(CComPtr<IDXGIOutput> p) :pOutput(p) {}
				output(const output&) = default;

				/*
				DXGI_OUTDUPL_DESC get_desc()
				{
					if()
				}
				*/
			};

			class adapter
			{
				CComPtr<IDXGIAdapter1> pAdapter;
			public:
				operator IDXGIAdapter1* () { return pAdapter; }
				adapter(CComPtr<IDXGIAdapter1> ccp) :pAdapter(ccp) {}
				adapter(const adapter&) = default;

				std::vector<output> outputs();

			};

			class factor1
			{
				CComPtr<IDXGIFactory1> pFactory;
			public:
				factor1()
				{
					HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)(&pFactory));
					if (hr != S_OK)
						throw PO::Platform::Dxgi::Error::dxgi_error{};
				}
				std::vector<adapter> adapters();
			};




		}
	}
}