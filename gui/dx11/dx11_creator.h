#pragma once
#include "dx11_define.h"
namespace PO
{
	namespace Dx11
	{
		enum class DST_format
		{
			D16,
			D24_UI8,
			F32,
			F32_UI8,
			UNKNOW
		};

		namespace Implement
		{

			DXGI_FORMAT translate_depth_stencil_format_to_dxgi_format(DST_format dsf);

			struct input_assember_resource_t
			{
				device_ptr& res;
				input_assember_resource_t(device_ptr& r) :res(r) {}

				bool create_vertex(vertex& v, const void* data, size_t ele, size_t num, std::vector<D3D11_INPUT_ELEMENT_DESC> layout, bool write_able = false);
				bool create_index(index& i, const void* data, size_t size, size_t num, DXGI_FORMAT DF, bool write_able = false);

				template<typename K> bool create_vertex(vertex& b, K* data, size_t num, std::vector<D3D11_INPUT_ELEMENT_DESC> layout, bool write_able = false) { return create_vertex(b, data, sizeof(K), num, std::move(layout), write_able); }
				template<typename K, typename A> bool create_vertex(vertex& b, const std::vector<K, A>& v, std::vector<D3D11_INPUT_ELEMENT_DESC> layout, bool write_able = false) { return create_vertex(b, v.data(), v.size(), std::move(layout), write_able); }
				template<typename K, typename A> bool create_index(index& b, K* data, size_t num, bool write_able = false) { return create_index(b, data, num, DXGI::data_format<K>::format, write_able); }
				template<typename K, typename A> bool create_index(index& b, const std::vector<K, A>& v, bool write_able = false) { return create_index(b, v.data(), sizeof(K), v.size(), DXGI::data_format<K>::format, write_able); }

				template<typename T, typename K>
				bool create_vertex(input_assember_d& iad, size_t solt, const std::vector<T, K>& v, std::vector<D3D11_INPUT_ELEMENT_DESC> layout, bool write_able = false) { vertex b; if (create_vertex(b, v, std::move(layout), write_able)) return (iad.set_vertex(solt, b), true); return false; }

				template<typename T>
				bool create_vertex(input_assember_d& iad, size_t solt, T* data, size_t size, std::vector<D3D11_INPUT_ELEMENT_DESC> layout, bool write_able = false) { vertex b; if (create_vertex(b, data, size, std::move(layout), write_able)) return (iad.set_vertex(solt, b), true); return false; }

				template<typename T, typename K>
				bool create_index(input_assember_d& iad, const std::vector<T, K>& v, bool write_able = false) { index b; if (create_index(b, v, write_able)) return (iad.set_index(b), true); return false; }

				bool update_layout(input_assember_d& iad, const vertex_shader_d& vs);
			};

			struct shader_resource_t
			{
				device_ptr& res;
				shader_resource_t(device_ptr& r) :res(r) {}
				//bool create_shader(vertex_shader_d& vsd, binary b);
				bool create_cbuffer(cbuffer& cb, size_t buffer_size, void* data = nullptr, bool write_able = true);
				template<typename T> bool create_cbuffer(cbuffer& cb, T* data, bool write_able = true) { return create_cbuffer(cb, sizeof(T), data, write_able); }
				template<typename T> bool create_cbuffer(shader_d& cb, size_t solt, T* data = nullptr, bool write_able = true) { cbuffer tem; if (create_cbuffer(tem, sizeof(T), data, write_able)) return cb.set_cbuffer(solt, tem), true; return false; }
				template<typename T> bool create_cbuffer(shader_d& cb, size_t solt, size_t size) { cbuffer tem; if (create_cbuffer(tem, size, nullptr, true)) return cb.set_cbuffer(solt, tem), true; return false; }

				bool create_sbuffer(sbuffer& sb, size_t element_size, size_t element_num, const void* data = nullptr, bool write_able = true);
				template<typename T> bool create_sbuffer(sbuffer& cb, T* data, size_t num, bool write_able = true) { return create_sbuffer(cb, sizeof(T), num, data, write_able); }
				template<typename T, typename K> bool create_sbuffer(sbuffer& cb, const std::vector<T, K>& v, bool write_able = true) { return create_sbuffer(cb, v.data(), v.size(), write_able); }

				bool create_tex2(texture2D_ptr& tp, DXGI_FORMAT DF, size_t w, size_t h, size_t miplevel, void* data = nullptr, size_t size = 0, bool write_able = true) {
					//create_tex2(device_ptr& cp, texture2D_ptr& t, DXGI_FORMAT format, size_t width, size_t height, size_t miplevel, D3D11_USAGE usage, UINT bind, void* data = nullptr, size_t line = 0)
					return SUCCEEDED(Dx11::create_tex2(res, tp, DF, w, h, miplevel, (write_able ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE), D3D11_BIND_SHADER_RESOURCE, data, size));
				}

				bool cast_SRV(SRV& srv, const sbuffer& s, size_t bit_offset, size_t element_count);
				bool cast_SRV(SRV& srv, const sbuffer& s);
				bool cast_SRV(SRV& srv, const texture2D_ptr& tp, size_t most_detail_mip, size_t miplevel);
				bool cast_SRV(SRV& srv, const texture3D_ptr& tp, size_t most_detail_mip, size_t miplevel);

				bool cast_SRV(shader_d& sd, size_t solt, const sbuffer& s, size_t bit_offset, size_t element_count);
				bool cast_SRV(shader_d& sd, size_t solt, const sbuffer& s);
				bool cast_SRV(shader_d& sd, size_t solt, const texture2D_ptr& tp, size_t most_detail_mip, size_t miplevel);
				bool cast_SRV(shader_d& sd, size_t solt, const texture3D_ptr& tp, size_t most_detail_mip, size_t miplevel);
				bool create_sample(sample_d& sd, const sample_s& ss);
				bool create_sample(shader_d& sd, size_t solt, const sample_s& ss);
			};

			struct vertex_shader_resource_t : shader_resource_t
			{
				using shader_resource_t::shader_resource_t;
				bool create_shader(vertex_shader_d& vsd, binary b);
			};

			struct pixel_shader_resource_t : shader_resource_t
			{
				using shader_resource_t::shader_resource_t;
				bool create_shader(pixel_shader_d& psd, const binary& b);
			};

			struct output_merge_resource_t
			{
				device_ptr& res;
				output_merge_resource_t(device_ptr& r) :res(r) {}

				bool create_DST(texture2D_ptr& tp, DST_format DF, size_t w, size_t h, size_t mip, void* data = nullptr, size_t data_length = 0) {
					return SUCCEEDED(Dx11::create_tex2(res, tp, translate_depth_stencil_format_to_dxgi_format(DF), w, h , mip, D3D11_USAGE_DEFAULT, D3D11_BIND_DEPTH_STENCIL, data, data_length));
				}
				bool create_DS_RS_T(texture2D_ptr& tp, size_t w, size_t h, size_t mip, void* data = nullptr, size_t data_length = 0) {
					return SUCCEEDED(Dx11::create_tex2(res, tp, translate_depth_stencil_format_to_dxgi_format(DST_format::D24_UI8), w, h, mip, D3D11_USAGE_DEFAULT, D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE, data, data_length));
				}
				bool create_DST(texture2D_ptr& tp, DST_format DF, const texture2D_ptr& in);
				bool create_RT_T(texture2D_ptr& tp, DXGI_FORMAT DF, size_t w, size_t h, size_t mip, void* data = nullptr, size_t data_length = 0) {
					return SUCCEEDED(Dx11::create_tex2(res, tp, DF, w, h, mip, D3D11_USAGE_DEFAULT, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, data, data_length));
				}

				bool cast_RTV(RTV_ptr& rtv, const texture2D_ptr& tp, size_t miplevel) {
					return SUCCEEDED(Dx11::cast_RTV(res, rtv,tp, miplevel));
				}

				bool cast_RTV(output_merge_d& omd, size_t solt, const texture2D_ptr& tp, size_t miplevel) {
					RTV tem;
					if (SUCCEEDED(Dx11::cast_RTV(res, tem.ptr, tp, miplevel)))
						return omd.set_RTV(solt, tem), true;
					return false;
				}

				bool cast_DSV(output_merge_d& omd, const texture2D_ptr& tp, size_t miplevel) {
					DSV tem;
					if (SUCCEEDED(Dx11::cast_DSV(res, tem.ptr, tp, miplevel)))
						return omd.set_DSV( tem), true;
					return false;
				}

				bool create_state(blend_d& bs, const blend_s& bd);
				bool create_state(output_merge_d& omd, const blend_s& bd) { blend_d tem; if (create_state(tem, bd)) return (omd.set_state(tem), true); return false; }
				bool create_state(depth_stencil_d& dss, const depth_stencil_s& dsd);
				bool create_state(output_merge_d& omd, const depth_stencil_s& dsd) { depth_stencil_d tem; if (create_state(tem, dsd)) return (omd.set_state(tem), true); return false; }
			};

			struct raterizer_resource_t
			{
				device_ptr& res;
				raterizer_resource_t(device_ptr& r) : res(r) {}
				bool create_state(raterizer_d& rs, const raterizer_s& rd);
				bool create_state(raterizer_d& rs, raterizer_s&& rd);
			};
		
			struct compute_shader_resource_t : shader_resource_t
			{
				using shader_resource_t::shader_resource_t;
				bool create_shader(compute_d& cd, const binary& b) 
				{
					cshader_ptr cp;
					if (SUCCEEDED(res->CreateComputeShader(b, static_cast<UINT>(b.size()), nullptr, &cp)))
						return cd.ptr = cp, true;
					return false;
				}
				bool create_UAT(texture2D_ptr& tp, DXGI_FORMAT DF, size_t w, size_t h, size_t miplevel, void* data = nullptr, size_t len = 0) {
					return SUCCEEDED(Dx11::create_tex2(res, tp, DF, w, h, miplevel, D3D11_USAGE_DEFAULT, D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE, data, len));
				}
				bool create_UAT(texture3D_ptr& tp, DXGI_FORMAT DF, size_t w, size_t h, size_t z, size_t miplevel, void* data = nullptr, size_t len = 0, size_t width = 0) {
					return SUCCEEDED(Dx11::create_tex3(res, tp, DF, w, h,z,  miplevel, D3D11_USAGE_DEFAULT, D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE, data, len, width));
				}
				bool cast_UAV(UAV& up, const texture2D_ptr& t, size_t mipslice) { return SUCCEEDED(Dx11::cast_UAV(res, up.ptr, t, mipslice)); }
				bool cast_UAV(UAV& up, const texture3D_ptr& t, size_t mipslice, size_t zstart, size_t zwidth) { return SUCCEEDED(Dx11::cast_UAV(res, up.ptr, t, mipslice, zstart, zwidth)); }
				bool cast_UAV(compute_d& up, size_t solt, const texture2D_ptr& t, size_t mipslice);
				bool cast_UAV(compute_d& up, size_t solt, const texture3D_ptr& t, size_t mipslice, size_t zstart, size_t zwidth);
			};
		};

		struct resource
		{
			device_ptr dp;
			Implement::input_assember_resource_t IA;
			Implement::vertex_shader_resource_t VS;
			Implement::raterizer_resource_t RS;
			Implement::pixel_shader_resource_t PS;
			Implement::compute_shader_resource_t CS;
			Implement::output_merge_resource_t OM;
			resource(const device_ptr& rp) :dp(rp), IA(dp), VS(dp), PS(dp), CS(dp), OM(dp), RS(dp) {}
		};
	}

	
}