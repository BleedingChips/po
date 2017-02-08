#include "dxgi.h"
#include <iostream>


/*
IDXGIFactory1 * pFactory;
HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)(&pFactory));
for (UINT i = 0; true; ++i)
{
	IDXGIAdapter1* add = nullptr;
	hr = pFactory->EnumAdapters1(i, &add);
	if (hr == S_OK)
	{
		std::cout << "add success:" << i << std::endl;

		for (UINT k = 0;; ++k)
		{
			IDXGIOutput* out;
			hr = add->EnumOutputs(k, &out);

			if (hr != S_OK)
			{
				break;
			}
			else {
				IDXGIOutput1* out1 = nullptr;
				hr = out->QueryInterface(__uuidof(IDXGIOutput1), (void**)out1);
				if (hr != S_OK)
				{
					std::cout << "out3434234" << std::endl;
					//break;
				}
				std::cout << "out success:" << k << std::endl;
				DXGI_MODE_DESC DMD[100];
				UINT number = 90;
				for (DXGI_FORMAT p = DXGI_FORMAT_UNKNOWN; p <= DXGI_FORMAT_B4G4R4A4_UNORM; )
				{
					hr = out->GetDisplayModeList(p, DXGI_ENUM_MODES_INTERLACED, &number, nullptr);
					if (hr == S_OK && number != 0)
					{
						std::cout << PO::Dxgi::Log::translate_DXGIEnum_To_String(p) << std::endl;
					}
					p = (DXGI_FORMAT)((UINT)p + 1);
				}

				for (UINT d = 0; d < number; ++d)
				{
					cout << DMD[d].Height << ',' << DMD[d].Width << "," << DMD[d].RefreshRate.Numerator << ',' << DMD[d].RefreshRate.Denominator << "   " << PO::Dxgi::Log::translate_DXGIFormat_To_String(DMD[d].Format) << std::endl;
				}

				cout << "output :" << (hr == S_OK) << " : " << number << endl;
			}

		}

	}
	else {
		std::cout << "end at:" << i << endl;
		break;
	}
}*/


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