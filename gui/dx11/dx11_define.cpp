#include "dx11_define.h"
#include "../../frame/define.h"
#include <fstream>
#undef max
namespace PO
{
	namespace Dx11
	{

		namespace Implement
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

			HRESULT create_buffer(Implement::resource_ptr& rp, Implement::buffer_ptr& ptr, D3D11_USAGE usage, UINT cpu_flag, UINT bind_flag, const void* data, size_t data_size, UINT misc_flag, size_t StructureByteStrides)
			{
				D3D11_BUFFER_DESC DBD
				{
					static_cast<UINT>(data_size),
					usage,
					bind_flag,
					cpu_flag,
					misc_flag,
					static_cast<UINT>(StructureByteStrides)
				};
				D3D11_SUBRESOURCE_DATA DSD{ data, 0, 0 };
				return rp->CreateBuffer(&DBD, ((data == nullptr) ? nullptr : &DSD), &ptr) == S_OK;
			}
				
			static Tool::scope_lock<std::vector<D3D11_SUBRESOURCE_DATA>> subresource_buffer;

			HRESULT create_texture_implement(resource_ptr& cp, texture1D_ptr& t, const tex1_size& size, const res_usagne& ru, void** data)
			{
				if (cp == nullptr || size.count == 0) return E_INVALIDARG;
				D3D11_TEXTURE1D_DESC DTD{ static_cast<UINT>(size.w), static_cast<UINT>(size.miplevel), static_cast<UINT>(size.count),
					size.DF, ru.usage, ru.bind_flag, ru.cpu_bind, ru.mis
				};
				if(data == nullptr) return cp->CreateTexture1D(&DTD, nullptr, &t);
				return subresource_buffer.lock([&](decltype(subresource_buffer)::type& buffer){
					buffer.resize(size.count);
					for (size_t i = 0; i < size.count; ++i)
						buffer[i] = D3D11_SUBRESOURCE_DATA{ data[i] };
					return cp->CreateTexture1D(&DTD, buffer.data(), &t);
				});
			}

			HRESULT create_texture_implement(resource_ptr& cp, texture2D_ptr& t, const tex2_size& size, const tex_sample& ts, const res_usagne& ru, void** data, size_t* line)
			{
				if (cp == nullptr || size.count == 0) return E_INVALIDARG;
				D3D11_TEXTURE2D_DESC DTD{ static_cast<UINT>(size.w), static_cast<UINT>(size.h), static_cast<UINT>(size.miplevel), static_cast<UINT>(size.count),
					size.DF, DXGI_SAMPLE_DESC{static_cast<UINT>(ts.num), static_cast<UINT>(ts.quality)},
					ru.usage, ru.bind_flag, ru.cpu_bind, ru.mis
				};
				if (data == nullptr) return cp->CreateTexture2D(&DTD, nullptr, &t);
				return subresource_buffer.lock([&](decltype(subresource_buffer)::type& buffer) {
					buffer.resize(size.count);
					for (size_t i = 0; i < size.count; ++i)
						buffer[i] = D3D11_SUBRESOURCE_DATA{ data[i], line[i] };
					return cp->CreateTexture2D(&DTD, buffer.data(), &t);
				});
			}

			HRESULT create_texture_implement(resource_ptr& cp, texture3D_ptr& t, const tex3_size& size, const res_usagne& ru, void* data, size_t line, size_t slice)
			{
				if (cp == nullptr) return E_INVALIDARG;
				D3D11_TEXTURE3D_DESC DTD{ static_cast<UINT>(size.w), static_cast<UINT>(size.h), static_cast<UINT>(size.miplevel), static_cast<UINT>(size.z),
					size.DF, ru.usage, ru.bind_flag, ru.cpu_bind, ru.mis
				};
				if (data == nullptr) return cp->CreateTexture3D(&DTD, nullptr, &t);
				D3D11_SUBRESOURCE_DATA DSD{ data, line, slice };
				return cp->CreateTexture3D(&DTD, &DSD, &t);
			}
			
			bool avalible_depth_texture_format(DXGI_FORMAT DF)
			{
				switch (DF)
				{
				case DXGI_FORMAT::DXGI_FORMAT_D16_UNORM:
				case DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT:
				case DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT:
				case DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
				case DXGI_FORMAT::DXGI_FORMAT_UNKNOWN:
					return true;
				default:
					return false;
				}
			}
		}
		

		Implement::resource_view_ptr cast_resource(Implement::resource_ptr& rp, const Implement::texture2D_ptr& pt)
		{
			Implement::resource_view_ptr ptr;
			if (pt != nullptr)
			{
				D3D11_TEXTURE2D_DESC tem;
				pt->GetDesc(&tem);
				D3D11_SHADER_RESOURCE_VIEW_DESC SRVD{ tem.Format };
				if ((tem.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE) == D3D11_RESOURCE_MISC_TEXTURECUBE)
				{
					SRVD.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
					SRVD.TextureCube = D3D11_TEXCUBE_SRV{ 0 ,  tem.MipLevels };
				}
				else if (tem.ArraySize > 1)
				{
					SRVD.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
					SRVD.Texture2DArray = D3D11_TEX2D_ARRAY_SRV{ 0,  tem.MipLevels , 0, tem.ArraySize };
				}
				else {
					SRVD.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
					SRVD.Texture2D = D3D11_TEX2D_SRV{ 0,  tem.MipLevels };
				}
				HRESULT re = rp->CreateShaderResourceView(pt, &SRVD, &ptr);
				volatile int i = 0;
			}
			return ptr;
		}

		Implement::resource_view_ptr cast_resource(Implement::resource_ptr& rp, const Implement::texture1D_ptr& pt)
		{
			Implement::resource_view_ptr ptr;
			if (pt != nullptr)
			{
				D3D11_TEXTURE1D_DESC tem;
				pt->GetDesc(&tem);
				D3D11_SHADER_RESOURCE_VIEW_DESC SRVD{ tem.Format };
				if (tem.ArraySize > 1)
				{
					SRVD.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1DARRAY;
					SRVD.Texture1DArray = D3D11_TEX1D_ARRAY_SRV{ 0,  tem.MipLevels , 0, tem.ArraySize };
				}
				else {
					SRVD.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
					SRVD.Texture1D = D3D11_TEX1D_SRV{ 0,  tem.MipLevels };
				}
				rp->CreateShaderResourceView(pt, &SRVD, &ptr);
			}
			return ptr;
		}

		Implement::resource_view_ptr cast_resource(Implement::resource_ptr& rp, const Implement::texture3D_ptr& pt)
		{
			Implement::resource_view_ptr ptr;
			if (pt != nullptr)
			{
				D3D11_TEXTURE3D_DESC tem;
				pt->GetDesc(&tem);
				D3D11_SHADER_RESOURCE_VIEW_DESC SRVD{ tem.Format };
				SRVD.ViewDimension = D3D11_SRV_DIMENSION::D3D10_1_SRV_DIMENSION_TEXTURE3D;
				SRVD.Texture3D = D3D11_TEX3D_SRV{ 0, tem.MipLevels };
				rp->CreateShaderResourceView(pt, &SRVD, &ptr);
			}
			return ptr;
		}

		namespace Implement
		{
			/*
			bool texture_ptr_type<1>::create_RTV(Implement::resource_ptr& cp, Implement::render_view_ptr& rvp, const type& t, size_t mipslice, size_t array_start, size_t array_count, bool all_range)
			{
				if (cp == nullptr || t == nullptr) return false;
				D3D11_TEXTURE1D_DESC DTD;
				t->GetDesc(&DTD);
				D3D11_RENDER_TARGET_VIEW_DESC RTVD{ DTD.Format };
				if (DTD.ArraySize > 1)
				{
					RTVD.ViewDimension = D3D11_RTV_DIMENSION::D3D11_RTV_DIMENSION_TEXTURE1DARRAY;
					using pair_t = std::pair<UINT, UINT>;
					pair_t pair = all_range ? pair_t{0, DTD.ArraySize} : pair_t{static_cast<UINT>(array_start), static_cast<UINT>(array_count)};
					RTVD.Texture1DArray = D3D11_TEX1D_ARRAY_RTV{ static_cast<UINT>(mipslice),  pair.first, pair.second };
				}
				else {
					RTVD.ViewDimension = D3D11_RTV_DIMENSION::D3D11_RTV_DIMENSION_TEXTURE1D;
					RTVD.Texture1D = D3D11_TEX1D_RTV{ static_cast<UINT>(mipslice) };
				}
				return SUCCEEDED(cp->CreateRenderTargetView(t, &RTVD, &rvp));
			}

			bool texture_ptr_type<1>::create_DSV(Implement::resource_ptr& cp, Implement::depth_stencil_view_ptr& dsv, const type& t, bool dr, bool sr, size_t mipslice, size_t array_start, size_t array_count, bool all_range)
			{
				if (cp == nullptr || t == nullptr) return false;
				D3D11_TEXTURE1D_DESC DTD;
				t->GetDesc(&DTD);
				if (!avalible_depth_texture_format(DTD.Format)) return false;
				UINT FLAG = (dr ? D3D11_DSV_READ_ONLY_DEPTH : 0) | (sr ? D3D11_DSV_READ_ONLY_STENCIL : 0);
				D3D11_DEPTH_STENCIL_VIEW_DESC DDSVD{ DTD.Format };
				if (DTD.ArraySize > 1)
				{
					DDSVD.ViewDimension = D3D11_DSV_DIMENSION::D3D11_DSV_DIMENSION_TEXTURE1DARRAY;
					using pair_t = std::pair<UINT, UINT>;
					pair_t pair = all_range ? pair_t{ 0, DTD.ArraySize } : pair_t{ static_cast<UINT>(array_start), static_cast<UINT>(array_count) };
					DDSVD.Texture1DArray = D3D11_TEX1D_ARRAY_DSV{ static_cast<UINT>(mipslice),  pair.first, pair.second };
				}
				else {
					DDSVD.ViewDimension = D3D11_DSV_DIMENSION::D3D11_DSV_DIMENSION_TEXTURE1D;
					DDSVD.Texture1D = D3D11_TEX1D_DSV{ static_cast<UINT>(mipslice) };
				}
				DDSVD.Flags = FLAG;
				return SUCCEEDED(cp->CreateDepthStencilView(t, &DDSVD, &dsv));
			}

			bool texture_ptr_type<1>::create(Implement::resource_ptr& cp, type& t, size_t w, size_t s, void* data, DXGI_FORMAT DF, size_t miplevel, D3D11_USAGE DU, UINT BIND, bool cpu_w, UINT mis)
			{
				D3D11_TEXTURE1D_DESC DTD
				{
					static_cast<UINT>(w),
					static_cast<UINT>(miplevel),
					static_cast<UINT>(s),
					DF,
					DU,

				};
				cp->CreateTexture1D()
			}

			bool texture_ptr_type<2>::create_RTV(Implement::resource_ptr& cp, Implement::render_view_ptr& rvp, const type& t, size_t mipslice, size_t array_start, size_t array_count, bool all_range)
			{
				if (cp == nullptr || t == nullptr) return false;
				D3D11_TEXTURE2D_DESC DTD;
				t->GetDesc(&DTD);
				D3D11_RENDER_TARGET_VIEW_DESC RTVD{ DTD.Format };
				if (DTD.ArraySize > 1)
				{
					RTVD.ViewDimension = D3D11_RTV_DIMENSION::D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
					using pair_t = std::pair<UINT, UINT>;
					pair_t pair = all_range ? pair_t{ 0, DTD.ArraySize } : pair_t{ static_cast<UINT>(array_start), static_cast<UINT>(array_count) };
					RTVD.Texture2DArray = D3D11_TEX2D_ARRAY_RTV{ static_cast<UINT>(mipslice),  pair.first, pair.second };
				}
				else {
					RTVD.ViewDimension = D3D11_RTV_DIMENSION::D3D11_RTV_DIMENSION_TEXTURE2D;
					RTVD.Texture2D = D3D11_TEX2D_RTV{ static_cast<UINT>(mipslice) };
				}
				return SUCCEEDED(cp->CreateRenderTargetView(t, &RTVD, &rvp));
			}

			bool texture_ptr_type<2>::create_ms_RTV(Implement::resource_ptr& cp, Implement::render_view_ptr& rvp, const type& t, size_t array_start, size_t array_count, bool all_range)
			{
				if (cp == nullptr || t == nullptr) return false;
				D3D11_TEXTURE2D_DESC DTD;
				t->GetDesc(&DTD);
				D3D11_RENDER_TARGET_VIEW_DESC RTVD{ DTD.Format };
				if (DTD.ArraySize > 1)
				{
					RTVD.ViewDimension = D3D11_RTV_DIMENSION::D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
					using pair_t = std::pair<UINT, UINT>;
					pair_t pair = all_range ? pair_t{ 0, DTD.ArraySize } : pair_t{ static_cast<UINT>(array_start), static_cast<UINT>(array_count) };
					RTVD.Texture2DMSArray = D3D11_TEX2DMS_ARRAY_RTV{ pair.first, pair.second };
				}
				else {
					RTVD.ViewDimension = D3D11_RTV_DIMENSION::D3D11_RTV_DIMENSION_TEXTURE2DMS;
					RTVD.Texture2DMS = D3D11_TEX2DMS_RTV{ };
				}
				return SUCCEEDED(cp->CreateRenderTargetView(t, &RTVD, &rvp));
			}

			bool texture_ptr_type<2>::create_DSV(Implement::resource_ptr& cp, Implement::depth_stencil_view_ptr& dsv, const type& t, bool dr, bool sr, size_t mipslice, size_t array_start, size_t array_count, bool all_range)
			{
				if (cp == nullptr || t == nullptr) return false;
				D3D11_TEXTURE2D_DESC DTD;
				t->GetDesc(&DTD);
				if (!avalible_depth_texture_format(DTD.Format)) return false;
				UINT FLAG = (dr ? D3D11_DSV_READ_ONLY_DEPTH : 0) | (sr ? D3D11_DSV_READ_ONLY_STENCIL : 0);
				D3D11_DEPTH_STENCIL_VIEW_DESC DDSVD{ DTD.Format };
				if (DTD.ArraySize > 1)
				{
					DDSVD.ViewDimension = D3D11_DSV_DIMENSION::D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
					using pair_t = std::pair<UINT, UINT>;
					pair_t pair = all_range ? pair_t{ 0, DTD.ArraySize } : pair_t{ static_cast<UINT>(array_start), static_cast<UINT>(array_count) };
					DDSVD.Texture2DArray = D3D11_TEX2D_ARRAY_DSV{ static_cast<UINT>(mipslice),  pair.first, pair.second };
				}
				else {
					DDSVD.ViewDimension = D3D11_DSV_DIMENSION::D3D11_DSV_DIMENSION_TEXTURE2D;
					DDSVD.Texture2D = D3D11_TEX2D_DSV{ static_cast<UINT>(mipslice) };
				}
				DDSVD.Flags = FLAG;
				return SUCCEEDED(cp->CreateDepthStencilView(t, &DDSVD, &dsv));
			}

			bool texture_ptr_type<2>::create_ms_DSV(Implement::resource_ptr& cp, Implement::depth_stencil_view_ptr& dsv, const type& t, bool dr, bool sr, size_t array_start, size_t array_count, bool all_range)
			{
				if (cp == nullptr || t == nullptr) return false;
				D3D11_TEXTURE2D_DESC DTD;
				t->GetDesc(&DTD);
				if (!avalible_depth_texture_format(DTD.Format)) return false;
				UINT FLAG = (dr ? D3D11_DSV_READ_ONLY_DEPTH : 0) | (sr ? D3D11_DSV_READ_ONLY_STENCIL : 0);
				D3D11_DEPTH_STENCIL_VIEW_DESC DDSVD{ DTD.Format };
				if (DTD.ArraySize > 1)
				{
					DDSVD.ViewDimension = D3D11_DSV_DIMENSION::D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY;
					using pair_t = std::pair<UINT, UINT>;
					pair_t pair = all_range ? pair_t{ 0, DTD.ArraySize } : pair_t{ static_cast<UINT>(array_start), static_cast<UINT>(array_count) };
					DDSVD.Texture2DMSArray = D3D11_TEX2DMS_ARRAY_DSV{ pair.first, pair.second };
				}
				else {
					DDSVD.ViewDimension = D3D11_DSV_DIMENSION::D3D11_DSV_DIMENSION_TEXTURE2DMS;
					DDSVD.Texture2DMS = D3D11_TEX2DMS_DSV{ };
				}
				DDSVD.Flags = FLAG;
				return SUCCEEDED(cp->CreateDepthStencilView(t, &DDSVD, &dsv));
			}

			bool texture_ptr_type<3>::create_RTV(Implement::resource_ptr& cp, Implement::render_view_ptr& rvp, const type& t, size_t mipslice, size_t array_start, size_t array_count, bool all_range)
			{
				if (cp == nullptr || t == nullptr) return false;
				D3D11_TEXTURE3D_DESC DTD;
				t->GetDesc(&DTD);
				D3D11_RENDER_TARGET_VIEW_DESC RTVD{ DTD.Format, D3D11_RTV_DIMENSION::D3D11_RTV_DIMENSION_TEXTURE3D };
				using pair_t = std::pair<UINT, UINT>;
				pair_t pair = all_range ? pair_t{ 0, DTD.Depth } : pair_t{ static_cast<UINT>(array_start), static_cast<UINT>(array_count) };
				RTVD.Texture3D = D3D11_TEX3D_RTV{ static_cast<UINT>(mipslice),  pair.first, pair.second };
				return SUCCEEDED(cp->CreateRenderTargetView(t, &RTVD, &rvp));
			}
			*/
		}

		void output_merge_d::set_render_implement(size_t o, Implement::render_view_ptr rv)
		{
			size_t cur = render_array.size();
			if (o + 1 > cur)
				render_array.insert(render_array.end(), o + 1 - cur, nullptr);
			auto& yu = render_array[o];
			if (yu != nullptr)
				yu->Release();
			yu = rv;
			if(yu!=nullptr) yu->AddRef();
		}

		output_merge_d::~output_merge_d()
		{
			for (auto &i : render_array)
				if (i != nullptr) i->Release();
		}

		input_assember_d::input_assember_d(const input_assember_d& ia)
			: vertex_array(ia.vertex_array), offset_array(ia.offset_array), element_array(ia.element_array), input_element(ia.input_element),
			primitive(ia.primitive), index_ptr(ia.index_ptr), offset(ia.offset), format(ia.format)
		{
			for (auto& itr : vertex_array)
				if (itr != nullptr) itr->AddRef();
		}

		input_assember_d::input_assember_d(input_assember_d&& ia)
			:vertex_array(std::move(ia.vertex_array)), offset_array(std::move(ia.offset_array)), element_array(std::move(ia.element_array)), input_element(std::move(ia.input_element)),
			primitive(ia.primitive), index_ptr(ia.index_ptr), offset(ia.offset), format(ia.format)
		{
			ia.index_ptr = nullptr;
		}

		input_assember_d::~input_assember_d()
		{
			for (auto& itr : vertex_array)
				if (itr != nullptr) itr->Release();
		}

		void input_assember_d::set_vertex_implement(Implement::buffer_ptr bp, size_t solt, const vertex_scr& vs)
		{
			size_t array_size = vertex_array.size();
			if (array_size >= solt)
			{
				size_t append = solt + 1 - array_size;
				vertex_array.reserve(solt + 1);
				vertex_array.insert(vertex_array.end(), append, nullptr);
				offset_array.reserve(solt + 1);
				offset_array.insert(offset_array.end(), append, 0);
				element_array.reserve(solt + 1);
				element_array.insert(element_array.end(), append, 0);
			}

			input_element.erase(std::remove_if(input_element.begin(), input_element.end(), [solt](D3D11_INPUT_ELEMENT_DESC& DI) {
				if (DI.InputSlot == solt)
					return true;
				return false;
			}), input_element.end());

			auto& arr = vertex_array[solt];
			if (arr != nullptr)
				arr->Release();
			arr = bp;
			if(arr != nullptr) arr->AddRef();
			offset_array[solt] = vs.offset;
			element_array[solt] = vs.element_size;
			auto ite = input_element.insert(input_element.end(), vs.layout.begin(), vs.layout.end());
			for (; ite != input_element.end(); ++ite)
				ite->InputSlot = static_cast<UINT>(solt);
		}

		void input_assember_d::set_index_implement(Implement::buffer_ptr bp, const index_scr& is)
		{
			index_ptr = bp;
			offset = is.offset;
			format = is.format;
		}

		namespace Implement
		{
			bool input_assember_resource_t::create_vertex_implement(Implement::buffer_ptr& bp, vertex_scr& scr, void* data, size_t ele, size_t num, std::vector<D3D11_INPUT_ELEMENT_DESC> layout, bool cpu_write)
			{
				Implement::buffer_ptr tem;
				if (SUCCEEDED(create_buffer(res, tem,
					(cpu_write ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE),
					(cpu_write ? D3D11_CPU_ACCESS_WRITE : 0),
					D3D11_BIND_VERTEX_BUFFER, data, static_cast<UINT>(ele * num), 0, 0)))
				{
					bp = tem;
					scr.element_size = static_cast<UINT>(ele);
					scr.num = num;
					scr.offset = 0;
					scr.layout = std::move(layout);
					return true;
				}
				return false;
			}

			bool input_assember_resource_t::create_index_implement(Implement::buffer_ptr& bp, index_scr& scr, void* data, size_t size, size_t num, DXGI_FORMAT DF, bool cpu_write)
			{
				Implement::buffer_ptr tem;
				if (SUCCEEDED(create_buffer(res, tem,
					(cpu_write ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE),
					(cpu_write ? D3D11_CPU_ACCESS_WRITE : 0),
					D3D11_BIND_INDEX_BUFFER, data, static_cast<UINT>(size * num), 0, 0))
					)
				{
					bp = tem;
					scr.format = DF;
					scr.num = num;
					scr.offset = 0;
					return true;
				}
				return false;
			}

			bool input_assember_resource_t::update_layout(input_assember_d& iad, const vertex_shader_d& vs)
			{
				if (res == nullptr) return false;
				Implement::layout_ptr lp;
				if (SUCCEEDED(res->CreateInputLayout(iad.input_element.data(), static_cast<UINT>(iad.input_element.size()), vs.code, static_cast<UINT>(vs.code.size()), &lp)))
				{
					iad.layout = lp;
					return true;
				}
				return false;
			}

			bool shader_resource_t::create_shader(vertex_shader_d& vsd, binary b)
			{
				if (res == nullptr) return false;
				Implement::vshader_ptr tem;
				size_t s = b.size();
				if (SUCCEEDED(res->CreateVertexShader(b, static_cast<UINT>(b.size()), nullptr, &tem)))
				{
					vsd.ptr = tem;
					vsd.code = std::move(b);
					return true;
				}
				return false;
			}

			bool shader_resource_t::create_shader(pixel_shader_d& vsd, const binary& b)
			{
				if (res == nullptr) return false;
				Implement::pshader_ptr tem;
				if (SUCCEEDED(res->CreatePixelShader(b, static_cast<UINT>(b.size()), nullptr, &tem)))
				{
					vsd.ptr = tem;
					return true;
				}
				return false;
			}

			bool output_merge_resource_t::create_render_view_ms(texture<2, texture_render_scr>& t, const texture_ptr_t<2>& p, size_t array_strat, size_t array_count)
			{
				if (texture_ptr_type<2>::create_ms_RTV(res, std::get<texture_render_scr>(t.scription).view, p, array_strat, array_count, false))
					return (t.ptr = p, true);
				return false;
			}

			bool output_merge_resource_t::create_render_view_ms(texture<2, texture_render_scr>& t, const texture_ptr_t<2>& p)
			{
				if (texture_ptr_type<2>::create_ms_RTV(res, std::get<texture_render_scr>(t.scription).view, p, 0, 0, true))
					return (t.ptr = p, true);
				return false;
			}

			bool output_merge_resource_t::create_depth_view_ms(texture<2, texture_depth_scr>& t, const texture_ptr_t<2>& p, bool dr, bool sr, size_t array_strat, size_t array_count)
			{
				if (texture_ptr_type<2>::create_ms_DSV(res, std::get<texture_depth_scr>(t.scription).view, p, dr, sr, array_strat, array_count, false))
					return (t.ptr = p, true);
				return false;
			}
	
			bool output_merge_resource_t::create_depth_view_ms(texture<2, texture_depth_scr>& t, const texture_ptr_t<2>& p, bool dr, bool sr)
			{
				if (texture_ptr_type<2>::create_ms_DSV(res, std::get<texture_depth_scr>(t.scription).view, p, dr, sr, 0, 0, true))
					return (t.ptr = p, true);
				return false;
			}

			bool input_assember_context_t::bind(input_assember_d& id)
			{
				if (cp == nullptr) return false;
				if (id.layout == nullptr) return false;
				cp->IASetInputLayout(id.layout);
				cp->IASetPrimitiveTopology(id.primitive);
				cp->IASetVertexBuffers(0, static_cast<UINT>(id.vertex_array.size()), id.vertex_array.data(), id.element_array.data(), id.offset_array.data());
				cp->IASetIndexBuffer(id.index_ptr, id.format, id.offset);
				if (id.vertex_array.size() > max_buffer_solt)
					max_buffer_solt = id.vertex_array.size();
				return true;
			}

			bool input_assember_context_t::rebind_layout(Implement::layout_ptr lp) 
			{ 
				if (cp == nullptr || lp == nullptr) return false; 
				cp->IASetInputLayout(lp);
				return true;
			}

			void input_assember_context_t::unbind()
			{
				if (cp == nullptr) return;
				static std::array<ID3D11Buffer*, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT> null_array = {};
				static std::array<UINT, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT> index_array = {};
				cp->IASetInputLayout(nullptr);
				cp->IASetVertexBuffers(0, static_cast<UINT>(max_buffer_solt), null_array.data(), index_array.data(), index_array.data());
				cp->IASetIndexBuffer(nullptr, DXGI_FORMAT::DXGI_FORMAT_A8_UNORM, 0);
				max_buffer_solt = 0;
			}

			bool vertex_shader_context_t::bind(const vertex_shader_d& vs)
			{
				if (cp == nullptr || vs.ptr == nullptr) return false;
				cp->VSSetShader(vs.ptr, nullptr, 0);
				return true;
			}

			void vertex_shader_context_t::unbind()
			{
				cp->VSSetShader(nullptr, nullptr, 0);
			}

			bool pixel_shader_context_t::bind(const pixel_shader_d& vs)
			{
				if (cp == nullptr || vs.ptr == nullptr) return false;
				cp->PSSetShader(vs.ptr, nullptr, 0);
				return true;
			}

			void pixel_shader_context_t::unbind()
			{
				if (cp == nullptr) return;
				cp->PSSetShader(nullptr, nullptr, 0);
			}

			bool output_merge_context_t::bind(const output_merge_d& od)
			{
				if (cp == nullptr) return false;
				cp->OMSetRenderTargets(static_cast<UINT>(od.render_array.size()), od.render_array.data(), od.depth);
				if (max_size < od.render_array.size())
					max_size = od.render_array.size();
				return true;
			}

			void output_merge_context_t::unbind()
			{
				if (cp == nullptr) return;
				static std::array<ID3D11RenderTargetView*, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> view_array = {};
				cp->OMSetRenderTargets(static_cast<UINT>(max_size), view_array.data(), nullptr);
				max_size = 0;
			}

			bool draw_range_context_t::draw(const draw_range_d& d)
			{
				if (cp == nullptr || !d.vertex) return false;
				if (d.index)
				{
					if (d.instance)
						cp->DrawIndexedInstanced(d.index.count(), d.instance.count(), d.index.at(), d.vertex.at(), d.instance.at());
					else
						cp->DrawIndexed(d.index.count(), d.index.at(), d.vertex.at());
				}
				else if (d.instance)
					cp->DrawInstanced(d.vertex.count(), d.instance.count(), d.vertex.at(), d.instance.at());
				else
					cp->Draw(d.vertex.count(), d.vertex.at());
				return true;
			}

		}
		

		/*
		bool vertex_factor::bind(Implement::resource_ptr& r)
		{
			std::for_each(vertex_ptr.begin(), vertex_ptr.end(), [](ID3D11Buffer*& ptr) {if (ptr != nullptr) { ptr->Release(); ptr = nullptr; } });
			vertex_ptr.clear(); vertex_offset.clear(); vertex_element.clear(); vertex_input_element.clear();
			vertex_layout = nullptr;
			index_ptr = nullptr;
			vshader = nullptr;
			gshader = nullptr;
			rp = r;
		}

		vertex_factor::~vertex_factor()
		{
			for (auto& itr : vertex_ptr)
				if (itr != nullptr)
					itr->Release();
		}

		bool vertex_factor::create_index(void* data, size_t element_size, size_t s, DXGI_FORMAT fo, D3D11_USAGE usage_flag, UINT cpu_falg, UINT flag)
		{
			if (rp == nullptr) return false;
			index_ptr = nullptr;
			HRESULT re = create_buffer(rp, index_ptr, usage_flag, cpu_falg, flag | D3D11_BIND_INDEX_BUFFER, data, s,  )
		}
		*/

		/*
		namespace Purpose
		{
			purpose input{ D3D11_USAGE::D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE, 0 };
			purpose output{ D3D11_USAGE::D3D11_USAGE_DEFAULT,  0, D3D11_BIND_FLAG::D3D11_BIND_STREAM_OUTPUT | D3D11_BIND_FLAG::D3D11_BIND_UNORDERED_ACCESS };
			purpose constant{ D3D11_USAGE::D3D11_USAGE_IMMUTABLE, 0, 0 };
			purpose transfer{ D3D11_USAGE::D3D11_USAGE_STAGING, UINT(D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_READ) | (UINT)(D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE),  0 };
		}

		bool buffer::create(Implement::resource_ptr& rp, D3D11_USAGE usage, UINT cpu_flag, UINT bind_flag, const void* data, size_t data_size, UINT misc_flag, size_t StructureByteStride)
		{
			//++vision;
			ptr = nullptr;
			D3D11_BUFFER_DESC DBD
			{
				static_cast<UINT>(data_size),
				usage,
				bind_flag,
				cpu_flag,
				misc_flag,
				static_cast<UINT>(StructureByteStride)
			};
			D3D11_SUBRESOURCE_DATA DSD{ data, 0, 0 };
			return rp->CreateBuffer(&DBD, ((data == nullptr) ? nullptr : &DSD), &ptr) == S_OK;
		}

		bool create_index_vertex_buffer(Implement::resource_ptr& rp, Purpose::purpose bp,
			const void* data, size_t buffer_size,
			index& ind, vertex& ver,
			size_t index_offset, DXGI_FORMAT format,
			size_t vertex_offset, size_t element_size, std::vector<D3D11_INPUT_ELEMENT_DESC> layout
		)
		{
			buffer b;
			ind.ptr = nullptr;
			ver.ptr = nullptr;
			if (b.create(
				rp, bp.usage, bp.cpu_flag, bp.additional_bind | D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER,
				data, buffer_size, 0, 0
			))
			{
				static_cast<buffer&>(ind) = b;
				static_cast<buffer&>(ver) = b;
				ind.offset = index_offset;
				ind.format = format;
				ver.desc = std::move(layout);
				ver.element_size = element_size;
				ver.offset = vertex_offset;
				return true;
			}
			return false;
		}

		bool pixel_creater::update_layout()
		{
			static std::vector<D3D11_INPUT_ELEMENT_DESC> des_buffer;
			static std::mutex buffer_mutex;
			if (rp == nullptr) return false;
			if (update_flag)
			{
				update_flag = false;
				layout = nullptr;
				std::lock_guard<std::mutex> lg(buffer_mutex);
				des_buffer.clear();
				for (size_t i = 0; i < D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT; ++i)
				{
					auto ite = des_buffer.insert(des_buffer.end(), vec[i].desc.begin(), vec[i].desc.end());
					for (; ite != des_buffer.end(); ++ite)
						ite->InputSlot = static_cast<UINT>(i);
				}
				update_flag = (rp->CreateInputLayout(des_buffer.data(), static_cast<UINT>(des_buffer.size()), vshader_binary, static_cast<UINT>(vshader_binary.size()), &layout) != S_OK);
			}
			return rp != nullptr;
		}

		DXGI_FORMAT adjust_texture_format(DXGI_FORMAT DF)
		{
			switch (DF)
			{
			case DXGI_FORMAT_R24G8_TYPELESS:
				return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			case DXGI_FORMAT_R32_TYPELESS:
				return DXGI_FORMAT_R32_FLOAT;
			}
			return DF;
		}

		bool pixel_creater::bind(Implement::resource_ptr& r)
		{
			update_flag = true;
			for (size_t i = 0; i < D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT; ++i)
				vec[i].clear();
			ind.clear();
			layout = nullptr;
			vshader = nullptr;
			gshader = nullptr;
			rp = r;
			if (rp != nullptr)
			{
				if (vshader_binary)
					rp->CreateVertexShader(vshader_binary, vshader_binary.size(), nullptr, &vshader);
				return true;
			}
			return false;
		}

		bool pixel_creater::load_vshader(std::u16string path)
		{
			if (rp == nullptr) return false;
			vshader = nullptr;
			if (vshader_binary.load_file(path))
				if (rp->CreateVertexShader(vshader_binary, vshader_binary.size(), nullptr, &vshader) == S_OK)
				{
					update_flag = true;
					return true;
				}
			return false;
		}

		bool pixel_creater::load_gshader(std::u16string path)
		{
			if (rp == nullptr) return false;
			gshader = nullptr;
			binary tem;
			if (tem.load_file(path))
				return rp->CreateGeometryShader(tem, tem.size(), nullptr, &gshader) == S_OK;
			return false;
		}

		bool pixel_creater::apply(Implement::context_ptr& cp)
		{
			if (!is_resource_available_for_context(rp,cp) || !update_layout()) return false;
			cp->IASetInputLayout(layout);
			cp->IASetPrimitiveTopology(primitive);
			cp->VSSetShader(vshader, nullptr, 0);
			ID3D11Buffer* array[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
			UINT offset[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
			UINT element[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
			for (size_t i = 0; i < D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT; ++i)
			{
				if (vec[i])
				{
					array[i] = vec[i].ptr;
					offset[i] = static_cast<UINT>(vec[i].offset);
					element[i] = static_cast<UINT>(vec[i].element_size);
				}
				else {
					array[i] = nullptr;
					offset[i] = 0;
					element[i] = 0;
				}
			}
			cp->IASetVertexBuffers(0, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT, array, element, offset);
			cp->IASetIndexBuffer(ind.ptr, ind.format, static_cast<UINT>(ind.offset));
			return true;
		}

		bool pixel_creater::draw(Implement::context_ptr& cp)
		{
			if (!is_resource_available_for_context(rp, cp)) return false;
			if (ind)
			{
				if (instance_r.count == 0)
					cp->DrawIndexed(index_r.count, index_r.start, vertex_r.start);
				else
					cp->DrawIndexedInstanced(index_r.count, instance_r.count, index_r.start, vertex_r.start, instance_r.start);
			}
			else {
				if (instance_r.count != 0)
					cp->DrawInstanced(vertex_r.count, instance_r.count, vertex_r.start, instance_r.start);
				else
					cp->Draw(vertex_r.count, vertex_r.start);
			}
			return true;
		}

		void material::bind(Implement::resource_ptr& r)
		{
			pshader = nullptr;
			rp = r;
		}
		
		bool pixel_state::apply(Implement::context_ptr& cp)
		{
			if (!is_resource_available_for_context(rp, cp) || !update()) return false;
			cp->RSSetState(rsp);
			return true;
		}

		bool pixel_state::update()
		{
			if (need_update)
			{
				rsp = nullptr;
				if (rp->CreateRasterizerState(&DRD, &rsp) != S_OK) return false;
				need_update = false;
			}
			return true;
		}

		void material_state::bind(Implement::resource_ptr& r)
		{
			dsp = nullptr;
			depth_stencil_update = true;
			bsp = nullptr;
			blend_update = true;
			rp = r;
		}

		bool material_state::update()
		{
			if (rp == nullptr) return false;
			if (depth_stencil_update)
			{
				dsp = nullptr;
				if (rp->CreateDepthStencilState(&DDSD, &dsp) != S_OK) return false;
				depth_stencil_update = false;
			}
			if (blend_update)
			{
				bsp = nullptr;
				HRESULT re = rp->CreateBlendState(&DBD, &bsp);
				if (re != S_OK) return false;
				blend_update = false;
			}
			return true;
		}

		bool material_state::apply(Implement::context_ptr& cp)
		{
			if (!is_resource_available_for_context(rp, cp) || !update()) return false;
			cp->OMSetBlendState(bsp, blend_factor.data(), sample_mask);
			cp->OMSetDepthStencilState(dsp, stencil_ref);
			return true;
		}

		Implement::texture2D_ptr create_render_target(Implement::resource_ptr& rp, size_t w, size_t h, DXGI_FORMAT DF)
		{
			Implement::texture2D_ptr ptr;
			D3D11_TEXTURE2D_DESC DTD
			{
				static_cast<UINT>(w),
				static_cast<UINT>(h),
				1,
				1,
				DF,
				DXGI_SAMPLE_DESC{1, 0},
				D3D11_USAGE_DEFAULT,
				D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
				0,
				0
			};
			rp->CreateTexture2D(&DTD, nullptr, &ptr);
			return ptr;
		}

		Implement::render_view_ptr cast_render_view(Implement::resource_ptr& rp, Implement::texture2D_ptr tp)
		{
			Implement::render_view_ptr ptr;
			if (rp == nullptr || tp == nullptr) return ptr;
			D3D11_TEXTURE2D_DESC tem;
			tp->GetDesc(&tem);
			if (
				((tem.BindFlags & D3D11_BIND_RENDER_TARGET) != D3D11_BIND_RENDER_TARGET) ||
				((tem.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE) == D3D11_RESOURCE_MISC_TEXTURECUBE)
				) return ptr;
			D3D11_RENDER_TARGET_VIEW_DESC DRTVD{ tem.Format };
			if (tem.ArraySize > 1)
			{
				DRTVD.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
				DRTVD.Texture2DArray = D3D11_TEX2D_ARRAY_RTV{0, 0, tem.ArraySize };
			}
			else {
				DRTVD.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
				DRTVD.Texture2D = D3D11_TEX2D_RTV{ 0 };
			}
			rp->CreateRenderTargetView(tp, &DRTVD, &ptr);
			return ptr;
		}*/

		/*
		bool constant_value::create_implement(void* data, size_t buffer_size, D3D11_USAGE usage, UINT cpu_flag)
		{
			if (rp == nullptr) return false;
			ID3D11Buffer* ptr = nullptr;
			D3D11_BUFFER_DESC DBD
			{
				static_cast<UINT>(buffer_size),
				usage,
				D3D11_BIND_CONSTANT_BUFFER,
				cpu_flag,
				0,
				static_cast<UINT>(0)
			};
			D3D11_SUBRESOURCE_DATA DSD{ data, 0, 0 };
			if (rp->CreateBuffer(&DBD, ((data == nullptr) ? nullptr : &DSD), &ptr) == S_OK)
			{
				buffer.push_back(ptr);
				return true;
			}
			return false;
		}
		*/

	}

























	/*
	namespace Dx11
	{
		
		namespace Implement
		{
			bool vertex_data::create_buffer_implement(
				Implement::resource_ptr& rp, D3D11_USAGE usage, UINT cpu_flag, UINT additional_bind,
				const void* data, size_t vertex_size, size_t vertex_count, size_t layout_count, void(*scr)(D3D11_INPUT_ELEMENT_DESC*, size_t solt)
			)
			{
				D3D11_BUFFER_DESC DBD
				{
					static_cast<UINT>(vertex_size * vertex_count),
					usage,
					D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER | additional_bind,
					cpu_flag,
					0,
					0
				};
				D3D11_SUBRESOURCE_DATA DSD{ data, 0, 0 };
				ptr = nullptr;
				HRESULT re = rp->CreateBuffer(&DBD, &DSD, &ptr);
				if (re == S_OK)
				{
					input_layout_count = layout_count;
					scription = scr;
					vision++;
					return true;
				}
				return false;
			}



			bool geometry_store::create_buffer_implement
			(
				Implement::resource_ptr& rp, D3D11_USAGE usage, UINT cpu_flag, UINT additional_bind,
				const void* data, size_t index_offset, size_t index_size, DXGI_FORMAT index_format,
				size_t vertex_offset, size_t vertex_size, size_t layout_count, void(*scription)(D3D11_INPUT_ELEMENT_DESC*, size_t solt)
			)
			{
				ptr = nullptr;
				D3D11_BUFFER_DESC DBD
				{
					static_cast<UINT>(vertex_size),
					usage,
					((index_size == 0) ? 0 : D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER ) |
					((vertex_size == 0) ? 0 : D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER ) |
					additional_bind,
					cpu_flag,
					0,
					0
				};
				D3D11_SUBRESOURCE_DATA DSD{ data, 0, 0 };
				HRESULT re = rp->CreateBuffer(&DBD, &DSD, &ptr);
				if (re == S_OK)
				{
					//input_layout_count = layout_count;
					//scription = scr;
					//vision++;
					return true;
				}
				return false;
			}

			bool geometry_store::create_buffer_implement
			(
				Implement::resource_ptr& rp, D3D11_USAGE usage, UINT cpu_flag, UINT additional_bind,
				const void* index_data, size_t index_size, DXGI_FORMAT index_format,
				const void* vertex_data, size_t vertex_size, size_t layout_count, void(*scription)(D3D11_INPUT_ELEMENT_DESC*, size_t solt)
			)
			{
				static std::vector<char> temporary_buffer;
				static std::mutex temporary_buffer_mutex;
				std::lock_guard<std::mutex> lg(temporary_buffer_mutex);
				temporary_buffer.resize(index_size + vertex_size, 0);
				std::memcpy(temporary_buffer.data(), index_data, index_size);
				std::memcpy(temporary_buffer.data() + index_size, vertex_data, vertex_size);
				ptr = nullptr;
				D3D11_BUFFER_DESC DBD
				{
					static_cast<UINT>(temporary_buffer.size()),
					usage,
					((index_size == 0) ? 0 : D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER) |
					((vertex_size == 0) ? 0 : D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER) |
					additional_bind,
					cpu_flag,
					0,
					0
				};
				D3D11_SUBRESOURCE_DATA DSD{ temporary_buffer.data(), 0, 0 };
				HRESULT re = rp->CreateBuffer(&DBD, &DSD, &ptr);
				if (re == S_OK)
				{
					//input_layout_count = layout_count;
					//scription = scr;
					//vision++;
					return true;
				}
				return false;
			}

			bool instance_store::create_buffer_implement(
				resource_ptr& rp, D3D11_USAGE usage, UINT cpu_flag, UINT additional_bind,
				const void* instance_data, size_t instance_size, size_t layout_count, void(*scription)(D3D11_INPUT_ELEMENT_DESC*, size_t solt)
			)
			{
				ptr = nullptr;
				D3D11_BUFFER_DESC DBD
				{
					static_cast<UINT>(instance_size),
					usage,
					D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER | additional_bind,
					cpu_flag,
					0,
					0
				};
				D3D11_SUBRESOURCE_DATA DSD{ instance_data, 0, 0 };
				HRESULT re = rp->CreateBuffer(&DBD, &DSD, &ptr);
				if (re == S_OK)
				{
					//input_layout_count = layout_count;
					//scription = scr;
					//vision++;
					return true;
				}
				return false;
			}

		}

















		vertex_pool::element_data::operator bool() const
		{
			return ptr && (
				ptr.able_cast<store_ref>() && static_cast<bool>(ptr.cast<store_ref>()) ||
				ptr.able_cast<weak_ref>() && !ptr.cast<weak_ref>().expired()
				);
		}

		auto vertex_pool::element_data::operator=(store_ref sr) ->element_data&
		{
			need_change = true;
			vision = sr->vision();
			ptr = std::move(sr);
			return *this;
		}

		HRESULT vertex_pool::create_vertex(size_t solt, void* data, size_t type_size, size_t data_size, size_t vertex_size, size_t layout_count, void(*func)(D3D11_INPUT_ELEMENT_DESC*, size_t solt))
		{
			element[solt].clear();
			if (res_ptr == nullptr)
				return E_INVALIDARG;
			D3D11_BUFFER_DESC DBD
			{
				static_cast<UINT>(data_size),
				D3D11_USAGE_IMMUTABLE,
				D3D11_BIND_VERTEX_BUFFER,
				0,
				0,
				0
			};
			Implement::buffer_ptr bp;
			D3D11_SUBRESOURCE_DATA DSD{ data,0,0 };
			HRESULT hre = res_ptr->CreateBuffer(&DBD, &DSD, &bp);
			if (SUCCEEDED(hre))
			{
				element[solt] = std::make_shared<vertex_buffer>(bp, layout_count, vertex_size, func);
				return S_OK;
			}
			return hre;
		}

		HRESULT vertex_pool::create_index(void* data, size_t data_size, DXGI_FORMAT DF)
		{
			index_ptr = {};
			if (res_ptr == nullptr)
				return E_INVALIDARG;
			D3D11_BUFFER_DESC DBD
			{
				static_cast<UINT>(data_size),
				D3D11_USAGE_IMMUTABLE,
				D3D11_BIND_VERTEX_BUFFER,
				0,
				0,
				0
			};
			Implement::buffer_ptr bp;
			D3D11_SUBRESOURCE_DATA DSD{ data,0,0 };
			HRESULT hre = res_ptr->CreateBuffer(&DBD, &DSD, &bp);
			if (SUCCEEDED(hre))
			{
				index_ptr = std::make_shared<index_buffer>(bp, DF, 0);
			}
			return hre;
		}

		bool vertex_pool::element_data::need_update()
		{
			bool result = false;
			if (ptr.able_cast<store_ref>() && ptr.cast<store_ref>())
				result = ptr.cast<store_ref>()->check_update(vision);
			else if (ptr.able_cast<weak_ref>() && !ptr.cast<weak_ref>().expired())
				result = (ptr.cast<weak_ref>().lock())->check_update(vision);
			if (need_change)
			{
				need_change = false;
				return true;
			}
			else
				return result;
		}

		void vertex_pool::clear()
		{
			for (size_t i = 0; i < D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT; ++i)
				element[i].clear();
			index_ptr = {};
			layout_state.clear();
			res_ptr.Release();
			primitive = D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			vertex = range{0, 0};
			instance = range{0, 0};
			index = range{0, 0};
		}
		
		bool vertex_pool::update(Implement::context_ptr& cp, binary& b)
		{
			if (res_ptr == nullptr)
				return false;
			binary::weak_ref bw(b);
			bool need_update = false;
			size_t element_layout_count = 0;
			for (size_t i = 0; i < D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT; ++i)
			{
				if (element[i])
				{
					auto& el = element[i].get_element();
					element_layout_count += el.input_layout_count;
				}
				need_update = element[i].need_update() || need_update;
			}
			if (element_layout_count == 0)
				return false;
			auto ite = layout_state.find(bw);
			if (ite != layout_state.end() && !need_update)
			{
				cp->IASetInputLayout(ite->second);
			}
			else {
				D3D11_INPUT_ELEMENT_DESC* tem = new D3D11_INPUT_ELEMENT_DESC[element_layout_count];
				auto array_ptr = tem;
				for (size_t i = 0; i < D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT; ++i)
				{
					if (element[i])
					{
						auto& el = element[i].get_element();
						(*el.scription)(array_ptr, i);
						array_ptr += el.input_layout_count;
					}
				}
				Implement::layout_ptr lay;
				HRESULT hre = res_ptr->CreateInputLayout(tem, static_cast<UINT>(element_layout_count), b, static_cast<UINT>(b.size()), &lay);
				//if()
				for (size_t i = 0; i < D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT; ++i)
				{
					D3D11_INPUT_ELEMENT_DESC pic = tem[i];
					i = i;
				}

				delete[](tem);
				layout_state[bw] = lay;
				cp->IASetInputLayout(lay);
			}
			size_t solt_count = 0;
			ID3D11Buffer* buffer_array[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
			UINT stride[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
			UINT offset[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
			for (size_t d = 0; d < D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT; ++d)
			{
				if (element[d])
				{
					auto& el = element[d].get_element();
					buffer_array[solt_count] = el.ptr;
					stride[solt_count] = static_cast<UINT>(el.vertex_size);
					offset[solt_count] = 0;
					++solt_count;
				}
			}
			cp->IASetVertexBuffers(0, static_cast<UINT>(solt_count), buffer_array, stride, offset);
			cp->IASetPrimitiveTopology(primitive);
			std::shared_ptr<index_buffer> index_ptr2;
			if (index_ptr.able_cast<std::shared_ptr<index_buffer>>())
				index_ptr2 = index_ptr.cast<std::shared_ptr<index_buffer>>();
			else if (index_ptr.able_cast<std::weak_ptr<index_buffer>>())
				index_ptr2 = index_ptr.cast<std::weak_ptr<index_buffer>>().lock();
			if (index_ptr2)
			{
				cp->IASetIndexBuffer(index_ptr2->ptr, index_ptr2->format, index_ptr2->offset);
				if (instance.count != 0)
					cp->DrawIndexedInstanced(static_cast<UINT>(index.count), static_cast<UINT>(instance.count), static_cast<UINT>(index.start), static_cast<UINT>(vertex.start), static_cast<UINT>(instance.start));
				else
					cp->DrawIndexed(static_cast<UINT>(index.count), static_cast<UINT>(index.start), static_cast<UINT>(vertex.start));
			}
			else {
				if (instance.count != 0)
					cp->DrawInstanced(static_cast<UINT>(vertex.count), static_cast<UINT>(instance.count), static_cast<UINT>(vertex.start), static_cast<UINT>(instance.start));
				else
					cp->Draw(static_cast<UINT>(vertex.count), static_cast<UINT>(vertex.start));
			}
			return true;
		}

		bool pipe_line::draw(Implement::context_ptr& cp, const_buffer& cb, vertex_pool& vp, size_t vertex_num)
		{
			if (res_ptr == nullptr)
				return false;
			if (true)
			{
				if (state_ptr == nullptr)
				{
					D3D11_RASTERIZER_DESC tem
					{
						D3D11_FILL_MODE::D3D11_FILL_SOLID,
						D3D11_CULL_MODE::D3D11_CULL_NONE,
						true,
						0,
						1.0,
						1.0,
						false
					};
					res_ptr->CreateRasterizerState(&tem, &state_ptr);
				}
				cp->RSSetState(state_ptr);
				cp->VSSetShader(vshader.ptr, nullptr, 0);
				cp->PSSetShader(pshader.ptr, nullptr, 0);
				cp->GSSetShader(gshader.ptr, nullptr, 0);
				return vp.update(cp, vshader.buffer);
			}
			return false;
		}

		HRESULT pipe_line::load_shader_v(binary&& b)
		{
			if (b && res_ptr != nullptr)
			{
				return vshader.load(res_ptr, std::move(b));
			}
			return E_INVALIDARG;
		}

		HRESULT pipe_line::load_shader_p(binary&& b)
		{
			if (b&& res_ptr != nullptr)
			{
				return pshader.load(res_ptr, std::move(b));
			}
			return E_INVALIDARG;
		}

		HRESULT pipe_line::load_shader_g(binary&& b)
		{
			if (b&& res_ptr != nullptr)
			{
				return gshader.load(res_ptr, std::move(b));
			}
			return E_INVALIDARG;
		}
	}
	*/
}