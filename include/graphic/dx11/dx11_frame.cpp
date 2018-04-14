#include "dx11_frame.h"
namespace
{
	/*
	struct start_init
	{
		start_init()
		{
			PO::io_task_instance().set_function(typeid(PO::binary), [](PO::io_block ib) -> PO::Tool::any {
				if (!ib.stream.good())
					__debugbreak();
				ib.stream.seekg(0, std::ios::end);
				auto end_poi = ib.stream.tellg();
				ib.stream.seekg(0, std::ios::beg);
				auto sta_poi = ib.stream.tellg();
				PO::binary info{ static_cast<size_t>(end_poi - sta_poi) };
				ib.stream.read(info, end_poi - sta_poi);
				return std::move(info);
			});
		}
	} init;
	*/

	uint32_t translate_usage_to_cpu_flag(D3D11_USAGE DU)
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

	static PO::Tool::scope_lock<std::vector<D3D11_SUBRESOURCE_DATA>> subresource_buffer;


	DXGI_FORMAT dstex_format_to_dsview_format(DXGI_FORMAT input)
	{
		switch (input)
		{
		case DXGI_FORMAT_R16_TYPELESS:
			return DXGI_FORMAT_D16_UNORM;
		case DXGI_FORMAT_R24G8_TYPELESS:
			return DXGI_FORMAT_D24_UNORM_S8_UINT;
		case DXGI_FORMAT_R32_FLOAT:
			return DXGI_FORMAT_D32_FLOAT;
		case DXGI_FORMAT_R32G8X24_TYPELESS:
			return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
		default:
			return input;
		}
	}
	DXGI_FORMAT dstex_format_to_srview_format(DXGI_FORMAT input)
	{
		switch (input)
		{
		case DXGI_FORMAT_R16_TYPELESS:
			return DXGI_FORMAT_R16_UNORM;
		case DXGI_FORMAT_R24G8_TYPELESS:
			return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		case DXGI_FORMAT_R32_FLOAT:
			return DXGI_FORMAT_R32_FLOAT;
		case DXGI_FORMAT_R32G8X24_TYPELESS:
			return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
		default:
			return input;
		}
	}

}

namespace PO
{
	namespace Dx11
	{

		DXGI_FORMAT translate_DST_format(DST_format dsf)
		{
			switch (dsf)
			{
			case DST_format::D16:
				return DXGI_FORMAT_R16_TYPELESS;
			case DST_format::D24_UI8:
				return DXGI_FORMAT_R24G8_TYPELESS;
			case DST_format::F32:
				return DXGI_FORMAT_R32_FLOAT;
			case DST_format::F32_UI8:
				return DXGI_FORMAT_R32G8X24_TYPELESS;
			default:
				return DXGI_FORMAT_UNKNOWN;
			}
		}

		uint32_t calculate_mipmap_count(uint32_t p, uint32_t mipmap)
		{
			if (mipmap != 0)
				return mipmap;
			else {
				uint32_t count = 1;
				while (p = p >> 1)
					++count;
				return count;
			}
		}

		namespace Implement
		{

			std::optional<HRESULT> buffer_interface::create_implement(device_ptr& dev, uint32_t width, D3D11_USAGE DU, uint32_t BIND, uint32_t misc_flag, uint32_t struct_byte, const void* data) noexcept
			{
				assert(dev);
				D3D11_BUFFER_DESC DBD
				{
					width,
					DU,
					BIND,
					::translate_usage_to_cpu_flag(DU),
					misc_flag,
					struct_byte
				};
				D3D11_SUBRESOURCE_DATA DSD{ data, 0, 0 };
				PO::Win32::com_ptr<ID3D11Buffer> tem;
				HRESULT re = dev->CreateBuffer(&DBD, (data == nullptr ? nullptr : &DSD), tem.adress());
				if (SUCCEEDED(re))
				{
					ptr = tem;
					return {};
				}
				return re;
			}

			uint32_t tex1_interface::size() const
			{
				if (ptr)
				{
					D3D11_TEXTURE1D_DESC DTD;
					ptr->GetDesc(&DTD);
					return DTD.Width;
				}
				return 0;
			}

			std::optional<HRESULT> tex1_interface::create_implement(device_ptr& dev, DXGI_FORMAT DF, uint32_t length, uint32_t miplevel, uint32_t count, D3D11_USAGE DU, uint32_t BIND, uint32_t misc, const tex1_source* source) noexcept
			{
				D3D11_TEXTURE1D_DESC DTD{ static_cast<uint32_t>(length), static_cast<uint32_t>(miplevel), static_cast<uint32_t>(count),
					DF, DU, BIND, translate_usage_to_cpu_flag(DU), misc
				};
				Win32::com_ptr<ID3D11Texture1D> tem_ptr;
				HRESULT re;
				if (source == nullptr)
					re = dev->CreateTexture1D(&DTD, nullptr, tem_ptr.adress());
				else {
					re = subresource_buffer.lock([&](decltype(subresource_buffer)::type& buffer) {
						buffer.resize(calculate_mipmap_count(length, miplevel) * count);
						for (size_t i = 0; i < count; ++i)
							buffer[i] = source[i];
						return dev->CreateTexture1D(&DTD, buffer.data(), tem_ptr.adress());
					});
				}
				if (SUCCEEDED(re))
				{
					ptr = tem_ptr;
					return {};
				}
				return re;
			}

			uint2 tex2_interface::size() const
			{
				if (ptr)
				{
					D3D11_TEXTURE2D_DESC DTD;
					ptr->GetDesc(&DTD);
					return { DTD.Width, DTD.Height };
				}
				return { 0 , 0 };
			}

			std::optional<HRESULT> tex2_interface::create_implement(device_ptr&  dev, DXGI_FORMAT DF, uint32_t2 size, uint32_t miplevel, uint32_t count, uint32_t sample_num, uint32_t sample_quality, D3D11_USAGE usage, uint32_t bind, uint32_t mis, const tex2_source* source) noexcept
			{
				D3D11_TEXTURE2D_DESC DTD{ size.x, size.y, miplevel, count, DF, DXGI_SAMPLE_DESC{ sample_num, sample_quality },
					usage, bind, translate_usage_to_cpu_flag(usage), mis
				};
				Win32::com_ptr<ID3D11Texture2D> tem_ptr;
				HRESULT re;
				if (source == nullptr)
					re = dev->CreateTexture2D(&DTD, nullptr, tem_ptr.adress());
				else
					re = subresource_buffer.lock([&](decltype(subresource_buffer)::type& buffer) {
					buffer.resize(calculate_mipmap_count(size, miplevel) * count);
					for (size_t i = 0; i < count; ++i)
						buffer[i] = source[i];
					return dev->CreateTexture2D(&DTD, buffer.data(), tem_ptr.adress());
				});
				if (SUCCEEDED(re))
				{
					ptr = tem_ptr;
					return {};
				}
				return re;
			}

			uint3 tex3_interface::size() const
			{
				if (ptr)
				{
					D3D11_TEXTURE3D_DESC DTD;
					ptr->GetDesc(&DTD);
					return { DTD.Width, DTD.Height, DTD.Depth };
				}
				return { 0, 0, 0 };
			}

			std::optional<HRESULT> tex3_interface::create_implement(device_ptr& c, DXGI_FORMAT DF, uint32_t3 size, uint32_t miplevel, D3D11_USAGE usage, uint32_t bind, uint32_t mis, const tex3_source* source) noexcept
			{
				D3D11_TEXTURE3D_DESC DTD{ size.x, size.y, size.z, miplevel,
					DF, usage, bind, translate_usage_to_cpu_flag(usage), mis
				};
				Win32::com_ptr<ID3D11Texture3D> tem_ptr;
				HRESULT re;
				if (source == nullptr) re = c->CreateTexture3D(&DTD, nullptr, tem_ptr.adress());
				else re = subresource_buffer.lock([&](decltype(subresource_buffer)::type& buffer) {
					buffer.resize(calculate_mipmap_count(size, miplevel));
					for (size_t i = 0; i < miplevel; ++i)
						buffer[i] = source[i];
					return c->CreateTexture3D(&DTD, buffer.data(), tem_ptr.adress());
				});
				if (SUCCEEDED(re)) {
					ptr = tem_ptr;
					return {};
				}
				return re;
			}
		}
		
		// buffer_structured **************************************
		shader_resource_view<buffer_structured> buffer_structured::cast_shader_resource_view(device_ptr& c, std::optional<uint32_t2> range) const
		{
			shader_resource_view<buffer_structured> tem;
			D3D11_SHADER_RESOURCE_VIEW_DESC DSRVD{ DXGI_FORMAT::DXGI_FORMAT_UNKNOWN, D3D11_SRV_DIMENSION_BUFFER };
			if (range)
				DSRVD.Buffer = D3D11_BUFFER_SRV{ range->x, range->y };
			else {
				D3D11_BUFFER_DESC DBD;
				ptr->GetDesc(&DBD);
				DSRVD.Buffer = D3D11_BUFFER_SRV{ 0, DBD.ByteWidth / DBD.StructureByteStride };
			}
			HRESULT re = c->CreateShaderResourceView(ptr, &DSRVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		unordered_access_view<buffer_structured> buffer_structured::cast_unordered_access_view(device_ptr& c, std::optional<uint32_t2> elemnt_start_and_count, std::optional<uint32_t> offset, bool is_append_or_consume) const
		{
			unordered_access_view<buffer_structured> tem;
			tem.offset = offset.value_or(-1);
			D3D11_UNORDERED_ACCESS_VIEW_DESC DUAVD{ DXGI_FORMAT_UNKNOWN, D3D11_UAV_DIMENSION_BUFFER };
			uint32_t2 element;
			if (elemnt_start_and_count)
				element = *elemnt_start_and_count;
			else {
				D3D11_BUFFER_DESC DBD;
				ptr->GetDesc(&DBD);
				element = uint32_t2{ 0, DBD.ByteWidth / DBD.StructureByteStride };
			}
			DUAVD.Buffer = D3D11_BUFFER_UAV{ element.x, element.y, static_cast<uint32_t>((is_append_or_consume ? D3D11_BUFFER_UAV_FLAG_APPEND : D3D11_BUFFER_UAV_FLAG_COUNTER)) };
			HRESULT re = c->CreateUnorderedAccessView(ptr, &DUAVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		// buffer_constant ***************************************
		static Tool::scope_lock<std::vector<char>> cbuffer_buffer;
		bool buffer_constant::create_raw(device_ptr& c, uint32_t width, const void* data)
		{
			uint32_t aligned_size = (width + 15) & ~(uint32_t{ 15 });
			aligned_size = aligned_size >= 128 ? aligned_size : 128;
			if (aligned_size != width && data != nullptr)
			{
				return cbuffer_buffer.lock([&, this](decltype(cbuffer_buffer)::type& b) {
					b.resize(aligned_size, '0');
					for (size_t i = 0; i < width; ++i)
						b[i] = static_cast<const char*>(data)[i];
					return Implement::buffer_interface::create_implement(c, aligned_size, (data == nullptr ? D3D11_USAGE_DEFAULT : D3D11_USAGE_IMMUTABLE), D3D11_BIND_CONSTANT_BUFFER, 0, 0, b.data());
				}).has_value();
			}
			else
				return Implement::buffer_interface::create_implement(c, aligned_size, (data == nullptr ? D3D11_USAGE_DEFAULT : D3D11_USAGE_IMMUTABLE), D3D11_BIND_CONSTANT_BUFFER, 0, 0, data).has_value();
		}

		// tex1 **************************************
		shader_resource_view<tex1> tex1::cast_shader_resource_view_as_format(device_ptr& c, DXGI_FORMAT DF, std::optional<uint32_t2> miplevel_range) const
		{
			shader_resource_view<tex1> tem;
			D3D11_TEXTURE1D_DESC DTD;
			ptr->GetDesc(&DTD);
			D3D11_SHADER_RESOURCE_VIEW_DESC DSRVD{ DTD.Format, D3D11_SRV_DIMENSION_TEXTURE1D };
			if (DF == DXGI_FORMAT_UNKNOWN)
			{
				if (av == ViewType::DS)
					DSRVD.Format = dstex_format_to_srview_format(DTD.Format);
			}
			else
				DSRVD.Format = DF;
			uint32_t2 mip = miplevel_range.value_or(uint32_t2{ 0, DTD.MipLevels });
			DSRVD.Texture1D = D3D11_TEX1D_SRV{ mip.x, mip.y };
			HRESULT re = c->CreateShaderResourceView(ptr, &DSRVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		depth_stencil_view<tex1> tex1::cast_depth_stencil_view(device_ptr& c, uint32_t miplevel) const
		{
			depth_stencil_view<tex1> tem;
			D3D11_TEXTURE1D_DESC DTD;
			ptr->GetDesc(&DTD);
			D3D11_DEPTH_STENCIL_VIEW_DESC DDSVD{ dstex_format_to_dsview_format(DTD.Format), D3D11_DSV_DIMENSION_TEXTURE1D, 0 };
			DDSVD.Texture1D = D3D11_TEX1D_DSV{ miplevel };
			HRESULT re = c->CreateDepthStencilView(ptr, &DDSVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		render_target_view<tex1> tex1::cast_render_target_view_as_format(device_ptr& c, DXGI_FORMAT DF, uint32_t miplevel) const
		{
			render_target_view<tex1> tem;
			D3D11_TEXTURE1D_DESC DTD;
			ptr->GetDesc(&DTD);
			D3D11_RENDER_TARGET_VIEW_DESC DRTVD{ DF == DXGI_FORMAT_UNKNOWN ? DTD.Format : DF, D3D11_RTV_DIMENSION_TEXTURE1D };
			DRTVD.Texture1D = D3D11_TEX1D_RTV{ miplevel };
			HRESULT re = c->CreateRenderTargetView(ptr, &DRTVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		unordered_access_view<tex1> tex1::cast_unordered_access_view_as_format(device_ptr& c, DXGI_FORMAT DF, uint32_t miplevel) const
		{
			unordered_access_view<tex1> tem;
			D3D11_TEXTURE1D_DESC DTD;
			ptr->GetDesc(&DTD);
			D3D11_UNORDERED_ACCESS_VIEW_DESC DUAVD{ DF == DXGI_FORMAT_UNKNOWN ? DTD.Format : DF, D3D11_UAV_DIMENSION_TEXTURE1D };
			DUAVD.Texture1D = D3D11_TEX1D_UAV{ miplevel };
			HRESULT re = c->CreateUnorderedAccessView(ptr, &DUAVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		// tex1_array **************************************
		shader_resource_view<tex1_array> tex1_array::cast_shader_resource_view_as_format(device_ptr& c, DXGI_FORMAT DF, std::optional<uint32_t2> miplevel_range, std::optional<uint32_t2> array_range) const
		{
			shader_resource_view<tex1_array> tem;
			D3D11_TEXTURE1D_DESC DTD;
			ptr->GetDesc(&DTD);
			D3D11_SHADER_RESOURCE_VIEW_DESC DSRVD{ DTD.Format, D3D11_SRV_DIMENSION_TEXTURE1DARRAY };
			if (DF == DXGI_FORMAT_UNKNOWN)
			{
				if (av == ViewType::DS)
					DSRVD.Format = dstex_format_to_srview_format(DTD.Format);
			}
			else
				DSRVD.Format = DF;
			uint32_t2 mip = miplevel_range.value_or(uint32_t2{ 0, DTD.MipLevels });
			uint32_t2 arr = array_range.value_or(uint32_t2{ 0, DTD.ArraySize });
			DSRVD.Texture1DArray = D3D11_TEX1D_ARRAY_SRV{ mip.x, mip.y, arr.x, arr.y };
			HRESULT re = c->CreateShaderResourceView(ptr, &DSRVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		depth_stencil_view<tex1_array> tex1_array::cast_depth_stencil_view(device_ptr& c, uint32_t miplevel, std::optional<uint32_t2> array_start_and_count) const
		{
			depth_stencil_view<tex1_array> tem;
			D3D11_TEXTURE1D_DESC DTD;
			ptr->GetDesc(&DTD);
			D3D11_DEPTH_STENCIL_VIEW_DESC DDSVD{ dstex_format_to_dsview_format(DTD.Format), D3D11_DSV_DIMENSION_TEXTURE1DARRAY, 0 };
			uint32_t2 array_size = array_start_and_count.value_or(uint32_t2{ 0, DTD.ArraySize });
			DDSVD.Texture1DArray = D3D11_TEX1D_ARRAY_DSV{ miplevel, array_size.x, array_size.y};
			HRESULT re = c->CreateDepthStencilView(ptr, &DDSVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		render_target_view<tex1_array> tex1_array::cast_render_target_view_as_format(device_ptr& c, DXGI_FORMAT DF, uint32_t miplevel, std::optional<uint32_t2> array_start_and_count) const
		{
			render_target_view<tex1_array> tem;
			D3D11_TEXTURE1D_DESC DTD;
			ptr->GetDesc(&DTD);
			D3D11_RENDER_TARGET_VIEW_DESC DRTVD{ DF == DXGI_FORMAT_UNKNOWN ? DTD.Format : DF, D3D11_RTV_DIMENSION_TEXTURE1DARRAY };
			uint32_t2 array_size = array_start_and_count.value_or(uint32_t2{ 0, DTD.ArraySize });
			DRTVD.Texture1DArray = D3D11_TEX1D_ARRAY_RTV{ miplevel, array_size.x, array_size.y };
			HRESULT re = c->CreateRenderTargetView(ptr, &DRTVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		unordered_access_view<tex1_array> tex1_array::cast_unordered_access_view_as_format(device_ptr& c, DXGI_FORMAT DF, uint32_t miplevel, std::optional<uint32_t2> array_start_and_count) const
		{
			unordered_access_view<tex1_array> tem;
			D3D11_TEXTURE1D_DESC DTD;
			ptr->GetDesc(&DTD);
			D3D11_UNORDERED_ACCESS_VIEW_DESC DUAVD{ DF == DXGI_FORMAT_UNKNOWN ? DTD.Format : DF, D3D11_UAV_DIMENSION_TEXTURE1DARRAY };
			uint32_t2 array_size = array_start_and_count.value_or(uint32_t2{ 0, DTD.ArraySize });
			DUAVD.Texture1DArray = D3D11_TEX1D_ARRAY_UAV{ miplevel, array_size.x, array_size.y };
			HRESULT re = c->CreateUnorderedAccessView(ptr, &DUAVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		// tex2 **************************************
		shader_resource_view<tex2> tex2::cast_shader_resource_view_as_format(device_ptr& c, DXGI_FORMAT DF, std::optional<uint32_t2> miplevel_range) const
		{
			shader_resource_view<tex2> tem;
			D3D11_TEXTURE2D_DESC DTD;
			ptr->GetDesc(&DTD);
			D3D11_SHADER_RESOURCE_VIEW_DESC DSRVD{ DTD.Format, D3D11_SRV_DIMENSION_TEXTURE2D };
			if (DF == DXGI_FORMAT_UNKNOWN)
			{
				if (av == ViewType::DS)
					DSRVD.Format = dstex_format_to_srview_format(DTD.Format);
			}
			else
				DSRVD.Format = DF;
			uint32_t2 mip = miplevel_range.value_or(uint32_t2{ 0, DTD.MipLevels });
			DSRVD.Texture2D = D3D11_TEX2D_SRV{ mip.x, mip.y };
			HRESULT re = c->CreateShaderResourceView(ptr, &DSRVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		depth_stencil_view<tex2> tex2::cast_depth_stencil_view(device_ptr& c, uint32_t miplevel) const
		{
			depth_stencil_view<tex2> tem;
			D3D11_TEXTURE2D_DESC DTD;
			ptr->GetDesc(&DTD);
			D3D11_DEPTH_STENCIL_VIEW_DESC DDSVD{ dstex_format_to_dsview_format(DTD.Format), D3D11_DSV_DIMENSION_TEXTURE2D, 0 };
			DDSVD.Texture2D = D3D11_TEX2D_DSV{ miplevel };
			HRESULT re = c->CreateDepthStencilView(ptr, &DDSVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		render_target_view<tex2> tex2::cast_render_target_view_as_format(device_ptr& c, DXGI_FORMAT DF, uint32_t miplevel) const
		{ 
			render_target_view<tex2> tem;
			D3D11_TEXTURE2D_DESC DTD;
			ptr->GetDesc(&DTD);
			D3D11_RENDER_TARGET_VIEW_DESC DRTVD{ DF == DXGI_FORMAT_UNKNOWN ? DTD.Format : DF, D3D11_RTV_DIMENSION_TEXTURE2D };
			DRTVD.Texture2D = D3D11_TEX2D_RTV{ miplevel };
			HRESULT re = c->CreateRenderTargetView(ptr, &DRTVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		unordered_access_view<tex2> tex2::cast_unordered_access_view_as_format(device_ptr& c, DXGI_FORMAT DF, uint32_t miplevel)const
		{
			unordered_access_view<tex2> tem;
			D3D11_TEXTURE2D_DESC DTD;
			ptr->GetDesc(&DTD);
			D3D11_UNORDERED_ACCESS_VIEW_DESC DUAVD{ DF == DXGI_FORMAT_UNKNOWN ? DTD.Format : DF, D3D11_UAV_DIMENSION_TEXTURE2D };
			DUAVD.Texture2D = D3D11_TEX2D_UAV{ miplevel };
			HRESULT re = c->CreateUnorderedAccessView(ptr, &DUAVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		// tex2_array ************************************************
		shader_resource_view<tex2_array> tex2_array::cast_shader_resource_view_as_format(device_ptr& c, DXGI_FORMAT DF, std::optional<uint32_t2> miplevel_range, std::optional<uint32_t2> array_range)const
		{
			shader_resource_view<tex2_array> tem;
			D3D11_TEXTURE2D_DESC DTD;
			ptr->GetDesc(&DTD);
			D3D11_SHADER_RESOURCE_VIEW_DESC DSRVD{ DTD.Format, D3D11_SRV_DIMENSION_TEXTURE2DARRAY };
			if (DF == DXGI_FORMAT_UNKNOWN)
			{
				if (av == ViewType::DS)
					DSRVD.Format = dstex_format_to_srview_format(DTD.Format);
			}
			else
				DSRVD.Format = DF;
			uint32_t2 mip = miplevel_range.value_or(uint32_t2{ 0, DTD.MipLevels });
			uint32_t2 arr = array_range.value_or(uint32_t2{ 0, DTD.ArraySize });
			DSRVD.Texture2DArray = D3D11_TEX2D_ARRAY_SRV{ mip.x, mip.y, arr.x, arr.y };
			HRESULT re = c->CreateShaderResourceView(ptr, &DSRVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		depth_stencil_view<tex2_array> tex2_array::cast_depth_stencil_view(device_ptr& c, uint32_t miplevel, std::optional<uint32_t2> array_start_and_count)const
		{
			depth_stencil_view<tex2_array> tem;
			D3D11_TEXTURE2D_DESC DTD;
			ptr->GetDesc(&DTD);
			D3D11_DEPTH_STENCIL_VIEW_DESC DDSVD{ dstex_format_to_dsview_format(DTD.Format), D3D11_DSV_DIMENSION_TEXTURE2DARRAY, 0 };
			uint32_t2 array_size = array_start_and_count.value_or(uint32_t2{ 0, DTD.ArraySize });
			DDSVD.Texture2DArray = D3D11_TEX2D_ARRAY_DSV{ miplevel, array_size.x, array_size.y };
			HRESULT re = c->CreateDepthStencilView(ptr, &DDSVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		render_target_view<tex2_array> tex2_array::cast_render_target_view_as_format(device_ptr& c, DXGI_FORMAT DF, uint32_t miplevel, std::optional<uint32_t2> array_start_and_count)const
		{
			render_target_view<tex2_array> tem;
			D3D11_TEXTURE2D_DESC DTD;
			ptr->GetDesc(&DTD);
			D3D11_RENDER_TARGET_VIEW_DESC DRTVD{ DF == DXGI_FORMAT_UNKNOWN ? DTD.Format : DF, D3D11_RTV_DIMENSION_TEXTURE2DARRAY };
			uint32_t2 array_size = array_start_and_count.value_or(uint32_t2{ 0, DTD.ArraySize });
			DRTVD.Texture2DArray = D3D11_TEX2D_ARRAY_RTV{ miplevel, array_size.x, array_size.y };
			HRESULT re = c->CreateRenderTargetView(ptr, &DRTVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		unordered_access_view<tex2_array> tex2_array::cast_unordered_access_view_as_format(device_ptr& c, DXGI_FORMAT DF, uint32_t miplevel, std::optional<uint32_t2> array_start_and_count)const
		{
			unordered_access_view<tex2_array> tem;
			D3D11_TEXTURE2D_DESC DTD;
			ptr->GetDesc(&DTD);
			D3D11_UNORDERED_ACCESS_VIEW_DESC DUAVD{ DF == DXGI_FORMAT_UNKNOWN ? DTD.Format : DF, D3D11_UAV_DIMENSION_TEXTURE2DARRAY };
			uint32_t2 array_size = array_start_and_count.value_or(uint32_t2{ 0, DTD.ArraySize });
			DUAVD.Texture2DArray = D3D11_TEX2D_ARRAY_UAV{ miplevel, array_size.x, array_size.y };
			HRESULT re = c->CreateUnorderedAccessView(ptr, &DUAVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		//tex2ms ********************************************************
		shader_resource_view<tex2ms> tex2ms::cast_shader_resource_view_as_format(device_ptr& c, DXGI_FORMAT DF)const
		{
			shader_resource_view<tex2ms> tem;
			D3D11_TEXTURE2D_DESC DTD;
			ptr->GetDesc(&DTD);
			D3D11_SHADER_RESOURCE_VIEW_DESC DSRVD{ DTD.Format, D3D11_SRV_DIMENSION_TEXTURE2DMS };
			if (DF == DXGI_FORMAT_UNKNOWN)
			{
				if (av == ViewType::DS)
					DSRVD.Format = dstex_format_to_srview_format(DTD.Format);
			}
			else
				DSRVD.Format = DF;
			DSRVD.Texture2DMS = D3D11_TEX2DMS_SRV{};
			HRESULT re = c->CreateShaderResourceView(ptr, &DSRVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		depth_stencil_view<tex2ms> tex2ms::cast_depth_stencil_view(device_ptr& c)const
		{
			depth_stencil_view<tex2ms> tem;
			D3D11_TEXTURE2D_DESC DTD;
			ptr->GetDesc(&DTD);
			D3D11_DEPTH_STENCIL_VIEW_DESC DDSVD{ dstex_format_to_dsview_format(DTD.Format), D3D11_DSV_DIMENSION_TEXTURE2DMS, 0 };
			DDSVD.Texture2DMS = D3D11_TEX2DMS_DSV{ };
			HRESULT re = c->CreateDepthStencilView(ptr, &DDSVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		render_target_view<tex2ms> tex2ms::cast_render_target_view_as_format(device_ptr& c, DXGI_FORMAT DF)const
		{
			render_target_view<tex2ms> tem;
			D3D11_TEXTURE2D_DESC DTD;
			ptr->GetDesc(&DTD);
			D3D11_RENDER_TARGET_VIEW_DESC DRTVD{ DF == DXGI_FORMAT_UNKNOWN ? DTD.Format : DF, D3D11_RTV_DIMENSION_TEXTURE2DMS };
			DRTVD.Texture2DMS = D3D11_TEX2DMS_RTV{ };
			HRESULT re = c->CreateRenderTargetView(ptr, &DRTVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		//tex2ms_array ********************************************************
		shader_resource_view<tex2ms_array> tex2ms_array::cast_shader_resource_view_as_format(device_ptr& c, DXGI_FORMAT DF, std::optional<uint32_t2> array_start_and_count)const
		{
			shader_resource_view<tex2ms_array> tem;
			D3D11_TEXTURE2D_DESC DTD;
			ptr->GetDesc(&DTD);
			D3D11_SHADER_RESOURCE_VIEW_DESC DSRVD{ DTD.Format, D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY };
			if (DF == DXGI_FORMAT_UNKNOWN)
			{
				if (av == ViewType::DS)
					DSRVD.Format = dstex_format_to_srview_format(DTD.Format);
			}
			else
				DSRVD.Format = DF;
			uint32_t2 array_size = array_start_and_count.value_or(uint32_t2{ 0, DTD.ArraySize });
			DSRVD.Texture2DMSArray = D3D11_TEX2DMS_ARRAY_SRV{ array_size.x, array_size.y };
			HRESULT re = c->CreateShaderResourceView(ptr, &DSRVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		depth_stencil_view<tex2ms_array> tex2ms_array::cast_depth_stencil_view(device_ptr& c, std::optional<uint32_t2> array_start_and_count)const
		{
			depth_stencil_view<tex2ms_array> tem;
			D3D11_TEXTURE2D_DESC DTD;
			ptr->GetDesc(&DTD);
			D3D11_DEPTH_STENCIL_VIEW_DESC DDSVD{ dstex_format_to_dsview_format(DTD.Format), D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY, 0 };
			uint32_t2 array_size = array_start_and_count.value_or(uint32_t2{ 0, DTD.ArraySize });
			DDSVD.Texture2DMSArray = D3D11_TEX2DMS_ARRAY_DSV{ array_size.x, array_size.y };
			HRESULT re = c->CreateDepthStencilView(ptr, &DDSVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		render_target_view<tex2ms_array> tex2ms_array::cast_render_target_view_as_format(device_ptr& c, DXGI_FORMAT DF, std::optional<uint32_t2> array_start_and_count)const
		{
			render_target_view<tex2ms_array> tem;
			D3D11_TEXTURE2D_DESC DTD;
			ptr->GetDesc(&DTD);
			D3D11_RENDER_TARGET_VIEW_DESC DRTVD{ DF == DXGI_FORMAT_UNKNOWN ? DTD.Format : DF, D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY };
			uint32_t2 array_size = array_start_and_count.value_or(uint32_t2{ 0, DTD.ArraySize });
			DRTVD.Texture2DMSArray = D3D11_TEX2DMS_ARRAY_RTV{ array_size.x, array_size.y };
			HRESULT re = c->CreateRenderTargetView(ptr, &DRTVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		//tex_cube ************************************************
		shader_resource_view<tex_cube> tex_cube::cast_shader_resource_view_as_format(device_ptr& c, DXGI_FORMAT DF, std::optional<uint32_t2> miplevel_range)const
		{
			shader_resource_view<tex_cube> tem;
			D3D11_TEXTURE2D_DESC DTD;
			ptr->GetDesc(&DTD);
			D3D11_SHADER_RESOURCE_VIEW_DESC DSRVD{ DTD.Format, D3D11_SRV_DIMENSION_TEXTURECUBE };
			if (DF == DXGI_FORMAT_UNKNOWN)
			{
				if (av == ViewType::DS)
					DSRVD.Format = dstex_format_to_srview_format(DTD.Format);
			}
			else
				DSRVD.Format = DF;
			uint32_t2 mip = miplevel_range.value_or(uint32_t2{ 0, DTD.MipLevels });
			DSRVD.TextureCube = D3D11_TEXCUBE_SRV{ mip.x, mip.y };
			HRESULT re = c->CreateShaderResourceView(ptr, &DSRVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		//tex_cube_array *******************************
		shader_resource_view<tex_cube_array> tex_cube_array::cast_shader_resource_view_as_format(device_ptr& c, DXGI_FORMAT DF, std::optional<uint32_t2> miplevel_range , std::optional<uint32_t2> array_start_and_count )const
		{
			shader_resource_view<tex_cube_array> tem;
			D3D11_TEXTURE2D_DESC DTD;
			ptr->GetDesc(&DTD);
			D3D11_SHADER_RESOURCE_VIEW_DESC DSRVD{ DTD.Format, D3D11_SRV_DIMENSION_TEXTURECUBEARRAY };
			if (DF == DXGI_FORMAT_UNKNOWN)
			{
				if (av == ViewType::DS)
					DSRVD.Format = dstex_format_to_srview_format(DTD.Format);
			}
			else
				DSRVD.Format = DF;
			uint32_t2 mip = miplevel_range.value_or(uint32_t2{ 0, DTD.MipLevels });
			uint32_t2 arr = array_start_and_count.value_or(uint32_t2{ 0, DTD.ArraySize });
			DSRVD.TextureCubeArray = D3D11_TEXCUBE_ARRAY_SRV{ mip.x, mip.y, arr.x, arr.y };
			HRESULT re = c->CreateShaderResourceView(ptr, &DSRVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		//tex3 ********************************************
		shader_resource_view<tex3> tex3::cast_shader_resource_view_as_format(device_ptr& c, DXGI_FORMAT DF, std::optional<uint32_t2> miplevel_range)const
		{
			shader_resource_view<tex3> tem;
			D3D11_TEXTURE3D_DESC DTD;
			ptr->GetDesc(&DTD);
			D3D11_SHADER_RESOURCE_VIEW_DESC DSRVD{ DF == DXGI_FORMAT_UNKNOWN ? DTD.Format : DF, D3D11_SRV_DIMENSION_TEXTURE3D };
			uint32_t2 mip = miplevel_range.value_or(uint32_t2{ 0, DTD.MipLevels });
			DSRVD.Texture3D = D3D11_TEX3D_SRV{ mip.x, mip.y };
			HRESULT re = c->CreateShaderResourceView(ptr, &DSRVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		render_target_view<tex3> tex3::cast_render_target_view_as_format(device_ptr& c, DXGI_FORMAT DF, uint32_t miplevel, std::optional<uint32_t2> array_start_and_count)const
		{
			render_target_view<tex3> tem;
			D3D11_TEXTURE3D_DESC DTD;
			ptr->GetDesc(&DTD);
			D3D11_RENDER_TARGET_VIEW_DESC DRTVD{ DF == DXGI_FORMAT_UNKNOWN ? DTD.Format : DF, D3D11_RTV_DIMENSION_TEXTURE3D };
			uint32_t2 array_size = array_start_and_count.value_or(uint32_t2{ 0, DTD.Depth });
			DRTVD.Texture3D = D3D11_TEX3D_RTV{ miplevel, array_size.x, array_size.y };
			HRESULT re = c->CreateRenderTargetView(ptr, &DRTVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		unordered_access_view<tex3> tex3::cast_unordered_access_view_as_format(device_ptr& c, DXGI_FORMAT DF, uint32_t miplevel, std::optional<uint32_t2> array_start_and_count)const
		{
			unordered_access_view<tex3> tem;
			D3D11_TEXTURE3D_DESC DTD;
			ptr->GetDesc(&DTD);
			D3D11_UNORDERED_ACCESS_VIEW_DESC DUAVD{ DF == DXGI_FORMAT_UNKNOWN ? DTD.Format : DF, D3D11_UAV_DIMENSION_TEXTURE3D };
			uint32_t2 array_size = array_start_and_count.value_or(uint32_t2{ 0, DTD.Depth });
			DUAVD.Texture3D = D3D11_TEX3D_UAV{ miplevel, array_size.x, array_size.y };
			HRESULT re = c->CreateUnorderedAccessView(ptr, &DUAVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		//sample_state *******************************
		sample_state::description sample_state::default_description = sample_state::description{
			D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP, 0.0f, 1,
			D3D11_COMPARISON_NEVER,{ 1.0f,1.0f,1.0f,1.0f }, -FLT_MAX, FLT_MAX
		};
		void sample_state::create(device_ptr& c, const description& scri)
		{
			decltype(ptr) tem_ptr;
			HRESULT re = c->CreateSamplerState(&scri, tem_ptr.adress());
			if (SUCCEEDED(re))
			{
				ptr = std::move(tem_ptr);
			}
			else
				throw re;
		}

		//raterizer_state ************************************
		raterizer_state::description raterizer_state::default_description = raterizer_state::description{
			D3D11_FILL_SOLID, D3D11_CULL_BACK, FALSE, 0,0.0f, 0.0f, TRUE, FALSE,FALSE
		};
		void raterizer_state::create(device_ptr& c, const description& scri)
		{
			decltype(ptr) tem_ptr;
			HRESULT re = c->CreateRasterizerState(&scri, tem_ptr.adress());
			if (SUCCEEDED(re))
			{
				ptr = std::move(tem_ptr);
			}
			else
				throw re;
		}

		//blend_state ****************************************
		blend_state::description blend_state::default_description = blend_state::description{
			FALSE, FALSE, D3D11_RENDER_TARGET_BLEND_DESC{ FALSE, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL }
		};
		void blend_state::create(device_ptr& c, const description& scri)
		{
			decltype(ptr) tem_ptr;
			HRESULT re = c->CreateBlendState(&scri, tem_ptr.adress());
			if (SUCCEEDED(re))
			{
				ptr = std::move(tem_ptr);
			}else
				throw re;
		}

		//depth_stencil_state ********************************
		depth_stencil_state::description depth_stencil_state::default_description = depth_stencil_state::description{
			TRUE, D3D11_DEPTH_WRITE_MASK_ALL, D3D11_COMPARISON_LESS, FALSE, D3D11_DEFAULT_STENCIL_READ_MASK, D3D11_DEFAULT_STENCIL_WRITE_MASK,
			D3D11_DEPTH_STENCILOP_DESC{ D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS },
			D3D11_DEPTH_STENCILOP_DESC{ D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS }
		};
		void depth_stencil_state::create(device_ptr& c, const description& scri)
		{
			decltype(ptr) tem_ptr;
			HRESULT re = c->CreateDepthStencilState(&scri, tem_ptr.adress());
			if (SUCCEEDED(re))
			{
				ptr = std::move(tem_ptr);
			}else
				throw re;
		}

		//*************************************************************************  output_merge_d
		output_merge_stage::output_merge_stage() : avalible_render_target_size(0)
		{
			for (auto& ite : target)
				ite = nullptr;
		}

		output_merge_stage& output_merge_stage::set(const render_target_view_interface& il)
		{
			if (avalible_render_target_size < 8)
			{
				target[avalible_render_target_size] = il.ptr;
				target[avalible_render_target_size]->AddRef();
				++avalible_render_target_size;
			}
			return *this;
		}

		output_merge_stage::~output_merge_stage()
		{
			for(auto& ptr : target)
			{
				if (ptr != nullptr)
					ptr->Release();
			}
		}

		void output_merge_stage::clear_render_target()
		{
			for (auto& ptr : target)
			{
				if (ptr != nullptr)
				{
					ptr->Release();
					ptr = nullptr;
				}
			}
			avalible_render_target_size = 0;
		}

		void output_merge_stage::clear()
		{
			clear_render_target();
			depth = depth_stencil_view_interface{};
		}

		output_merge_stage::output_merge_stage(const output_merge_stage& oms) : target(oms.target), depth(oms.depth), avalible_render_target_size(oms.avalible_render_target_size)
		{
			for (auto& ite : target)
			{
				if (ite != nullptr)
					ite->AddRef();
			}
		}

		output_merge_stage::output_merge_stage(output_merge_stage&& oms) : target(oms.target), depth(std::move(oms.depth)), avalible_render_target_size(oms.avalible_render_target_size)
		{
			for (auto& ite : oms.target)
				ite = nullptr;
			oms.avalible_render_target_size = 0;
		}

		output_merge_stage& output_merge_stage::operator=(const output_merge_stage& oms)
		{
			output_merge_stage oms_tem(oms);
			for (auto& ptr : target)
			{
				if (ptr != nullptr)
				{
					ptr->Release();
					ptr = nullptr;
				}
			}
			target = oms_tem.target;
			avalible_render_target_size = oms_tem.avalible_render_target_size;
			for (auto& ite : target)
			{
				if (ite != nullptr)
					ite->AddRef();
			}
			depth = oms_tem.depth;
			return *this;
		}
		output_merge_stage& output_merge_stage::operator=(output_merge_stage&& oms)
		{
			output_merge_stage oms_tem(std::move(oms));
			for (auto& ptr : target)
			{
				if (ptr != nullptr)
				{
					ptr->Release();
					ptr = nullptr;
				}
			}
			target = oms_tem.target;
			avalible_render_target_size = oms_tem.avalible_render_target_size;
			oms_tem.avalible_render_target_size = 0;
			for (auto& ite : oms_tem.target)
				ite = nullptr;
			depth = std::move(oms_tem.depth);
			return *this;
		}

		//shader *************************
		void shader_vertex::create(device_ptr& c, std::shared_ptr<PO::Dx::shader_binary> p)
		{
			if (p)
			{
				decltype(ptr) tem_ptr;
				HRESULT re = c->CreateVertexShader(*p, *p, nullptr, tem_ptr.adress());
				if (SUCCEEDED(re)) {
					code = std::move(p);
					ptr = std::move(tem_ptr);
				}else
					throw(re);
			}
		}

		void shader_pixel::create(device_ptr& c, const PO::Dx::shader_binary& p)
		{
			if (p)
			{
				decltype(ptr) tem_ptr;
				HRESULT re = c->CreatePixelShader(p, p, nullptr, tem_ptr.adress());
				if (SUCCEEDED(re)) {
					ptr = std::move(tem_ptr);
				}else
					throw(re);
			}
		}

		void shader_compute::create(device_ptr& c, const PO::Dx::shader_binary& p)
		{
			if (p)
			{
				decltype(ptr) tem_ptr;
				HRESULT re = c->CreateComputeShader(p, p, nullptr, tem_ptr.adress());
				if (SUCCEEDED(re)) {
					ptr = std::move(tem_ptr);
				}else
					throw(re);
			}
		}
	
		// input_layout ***************************************************
		void input_layout::create(device_ptr& c, const layout_view& view, const shader_vertex& sv)
		{
			Win32::com_ptr<ID3D11InputLayout> lp;
			HRESULT re = c->CreateInputLayout(view.ptr, view.size, sv.code ? static_cast<const void*>(*sv.code) : nullptr, sv.code ? static_cast<uint32_t>(*sv.code) : 0, lp.adress());
			if (SUCCEEDED(re))
			{
				ptr = std::move(lp);
			}
			else throw re;
		}




		namespace Implement
		{

			static std::array<ID3D11Buffer*, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT> nullptr_cbuffer;
			static std::array<ID3D11ShaderResourceView*, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT> nullptr_shader_resource_view;
			static std::array<ID3D11SamplerState*, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT> nullptr_sampler_state;

			template<typename T, typename K, typename L>
			void binding_implement(ID3D11DeviceContext& con, uint32_t& count, const T& input, const K& null, void (__stdcall ID3D11DeviceContext::* f)(uint32_t, uint32_t, L))
			{
				uint32_t input_size = static_cast<uint32_t>(input.size());
				if (count > input_size)
					(con.*f)(input_size, count - input_size, null.data());
				count = input_size;
				if(count != 0)
					(con.*f)(0, count, input.data());
			}

			template<typename K, typename L>
			void unbinding_implement(ID3D11DeviceContext& con, uint32_t& count, const K& null, void (__stdcall ID3D11DeviceContext::* f)(uint32_t, uint32_t, L))
			{
				if (count != 0)
				{
					(con.*f)(0, count, null.data());
					count = 0;
				}
			}

			template<typename K, typename L>
			void extract_implement(ID3D11DeviceContext& con, uint32_t count, K& out, void (__stdcall ID3D11DeviceContext::* f)(uint32_t, uint32_t, L))
			{
				out.resize(count);
				(con.*f)(0, count, out.data());
			}

			void shader_context_t::unbind(Win32::com_ptr<ID3D11DeviceContext>& cp,
				void (__stdcall ID3D11DeviceContext::* cb_f)(uint32_t, uint32_t, ID3D11Buffer* const *),
				void (__stdcall ID3D11DeviceContext::* srv_f)(uint32_t, uint32_t, ID3D11ShaderResourceView* const *),
				void (__stdcall ID3D11DeviceContext::* sample_f)(uint32_t, uint32_t, ID3D11SamplerState* const *)
			)
			{
				unbinding_implement(*cp, cb_count, nullptr_cbuffer, cb_f);
				unbinding_implement(*cp, srv_count, nullptr_shader_resource_view, srv_f);
				unbinding_implement(*cp, sample_count, nullptr_sampler_state, sample_f);
			}

			void shader_context_t::bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const buffer_constant& id, uint32_t solt,
				void(__stdcall ID3D11DeviceContext::* cb_f)(uint32_t, uint32_t, ID3D11Buffer* const *)
			)
			{
				if (cb_count <= solt)
				{
					cb_count = static_cast<uint32_t>(solt + 1);
				}
				ID3D11Buffer* const buffer[1] = { id.ptr };
				(cp->*cb_f)(static_cast<uint32_t>(solt), 1, buffer);
			}

			void shader_context_t::bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const shader_resource_view_interface& id, uint32_t solt,
				void(__stdcall ID3D11DeviceContext::* srv_f)(uint32_t, uint32_t, ID3D11ShaderResourceView* const *)
			)
			{
				if (srv_count <= solt)
				{
					srv_count = static_cast<uint32_t>(solt + 1);
				}
				ID3D11ShaderResourceView * const buffer[1] = { id.ptr };
				(cp->*srv_f)(static_cast<uint32_t>(solt), 1, buffer);
			}

			void shader_context_t::bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const sample_state& id, uint32_t solt,
				void(__stdcall ID3D11DeviceContext::* srv_f)(uint32_t, uint32_t, ID3D11SamplerState* const *)
			)
			{
				if (sample_count <= solt)
				{
					sample_count = static_cast<uint32_t>(solt + 1);
				}
				ID3D11SamplerState * const buffer[1] = { id.ptr };
				(cp->*srv_f)(static_cast<uint32_t>(solt), 1, buffer);
			}

			static std::array<ID3D11Buffer*, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT> input_assember_context_nullptr_array = {};
			static std::array<uint32_t, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT> input_assember_context_zero_array = {};

			/*****  input_assember_context_t   ******************************************************************************************/
			void input_assember_context_t::bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const buffer_vertex& vi, uint32_t solt)
			{
				if (vb_count <= solt)
					vb_count = solt + 1;
				ID3D11Buffer * const buffer[1] = { vi.ptr };
				uint32_t offset = 0;
				cp->IASetVertexBuffers(solt, 1, buffer, &vi.stride, &offset);
			}
			void input_assember_context_t::bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const buffer_index& iv)
			{
				cp->IASetIndexBuffer(iv.ptr, iv.DF, 0);
			}

			/*
			void input_assember_context_t::extract(Win32::com_ptr<ID3D11DeviceContext>& cp, input_assember_stage& id)
			{
				input_assember_stage is;
				cp->IAGetPrimitiveTopology(&is.primitive);
				
				is.vertex_array.resize(vb_count);
				is.element_array.resize(vb_count);
				is.offset_array.resize(vb_count);

				cp->IAGetVertexBuffers(0, vb_count, is.vertex_array.data(), is.element_array.data(), is.offset_array.data());
				cp->IAGetIndexBuffer(is.index_ptr.adress(), &is.format, &is.offset);
				id = std::move(is);
			}

			void input_assember_context_t::extract(Win32::com_ptr<ID3D11DeviceContext>& cp, input_layout& id)
			{
				input_layout tem;
				cp->IAGetInputLayout(tem.ptr.adress());
				id = std::move(tem);
			}
			*/

			void input_assember_context_t::unbind(Win32::com_ptr<ID3D11DeviceContext>& cp)
			{
				cp->IASetInputLayout(nullptr);
				cp->IASetVertexBuffers(0, vb_count, input_assember_context_nullptr_array.data(), input_assember_context_zero_array.data(), input_assember_context_zero_array.data());
				cp->IASetIndexBuffer(nullptr, DXGI_FORMAT::DXGI_FORMAT_R16_UINT, 0);
				cp->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				vb_count = 0;
			}

			void input_assember_context_t::clear(Win32::com_ptr<ID3D11DeviceContext>& cp)
			{
				unbind(cp);
			}

			/*****  vertex_shader_context_t   ******************************************************************************************/
			void vertex_stage_context_t::unbind(Win32::com_ptr<ID3D11DeviceContext>& cp)
			{
				cp->VSSetShader(nullptr, nullptr, 0);
				shader_context_t::unbind(
					cp,
					&ID3D11DeviceContext::VSSetConstantBuffers,
					&ID3D11DeviceContext::VSSetShaderResources,
					&ID3D11DeviceContext::VSSetSamplers
				);
			}

			/*****  raterizer_context_t   ******************************************************************************************/
			void raterizer_context_t::bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const viewport& rs)
			{
				cp->RSSetViewports(1, &(rs.view));
			}
			/*
			void raterizer_context_t::bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const viewports& rs)
			{
				//cp->RSSetState(rs.ptr);
				//view_count.bind(rs.views, )
				cp->RSSetScissorRects(static_cast<uint32_t>(rs.scissor.size()), rs.scissor.data());
				cp->RSSetViewports(static_cast<uint32_t>(rs.views.size()), rs.views.data());
				//cp->PSS
			}
			*/


			void raterizer_context_t::bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const raterizer_state& rs)
			{
				cp->RSSetState(rs.ptr);
				//cp->RSSetScissorRects(static_cast<uint32_t>(rs.scissor.size()), rs.scissor.data());
				//cp->RSSetViewports(static_cast<uint32_t>(rs.viewports.size()), rs.viewports.data());
				//cp->PSS
			}
			

			void raterizer_context_t::unbind(Win32::com_ptr<ID3D11DeviceContext>& cp)
			{
				/*
				cp->RSSetState(nullptr);
				cp->RSSetScissorRects(0, nullptr);
				cp->RSSetViewports(0, nullptr);
				*/
			}

			void raterizer_context_t::clear(Win32::com_ptr<ID3D11DeviceContext>& cp)
			{
				cp->RSSetState(nullptr);
				cp->RSSetScissorRects(0, nullptr);
				cp->RSSetViewports(0, nullptr);
			}

			/*****  pixel_stage_context_t   ******************************************************************************************/
			void pixel_stage_context_t::unbind(Win32::com_ptr<ID3D11DeviceContext>& cp)
			{
				cp->PSSetShader(nullptr, nullptr, 0);

				shader_context_t::unbind(
					cp,
					&ID3D11DeviceContext::PSSetConstantBuffers,
					&ID3D11DeviceContext::PSSetShaderResources,
					&ID3D11DeviceContext::PSSetSamplers
				);
					
			}
			
			static std::array<ID3D11RenderTargetView*, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> nullptr_render_target;
			/*****  output_merge_context_t   ******************************************************************************************/
			void output_merge_context_t::bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const output_merge_stage& od)
			{
				cp->OMSetRenderTargets(static_cast<uint32_t>(od.target.size()), od.target.data(), od.depth.ptr);
			}

			void output_merge_context_t::bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const blend_state& bs) {
				cp->OMSetBlendState(bs.ptr, bs.bind_factor.data(), bs.sample_mask);
			}
			void output_merge_context_t::bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const depth_stencil_state& dss) {
				cp->OMSetDepthStencilState(dss.ptr, dss.stencil_ref);
			}

			void output_merge_context_t::unbind(Win32::com_ptr<ID3D11DeviceContext>& cp)
			{
				cp->OMSetRenderTargets(0, nullptr, nullptr);
			}

			void output_merge_context_t::extract(Win32::com_ptr<ID3D11DeviceContext>& cp, output_merge_stage& ps)
			{
				output_merge_stage tem;
				cp->OMGetRenderTargets(8, tem.target.data(), tem.depth.ptr.adress());
				ps = std::move(tem);
			}

			void output_merge_context_t::clear_render_target(Win32::com_ptr<ID3D11DeviceContext>& cp, output_merge_stage& omd, uint32_t solt, const std::array<float, 4>& color)
			{
				if(solt < 8)
					cp->ClearRenderTargetView(omd.target[solt], color.data());
			}

			void output_merge_context_t::clear_render_target(Win32::com_ptr<ID3D11DeviceContext>& cp, output_merge_stage& omd, const std::array<float, 4>& color)
			{
				for (auto& ra : omd.target)
					if (ra != nullptr)
						cp->ClearRenderTargetView(ra, color.data());
			}

			void output_merge_context_t::clear_depth(Win32::com_ptr<ID3D11DeviceContext>& cp, output_merge_stage& omd, float depth)
			{
				if (omd.depth.ptr)
					cp->ClearDepthStencilView(omd.depth.ptr, D3D11_CLEAR_DEPTH, depth, 0);
			}

			void output_merge_context_t::clear_stencil(Win32::com_ptr<ID3D11DeviceContext>& cp, output_merge_stage& omd, uint8_t ref)
			{
				if (omd.depth.ptr)
					cp->ClearDepthStencilView(omd.depth.ptr, D3D11_CLEAR_STENCIL, 0.0f, ref);
			}

			void output_merge_context_t::clear_depth_stencil(Win32::com_ptr<ID3D11DeviceContext>& cp, output_merge_stage& omd, float depth, uint8_t ref)
			{
				if (omd.depth.ptr)
					cp->ClearDepthStencilView(omd.depth.ptr, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth, ref);
			}

			/*****  draw_range_context_t   ******************************************************************************************/
			/*
			void draw_range_context_t::draw(context_ptr& cp, const draw_range_d& d)
			{
				auto& var = d.data;
				if (var.able_cast<draw_range_d::vertex_d>())
					return draw_type = type::RENDERER, cp->Draw(var.cast<draw_range_d::vertex_d>().vertex_count, var.cast<draw_range_d::vertex_d>().start_vertex_location);
				if (var.able_cast<draw_range_d::index_d>())
				{
					auto& ind = var.cast<draw_range_d::index_d>();
					return draw_type = type::RENDERER, cp->DrawIndexed(ind.index_count, ind.start_index_location, ind.base_vertex_location);
				}
				if (var.able_cast<draw_range_d::instance_d>())
				{
					auto& ins = var.cast<draw_range_d::instance_d>();
					return draw_type = type::RENDERER, cp->DrawInstanced(ins.vertex_pre_instance, ins.instance_count, ins.start_vertex_location, ins.start_instance_location);
				}
				if (var.able_cast<draw_range_d::instance_index_d>())
				{
					auto& in = var.cast<draw_range_d::instance_index_d>();
					return draw_type = type::RENDERER, cp->DrawIndexedInstanced(in.index_pre_instance, in.instance_count, in.start_index_location, in.base_vertex_location, in.start_instance_location);
				}
				if (var.able_cast<draw_range_d::dispatch_d>())
				{
					auto& in = var.cast<draw_range_d::dispatch_d>();
					return draw_type = type::COMPUTE, cp->Dispatch(in.X, in.Y, in.Z);
				}
			}

			auto draw_range_context_t::unbind(Win32::com_ptr<ID3D11DeviceContext>& cp) -> type
			{
				type p = draw_type;
				draw_type = type::NONE;
				return p;
			}
			*/


			/*****  compute_shader_context_t   ******************************************************************************************/
			static std::array<ID3D11UnorderedAccessView*, D3D11_1_UAV_SLOT_COUNT> nullptr_unordered_access_array;
			static std::array<uint32_t, D3D11_1_UAV_SLOT_COUNT> nullptr_unordered_access_offset_array;

			void compute_stage_context_t::unbind(Win32::com_ptr<ID3D11DeviceContext>& cp)
			{
				cp->CSSetShader(nullptr, nullptr, 0);
				shader_context_t::unbind(
					cp,
					&ID3D11DeviceContext::CSSetConstantBuffers,
					&ID3D11DeviceContext::CSSetShaderResources,
					&ID3D11DeviceContext::CSSetSamplers
				);
				if (count != 0)
				{
					cp->CSSetUnorderedAccessViews(0, count, nullptr_unordered_access_array.data(), nullptr_unordered_access_offset_array.data());
					count = 0;
				}
			}

			void compute_stage_context_t::bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const unordered_access_view_interface& cb, uint32_t solt)
			{
				if (count <= solt)
					count = static_cast<uint32_t>(solt + 1);
				ID3D11UnorderedAccessView * const buffer[1] = { cb.ptr };
				cp->CSSetUnorderedAccessViews(static_cast<uint32_t>(solt), 1, buffer, &cb.offset);
			}

		}

		/*****  pipeline_implement   ******************************************************************************************/

		void stage_context::unbind() noexcept {
			m_CS.unbind(ptr); m_IA.unbind(ptr); m_VS.unbind(ptr); m_RA.unbind(ptr);
			m_PS.unbind(ptr); m_OM.unbind(ptr);
			call_require = {};
		}
		void stage_context::clear() noexcept
		{
			call_require = {};
			m_CS.clear(ptr); m_IA.clear(ptr); m_VS.clear(ptr); m_RA.clear(ptr);
			m_PS.clear(ptr); m_OM.clear(ptr);
		}

		bool stage_context::apply() noexcept
		{
			if (std::holds_alternative<dispatch_call>(call_require))
			{
				auto& call_command = std::get<dispatch_call>(call_require);
				assert(call_command.x != 0 && call_command.y != 0 && call_command.z != 0);
				ptr->Dispatch(call_command.x, call_command.y, call_command.z);
			}
			else if (std::holds_alternative<vertex_call>(call_require))
			{
				auto& call_command = std::get<vertex_call>(call_require);
				ptr->Draw(call_command.count, call_command.start);
			}
			else if (std::holds_alternative<index_call>(call_require))
			{
				auto& call_command = std::get<index_call>(call_require);
				ptr->DrawIndexed(call_command.index_count, call_command.index_start, call_command.vertex_start);
			}
			else if (std::holds_alternative<vertex_instance_call>(call_require))
			{
				auto& call_command = std::get<vertex_instance_call>(call_require);
				ptr->DrawInstanced(call_command.vertex_pre_instance, call_command.instance_count, call_command.vertex_start, call_command.instance_start);
			}
			else if (std::holds_alternative<index_instance_call>(call_require))
			{
				auto& call_command = std::get<index_instance_call>(call_require);
				ptr->DrawIndexedInstanced(call_command.index_pre_instance, call_command.instance_count, call_command.index_start, call_command.base_vertex, call_command.instance_start);
			}
			else
				return false;
			return true;
		}
	}

	
}