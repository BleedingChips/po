#include "dxgi.h"
#include <iostream>
namespace
{
	/*
	struct dxgi_factory
	{
		std::vector<PO::Platform::Dxgi::adapter> all_adapter;
		dxgi_factory()
		{
			CComPtr<IDXGIFactory1> pFactory;
			//pFactory->CreateSwapChain
			HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)(&pFactory));
			if (hr != S_OK)
				throw PO::Platform::Dxgi::Error::dxgi_error{};

			for (UINT i = 0; i< 99; ++i)
			{
				CComPtr<IDXGIAdapter1> tem_ptr;
				
				hr = pFactory->EnumAdapters1(i, &tem_ptr);

				if (hr == S_OK)
				{
					DXGI_ADAPTER_DESC1 DAD;
					tem_ptr->GetDesc1(&DAD);
					std::wcout << DAD.Description << L","<< DAD.DedicatedVideoMemory<< L","<<DAD.DedicatedSystemMemory<<L","<<DAD.SharedSystemMemory<<std::endl;
					all_adapter.push_back(PO::Platform::Dxgi::adapter{ tem_ptr });
				}
				else
					break;
			}
		}
	}main_factory;
	*/
}


namespace
{
	
}



namespace PO
{
	namespace Platform
	{
		namespace Dxgi
		{
			std::vector<adapter> factor1::adapters()
			{
				std::vector<adapter> all_adapter;
				for (UINT i = 0; ; ++i)
				{
					CComPtr<IDXGIAdapter1> tem_ptr;
					HRESULT hr = pFactory->EnumAdapters1(i, &tem_ptr);
					if (hr == S_OK)
					{
						all_adapter.push_back(PO::Platform::Dxgi::adapter{ tem_ptr });
					}
					else
						break;
				}
				return all_adapter;
			}

			std::vector<output> adapter::outputs()
			{
				std::vector<output> all_output;
				if (pAdapter != nullptr)
				{
					for (UINT i = 0; ; ++i)
					{
						CComPtr<IDXGIOutput> out;
						HRESULT hr = pAdapter->EnumOutputs(i, &out);
						if (hr == S_OK)
						{
							all_output.push_back(out);
						}
						else
							break;
					}
				}
				return all_output;
			}

		}
	}
}