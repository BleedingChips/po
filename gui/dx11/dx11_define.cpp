#include "dx11_define.h"
#include "../../frame/define.h"
#include <fstream>
#undef max
namespace PO
{
	namespace Dx11
	{
		UINT translate_usage_to_cpu_flag(D3D11_USAGE DU)
		{
			switch (DU)
			{
			case D3D11_USAGE_DEFAULT:
				return 0;
			case D3D11_USAGE_IMMUTABLE:
				return 0;
			case D3D11_USAGE_DYNAMIC:
				return D3D11_CPU_ACCESS_WRITE;
			case D3D11_USAGE_STAGING:
				return D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
			}
			return 0;
		}

		HRESULT create_buffer(device_ptr& rp, buffer_ptr& ptr, D3D11_USAGE usage, UINT bind_flag, const void* data, size_t data_size, UINT misc_flag, size_t StructureByteStrides)
		{
			D3D11_BUFFER_DESC DBD
			{
				static_cast<UINT>(data_size),
				usage,
				bind_flag,
				translate_usage_to_cpu_flag(usage),
				misc_flag,
				static_cast<UINT>(StructureByteStrides)
			};
			D3D11_SUBRESOURCE_DATA DSD{ data, 0, 0 };
			return rp->CreateBuffer(&DBD, (data == nullptr ? nullptr : &DSD), &ptr);
		}

		static Tool::scope_lock<std::vector<D3D11_SUBRESOURCE_DATA>> subresource_buffer;

		HRESULT create_tex1_implement(device_ptr& cp, texture1D_ptr& t, DXGI_FORMAT format, size_t length, size_t miplevel, size_t count, D3D11_USAGE usage, UINT bind, UINT mis, void** data)
		{
			if (cp == nullptr || count == 0) return E_INVALIDARG;
			D3D11_TEXTURE1D_DESC DTD{ static_cast<UINT>(length), static_cast<UINT>(miplevel), static_cast<UINT>(count),
				format, usage, bind, translate_usage_to_cpu_flag(usage), mis
			};
			if (data == nullptr) return cp->CreateTexture1D(&DTD, nullptr, &t);
			if (*data == nullptr) return cp->CreateTexture1D(&DTD, nullptr, &t);
			return subresource_buffer.lock([&](decltype(subresource_buffer)::type& buffer) {
				buffer.resize(count);
				for (size_t i = 0; i < count; ++i)
					buffer[i] = D3D11_SUBRESOURCE_DATA{ data[i] };
				return cp->CreateTexture1D(&DTD, buffer.data(), &t);
			});
		}

		HRESULT create_tex2_implement(device_ptr& cp, texture2D_ptr& t, DXGI_FORMAT format, size_t width, size_t height, size_t miplevel, size_t count, size_t sample_num, size_t sample_quality, D3D11_USAGE usage, UINT bind, UINT mis, void** data, size_t* line)
		{
			if (cp == nullptr || count == 0) return E_INVALIDARG;
			D3D11_TEXTURE2D_DESC DTD{ static_cast<UINT>(width), static_cast<UINT>(height), static_cast<UINT>(miplevel), static_cast<UINT>(count),
				format, DXGI_SAMPLE_DESC{ static_cast<UINT>(sample_num), static_cast<UINT>(sample_quality) },
				usage, bind, translate_usage_to_cpu_flag(usage), mis
			};
			if (data == nullptr) return cp->CreateTexture2D(&DTD, nullptr, &t);
			if (*data == nullptr) return cp->CreateTexture2D(&DTD, nullptr, &t);
			return subresource_buffer.lock([&](decltype(subresource_buffer)::type& buffer) {
				buffer.resize(count);
				for (size_t i = 0; i < count; ++i)
					buffer[i] = D3D11_SUBRESOURCE_DATA{ data[i], static_cast<UINT>(line[i]) };
				return cp->CreateTexture2D(&DTD, buffer.data(), &t);
			});
		}

		HRESULT create_tex3_implement(device_ptr& cp, texture3D_ptr& t, DXGI_FORMAT format, size_t width, size_t height, size_t depth, size_t miplevel, D3D11_USAGE usage, UINT bind, UINT mis, void* data, size_t line, size_t slice)
		{
			if (cp == nullptr) return E_INVALIDARG;
			D3D11_TEXTURE3D_DESC DTD{ static_cast<UINT>(width), static_cast<UINT>(height), static_cast<UINT>(depth), static_cast<UINT>(miplevel),
				format, usage, bind, translate_usage_to_cpu_flag(usage), mis
			};
			if (data == nullptr) return cp->CreateTexture3D(&DTD, nullptr, &t);
			D3D11_SUBRESOURCE_DATA DSD{ data, static_cast<UINT>(line), static_cast<UINT>(slice) };
			return cp->CreateTexture3D(&DTD, &DSD, &t);
		}

		HRESULT cast_SRV(device_ptr& cp, SRV_ptr& rtv, const buffer_ptr& t, DXGI_FORMAT DF, size_t element_start, size_t element_count)
		{
			if (cp == nullptr || t == nullptr) return E_INVALIDARG;
			D3D11_SHADER_RESOURCE_VIEW_DESC DSRVD{ DF, D3D11_SRV_DIMENSION_BUFFER };
			DSRVD.Buffer = D3D11_BUFFER_SRV{ static_cast<UINT>(element_start), static_cast<UINT>(element_count) };
			return cp->CreateShaderResourceView(t, &DSRVD, &rtv);
		}

		HRESULT cast_SRV_structured(device_ptr& cp, SRV_ptr& rtv, const buffer_ptr& t, size_t bit_offset, size_t element_count)
		{
			if (cp == nullptr || t == nullptr) return E_INVALIDARG;
			D3D11_BUFFER_DESC DBD;
			t->GetDesc(&DBD);
			if ((DBD.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED) != D3D11_RESOURCE_MISC_BUFFER_STRUCTURED) return false;
			D3D11_SHADER_RESOURCE_VIEW_DESC DSRVD{ DXGI_FORMAT::DXGI_FORMAT_UNKNOWN, D3D11_SRV_DIMENSION_BUFFER };
			DSRVD.Buffer = D3D11_BUFFER_SRV{ static_cast<UINT>(bit_offset), static_cast<UINT>(element_count) };
			return cp->CreateShaderResourceView(t, &DSRVD, &rtv);
		}

		HRESULT cast_SRV(device_ptr& cp, SRV_ptr& rtv, const texture1D_ptr& t, size_t most_detailed_mip, size_t miplevel)
		{
			if (cp == nullptr || t == nullptr) return E_INVALIDARG;
			D3D11_TEXTURE1D_DESC DTD;
			t->GetDesc(&DTD);
			D3D11_SHADER_RESOURCE_VIEW_DESC DSRVD{ DTD.Format, D3D11_SRV_DIMENSION_TEXTURE1D };
			DSRVD.Texture1D = D3D11_TEX1D_SRV{ static_cast<UINT>(most_detailed_mip), static_cast<UINT>(miplevel) };
			return cp->CreateShaderResourceView(t, &DSRVD, &rtv);
		}

		HRESULT cast_SRV_array(device_ptr& cp, SRV_ptr& rtv, const texture1D_ptr& t, size_t most_detailed_mip, size_t miplevel, size_t array_start, size_t array_count)
		{
			if (cp == nullptr || t == nullptr) return E_INVALIDARG;
			D3D11_TEXTURE1D_DESC DTD;
			t->GetDesc(&DTD);
			D3D11_SHADER_RESOURCE_VIEW_DESC DSRVD{ DTD.Format, D3D11_SRV_DIMENSION_TEXTURE1DARRAY };
			DSRVD.Texture1DArray = D3D11_TEX1D_ARRAY_SRV{ static_cast<UINT>(most_detailed_mip), static_cast<UINT>(miplevel), static_cast<UINT>(array_start), static_cast<UINT>(array_count) };
			return cp->CreateShaderResourceView(t, &DSRVD, &rtv);
		}

		HRESULT cast_SRV(device_ptr& cp, SRV_ptr& rtv, const texture2D_ptr& t, size_t most_detailed_mip, size_t miplevel)
		{
			if (cp == nullptr || t == nullptr) return E_INVALIDARG;
			D3D11_TEXTURE2D_DESC DTD;
			t->GetDesc(&DTD);
			D3D11_SHADER_RESOURCE_VIEW_DESC DSRVD{ DTD.Format, D3D11_SRV_DIMENSION_TEXTURE2D };
			DSRVD.Texture2D = D3D11_TEX2D_SRV{ static_cast<UINT>(most_detailed_mip), static_cast<UINT>(miplevel) };
			return cp->CreateShaderResourceView(t, &DSRVD, &rtv);
		}

		HRESULT cast_SRV_array(device_ptr& cp, SRV_ptr& rtv, const texture2D_ptr& t, size_t most_detailed_mip, size_t miplevel, size_t array_start, size_t array_count)
		{
			if (cp == nullptr || t == nullptr) return E_INVALIDARG;
			D3D11_TEXTURE2D_DESC DTD;
			t->GetDesc(&DTD);
			D3D11_SHADER_RESOURCE_VIEW_DESC DSRVD{ DTD.Format, D3D11_SRV_DIMENSION_TEXTURE2DARRAY };
			DSRVD.Texture2DArray = D3D11_TEX2D_ARRAY_SRV{ static_cast<UINT>(most_detailed_mip), static_cast<UINT>(miplevel), static_cast<UINT>(array_start), static_cast<UINT>(array_count) };
			return cp->CreateShaderResourceView(t, &DSRVD, &rtv);
		}

		HRESULT cast_SRV_ms(device_ptr& cp, SRV_ptr& rtv, const texture2D_ptr& t)
		{
			if (cp == nullptr || t == nullptr) return E_INVALIDARG;
			D3D11_TEXTURE2D_DESC DTD;
			t->GetDesc(&DTD);
			D3D11_SHADER_RESOURCE_VIEW_DESC DSRVD{ DTD.Format, D3D11_SRV_DIMENSION_TEXTURE2DMS };
			DSRVD.Texture2DMS = D3D11_TEX2DMS_SRV{};
			return cp->CreateShaderResourceView(t, &DSRVD, &rtv);
		}

		HRESULT cast_SRV_ms_array(device_ptr& cp, SRV_ptr& rtv, const texture2D_ptr& t, size_t array_start, size_t array_count)
		{
			if (cp == nullptr || t == nullptr) return E_INVALIDARG;
			D3D11_TEXTURE2D_DESC DTD;
			t->GetDesc(&DTD);
			D3D11_SHADER_RESOURCE_VIEW_DESC DSRVD{ DTD.Format, D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY };
			DSRVD.Texture2DMSArray = D3D11_TEX2DMS_ARRAY_SRV{ static_cast<UINT>(array_start), static_cast<UINT>(array_count) };
			return cp->CreateShaderResourceView(t, &DSRVD, &rtv);
		}

		HRESULT cast_SRV_cube(device_ptr& cp, SRV_ptr& rtv, const texture2D_ptr& t, size_t most_detailed_mip, size_t miplevel)
		{
			if (cp == nullptr || t == nullptr) return E_INVALIDARG;
			D3D11_TEXTURE2D_DESC DTD;
			t->GetDesc(&DTD);
			D3D11_SHADER_RESOURCE_VIEW_DESC DSRVD{ DTD.Format, D3D11_SRV_DIMENSION_TEXTURECUBE };
			DSRVD.TextureCube = D3D11_TEXCUBE_SRV{ static_cast<UINT>(most_detailed_mip), static_cast<UINT>(miplevel) };
			return cp->CreateShaderResourceView(t, &DSRVD, &rtv);
		}

		HRESULT cast_SRV_cube_array(device_ptr& cp, SRV_ptr& rtv, const texture2D_ptr& t, size_t most_detailed_mip, size_t miplevel, size_t array_start, size_t array_count)
		{
			if (cp == nullptr || t == nullptr) return E_INVALIDARG;
			D3D11_TEXTURE2D_DESC DTD;
			t->GetDesc(&DTD);
			D3D11_SHADER_RESOURCE_VIEW_DESC DSRVD{ DTD.Format, D3D11_SRV_DIMENSION_TEXTURECUBEARRAY };
			DSRVD.TextureCubeArray = D3D11_TEXCUBE_ARRAY_SRV{ static_cast<UINT>(most_detailed_mip), static_cast<UINT>(miplevel), static_cast<UINT>(array_start), static_cast<UINT>(array_count) };
			return cp->CreateShaderResourceView(t, &DSRVD, &rtv);
		}

		HRESULT cast_SRV(device_ptr& cp, SRV_ptr& rtv, const texture3D_ptr& t, size_t most_detailed_mip, size_t miplevel)
		{
			if (cp == nullptr || t == nullptr) return E_INVALIDARG;
			D3D11_TEXTURE3D_DESC DTD;
			t->GetDesc(&DTD);
			D3D11_SHADER_RESOURCE_VIEW_DESC DSRVD{ DTD.Format, D3D11_SRV_DIMENSION_TEXTURE3D };
			DSRVD.Texture3D = D3D11_TEX3D_SRV{ static_cast<UINT>(most_detailed_mip), static_cast<UINT>(miplevel) };
			return cp->CreateShaderResourceView(t, &DSRVD, &rtv);
		}

		HRESULT cast_RTV(device_ptr& cp, RTV_ptr& rtv, const texture1D_ptr& t, size_t miplevel)
		{
			if (cp == nullptr || t == nullptr) return E_INVALIDARG;
			D3D11_TEXTURE1D_DESC DTD;
			t->GetDesc(&DTD);
			D3D11_RENDER_TARGET_VIEW_DESC DRTVD{ DTD.Format, D3D11_RTV_DIMENSION_TEXTURE1D };
			DRTVD.Texture1D = D3D11_TEX1D_RTV{ static_cast<UINT>(miplevel) };
			return cp->CreateRenderTargetView(t, &DRTVD, &rtv);
		}

		HRESULT cast_RTV_array(device_ptr& cp, RTV_ptr& rtv, const texture1D_ptr& t, size_t miplevel, size_t array_start, size_t array_count)
		{
			if (cp == nullptr || t == nullptr) return E_INVALIDARG;
			D3D11_TEXTURE1D_DESC DTD;
			t->GetDesc(&DTD);
			D3D11_RENDER_TARGET_VIEW_DESC DRTVD{ DTD.Format, D3D11_RTV_DIMENSION_TEXTURE1DARRAY };
			DRTVD.Texture1DArray = D3D11_TEX1D_ARRAY_RTV{ static_cast<UINT>(miplevel), static_cast<UINT>(array_start), static_cast<UINT>(array_count) };
			return cp->CreateRenderTargetView(t, &DRTVD, &rtv);
		}

		HRESULT cast_RTV(device_ptr& cp, RTV_ptr& rtv, const texture2D_ptr& t, size_t miplevel)
		{
			if (cp == nullptr || t == nullptr) return E_INVALIDARG;
			D3D11_TEXTURE2D_DESC DTD;
			t->GetDesc(&DTD);
			D3D11_RENDER_TARGET_VIEW_DESC DRTVD{ DTD.Format, D3D11_RTV_DIMENSION_TEXTURE2D };
			DRTVD.Texture2D = D3D11_TEX2D_RTV{ static_cast<UINT>(miplevel) };
			return cp->CreateRenderTargetView(t, &DRTVD, &rtv);
		}

		HRESULT cast_RTV_array(device_ptr& cp, RTV_ptr& rtv, const texture2D_ptr& t, size_t miplevel, size_t array_start, size_t array_count)
		{
			if (cp == nullptr || t == nullptr) return E_INVALIDARG;
			D3D11_TEXTURE2D_DESC DTD;
			t->GetDesc(&DTD);
			D3D11_RENDER_TARGET_VIEW_DESC DRTVD{ DTD.Format, D3D11_RTV_DIMENSION_TEXTURE2DARRAY };
			DRTVD.Texture2DArray = D3D11_TEX2D_ARRAY_RTV{ static_cast<UINT>(miplevel), static_cast<UINT>(array_start), static_cast<UINT>(array_count) };
			return cp->CreateRenderTargetView(t, &DRTVD, &rtv);
		}

		HRESULT cast_RTV_ms(device_ptr& cp, RTV_ptr& rtv, const texture2D_ptr& t)
		{
			if (cp == nullptr || t == nullptr) return E_INVALIDARG;
			D3D11_TEXTURE2D_DESC DTD;
			t->GetDesc(&DTD);
			D3D11_RENDER_TARGET_VIEW_DESC DRTVD{ DTD.Format, D3D11_RTV_DIMENSION_TEXTURE2DMS };
			DRTVD.Texture2DMS = D3D11_TEX2DMS_RTV{};
			return cp->CreateRenderTargetView(t, &DRTVD, &rtv);
		}

		HRESULT cast_RTV_ms_array(device_ptr& cp, RTV_ptr& rtv, const texture2D_ptr& t, size_t array_start, size_t array_count)
		{
			if (cp == nullptr || t == nullptr) return E_INVALIDARG;
			D3D11_TEXTURE2D_DESC DTD;
			t->GetDesc(&DTD);
			D3D11_RENDER_TARGET_VIEW_DESC DRTVD{ DTD.Format, D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY };
			DRTVD.Texture2DMSArray = D3D11_TEX2DMS_ARRAY_RTV{ static_cast<UINT>(array_start), static_cast<UINT>(array_count) };
			return cp->CreateRenderTargetView(t, &DRTVD, &rtv);
		}

		HRESULT cast_RTV(device_ptr& cp, RTV_ptr& rtv, const texture3D_ptr& t, size_t miplevel, size_t start_z, size_t z_count)
		{
			if (cp == nullptr || t == nullptr) return E_INVALIDARG;
			D3D11_TEXTURE3D_DESC DTD;
			t->GetDesc(&DTD);
			D3D11_RENDER_TARGET_VIEW_DESC DRTVD{ DTD.Format, D3D11_RTV_DIMENSION_TEXTURE3D };
			DRTVD.Texture3D = D3D11_TEX3D_RTV{ static_cast<UINT>(miplevel), static_cast<UINT>(start_z),  static_cast<UINT>(z_count) };
			return cp->CreateRenderTargetView(t, &DRTVD, &rtv);
		}

		HRESULT cast_DSV(device_ptr& cp, DSV_ptr& dsv, const texture1D_ptr& t, size_t miplevel)
		{
			if (cp == nullptr || t == nullptr) return E_INVALIDARG;
			D3D11_TEXTURE1D_DESC DTD;
			t->GetDesc(&DTD);
			D3D11_DEPTH_STENCIL_VIEW_DESC DRTVD{ DTD.Format, D3D11_DSV_DIMENSION_TEXTURE1D, 0 };
			DRTVD.Texture1D = D3D11_TEX1D_DSV{ static_cast<UINT>(miplevel) };
			return cp->CreateDepthStencilView(t, &DRTVD, &dsv);
		}

		HRESULT cast_DSV_array(device_ptr& cp, DSV_ptr& dsv, const texture1D_ptr& t, size_t miplevel, size_t array_start, size_t array_count)
		{
			if (cp == nullptr || t == nullptr) return E_INVALIDARG;
			D3D11_TEXTURE1D_DESC DTD;
			t->GetDesc(&DTD);
			D3D11_DEPTH_STENCIL_VIEW_DESC DRTVD{ DTD.Format, D3D11_DSV_DIMENSION_TEXTURE1DARRAY, 0 };
			DRTVD.Texture1DArray = D3D11_TEX1D_ARRAY_DSV{ static_cast<UINT>(miplevel), static_cast<UINT>(array_start), static_cast<UINT>(array_count) };
			return cp->CreateDepthStencilView(t, &DRTVD, &dsv);
		}

		HRESULT cast_DSV(device_ptr& cp, DSV_ptr& dsv, const texture2D_ptr& t, size_t miplevel)
		{
			if (cp == nullptr || t == nullptr) return E_INVALIDARG;
			D3D11_TEXTURE2D_DESC DTD;
			t->GetDesc(&DTD);
			D3D11_DEPTH_STENCIL_VIEW_DESC DRTVD{ DTD.Format, D3D11_DSV_DIMENSION_TEXTURE2D, 0 };
			DRTVD.Texture2D = D3D11_TEX2D_DSV{ static_cast<UINT>(miplevel) };
			return cp->CreateDepthStencilView(t, &DRTVD, &dsv);
		}

		HRESULT cast_DSV_array(device_ptr& cp, DSV_ptr& dsv, const texture2D_ptr& t, size_t miplevel, size_t array_start, size_t array_count)
		{
			if (cp == nullptr || t == nullptr) return E_INVALIDARG;
			D3D11_TEXTURE2D_DESC DTD;
			t->GetDesc(&DTD);
			D3D11_DEPTH_STENCIL_VIEW_DESC DRTVD{ DTD.Format, D3D11_DSV_DIMENSION_TEXTURE2DARRAY, 0 };
			DRTVD.Texture2DArray = D3D11_TEX2D_ARRAY_DSV{ static_cast<UINT>(miplevel), static_cast<UINT>(array_start), static_cast<UINT>(array_count) };
			return cp->CreateDepthStencilView(t, &DRTVD, &dsv);
		}

		HRESULT cast_DSV_ms(device_ptr& cp, DSV_ptr& dsv, const texture2D_ptr& t)
		{
			if (cp == nullptr || t == nullptr) return E_INVALIDARG;
			D3D11_TEXTURE2D_DESC DTD;
			t->GetDesc(&DTD);
			D3D11_DEPTH_STENCIL_VIEW_DESC DRTVD{ DTD.Format, D3D11_DSV_DIMENSION_TEXTURE2DMS, 0 };
			DRTVD.Texture2DMS = D3D11_TEX2DMS_DSV{};
			return cp->CreateDepthStencilView(t, &DRTVD, &dsv);
		}

		HRESULT cast_DSV_ms_array(device_ptr& cp, DSV_ptr& dsv, const texture2D_ptr& t, size_t array_start, size_t array_count)
		{
			if (cp == nullptr || t == nullptr) return E_INVALIDARG;
			D3D11_TEXTURE2D_DESC DTD;
			t->GetDesc(&DTD);
			D3D11_DEPTH_STENCIL_VIEW_DESC DRTVD{ DTD.Format, D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY, 0 };
			DRTVD.Texture2DMSArray = D3D11_TEX2DMS_ARRAY_DSV{ static_cast<UINT>(array_start), static_cast<UINT>(array_count) };
			return cp->CreateDepthStencilView(t, &DRTVD, &dsv);
		}

		HRESULT cast_DSV_readonly(device_ptr& cp, DSV_ptr& dsv, const texture1D_ptr& t, size_t miplevel, bool depth_f_stencil_t)
		{
			if (cp == nullptr || t == nullptr) return E_INVALIDARG;
			D3D11_TEXTURE1D_DESC DTD;
			t->GetDesc(&DTD);
			D3D11_DEPTH_STENCIL_VIEW_DESC DRTVD{ DTD.Format, D3D11_DSV_DIMENSION_TEXTURE1D, static_cast<UINT>((depth_f_stencil_t ? D3D11_DSV_READ_ONLY_STENCIL : D3D11_DSV_READ_ONLY_DEPTH)) };
			DRTVD.Texture1D = D3D11_TEX1D_DSV{ static_cast<UINT>(miplevel) };
			return cp->CreateDepthStencilView(t, &DRTVD, &dsv);
		}

		HRESULT cast_DSV_array_readonly(device_ptr& cp, DSV_ptr& dsv, const texture1D_ptr& t, size_t miplevel, size_t array_start, size_t array_count, bool depth_f_stencil_t)
		{
			if (cp == nullptr || t == nullptr) return E_INVALIDARG;
			D3D11_TEXTURE1D_DESC DTD;
			t->GetDesc(&DTD);
			D3D11_DEPTH_STENCIL_VIEW_DESC DRTVD{ DTD.Format, D3D11_DSV_DIMENSION_TEXTURE1DARRAY, static_cast<UINT>((depth_f_stencil_t ? D3D11_DSV_READ_ONLY_STENCIL : D3D11_DSV_READ_ONLY_DEPTH)) };
			DRTVD.Texture1DArray = D3D11_TEX1D_ARRAY_DSV{ static_cast<UINT>(miplevel), static_cast<UINT>(array_start), static_cast<UINT>(array_count) };
			return cp->CreateDepthStencilView(t, &DRTVD, &dsv);
		}

		HRESULT cast_DSV_readonly(device_ptr& cp, DSV_ptr& dsv, const texture2D_ptr& t, size_t miplevel, bool depth_f_stencil_t)
		{
			if (cp == nullptr || t == nullptr) return E_INVALIDARG;
			D3D11_TEXTURE2D_DESC DTD;
			t->GetDesc(&DTD);
			D3D11_DEPTH_STENCIL_VIEW_DESC DRTVD{ DTD.Format, D3D11_DSV_DIMENSION_TEXTURE2D, static_cast<UINT>((depth_f_stencil_t ? D3D11_DSV_READ_ONLY_STENCIL : D3D11_DSV_READ_ONLY_DEPTH)) };
			DRTVD.Texture2D = D3D11_TEX2D_DSV{ static_cast<UINT>(miplevel) };
			return cp->CreateDepthStencilView(t, &DRTVD, &dsv);
		}

		HRESULT cast_DSV_array_readonly(device_ptr& cp, DSV_ptr& dsv, const texture2D_ptr& t, size_t miplevel, size_t array_start, size_t array_count, bool depth_f_stencil_t)
		{
			if (cp == nullptr || t == nullptr) return E_INVALIDARG;
			D3D11_TEXTURE2D_DESC DTD;
			t->GetDesc(&DTD);
			D3D11_DEPTH_STENCIL_VIEW_DESC DRTVD{ DTD.Format, D3D11_DSV_DIMENSION_TEXTURE2DARRAY, static_cast<UINT>((depth_f_stencil_t ? D3D11_DSV_READ_ONLY_STENCIL : D3D11_DSV_READ_ONLY_DEPTH)) };
			DRTVD.Texture2DArray = D3D11_TEX2D_ARRAY_DSV{ static_cast<UINT>(miplevel), static_cast<UINT>(array_start), static_cast<UINT>(array_count) };
			return cp->CreateDepthStencilView(t, &DRTVD, &dsv);
		}

		HRESULT cast_DSV_ms_readonly(device_ptr& cp, DSV_ptr& dsv, const texture2D_ptr& t, bool depth_f_stencil_t)
		{
			if (cp == nullptr || t == nullptr) return E_INVALIDARG;
			D3D11_TEXTURE2D_DESC DTD;
			t->GetDesc(&DTD);
			D3D11_DEPTH_STENCIL_VIEW_DESC DRTVD{ DTD.Format, D3D11_DSV_DIMENSION_TEXTURE2DMS, static_cast<UINT>((depth_f_stencil_t ? D3D11_DSV_READ_ONLY_STENCIL : D3D11_DSV_READ_ONLY_DEPTH)) };
			DRTVD.Texture2DMS = D3D11_TEX2DMS_DSV{};
			return cp->CreateDepthStencilView(t, &DRTVD, &dsv);
		}

		HRESULT cast_DSV_ms_array_readonly(device_ptr& cp, DSV_ptr& dsv, const texture2D_ptr& t, size_t array_start, size_t array_count, bool depth_f_stencil_t)
		{
			if (cp == nullptr || t == nullptr) return E_INVALIDARG;
			D3D11_TEXTURE2D_DESC DTD;
			t->GetDesc(&DTD);
			D3D11_DEPTH_STENCIL_VIEW_DESC DRTVD{ DTD.Format, D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY, static_cast<UINT>((depth_f_stencil_t ? D3D11_DSV_READ_ONLY_STENCIL : D3D11_DSV_READ_ONLY_DEPTH)) };
			DRTVD.Texture2DMSArray = D3D11_TEX2DMS_ARRAY_DSV{ static_cast<UINT>(array_start), static_cast<UINT>(array_count) };
			return cp->CreateDepthStencilView(t, &DRTVD, &dsv);
		}

		HRESULT cast_UAV(device_ptr& dp, UAV_ptr& up, const texture2D_ptr& tp, size_t mipslice)
		{
			if (dp == nullptr || tp == nullptr) return false;
			D3D11_TEXTURE2D_DESC DTD;
			tp->GetDesc(&DTD);
			D3D11_UNORDERED_ACCESS_VIEW_DESC DUAVD{ DTD.Format, D3D11_UAV_DIMENSION_TEXTURE2D };
			DUAVD.Texture2D = D3D11_TEX2D_UAV{static_cast<UINT>(mipslice)};
			return dp->CreateUnorderedAccessView(tp, &DUAVD, &up);
		}

		HRESULT cast_UAV(device_ptr& dp, UAV_ptr& up, const texture3D_ptr& tp, size_t mipslice, size_t zslicelocation, size_t zsize)
		{
			if (dp == nullptr || tp == nullptr) return false;
			D3D11_TEXTURE3D_DESC DTD;
			tp->GetDesc(&DTD);
			D3D11_UNORDERED_ACCESS_VIEW_DESC DUAVD{ DTD.Format, D3D11_UAV_DIMENSION_TEXTURE3D };
			DUAVD.Texture3D = D3D11_TEX3D_UAV{ static_cast<UINT>(mipslice), static_cast<UINT>(zslicelocation) , static_cast<UINT>(zsize) };
			return dp->CreateUnorderedAccessView(tp, &DUAVD, &up);
		}

		void viewport_fill_texture(D3D11_VIEWPORT& DV, ID3D11Texture2D* t, float min_depth, float max_depth)
		{
			viewport_capture_texture(DV, t, 0.0f, 0.0f, 1.0f, 1.0f, min_depth, max_depth);
		}
		void viewport_capture_texture(D3D11_VIEWPORT& DV, ID3D11Texture2D* t, float top_left_x_rate, float top_left_y_rate, float x_rate, float y_rate, float min_depth, float max_depth)
		{
			if (t == nullptr)
				DV = D3D11_VIEWPORT{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
			D3D11_TEXTURE2D_DESC DTD;
			t->GetDesc(&DTD);
			float w = static_cast<float>(DTD.Width);
			float h = static_cast<float>(DTD.Height);
			DV = D3D11_VIEWPORT{ w * top_left_x_rate, h * top_left_y_rate, w * x_rate, h * y_rate, min_depth, max_depth };
		}

		/*****  raterizer_s   *******************************************************************************************/

		raterizer_s::raterizer_s() :desc(D3D11_RASTERIZER_DESC{
			D3D11_FILL_SOLID,
			D3D11_CULL_BACK,
			FALSE,
			0,0.0f, 0.0f, TRUE, FALSE,FALSE
		}) {}

		void raterizer_s::view_fill_texture(size_t solt, const texture2D_ptr& t, float min_depth, float max_depth)
		{
			if (viewports.size() <= solt)
				viewports.insert(viewports.end(), solt + 1 - viewports.size(), D3D11_VIEWPORT{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f });
			viewport_fill_texture(viewports[solt], t, min_depth, max_depth);
		}

		/*****  blend_s   ***********************************************************************************************/

		blend_s::blend_s() : desc(D3D11_BLEND_DESC{
			FALSE, FALSE, D3D11_RENDER_TARGET_BLEND_DESC{FALSE, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL}
		}), factor{ 1.0, 1.0, 1.0, 1.0 }, sample_mask(0xffffffff)
		{}

		/*****  depth_stencil_s   ****************************************************************************************/

		depth_stencil_s::depth_stencil_s() : desc(D3D11_DEPTH_STENCIL_DESC{
			TRUE, D3D11_DEPTH_WRITE_MASK_ALL, D3D11_COMPARISON_LESS, FALSE, D3D11_DEFAULT_STENCIL_READ_MASK, D3D11_DEFAULT_STENCIL_WRITE_MASK,
			D3D11_DEPTH_STENCILOP_DESC{ D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS },
			D3D11_DEPTH_STENCILOP_DESC{ D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS }
		}), stencil_ref(0) {}

		/*****  sample_s   ****************************************************************************************/
		sample_s::sample_s() : desc(D3D11_SAMPLER_DESC{
			D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP, 0.0f, 1,
			D3D11_COMPARISON_NEVER, {1.0f,1.0f,1.0f,1.0f}, -FLT_MAX, FLT_MAX
		}) {}


		/*****  output_merge_d   ****************************************************************************************/

		void output_merge_d::set_RTV(size_t o, const RTV& rv)
		{
			render_array.set(o, rv.ptr);
		}

		/*****  shader_d   ***********************************************************************************************/

		void shader_d::set_cbuffer(size_t solt, const cbuffer& c) { cbuffer_array.set(solt, c.ptr); }

		void shader_d::set_SRV(size_t solt, const SRV& p) { SRV_array.set(solt, p.ptr); }

		void shader_d::set_sample(size_t solt, const sample_d& sd) { sample_array.set(solt, sd.ptr); }

		/*****  input_assember_d   ******************************************************************************************/

		input_assember_d::input_assember_d(input_assember_d&& ia)
			:vertex_array(std::move(ia.vertex_array)), offset_array(std::move(ia.offset_array)), element_array(std::move(ia.element_array)), input_element(std::move(ia.input_element)),
			primitive(ia.primitive), index_ptr(ia.index_ptr), offset(ia.offset), format(ia.format)
		{
			ia.index_ptr = nullptr;
		}

		void input_assember_d::set_vertex(size_t solt, const vertex& v)
		{
			size_t array_size = vertex_array.size();
			vertex_array.set(solt, v.ptr);
			if (array_size >= solt)
			{
				size_t append = solt + 1 - array_size;
				offset_array.insert(offset_array.end(), append, 0);
				element_array.insert(element_array.end(), append, 0);
			}

			input_element.erase(std::remove_if(input_element.begin(), input_element.end(), [solt](D3D11_INPUT_ELEMENT_DESC& DI) {
				if (DI.InputSlot == solt)
					return true;
				return false;
			}), input_element.end());

			offset_array[solt] = v.offset;
			element_array[solt] = v.element_size;
			auto ite = input_element.insert(input_element.end(), v.layout.begin(), v.layout.end());
			for (; ite != input_element.end(); ++ite)
				ite->InputSlot = static_cast<UINT>(solt);
		}

		void input_assember_d::set_index(const index& s)
		{
			index_ptr = s.ptr;
			offset = s.offset;
			format = s.format;
		}

		void input_assember_d::set_index_vertex(size_t solt, const index_vertex& s)
		{
			size_t array_size = vertex_array.size();
			vertex_array.set(solt, s.ptr);
			if (array_size >= solt)
			{
				size_t append = solt + 1 - array_size;
				offset_array.insert(offset_array.end(), append, 0);
				element_array.insert(element_array.end(), append, 0);
			}

			input_element.erase(std::remove_if(input_element.begin(), input_element.end(), [solt](D3D11_INPUT_ELEMENT_DESC& DI) {
				if (DI.InputSlot == solt)
					return true;
				return false;
			}), input_element.end());

			offset_array[solt] = s.v_offset;
			element_array[solt] = s.v_element_size;
			auto ite = input_element.insert(input_element.end(), s.v_layout.begin(), s.v_layout.end());
			for (; ite != input_element.end(); ++ite)
				ite->InputSlot = static_cast<UINT>(solt);
			index_ptr = s.ptr;
			offset = s.i_offset;
			format = s.i_format;
		}

		/*****  compute_d   ******************************************************************************************/

		void compute_d::set_UAV(size_t solt, const UAV& up)
		{
			UAV_array.set(solt, up.ptr);
			if (solt >= offset.size())
				offset.insert(offset.end(), solt + 1 - offset.size(), 0);
			offset[solt] = up.offset;
		}
	}
}