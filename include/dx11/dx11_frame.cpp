#include "dx11_frame.h"
namespace PO
{
	namespace Dx11
	{

		//*************************************************************************  input_assember_d
		sample_state::scription sample_state::default_scription = sample_state::scription{
			D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP, 0.0f, 1,
			D3D11_COMPARISON_NEVER,{ 1.0f,1.0f,1.0f,1.0f }, -FLT_MAX, FLT_MAX
		};

		//*************************************************************************  viewports
		viewports& viewports::set(const D3D11_VIEWPORT& port, size_t solt)
		{
			if (views.size() <= solt)
				views.resize(solt + 1);
			views[solt] = port;
			return *this;
		}
		viewports& viewports::fill_texture(size_t solt, const tex2& t, float min_depth, float max_depth)
		{
			if (!t.ptr) throw E_INVALIDARG;
			D3D11_TEXTURE2D_DESC DTD;
			t.ptr->GetDesc(&DTD);
			return set(D3D11_VIEWPORT{0.0f, 0.0f, static_cast<float>(DTD.Width), static_cast<float>(DTD.Height), min_depth, max_depth}, solt);
		}
		viewports& viewports::capture_texture(size_t solt, const tex2& t, float top_left_x_rate, float top_left_y_rate, float button_right_x_rate, float button_right_y_rate, float min_depth, float max_depth)
		{
			if (!t.ptr) throw E_INVALIDARG;
			D3D11_TEXTURE2D_DESC DTD;
			t.ptr->GetDesc(&DTD);
			return set(D3D11_VIEWPORT{ DTD.Width * top_left_x_rate, DTD.Height * button_right_x_rate, DTD.Width * button_right_x_rate, DTD.Height * button_right_y_rate, min_depth, max_depth }, solt);
		}

		//*************************************************************************  raterizer_state
		raterizer_state::scription raterizer_state::default_scription = raterizer_state::scription{
			D3D11_FILL_SOLID, D3D11_CULL_BACK, FALSE, 0,0.0f, 0.0f, TRUE, FALSE,FALSE
		};

		//*************************************************************************  blend_state
		blend_state::scription blend_state::default_scription = blend_state::scription{
			FALSE, FALSE, D3D11_RENDER_TARGET_BLEND_DESC{ FALSE, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL }
		};

		//*************************************************************************  depth_stencil_state
		depth_stencil_state::scription depth_stencil_state::default_scription = depth_stencil_state::scription{
			TRUE, D3D11_DEPTH_WRITE_MASK_ALL, D3D11_COMPARISON_LESS, FALSE, D3D11_DEFAULT_STENCIL_READ_MASK, D3D11_DEFAULT_STENCIL_WRITE_MASK,
			D3D11_DEPTH_STENCILOP_DESC{ D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS },
			D3D11_DEPTH_STENCILOP_DESC{ D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS }
		};

		//*************************************************************************  shader_resource_d
		shader_stage& shader_stage::set(const constant_buffer& cb, size_t solt) { cbuffer_array.set(solt, cb.ptr); return *this; }
		shader_stage& shader_stage::set(const shader_resource_view& ptr, size_t solt) { shader_resource_view_array.set(solt, ptr.ptr); return *this; }
		shader_stage& shader_stage::set(const sample_state& sd, size_t solt) { sample_array.set(solt, sd.ptr); return *this; }

		//*************************************************************************  input_assember_d
		input_assember_stage& input_assember_stage::set(const vertex& v, size_t solt)
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
			return *this;
		}

		input_assember_stage& input_assember_stage::set(index ind)
		{
			index_ptr = std::move(ind.ptr);
			offset = ind.offset;
			format = ind.format;
			return *this;
		}

		input_assember_stage& input_assember_stage::set(const index_vertex& s, size_t solt)
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
			return *this;
		}

		//*************************************************************************  output_merge_d
		output_merge_stage& output_merge_stage::set(const render_target_view& rtv, size_t o) { render_array.set(o, rtv.ptr); return *this; }

		//*************************************************************************  compute_shader_d
		compute_stage& compute_stage::set(const unordered_access_view& uav, size_t solt)
		{
			UAV_array.set(solt, uav.ptr);
			if (solt >= offset.size())
				offset.insert(offset.end(), solt + 1 - offset.size(), 0);
			offset[solt] = uav.offset;
			return *this;
		}

		//*************************************************************************  creator

		DXGI_FORMAT creator::translate_depth_stencil_format_to_dxgi_format(DST_format dsf)
		{
			switch (dsf)
			{
			case DST_format::D16:
				return DXGI_FORMAT_D16_UNORM;
			case DST_format::D24_UI8:
				return DXGI_FORMAT_D24_UNORM_S8_UINT;
			case DST_format::F32:
				return DXGI_FORMAT_D32_FLOAT;
			case DST_format::F32_UI8:
				return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
			default:
				return DXGI_FORMAT_UNKNOWN;
			}
		}

		UINT creator::translate_usage_to_cpu_flag(D3D11_USAGE DU)
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

		sample_state creator::create_sample_state(const sample_state::scription& scri)
		{
			sample_state ss;
			HRESULT re = dev->CreateSamplerState(&scri, ss.ptr.adress());
			if (SUCCEEDED(re)) return ss;
			throw re;
		}

		raterizer_state creator::create_raterizer_state(const raterizer_state::scription& scri)
		{
			raterizer_state rs;
			HRESULT re = dev->CreateRasterizerState(&scri, rs.ptr.adress());
			if (SUCCEEDED(re)) return rs;
			throw re;
		}

		blend_state creator::create_blend_state(const blend_state::scription& scri, std::array<float, 4> bind_factor, UINT sample_mask)
		{
			blend_state bs;
			HRESULT re = dev->CreateBlendState(&scri, bs.ptr.adress());
			if (SUCCEEDED(re))
			{
				bs.bind_factor = bind_factor;
				bs.sample_mask = sample_mask;
				return bs;
			}
			throw re;
		}

		depth_stencil_state creator::create_depth_stencil_state(const depth_stencil_state::scription& scri, UINT stencil_ref)
		{
			depth_stencil_state dss;
			HRESULT re = dev->CreateDepthStencilState(&scri, dss.ptr.adress());
			if (SUCCEEDED(re))
			{
				dss.stencil_ref = stencil_ref;
				return dss;
			}
			throw re;
		}

		void creator::update_layout(input_assember_stage& ia, const vertex_shader& vd)
		{
			Win32::com_ptr<ID3D11InputLayout> lp;
			HRESULT re = dev->CreateInputLayout(ia.input_element.data(), static_cast<UINT>(ia.input_element.size()), vd.code, static_cast<UINT>(vd.code.size()), lp.adress());
			if (SUCCEEDED(re)) ia.layout = lp;
			else throw re;
		}

		Win32::com_ptr<ID3D11Buffer> creator::create_buffer_implement(UINT width, D3D11_USAGE DU, UINT BIND, UINT misc_flag, UINT struct_byte, const void* data)
		{
			D3D11_BUFFER_DESC DBD
			{
				width,
				DU,
				BIND,
				translate_usage_to_cpu_flag(DU),
				misc_flag,
				struct_byte
			};
			D3D11_SUBRESOURCE_DATA DSD{ data, 0, 0 };
			Win32::com_ptr<ID3D11Buffer> tem;
			HRESULT re = dev->CreateBuffer(&DBD, (data == nullptr ? nullptr : &DSD), tem.adress());
			if (SUCCEEDED(re))
				return tem;
			throw re;
		}

		vertex creator::create_vertex(const void* data, UINT ele, UINT num, std::vector<D3D11_INPUT_ELEMENT_DESC> layout, bool write_able)
		{
			return vertex{
				create_buffer_implement(ele * num, (write_able ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE), D3D11_BIND_VERTEX_BUFFER, 0, 0, data),
				0, ele, num, std::move(layout)
			};
		}

		index creator::create_index(const void* data, UINT size, UINT num, DXGI_FORMAT DF, bool write_able)
		{
			return index{
				create_buffer_implement(size * num, (write_able ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE), D3D11_BIND_INDEX_BUFFER, 0, 0, data),
				0, DF, num
			};
		}

		vertex_shader creator::create_vertex_shader(binary b)
		{
			Win32::com_ptr<ID3D11VertexShader> ptr;
			HRESULT re = dev->CreateVertexShader(b, static_cast<UINT>(b.size()), nullptr, ptr.adress());
			if (SUCCEEDED(re)) return vertex_shader{ std::move(b), std::move(ptr) };
			throw(re);
		}

		pixel_shader creator::create_pixel_shader(const binary& b)
		{
			Win32::com_ptr<ID3D11PixelShader> ptr;
			HRESULT re = dev->CreatePixelShader(b, static_cast<UINT>(b.size()), nullptr, ptr.adress());
			if (SUCCEEDED(re)) return pixel_shader{ std::move(ptr) };
			throw(re);
		}

		compute_shader creator::create_compute_shader(const binary& b)
		{
			Win32::com_ptr<ID3D11ComputeShader> ptr;
			HRESULT re = dev->CreateComputeShader(b, static_cast<UINT>(b.size()), nullptr, ptr.adress());
			if (SUCCEEDED(re)) return compute_shader{ std::move(ptr) };
			throw(re);
		}

		static Tool::scope_lock<std::vector<char>> cbuffer_buffer;

		constant_buffer creator::create_constant_buffer(UINT width, const void* data, bool write_enable)
		{
			UINT aligned_size = (width + 15) & ~(UINT{ 15 });
			aligned_size = aligned_size >= 64 ? aligned_size : 64;
			constant_buffer cb;
			if (aligned_size != width && data != nullptr)
			{
				cbuffer_buffer.lock([&, this](decltype(cbuffer_buffer)::type& b) {
					b.resize(aligned_size, '0');
					for (size_t i = 0; i < width; ++i)
						b[i] = static_cast<const char*>(data)[i];
					cb.ptr = create_buffer_implement(aligned_size, (write_enable ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE), D3D11_BIND_CONSTANT_BUFFER, 0, 0, b.data());
				});
			}
			else
				cb.ptr = create_buffer_implement(aligned_size, (write_enable ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE), D3D11_BIND_CONSTANT_BUFFER, 0, 0, data);
			return cb;
		}

		static Tool::scope_lock<std::vector<D3D11_SUBRESOURCE_DATA>> subresource_buffer;

		tex1 creator::create_tex1_implement(DXGI_FORMAT DF, UINT length, UINT miplevel, UINT count, D3D11_USAGE DU, UINT BIND, UINT misc, void** data)
		{
			D3D11_TEXTURE1D_DESC DTD{ static_cast<UINT>(length), static_cast<UINT>(miplevel), static_cast<UINT>(count),
				DF, DU, BIND, translate_usage_to_cpu_flag(DU), misc
			};
			tex1 tem;
			HRESULT re;
			if (data == nullptr)
				re = dev->CreateTexture1D(&DTD, nullptr, tem.ptr.adress());
			else {
				re = subresource_buffer.lock([&](decltype(subresource_buffer)::type& buffer) {
					buffer.resize(count);
					for (size_t i = 0; i < count; ++i)
						buffer[i] = D3D11_SUBRESOURCE_DATA{ data[i] };
					return dev->CreateTexture1D(&DTD, buffer.data(), tem.ptr.adress());
				});
			}
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		tex2 creator::create_tex2_implement(DXGI_FORMAT DF, UINT width, UINT height, UINT miplevel, UINT count, UINT sample_num, UINT sample_quality, D3D11_USAGE usage, UINT bind, UINT mis, void** data, UINT* line)
		{
			D3D11_TEXTURE2D_DESC DTD{ width, height, miplevel, count, DF, DXGI_SAMPLE_DESC{ sample_num, sample_quality },
				usage, bind, translate_usage_to_cpu_flag(usage), mis
			};
			tex2 tem;
			HRESULT re;
			if (data == nullptr)
				re = dev->CreateTexture2D(&DTD, nullptr, tem.ptr.adress());
			else
				re = subresource_buffer.lock([&](decltype(subresource_buffer)::type& buffer) {
				buffer.resize(count);
				for (size_t i = 0; i < count; ++i)
					buffer[i] = D3D11_SUBRESOURCE_DATA{ data[i], static_cast<UINT>(line[i]) };
				return dev->CreateTexture2D(&DTD, buffer.data(), tem.ptr.adress());
			});
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		tex3 creator::create_tex3_implement(DXGI_FORMAT DF, UINT width, UINT height, UINT depth, UINT miplevel, D3D11_USAGE usage, UINT bind, UINT mis, void* data, UINT line, UINT slice)
		{
			D3D11_TEXTURE3D_DESC DTD{ width, height, depth, miplevel,
				DF, usage, bind, translate_usage_to_cpu_flag(usage), mis
			};
			tex3 tem;
			HRESULT re;
			if (data == nullptr) re = dev->CreateTexture3D(&DTD, nullptr, tem.ptr.adress());
			else {
				D3D11_SUBRESOURCE_DATA DSD{ data, static_cast<UINT>(line), static_cast<UINT>(slice) };
				re = dev->CreateTexture3D(&DTD, &DSD, tem.ptr.adress());
			}
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		tex2 creator::create_tex2_render_target(DXGI_FORMAT DF, const tex2& t)
		{
			D3D11_TEXTURE2D_DESC DTD;
			auto p = t.ptr;
			p.ptr->GetDesc(&DTD);
			return create_tex2_implement(DF, DTD.Width, DTD.Height, DTD.MipLevels, DTD.ArraySize, DTD.SampleDesc.Count, DTD.SampleDesc.Quality, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET, DTD.MiscFlags, nullptr, nullptr);
		}

		tex2 creator::create_tex2_depth_stencil(DST_format DF, const tex2& t, void** data, UINT* line)
		{
			D3D11_TEXTURE2D_DESC DTD;
			auto p = t.ptr;
			p.ptr->GetDesc(&DTD);
			return create_tex2_implement(translate_depth_stencil_format_to_dxgi_format(DF), DTD.Width, DTD.Height, DTD.MipLevels, DTD.ArraySize, DTD.SampleDesc.Count, DTD.SampleDesc.Quality, D3D11_USAGE_DEFAULT, D3D11_BIND_DEPTH_STENCIL, 0, data, line);
		}

		shader_resource_view creator::cast_shader_resource_view(const structed_buffer& t, Tool::optional<UINT2> range)
		{
			shader_resource_view tem;
			D3D11_SHADER_RESOURCE_VIEW_DESC DSRVD{ DXGI_FORMAT::DXGI_FORMAT_UNKNOWN, D3D11_SRV_DIMENSION_BUFFER };
			auto ptr = t.ptr;
			if (range)
				DSRVD.Buffer = D3D11_BUFFER_SRV{ range->x, range->y };
			else {
				D3D11_BUFFER_DESC DBD;
				ptr->GetDesc(&DBD);
				DSRVD.Buffer = D3D11_BUFFER_SRV{ 0, DBD.ByteWidth / DBD.StructureByteStride };
			}
			HRESULT re = dev->CreateShaderResourceView(ptr, &DSRVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		shader_resource_view creator::cast_shader_resource_view(const tex1& t, Tool::optional<UINT2> mip_range)
		{
			shader_resource_view tem;
			auto ptr = t.ptr;
			D3D11_TEXTURE1D_DESC DTD;
			ptr->GetDesc(&DTD);
			D3D11_SHADER_RESOURCE_VIEW_DESC DSRVD{ DTD.Format, D3D11_SRV_DIMENSION_TEXTURE1D };
			UINT2 mip = mip_range ? *mip_range : UINT2{ 0, DTD.MipLevels };
			DSRVD.Texture1D = D3D11_TEX1D_SRV{ mip.x, mip.y };
			HRESULT re = dev->CreateShaderResourceView(ptr, &DSRVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		shader_resource_view creator::cast_shader_resource_view(const tex2& t, Tool::optional<UINT2> mip_range)
		{
			shader_resource_view tem;
			auto ptr = t.ptr;
			D3D11_TEXTURE2D_DESC DTD;
			ptr->GetDesc(&DTD);
			D3D11_SHADER_RESOURCE_VIEW_DESC DSRVD{ DTD.Format, D3D11_SRV_DIMENSION_TEXTURE2D };
			UINT2 mip = mip_range ? *mip_range : UINT2{ 0, DTD.MipLevels };
			DSRVD.Texture2D = D3D11_TEX2D_SRV{ mip.x, mip.y };
			HRESULT re = dev->CreateShaderResourceView(ptr, &DSRVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		shader_resource_view creator::cast_shader_resource_view_array(const tex1& t, Tool::optional<UINT2> mip_range, Tool::optional<UINT2> array)
		{
			shader_resource_view tem;
			auto ptr = t.ptr;
			D3D11_TEXTURE1D_DESC DTD;
			ptr->GetDesc(&DTD);
			D3D11_SHADER_RESOURCE_VIEW_DESC DSRVD{ DTD.Format, D3D11_SRV_DIMENSION_TEXTURE1DARRAY };
			UINT2 mip = mip_range ? *mip_range : UINT2{0, DTD.MipLevels};
			UINT2 arr = array ? *array : UINT2{ 0, DTD.ArraySize };
			DSRVD.Texture1DArray = D3D11_TEX1D_ARRAY_SRV{ mip.x, mip.y, arr.x, arr.y };
			HRESULT re = dev->CreateShaderResourceView(ptr, &DSRVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		shader_resource_view creator::cast_shader_resource_view_array(const tex2& t, Tool::optional<UINT2> mip_range, Tool::optional<UINT2> array)
		{
			shader_resource_view tem;
			auto ptr = t.ptr;
			D3D11_TEXTURE2D_DESC DTD;
			ptr->GetDesc(&DTD);
			D3D11_SHADER_RESOURCE_VIEW_DESC DSRVD{ DTD.Format, D3D11_SRV_DIMENSION_TEXTURE2DARRAY };
			UINT2 mip = mip_range ? *mip_range : UINT2{ 0, DTD.MipLevels };
			UINT2 arr = array ? *array : UINT2{ 0, DTD.ArraySize };
			DSRVD.Texture2DArray = D3D11_TEX2D_ARRAY_SRV{ mip.x, mip.y, arr.x, arr.y };
			HRESULT re = dev->CreateShaderResourceView(ptr, &DSRVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		shader_resource_view creator::cast_shader_resource_view_ms(const tex2& t)
		{
			shader_resource_view tem;
			auto ptr = t.ptr;
			D3D11_TEXTURE2D_DESC DTD;
			ptr->GetDesc(&DTD);
			D3D11_SHADER_RESOURCE_VIEW_DESC DSRVD{ DTD.Format, D3D11_SRV_DIMENSION_TEXTURE2DMS };
			DSRVD.Texture2DMS = D3D11_TEX2DMS_SRV{};
			HRESULT re = dev->CreateShaderResourceView(ptr, &DSRVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		shader_resource_view creator::cast_shader_resource_view_ms_array(const tex2& t, Tool::optional<UINT2> array)
		{
			shader_resource_view tem;
			auto ptr = t.ptr;
			D3D11_TEXTURE2D_DESC DTD;
			ptr->GetDesc(&DTD);
			D3D11_SHADER_RESOURCE_VIEW_DESC DSRVD{ DTD.Format, D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY };
			UINT2 arr = array ? *array : UINT2{ 0, DTD.ArraySize };
			DSRVD.Texture2DMSArray = D3D11_TEX2DMS_ARRAY_SRV{ arr.x, arr.y };
			HRESULT re = dev->CreateShaderResourceView(ptr, &DSRVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		shader_resource_view creator::cast_shader_resource_view_cube(const tex2& t, Tool::optional<UINT2> mip_range)
		{
			shader_resource_view tem;
			auto ptr = t.ptr;
			D3D11_TEXTURE2D_DESC DTD;
			ptr->GetDesc(&DTD);
			D3D11_SHADER_RESOURCE_VIEW_DESC DSRVD{ DTD.Format, D3D11_SRV_DIMENSION_TEXTURECUBE };
			UINT2 mip = mip_range ? *mip_range : UINT2{ 0, DTD.MipLevels };
			DSRVD.TextureCube = D3D11_TEXCUBE_SRV{ mip.x, mip.y };
			HRESULT re = dev->CreateShaderResourceView(ptr, &DSRVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		shader_resource_view creator::cast_shader_resource_view_cube_array(const tex2& t, Tool::optional<UINT2> mip_range, Tool::optional<UINT2> array)
		{
			shader_resource_view tem;
			auto ptr = t.ptr;
			D3D11_TEXTURE2D_DESC DTD;
			ptr->GetDesc(&DTD);
			D3D11_SHADER_RESOURCE_VIEW_DESC DSRVD{ DTD.Format, D3D11_SRV_DIMENSION_TEXTURECUBEARRAY };
			UINT2 mip = mip_range ? *mip_range : UINT2{ 0, DTD.MipLevels };
			UINT2 arr = array ? *array : UINT2{ 0, DTD.ArraySize / 6 };
			DSRVD.TextureCubeArray = D3D11_TEXCUBE_ARRAY_SRV{ mip.x, mip.y, arr.x, arr.y };
			HRESULT re = dev->CreateShaderResourceView(ptr, &DSRVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		shader_resource_view creator::cast_shader_resource_view(const tex3& t, Tool::optional<UINT2> mip_range)
		{
			shader_resource_view tem;
			auto ptr = t.ptr;
			D3D11_TEXTURE3D_DESC DTD;
			ptr->GetDesc(&DTD);
			D3D11_SHADER_RESOURCE_VIEW_DESC DSRVD{ DTD.Format, D3D11_SRV_DIMENSION_TEXTURE3D };
			UINT2 mip = mip_range ? *mip_range : UINT2{ 0, DTD.MipLevels };
			DSRVD.Texture3D = D3D11_TEX3D_SRV{ mip.x, mip.y };
			HRESULT re = dev->CreateShaderResourceView(ptr, &DSRVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		render_target_view creator::cast_render_target_view(const tex1& t, UINT miplevel)
		{
			render_target_view tem;
			auto ptr = t.ptr;
			D3D11_TEXTURE1D_DESC DTD;
			ptr->GetDesc(&DTD);
			D3D11_RENDER_TARGET_VIEW_DESC DRTVD{ DTD.Format, D3D11_RTV_DIMENSION_TEXTURE1D };
			DRTVD.Texture1D = D3D11_TEX1D_RTV{ miplevel };
			HRESULT re = dev->CreateRenderTargetView(ptr, &DRTVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		render_target_view creator::cast_render_target_view(const tex2& t, UINT miplevel)
		{
			render_target_view tem;
			auto ptr = t.ptr;
			D3D11_TEXTURE2D_DESC DTD;
			ptr->GetDesc(&DTD);
			D3D11_RENDER_TARGET_VIEW_DESC DRTVD{ DTD.Format, D3D11_RTV_DIMENSION_TEXTURE2D };
			DRTVD.Texture2D = D3D11_TEX2D_RTV{ miplevel };
			HRESULT re = dev->CreateRenderTargetView(ptr, &DRTVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		render_target_view creator::cast_render_target_view(const tex3& t, UINT miplevel, Tool::optional<UINT2> z_range)
		{
			render_target_view tem;
			auto ptr = t.ptr;
			D3D11_TEXTURE3D_DESC DTD;
			ptr->GetDesc(&DTD);
			D3D11_RENDER_TARGET_VIEW_DESC DRTVD{ DTD.Format, D3D11_RTV_DIMENSION_TEXTURE3D };
			UINT2 z = z_range ? *z_range : UINT2{ 0, DTD.Depth };
			DRTVD.Texture3D = D3D11_TEX3D_RTV{ miplevel, z.x, z.y };
			HRESULT re = dev->CreateRenderTargetView(ptr, &DRTVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		render_target_view creator::cast_render_target_view_array(const tex1& t, UINT miplevel, Tool::optional<UINT2> array_range)
		{
			render_target_view tem;
			auto ptr = t.ptr;
			D3D11_TEXTURE1D_DESC DTD;
			ptr->GetDesc(&DTD);
			D3D11_RENDER_TARGET_VIEW_DESC DRTVD{ DTD.Format, D3D11_RTV_DIMENSION_TEXTURE1DARRAY };
			UINT2 arr = array_range ? *array_range : UINT2{0, DTD.ArraySize};
			DRTVD.Texture1DArray = D3D11_TEX1D_ARRAY_RTV{ miplevel, arr.x, arr.y };
			HRESULT re = dev->CreateRenderTargetView(ptr, &DRTVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		render_target_view creator::cast_render_target_view_array(const tex2& t, UINT miplevel, Tool::optional<UINT2> array_range)
		{
			render_target_view tem;
			auto ptr = t.ptr;
			D3D11_TEXTURE2D_DESC DTD;
			ptr->GetDesc(&DTD);
			D3D11_RENDER_TARGET_VIEW_DESC DRTVD{ DTD.Format, D3D11_RTV_DIMENSION_TEXTURE2DARRAY };
			UINT2 arr = array_range ? *array_range : UINT2{ 0, DTD.ArraySize };
			DRTVD.Texture2DArray = D3D11_TEX2D_ARRAY_RTV{ miplevel, arr.x, arr.y };
			HRESULT re = dev->CreateRenderTargetView(ptr, &DRTVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		render_target_view creator::cast_render_target_view_ms(const tex2& t)
		{
			render_target_view tem;
			auto ptr = t.ptr;
			D3D11_TEXTURE2D_DESC DTD;
			ptr->GetDesc(&DTD);
			D3D11_RENDER_TARGET_VIEW_DESC DRTVD{ DTD.Format, D3D11_RTV_DIMENSION_TEXTURE2DMS };
			DRTVD.Texture2DMS = D3D11_TEX2DMS_RTV{};
			HRESULT re = dev->CreateRenderTargetView(ptr, &DRTVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		render_target_view creator::cast_render_target_view_ms_array(const tex2& t, Tool::optional<UINT2> array_range)
		{
			render_target_view tem;
			auto ptr = t.ptr;
			D3D11_TEXTURE2D_DESC DTD;
			ptr->GetDesc(&DTD);
			D3D11_RENDER_TARGET_VIEW_DESC DRTVD{ DTD.Format, D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY };
			UINT2 arr = array_range ? *array_range : UINT2{ 0, DTD.ArraySize };
			DRTVD.Texture2DMSArray = D3D11_TEX2DMS_ARRAY_RTV{ arr.x, arr.y };
			HRESULT re = dev->CreateRenderTargetView(ptr, &DRTVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		depth_stencil_view creator::cast_depth_setncil_view(const tex1& t, UINT miplevel, Tool::optional<bool> depth_read_only)
		{
			depth_stencil_view tem;
			auto ptr = t.ptr;
			D3D11_TEXTURE1D_DESC DTD;
			ptr->GetDesc(&DTD);
			D3D11_DEPTH_STENCIL_VIEW_DESC DDSVD{ DTD.Format, D3D11_DSV_DIMENSION_TEXTURE1D, 0 };
			DDSVD.Texture1D = D3D11_TEX1D_DSV{ miplevel };
			HRESULT re = dev->CreateDepthStencilView(ptr, &DDSVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		depth_stencil_view creator::cast_depth_setncil_view(const tex2& t, UINT miplevel, Tool::optional<bool> depth_read_only)
		{
			depth_stencil_view tem;
			auto ptr = t.ptr;
			D3D11_TEXTURE2D_DESC DTD;
			ptr->GetDesc(&DTD);
			D3D11_DEPTH_STENCIL_VIEW_DESC DDSVD{ DTD.Format, D3D11_DSV_DIMENSION_TEXTURE2D, 0 };
			DDSVD.Texture2D = D3D11_TEX2D_DSV{ miplevel };
			HRESULT re = dev->CreateDepthStencilView(ptr, &DDSVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		depth_stencil_view creator::cast_depth_setncil_view_array(const tex1& t, UINT miplevel, Tool::optional<UINT2> array_range, Tool::optional<bool> depth_read_only)
		{
			depth_stencil_view tem;
			auto ptr = t.ptr;
			D3D11_TEXTURE1D_DESC DTD;
			ptr->GetDesc(&DTD);
			D3D11_DEPTH_STENCIL_VIEW_DESC DDSVD{ DTD.Format, D3D11_DSV_DIMENSION_TEXTURE1DARRAY, 0 };
			UINT2 arr = array_range ? *array_range : UINT2{ 0, DTD.ArraySize };
			DDSVD.Texture1DArray = D3D11_TEX1D_ARRAY_DSV{ miplevel, arr.x, arr.y };
			HRESULT re = dev->CreateDepthStencilView(ptr, &DDSVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		depth_stencil_view creator::cast_depth_setncil_view_array(const tex2& t, UINT miplevel, Tool::optional<UINT2> array_range, Tool::optional<bool> depth_read_only)
		{
			depth_stencil_view tem;
			auto ptr = t.ptr;
			D3D11_TEXTURE2D_DESC DTD;
			ptr->GetDesc(&DTD);
			D3D11_DEPTH_STENCIL_VIEW_DESC DDSVD{ DTD.Format, D3D11_DSV_DIMENSION_TEXTURE2DARRAY, 0 };
			UINT2 arr = array_range ? *array_range : UINT2{ 0, DTD.ArraySize };
			DDSVD.Texture2DArray = D3D11_TEX2D_ARRAY_DSV{ miplevel, arr.x, arr.y };
			HRESULT re = dev->CreateDepthStencilView(ptr, &DDSVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		depth_stencil_view creator::cast_depth_setncil_view_ms(const tex2& t, Tool::optional<bool> depth_read_only)
		{
			depth_stencil_view tem;
			auto ptr = t.ptr;
			D3D11_TEXTURE2D_DESC DTD;
			ptr->GetDesc(&DTD);
			D3D11_DEPTH_STENCIL_VIEW_DESC DDSVD{ DTD.Format, D3D11_DSV_DIMENSION_TEXTURE2DMS, 0 };
			DDSVD.Texture2DMS = D3D11_TEX2DMS_DSV{};
			HRESULT re = dev->CreateDepthStencilView(ptr, &DDSVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		depth_stencil_view creator::cast_depth_setncil_view_ms_array(const tex2& t, Tool::optional<UINT2> array_range, Tool::optional<bool> depth_read_only)
		{
			depth_stencil_view tem;
			auto ptr = t.ptr;
			D3D11_TEXTURE2D_DESC DTD;
			ptr->GetDesc(&DTD);
			D3D11_DEPTH_STENCIL_VIEW_DESC DDSVD{ DTD.Format, D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY, 0 };
			UINT2 arr = array_range ? *array_range : UINT2{ 0, DTD.ArraySize };
			DDSVD.Texture2DMSArray = D3D11_TEX2DMS_ARRAY_DSV{ arr.x, arr.y };
			HRESULT re = dev->CreateDepthStencilView(ptr, &DDSVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		unordered_access_view creator::cast_unordered_access_view(const structed_buffer& tp, Tool::optional<UINT2> elemnt_start_and_count, Tool::optional<UINT> offset, bool is_append_or_consume)
		{
			unordered_access_view tem;
			tem.offset = offset ? *offset : -1;
			D3D11_UNORDERED_ACCESS_VIEW_DESC DUAVD{ DXGI_FORMAT_UNKNOWN, D3D11_UAV_DIMENSION_BUFFER };
			UINT2 element;
			if (elemnt_start_and_count)
				element = *elemnt_start_and_count;
			else {
				D3D11_BUFFER_DESC DBD;
				tp.ptr->GetDesc(&DBD);
				element = UINT2{ 0, DBD.ByteWidth / DBD.StructureByteStride };
			}
			DUAVD.Buffer = D3D11_BUFFER_UAV{ element.x, element.y, static_cast<UINT>((is_append_or_consume ? D3D11_BUFFER_UAV_FLAG_APPEND : D3D11_BUFFER_UAV_FLAG_COUNTER)) };
			HRESULT re = dev->CreateUnorderedAccessView(tp.ptr, &DUAVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		unordered_access_view creator::cast_unordered_access_view(const tex1& t, UINT mipslice)
		{
			unordered_access_view tem;
			auto ptr = t.ptr;
			D3D11_TEXTURE1D_DESC DTD;
			ptr->GetDesc(&DTD);
			D3D11_UNORDERED_ACCESS_VIEW_DESC DUAVD{ DTD.Format, D3D11_UAV_DIMENSION_TEXTURE1D };
			DUAVD.Texture1D = D3D11_TEX1D_UAV{ mipslice };
			HRESULT re = dev->CreateUnorderedAccessView(ptr, &DUAVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		unordered_access_view creator::cast_unordered_access_view_array(const tex1& t, UINT mipslice, Tool::optional<UINT2> array_range)
		{
			unordered_access_view tem;
			tem.offset = 0;
			D3D11_TEXTURE1D_DESC DTD;
			t.ptr->GetDesc(&DTD);
			D3D11_UNORDERED_ACCESS_VIEW_DESC DUAVD{ DTD.Format, D3D11_UAV_DIMENSION_TEXTURE1DARRAY };
			UINT2 arr = array_range ? *array_range : UINT2{ 0, DTD.ArraySize };
			DUAVD.Texture1DArray = D3D11_TEX1D_ARRAY_UAV{ mipslice, arr.x, arr.y };
			HRESULT re = dev->CreateUnorderedAccessView(t.ptr, &DUAVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		unordered_access_view creator::cast_unordered_access_view(const tex2& t, UINT mipslice)
		{
			unordered_access_view tem;
			tem.offset = 0;
			D3D11_TEXTURE2D_DESC DTD;
			t.ptr->GetDesc(&DTD);
			D3D11_UNORDERED_ACCESS_VIEW_DESC DUAVD{ DTD.Format, D3D11_UAV_DIMENSION_TEXTURE2D };
			DUAVD.Texture2D = D3D11_TEX2D_UAV{ mipslice };
			HRESULT re = dev->CreateUnorderedAccessView(t.ptr, &DUAVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		unordered_access_view creator::cast_unordered_access_view_array(const tex2& t, UINT mipslice, Tool::optional<UINT2> array_range)
		{
			unordered_access_view tem;
			tem.offset = 0;
			auto ptr = t.ptr;
			D3D11_TEXTURE2D_DESC DTD;
			t.ptr->GetDesc(&DTD);
			D3D11_UNORDERED_ACCESS_VIEW_DESC DUAVD{ DTD.Format, D3D11_UAV_DIMENSION_TEXTURE2DARRAY };
			UINT2 arr = array_range ? *array_range : UINT2{ 0, DTD.ArraySize };
			DUAVD.Texture2DArray = D3D11_TEX2D_ARRAY_UAV{ mipslice, arr.x, arr.y };
			HRESULT re = dev->CreateUnorderedAccessView(t.ptr, &DUAVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}

		unordered_access_view creator::cast_unordered_access_view(const tex3& t, UINT mipslice, Tool::optional<UINT2> z_range)
		{
			unordered_access_view tem;
			tem.offset = 0;
			D3D11_TEXTURE3D_DESC DTD;
			t.ptr->GetDesc(&DTD);
			D3D11_UNORDERED_ACCESS_VIEW_DESC DUAVD{ DTD.Format, D3D11_UAV_DIMENSION_TEXTURE3D };
			UINT2 arr = z_range ? *z_range : UINT2{ 0, DTD.Depth };
			DUAVD.Texture3D = D3D11_TEX3D_UAV{ mipslice, arr.x, arr.y };
			HRESULT re = dev->CreateUnorderedAccessView(t.ptr, &DUAVD, tem.ptr.adress());
			if (SUCCEEDED(re)) return tem;
			throw re;
		}


		namespace Implement
		{

			static std::array<ID3D11Buffer*, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT> input_assember_context_nullptr_array = {};
			static std::array<UINT, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT> input_assember_context_zero_array = {};

			/*****  input_assember_context_t   ******************************************************************************************/
			void input_assember_context_t::bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const input_assember_stage& id)
			{
				cp->IASetInputLayout(id.layout);
				cp->IASetPrimitiveTopology(id.primitive);
				if (id.vertex_array.size() < max_buffer_solt)
					cp->IASetVertexBuffers(static_cast<UINT>(id.vertex_array.size() - 1), static_cast<UINT>(max_buffer_solt - id.vertex_array.size()), input_assember_context_nullptr_array.data(),
						input_assember_context_zero_array.data(), input_assember_context_zero_array.data());
				max_buffer_solt = id.vertex_array.size();
				cp->IASetVertexBuffers(0, static_cast<UINT>(id.vertex_array.size()), id.vertex_array.data(), id.element_array.data(), id.offset_array.data());
				cp->IASetIndexBuffer(id.index_ptr, id.format, id.offset);
			}

			void input_assember_context_t::unbind(Win32::com_ptr<ID3D11DeviceContext>& cp)
			{
				cp->IASetInputLayout(nullptr);
				cp->IASetVertexBuffers(0, static_cast<UINT>(max_buffer_solt), input_assember_context_nullptr_array.data(), input_assember_context_zero_array.data(), input_assember_context_zero_array.data());
				cp->IASetIndexBuffer(nullptr, DXGI_FORMAT::DXGI_FORMAT_R16_UINT, 0);
				max_buffer_solt = 0;
			}

			void input_assember_context_t::clear(Win32::com_ptr<ID3D11DeviceContext>& cp)
			{
				unbind(cp);
			}

			static std::array<ID3D11Buffer*, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT> nullptr_cbuffer;
			static std::array<ID3D11ShaderResourceView*, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT> nullptr_shader_resource_view;
			static std::array<ID3D11SamplerState*, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT> nullptr_sampler_state;

			/*****  vertex_shader_context_t   ******************************************************************************************/
			void vertex_shader_context_t::bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const vertex_stage& vs)
			{
				cp->VSSetShader(vs.code.ptr, nullptr, 0);
				if (!vs.cbuffer_array.empty())
					cp->VSSetConstantBuffers(0, static_cast<UINT>(vs.cbuffer_array.size()), vs.cbuffer_array.data());

				if (!vs.shader_resource_view_array.empty())
				{
					cp->VSSetShaderResources(0, static_cast<UINT>(vs.shader_resource_view_array.size()), vs.shader_resource_view_array.data());
					max_shader_resource_view = max_shader_resource_view < vs.shader_resource_view_array.size() ? vs.shader_resource_view_array.size() : max_shader_resource_view;
				}

				if (!vs.sample_array.empty()) {
					cp->VSSetSamplers(0, static_cast<UINT>(vs.sample_array.size()), vs.sample_array.data());
					max_sample = max_sample < vs.sample_array.size() ? vs.sample_array.size() : max_sample;
				}
					
			}

			void vertex_shader_context_t::unbind(Win32::com_ptr<ID3D11DeviceContext>& cp)
			{
				cp->VSSetShader(nullptr, nullptr, 0);
				if (max_shader_resource_view != 0)
				{
					cp->VSSetShaderResources(0, static_cast<UINT>(max_shader_resource_view), nullptr_shader_resource_view.data());
					max_shader_resource_view = 0;
				}
					
			}
			void vertex_shader_context_t::clear(Win32::com_ptr<ID3D11DeviceContext>& cp)
			{
				cp->VSSetConstantBuffers(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, nullptr_cbuffer.data());
				if (max_sample != 0)
				{
					cp->VSSetSamplers(0, static_cast<UINT>(max_sample), nullptr_sampler_state.data());
					max_sample = 0;
				}
			}

			/*****  raterizer_context_t   ******************************************************************************************/
			void raterizer_context_t::bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const raterizer_state& rs)
			{
				cp->RSSetState(rs.ptr);
				//cp->RSSetScissorRects(static_cast<UINT>(rs.scissor.size()), rs.scissor.data());
				//cp->RSSetViewports(static_cast<UINT>(rs.viewports.size()), rs.viewports.data());
				//cp->PSS
			}

			void raterizer_context_t::bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const viewports& rs)
			{
				//cp->RSSetState(rs.ptr);
				cp->RSSetScissorRects(static_cast<UINT>(rs.scissor.size()), rs.scissor.data());
				cp->RSSetViewports(static_cast<UINT>(rs.views.size()), rs.views.data());
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

			/*****  pixel_shader_context_t   ******************************************************************************************/
			void pixel_shader_context_t::bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const pixel_stage& vs)
			{
				cp->PSSetShader(vs.code.ptr, nullptr, 0);
				if (!vs.cbuffer_array.empty())
					cp->PSSetConstantBuffers(0, static_cast<UINT>(vs.cbuffer_array.size()), vs.cbuffer_array.data());

				if (!vs.shader_resource_view_array.empty())
				{
					cp->PSSetShaderResources(0, static_cast<UINT>(vs.shader_resource_view_array.size()), vs.shader_resource_view_array.data());
					max_shader_resource_view = max_shader_resource_view < vs.shader_resource_view_array.size() ? vs.shader_resource_view_array.size() : max_shader_resource_view;
				}


				if (!vs.sample_array.empty()) {
					cp->PSSetSamplers(0, static_cast<UINT>(vs.sample_array.size()), vs.sample_array.data());
					max_sample = max_sample < vs.sample_array.size() ? vs.sample_array.size() : max_sample;
				}

			}

			void pixel_shader_context_t::unbind(Win32::com_ptr<ID3D11DeviceContext>& cp)
			{
				cp->PSSetShader(nullptr, nullptr, 0);

				if (max_shader_resource_view != 0)
				{
					cp->PSSetShaderResources(0, static_cast<UINT>(max_shader_resource_view), nullptr_shader_resource_view.data());
					max_shader_resource_view = 0;
				}
					
			}

			void pixel_shader_context_t::clear(Win32::com_ptr<ID3D11DeviceContext>& cp)
			{
				unbind(cp);
				cp->PSSetConstantBuffers(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, nullptr_cbuffer.data());
				if (max_sample != 0)
				{
					cp->PSSetSamplers(0, static_cast<UINT>(max_sample), nullptr_sampler_state.data());
					max_sample = 0;
				}
				
			}
			
			static std::array<ID3D11RenderTargetView*, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> nullptr_render_target;
			/*****  output_merge_context_t   ******************************************************************************************/
			void output_merge_context_t::bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const output_merge_stage& od)
			{
				if (max_render_target > od.render_array.size()) 
					cp->OMSetRenderTargets(static_cast<UINT>(max_render_target), nullptr_render_target.data(), nullptr);
				max_render_target = od.render_array.size();
				cp->OMSetRenderTargets(static_cast<UINT>(od.render_array.size()), od.render_array.data(), od.depth.ptr);
			}

			void output_merge_context_t::bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const blend_state& bs) {
				cp->OMSetBlendState(bs.ptr, bs.bind_factor.data(), bs.sample_mask);
			}
			void output_merge_context_t::bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const depth_stencil_state& dss) {
				cp->OMSetDepthStencilState(dss.ptr, dss.stencil_ref);
			}

			void output_merge_context_t::unbind(Win32::com_ptr<ID3D11DeviceContext>& cp)
			{
				cp->OMSetRenderTargets(static_cast<UINT>(max_render_target), nullptr_render_target.data(), nullptr);
				max_render_target = 0;
			}

			void output_merge_context_t::clear(Win32::com_ptr<ID3D11DeviceContext>& cp)
			{
				unbind(cp);
				float co[4] = { 1.0, 1.0, 1.0, 1.0 };
				cp->OMSetBlendState(nullptr, co, 0);
				cp->OMSetDepthStencilState(nullptr, 0);
			}

			void output_merge_context_t::clear_render_target(Win32::com_ptr<ID3D11DeviceContext>& cp, output_merge_stage& omd, size_t solt, const std::array<float, 4>& color)
			{
				if (omd.render_array.size() > solt)
					cp->ClearRenderTargetView(omd.render_array[solt], color.data());
			}

			void output_merge_context_t::clear_render_target(Win32::com_ptr<ID3D11DeviceContext>& cp, output_merge_stage& omd, const std::array<float, 4>& color)
			{
				for (auto& ra : omd.render_array)
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
			static std::array<UINT, D3D11_1_UAV_SLOT_COUNT> nullptr_unordered_access_offset_array;

			void compute_shader_context_t::bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const compute_stage& cd)
			{
				cp->CSSetShader(cd.code.ptr, nullptr, 0);

				cp->CSSetUnorderedAccessViews(0, static_cast<UINT>(cd.UAV_array.size()), cd.UAV_array.data(), cd.offset.data());
				max_unorder_access_view = max_unorder_access_view < cd.UAV_array.size() ? cd.UAV_array.size() : max_unorder_access_view;

				if (!cd.cbuffer_array.empty())
					cp->CSSetConstantBuffers(0, static_cast<UINT>(cd.cbuffer_array.size()), cd.cbuffer_array.data());

				if (!cd.shader_resource_view_array.empty())
				{
					cp->CSSetShaderResources(0, static_cast<UINT>(cd.shader_resource_view_array.size()), cd.shader_resource_view_array.data());
					max_shader_resource_view = max_shader_resource_view < cd.shader_resource_view_array.size() ? cd.shader_resource_view_array.size() : max_shader_resource_view;
				}
				
				if (!cd.sample_array.empty()) {
					cp->CSSetSamplers(0, static_cast<UINT>(cd.sample_array.size()), cd.sample_array.data());
					max_sample = max_sample < cd.sample_array.size() ? cd.sample_array.size() : max_sample;
				}
					
			}

			void compute_shader_context_t::unbind(Win32::com_ptr<ID3D11DeviceContext>& cp)
			{
				cp->CSSetShader(nullptr, nullptr, 0);
				if (max_shader_resource_view != 0)
				{
					cp->CSSetShaderResources(0, static_cast<UINT>(max_shader_resource_view), nullptr_shader_resource_view.data());
					max_shader_resource_view = 0;
				}
				if (max_unorder_access_view != 0)
				{
					cp->CSSetUnorderedAccessViews(0, static_cast<UINT>(max_unorder_access_view), nullptr_unordered_access_array.data(), nullptr_unordered_access_offset_array.data());
					max_unorder_access_view = 0;
				}
			}

			void compute_shader_context_t::clear(Win32::com_ptr<ID3D11DeviceContext>& cp)
			{
				unbind(cp);
				cp->CSSetConstantBuffers(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, nullptr_cbuffer.data());
				if (max_sample != 0)
				{
					cp->CSSetSamplers(0, static_cast<UINT>(max_sample), nullptr_sampler_state.data());
					max_sample = 0;
				}
				
			}

		}

		/*****  pipe   ******************************************************************************************/

		void pipeline::unbind() {
			CS.unbind(ptr); IA.unbind(ptr); VS.unbind(ptr); RA.unbind(ptr);
			PS.unbind(ptr); OM.unbind(ptr);
			last_mode = DrawMode::NONE;
		}

		void pipeline::dispatch(UINT x, UINT y, UINT z) {
			if (last_mode == DrawMode::PIPELINE) {
				IA.unbind(ptr); VS.unbind(ptr); RA.unbind(ptr);
				PS.unbind(ptr); OM.unbind(ptr);
			}
			last_mode = DrawMode::COMPLUTE;
			ptr->Dispatch(x, y, z);
		}

		void pipeline::draw_vertex(UINT count, UINT start) {
			if (last_mode == DrawMode::PIPELINE) {
				CS.unbind(ptr);
			}
			last_mode = DrawMode::PIPELINE;
			ptr->Draw(count, start);
		}

		void pipeline::draw_index(UINT index_count, UINT index_start, UINT vertex_start) {
			if (last_mode == DrawMode::PIPELINE) {
				CS.unbind(ptr);
			}
			last_mode = DrawMode::PIPELINE;
			ptr->DrawIndexed(index_count, index_start, vertex_start);
		}

		void pipeline::draw_vertex_instance(UINT vertex_pre_instance, UINT instance_count, UINT vertex_start, UINT instance_start) {
			if (last_mode == DrawMode::PIPELINE) {
				CS.unbind(ptr);
			}
			last_mode = DrawMode::PIPELINE;
			ptr->DrawInstanced(vertex_pre_instance, instance_count, vertex_start, instance_start);
		}

		void pipeline::draw_index_instance(UINT index_pre_instance, UINT index_count, UINT index_start, UINT vertex_start, UINT instance_start) {
			if (last_mode == DrawMode::PIPELINE) {
				CS.unbind(ptr);
			}
			last_mode = DrawMode::PIPELINE;
			ptr->DrawIndexedInstanced(index_pre_instance, index_count, index_start, vertex_start, instance_start);
		}

		void pipeline::clear() {
			CS.clear(ptr); IA.clear(ptr); VS.clear(ptr); RA.clear(ptr);
			PS.clear(ptr); OM.clear(ptr);
		}
	}
}