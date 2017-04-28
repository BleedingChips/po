#include "dx11_creator.h"
namespace PO
{
	namespace Dx11
	{
		namespace Implement
		{
			DXGI_FORMAT translate_depth_stencil_format_to_dxgi_format(DST_format dsf)
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

			/*****  input_assember_resource_t   ******************************************************************************************/
			bool input_assember_resource_t::create_vertex(vertex& v, const void* data, size_t ele, size_t num, std::vector<D3D11_INPUT_ELEMENT_DESC> layout, bool write_able)
			{
				buffer_ptr tem;
				if (SUCCEEDED(create_buffer(res, tem, (write_able ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE), D3D11_BIND_VERTEX_BUFFER, data, static_cast<UINT>(ele * num), 0, 0)))
					return (v = vertex{ tem, 0, static_cast<UINT>(ele), num, std::move(layout) }), true;
				return false;
			}

			bool input_assember_resource_t::create_index(index& i, const void* data, size_t size, size_t num, DXGI_FORMAT DF, bool write_able)
			{
				buffer_ptr tem;
				if ( SUCCEEDED(create_buffer(res, tem, (write_able ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE), D3D11_BIND_INDEX_BUFFER, data, static_cast<UINT>(size * num), 0, 0)))
					return (i = index{ tem, 0, DF , num }), true;
				return false;
			}

			bool input_assember_resource_t::update_layout(input_assember_d& iad, const vertex_shader_d& vs)
			{
				if (res == nullptr) return false;
				layout_ptr lp;
				if (SUCCEEDED(res->CreateInputLayout(iad.input_element.data(), static_cast<UINT>(iad.input_element.size()), vs.code, static_cast<UINT>(vs.code.size()), &lp)))
					return iad.layout = lp, true;
				return false;
			}

			static Tool::scope_lock<std::vector<char>> cbuffer_buffer;


			/*****  shader_resource_t   ******************************************************************************************/
			bool shader_resource_t::create_cbuffer(cbuffer& cb, size_t buffer_size, void* data, bool write_able)
			{
				size_t size = (buffer_size + 15) & ~(size_t{ 15 });
				if (size != buffer_size && data != nullptr)
				{
					return cbuffer_buffer.lock([&, this](decltype(cbuffer_buffer)::type& b) {
						b.resize(size);
						for (size_t i = 0; i < buffer_size; ++i)
							b[i] = static_cast<char*>(data)[i];
						buffer_ptr tem;
						if (SUCCEEDED(create_buffer(res, tem, (write_able ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE), D3D11_BIND_CONSTANT_BUFFER, b.data(), static_cast<UINT>(size), 0, 0)))
							return cb.ptr = tem, true;
						return false;
					});
				}
				else
				{
					buffer_ptr tem;
					if (SUCCEEDED(create_buffer(res, tem,  (write_able ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE), D3D11_BIND_CONSTANT_BUFFER , data, static_cast<UINT>(size), 0, 0)))
						return cb.ptr = tem, true;
					return false;
				}
			}

			bool shader_resource_t::cast_SRV(shader_d& sd, size_t solt, const texture2D_ptr& tp, size_t most_detail_mip, size_t miplevel) {
				SRV tem;
				if (SUCCEEDED(Dx11::cast_SRV(res, tem.ptr, tp, most_detail_mip, miplevel)))
					return sd.set_SRV(solt, tem), true;
				return false;
			}

			bool shader_resource_t::cast_SRV(shader_d& sd, size_t solt, const texture3D_ptr& tp, size_t most_detail_mip, size_t miplevel)
			{
				SRV tem;
				if (SUCCEEDED(Dx11::cast_SRV(res, tem.ptr, tp, most_detail_mip, miplevel)))
					return sd.set_SRV(solt, tem), true;
				return false;
			}

			bool shader_resource_t::create_sample(sample_d& sd, const sample_s& ss)
			{
				sample_state_ptr ssp;
				if (SUCCEEDED(res->CreateSamplerState(&ss.desc, &ssp)))
					return sd = sample_d{ ssp }, true;
				return false;
			}

			bool shader_resource_t::create_sample(shader_d& sd, size_t solt, const sample_s& ss)
			{
				sample_d tsd;
				if (create_sample(tsd, ss))
					return sd.set_sample(solt, tsd), true;
				return false;
			}

			/*****  vertex_shader_resource_t   ******************************************************************************************/
			bool vertex_shader_resource_t::create_shader(vertex_shader_d& vsd, binary b)
			{
				if (res == nullptr) return false;
				vshader_ptr tem;
				size_t s = b.size();
				if (SUCCEEDED(res->CreateVertexShader(b, static_cast<UINT>(b.size()), nullptr, &tem)))
				{
					vsd.ptr = tem;
					vsd.code = std::move(b);
					return true;
				}
				return false;
			}

			/*****  pixel_shader_resource_t   ******************************************************************************************/
			bool pixel_shader_resource_t::create_shader(pixel_shader_d& vsd, const binary& b)
			{
				if (res == nullptr) return false;
				pshader_ptr tem;
				if (SUCCEEDED(res->CreatePixelShader(b, static_cast<UINT>(b.size()), nullptr, &tem)))
				{
					vsd.ptr = tem;
					return true;
				}
				return false;
			}

			/*****  output_merge_resource_t   ******************************************************************************************/
			bool output_merge_resource_t::create_DST(texture2D_ptr& tp, DST_format DF, const texture2D_ptr& in)
			{
				D3D11_TEXTURE2D_DESC DTD;
				if (in == nullptr) return false;
				in->GetDesc(&DTD);
				D3D11_TEXTURE2D_DESC DTD2{ DTD.Width, DTD.Height, DTD.MipLevels, DTD.ArraySize,
					translate_depth_stencil_format_to_dxgi_format(DF), DTD.SampleDesc,
					D3D11_USAGE_DEFAULT, D3D11_BIND_DEPTH_STENCIL, translate_usage_to_cpu_flag(D3D11_USAGE_DEFAULT), 0
				};
				return SUCCEEDED(res->CreateTexture2D(&DTD2, nullptr, &tp));
			}

			bool output_merge_resource_t::create_state(blend_d& bs, const blend_s& bd)
			{
				if (res == nullptr) return false;
				blend_state_ptr tem;
				if (SUCCEEDED(res->CreateBlendState(&bd.desc, &tem)))
					return (bs = blend_d{ tem, bd.factor, bd.sample_mask }, true);
				return false;
			}

			bool output_merge_resource_t::create_state(depth_stencil_d& dss, const depth_stencil_s& dsd)
			{
				if (res == nullptr) return false;
				depth_stencil_state_ptr tem;
				if (SUCCEEDED(res->CreateDepthStencilState(&dsd.desc, &tem)))
					return (dss = depth_stencil_d{ tem, dsd.stencil_ref }, true);
				return false;
			}

			/*****  raterizer_resource_t   ******************************************************************************************/
			bool raterizer_resource_t::create_state(raterizer_d& rs, const raterizer_s& rd)
			{
				if (res == nullptr) return false;
				raterizer_state_ptr tem;
				if (SUCCEEDED(res->CreateRasterizerState(&rd.desc, &tem)))
					return (rs = raterizer_d{ tem, rd.viewports, rd.scissor }, true);
				return false;
			}

			bool raterizer_resource_t::create_state(raterizer_d& rs, raterizer_s&& rd)
			{
				if (res == nullptr) return false;
				raterizer_state_ptr tem;
				if (SUCCEEDED(res->CreateRasterizerState(&rd.desc, &tem)))
					return (rs = raterizer_d{ tem, std::move(rd.viewports), std::move(rd.scissor) }, true);
				return false;
			}

			/*****  compute_shader_resource_t   ******************************************************************************************/
			bool compute_shader_resource_t::cast_UAV(compute_d& up, size_t solt, const texture2D_ptr& t, size_t mipslice)
			{
				UAV u;
				if (SUCCEEDED(Dx11::cast_UAV(res, u.ptr, t, mipslice)))
					return up.set_UAV(solt, u), true;
				return false;
			}

			bool compute_shader_resource_t::cast_UAV(compute_d& up, size_t solt, const texture3D_ptr& t, size_t mipslice, size_t zstart, size_t zwidth)
			{
				UAV u;
				if (SUCCEEDED(Dx11::cast_UAV(res, u.ptr, t, mipslice, zstart, zwidth)))
					return up.set_UAV(solt, u), true;
				return false;
			}

		}
	}
}