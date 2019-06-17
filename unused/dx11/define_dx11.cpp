#include "define_dx11.h"
#include <assert.h>
#include <array>
namespace PO::Dx11
{
	D3D11_RTV_DIMENSION translate_to_rtv_dimension(D3D11_RESOURCE_DIMENSION dim, bool is_array, bool is_mulity_sample) noexcept
	{
		switch (dim)
		{
		case D3D11_RESOURCE_DIMENSION_UNKNOWN:
			return D3D11_RTV_DIMENSION_UNKNOWN;
		case D3D11_RESOURCE_DIMENSION_BUFFER:
			return D3D11_RTV_DIMENSION_BUFFER;
		case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
			return is_array ? D3D11_RTV_DIMENSION_TEXTURE1D : D3D11_RTV_DIMENSION_TEXTURE1DARRAY;
		case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
			return is_array ?
				(is_mulity_sample ? D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY : D3D11_RTV_DIMENSION_TEXTURE2DARRAY)
				: (is_mulity_sample ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D);
		case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
			return D3D11_RTV_DIMENSION_TEXTURE3D;
		default:
			assert(false);
			return D3D11_RTV_DIMENSION_UNKNOWN;
		}
	}

	HRESULT create_device(Device** output, DeviceContext** output2, D3D_FEATURE_LEVEL& output3) noexcept
	{
		D3D_FEATURE_LEVEL lel[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_11_1 };
		D3D_FEATURE_LEVEL final_level;
		//D3D11CreateDevice
		return D3D11CreateDevice(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			D3D11_CREATE_DEVICE_BGRA_SUPPORT |
			//D3D11_CREATE_DEVICE_DISABLE_GPU_TIMEOUT | 
			D3D11_CREATE_DEVICE_DEBUG,
			lel,
			1,
			D3D11_SDK_VERSION,
			output,
			&final_level,
			output2
		);
	}

	template<typename T, size_t count> struct zero_array : std::array<T, count>
	{
		zero_array() { std::memset(std::array<T, count>::data(), 0, sizeof(T) * count); }
	};

	static zero_array<ID3D11Buffer*, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT> nullptr_cbuffer;
	static zero_array<ID3D11ShaderResourceView*, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT> nullptr_shader_resource_view;
	static zero_array<ID3D11SamplerState*, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT> nullptr_sampler_state;
	static zero_array<ID3D11UnorderedAccessView*, D3D11_1_UAV_SLOT_COUNT> nullptr_unordered_access_view;
	static zero_array<uint32_t, D3D11_1_UAV_SLOT_COUNT> nullptr_unordered_access_offset_array;
	static zero_array<ID3D11RenderTargetView*, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> nullptr_render_target;

	void clear_compute_state(DeviceContext& c)
	{
		c.CSSetConstantBuffers(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, nullptr_cbuffer.data());
		c.CSSetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, nullptr_sampler_state.data());
		c.CSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, nullptr_shader_resource_view.data());
		c.CSSetShader(nullptr, nullptr, 0);
		c.CSSetUnorderedAccessViews(0, 7, nullptr_unordered_access_view.data(), nullptr_unordered_access_offset_array.data());
	}

	void clear_stage(const shader_stage_clear_flag& flag, DeviceContext& c)
	{
		if (flag.vs)
		{
			c.VSSetConstantBuffers(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, nullptr_cbuffer.data());
			c.VSSetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, nullptr_sampler_state.data());
			c.VSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, nullptr_shader_resource_view.data());
			c.VSSetShader(nullptr, nullptr, 0);
		}
		if (flag.hs)
		{
			c.HSSetConstantBuffers(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, nullptr_cbuffer.data());
			c.HSSetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, nullptr_sampler_state.data());
			c.HSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, nullptr_shader_resource_view.data());
			c.HSSetShader(nullptr, nullptr, 0);
		}
		if (flag.ds)
		{
			c.DSSetConstantBuffers(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, nullptr_cbuffer.data());
			c.DSSetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, nullptr_sampler_state.data());
			c.DSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, nullptr_shader_resource_view.data());
			c.DSSetShader(nullptr, nullptr, 0);
		}
		if (flag.gs)
		{
			c.GSSetConstantBuffers(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, nullptr_cbuffer.data());
			c.GSSetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, nullptr_sampler_state.data());
			c.GSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, nullptr_shader_resource_view.data());
			c.GSSetShader(nullptr, nullptr, 0);
		}
		if (flag.so)
			c.SOSetTargets(0, nullptr, nullptr);
		if (flag.ps)
		{
			c.PSSetConstantBuffers(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, nullptr_cbuffer.data());
			c.PSSetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, nullptr_sampler_state.data());
			c.PSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, nullptr_shader_resource_view.data());
			c.PSSetShader(nullptr, nullptr, 0);
		}
		if(flag.om)
			c.OMSetRenderTargetsAndUnorderedAccessViews(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, nullptr_render_target.data(), nullptr, 0, D3D11_1_UAV_SLOT_COUNT, nullptr_unordered_access_view.data(), nullptr_unordered_access_offset_array.data());
	}

	HRESULT create_swap_chain(SwapChain** result, RenderTargetView** output, Device& dev, DXGI::Format format, HWND hwnd, UINT width, UINT height) noexcept
	{
		DXGI_SWAP_CHAIN_DESC1 desc{
			width , height,
			format,
			FALSE,
			DXGI_SAMPLE_DESC{ 1, 0 },
			DXGI_USAGE_RENDER_TARGET_OUTPUT,
			1,
			DXGI_SCALING_STRETCH,
			DXGI_SWAP_EFFECT_DISCARD,
			DXGI_ALPHA_MODE_UNSPECIFIED,
			//DXGI_SWAP_CHAIN_FLAG::DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
			0
		};
		CComPtr<IDXGIDevice> pDXGIDevice;
		HRESULT re = dev.QueryInterface(__uuidof(IDXGIDevice), (void **)&pDXGIDevice);
		if (SUCCEEDED(re))
		{
			CComPtr<IDXGIAdapter> pDXGIAdapter;
			re = pDXGIDevice->GetParent(__uuidof(IDXGIAdapter), (void **)&pDXGIAdapter);
			if (SUCCEEDED(re))
			{
				CComPtr<IDXGIFactory2> pIDXGIFactory2;
				re = pDXGIAdapter->GetParent(__uuidof(IDXGIFactory2), (void **)&pIDXGIFactory2);
				if (SUCCEEDED(re))
				{
					re = pIDXGIFactory2->CreateSwapChainForHwnd(&dev, hwnd, &desc, nullptr, nullptr, result);
					if (SUCCEEDED(re))
					{
						CComPtr<Resource> resource;
						HRESULT re = (*result)->GetBuffer(0, __uuidof(Resource), reinterpret_cast<void**>(&resource));
						if (SUCCEEDED(re))
						{
							return create_texture2D_rtv(output, dev, *resource, format, 0);
						}
					}
				}
			}
		}
		return re;
	}

	HRESULT create_texture2D(Texture2D** output, Device& dev, D3D11_USAGE usage, UINT bind, UINT cpu_access, DXGI::Format format, uint32_t x, uint32_t y, uint32_t mipmap, const std::byte** buffer, const uint32_t* line_count) noexcept
	{
		D3D11_TEXTURE2D_DESC desc{ x, y, mipmap, 1, format , DXGI_SAMPLE_DESC{ 1, 0 },  usage, bind, cpu_access, 0 };
		uint32_t pixel_size = DXGI::calculate_pixel_size(format);
		if (buffer != nullptr)
		{
			if (mipmap <= 1)
			{
				D3D11_SUBRESOURCE_DATA data{ *buffer, line_count[0] };
				return dev.CreateTexture2D(&desc, &data, output);
			}
			else {
				std::unique_ptr<D3D11_SUBRESOURCE_DATA[]> data_buffer{ new D3D11_SUBRESOURCE_DATA[mipmap] };
				for (UINT i = 0; i < mipmap; ++i)
					data_buffer[i] = D3D11_SUBRESOURCE_DATA{ buffer[i], line_count[i] };
				return dev.CreateTexture2D(&desc, data_buffer.get(), output);
			}
		}
		else {
			return dev.CreateTexture2D(&desc, nullptr, output);
		}
		
	}
	
	HRESULT create_unordered_access_texture2D(Texture2D** output, Device& dev, DXGI::Format format, uint32_t x, uint32_t y, uint32_t mipmap, const std::byte** buffer, const uint32_t* line_count) noexcept
	{
		return create_texture2D(output, dev, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS, 0, format, x, y, mipmap, buffer, line_count);
	}

	HRESULT create_texture2D_uav(UnorderedAccessView** output, Device& dev, Resource& r, DXGI::Format format, uint32_t mipmap) noexcept
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC desc{ format, D3D11_UAV_DIMENSION_TEXTURE2D };
		desc.Texture2D = D3D11_TEX2D_UAV{ mipmap };
		return dev.CreateUnorderedAccessView(&r, &desc, output);
	}

	HRESULT create_texture2D_rtv(RenderTargetView** output, Device& dev, Resource& resource, DXGI::Format format, UINT mipmap) noexcept
	{
		D3D11_RENDER_TARGET_VIEW_DESC rt_des{ format, D3D11_RTV_DIMENSION_TEXTURE2D };
		rt_des.Texture2D = D3D11_TEX2D_RTV{ mipmap };
		return dev.CreateRenderTargetView(&resource, &rt_des, output);
	}

	HRESULT create_texture2D_srv(ShaderResourceView** output, Device& dev, Resource& resource, DXGI::Format format, UINT mipmap_start, UINT mipmap_count) noexcept
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC sr_des{ format, D3D11_SRV_DIMENSION_TEXTURE2D };
		sr_des.Texture2D = D3D11_TEX2D_SRV{ mipmap_start , mipmap_count };
		return dev.CreateShaderResourceView(&resource, &sr_des, output);
	}

	HRESULT create_vertex(Buffer** output, Device& dev, const std::byte* buffer, UINT width) noexcept
	{
		D3D11_SUBRESOURCE_DATA DSD{ buffer };
		D3D11_BUFFER_DESC desc{ width, D3D11_USAGE_DEFAULT, D3D11_BIND_VERTEX_BUFFER, 0, 0};
		return dev.CreateBuffer(&desc, &DSD, output);
	}

	HRESULT create_index(Buffer** output, Device& dev, const std::byte* buffer, UINT width) noexcept
	{
		D3D11_SUBRESOURCE_DATA DSD{ buffer };
		D3D11_BUFFER_DESC desc{ width, D3D11_USAGE_DEFAULT, D3D11_BIND_INDEX_BUFFER, 0, 0 };
		return dev.CreateBuffer(&desc, &DSD, output);
	}

	BYTE calculate_mask(BYTE input)
	{
		switch (input)
		{
		case 1:
			return 1;
		case 3:
			return 2;
		case 7:
			return 3;
		case 15:
			return 4;
		}
		assert(false);
		return 0;
	}

	HRESULT create_gs_shader_with_stream_out(ShaderGS** output, Device& dev, const std::byte* buffer, size_t length, ShaderReflection& reflection, bool need_stream_out, UINT stream_out)
	{
		D3D11_SHADER_DESC desc;
		reflection.GetDesc(&desc);
		std::vector<D3D11_SO_DECLARATION_ENTRY> dec;
		dec.reserve(desc.OutputParameters);
		for (UINT i = 0; i < desc.OutputParameters; ++i)
		{
			D3D11_SIGNATURE_PARAMETER_DESC desc_out;
			reflection.GetOutputParameterDesc(i, &desc_out);
			dec.push_back(
				D3D11_SO_DECLARATION_ENTRY{
					desc_out.Stream,desc_out.SemanticName,desc_out.SemanticIndex,
					0, calculate_mask(desc_out.Mask), 0
				}
			);
		}
		UINT stride = 0;
		UINT output_solt = need_stream_out ? stream_out : D3D11_SO_NO_RASTERIZED_STREAM;
		return dev.CreateGeometryShaderWithStreamOutput(buffer, length,
			dec.data(), static_cast<UINT>(dec.size()), &stride, 1, output_solt, nullptr, output
		);
	}


}