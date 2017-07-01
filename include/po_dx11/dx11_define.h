#pragma once
#include <d3d11.h>
#include <Atlbase.h>
#include <Windows.h>
#include <dxgi.h>
#include <DirectXMath.h>
#include <DirectXMathVector.inl>
#include <string>
#include <vector>
#include <memory>
#include <array>
#include <map>
#include "../po/frame/define.h"
#include "../po_dx/dx_math.h"
namespace PO
{
	namespace Dx11
	{
		using namespace Dx;

		using device_ptr = CComPtr<ID3D11Device>;
		using context_ptr = CComPtr<ID3D11DeviceContext>;
		using swapchain_ptr = CComPtr<IDXGISwapChain>;
		using buffer_ptr = CComPtr<ID3D11Buffer>;
		using vshader_ptr = CComPtr<ID3D11VertexShader>;
		using pshader_ptr = CComPtr<ID3D11PixelShader>;
		using layout_ptr = CComPtr<ID3D11InputLayout>;
		using texture2D_ptr = CComPtr<ID3D11Texture2D>;
		using texture1D_ptr = CComPtr<ID3D11Texture1D>;
		using texture3D_ptr = CComPtr<ID3D11Texture3D>;

		using UAV_ptr = CComPtr<ID3D11UnorderedAccessView>;
		using RTV_ptr = CComPtr<ID3D11RenderTargetView>;
		using DSV_ptr = CComPtr<ID3D11DepthStencilView>;
		using SRV_ptr = CComPtr<ID3D11ShaderResourceView>;

		using sample_state_ptr = CComPtr<ID3D11SamplerState>;

		using cshader_ptr = CComPtr<ID3D11ComputeShader>;
		using gshader_ptr = CComPtr<ID3D11GeometryShader>;
		using dshader_ptr = CComPtr<ID3D11DomainShader>;

		using raterizer_state_ptr = CComPtr<ID3D11RasterizerState>;
		using blend_state_ptr = CComPtr<ID3D11BlendState>;
		using depth_stencil_state_ptr = CComPtr<ID3D11DepthStencilState>;

		
		UINT translate_usage_to_cpu_flag(D3D11_USAGE DU);

		HRESULT create_buffer(device_ptr& rp, buffer_ptr& ptr, D3D11_USAGE usage, UINT bind_flag, const void* data, size_t data_size, UINT misc_flag, size_t StructureByteStrides);

		HRESULT create_tex1_implement(device_ptr& cp, texture1D_ptr& t, DXGI_FORMAT format, size_t length, size_t miplevel, size_t count, D3D11_USAGE usage, UINT bind, UINT mis, void** data);
		inline HRESULT create_tex1(device_ptr& cp, texture1D_ptr& t, DXGI_FORMAT format, size_t length, size_t miplevel, D3D11_USAGE usage, UINT bind, void* data = nullptr) { return create_tex1_implement(cp, t, format, length, miplevel, 1, usage, bind, 0, (data == nullptr) ? nullptr : &data); }
		inline HRESULT create_tex1_array(device_ptr& cp, texture1D_ptr& t, DXGI_FORMAT format, size_t length, size_t miplevel, size_t count, D3D11_USAGE usage, UINT bind, void** data = nullptr) { return create_tex1_implement(cp, t, format, length, miplevel, count, usage, bind, 0, data); }

		HRESULT create_tex2_implement(device_ptr& cp, texture2D_ptr& t, DXGI_FORMAT format, size_t width, size_t height, size_t miplevel, size_t count, size_t sample_num, size_t sample_quality, D3D11_USAGE usage, UINT bind, UINT mis, void** data, size_t* line);
		
		inline HRESULT create_tex2(device_ptr& cp, texture2D_ptr& t, DXGI_FORMAT format, size_t width, size_t height, size_t miplevel, D3D11_USAGE usage, UINT bind, void* data = nullptr, size_t line = 0) { return create_tex2_implement(cp, t, format, width, height, miplevel, 1, 1, 0, usage, bind, 0, &data, &line); }
		inline HRESULT create_tex2_array(device_ptr& cp, texture2D_ptr& t, DXGI_FORMAT format, size_t width, size_t height, size_t miplevel, size_t count, D3D11_USAGE usage, UINT bind, bool cube = false, void** data = nullptr, size_t* line = nullptr) { return create_tex2_implement(cp, t, format, width, height, miplevel, count, 1, 0, usage, bind, (cube ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0), data, line); }
		inline HRESULT create_tex2_ms(device_ptr& cp, texture2D_ptr& t, DXGI_FORMAT format, size_t width, size_t height, size_t sample_num, size_t sample_quality, D3D11_USAGE usage, UINT bind, void* data = nullptr, size_t line = 0) { return create_tex2_implement(cp, t, format, width, height, 1, 1, sample_num, sample_quality, usage, bind, 0, &data, &line); }
		inline HRESULT create_tex2_ms_array(device_ptr& cp, texture2D_ptr& t, DXGI_FORMAT format, size_t width, size_t height, size_t count, size_t sample_num, size_t sample_quality, D3D11_USAGE usage, UINT bind, void** data = nullptr, size_t* line = nullptr) { return create_tex2_implement(cp, t, format, width, height, 1, count, sample_num, sample_quality, usage, bind, 0, data, line); }

		HRESULT create_tex3_implement(device_ptr& cp, texture3D_ptr& t, DXGI_FORMAT format, size_t width, size_t height, size_t depth, size_t miplevel, D3D11_USAGE usage, UINT bind, UINT mis, void* data = nullptr, size_t line = 0, size_t slice = 0);
		inline HRESULT create_tex3(device_ptr& cp, texture3D_ptr& t, DXGI_FORMAT format, size_t width, size_t height, size_t depth, size_t miplevel, D3D11_USAGE usage, UINT bind, void* data = nullptr, size_t line = 0, size_t slice = 0) { return create_tex3_implement(cp, t, format, width, height, depth, miplevel, usage, bind, 0, data, line, slice); }

		
		HRESULT cast_SRV(device_ptr& cp, SRV_ptr& rtv, const buffer_ptr& t, DXGI_FORMAT DF, size_t start_start, size_t array_count);
		HRESULT cast_SRV_structured(device_ptr& cp, SRV_ptr& rtv, const buffer_ptr& t, size_t bit_offset, size_t element_count);
		HRESULT cast_SRV_structured(device_ptr& cp, SRV_ptr& rtv, const buffer_ptr& t);

		HRESULT cast_SRV(device_ptr& cp, SRV_ptr& rtv, const texture1D_ptr& t, size_t most_detailed_mip, size_t miplevel);
		HRESULT cast_SRV_array(device_ptr& cp, SRV_ptr& rtv, const texture1D_ptr& t, size_t most_detailed_mip, size_t miplevel, size_t array_start, size_t array_count);

		HRESULT cast_SRV(device_ptr& cp, SRV_ptr& rtv, const texture2D_ptr& t, size_t most_detailed_mip, size_t miplevel);
		HRESULT cast_SRV_array(device_ptr& cp, SRV_ptr& rtv, const texture2D_ptr& t, size_t most_detailed_mip, size_t miplevel, size_t array_start, size_t array_count);

		HRESULT cast_SRV_ms(device_ptr& cp, SRV_ptr& rtv, const texture2D_ptr& t);
		HRESULT cast_SRV_ms_array(device_ptr& cp, SRV_ptr& rtv, const texture2D_ptr& t, size_t array_start, size_t array_count);

		HRESULT cast_SRV_cube(device_ptr& cp, SRV_ptr& rtv, const texture2D_ptr& t, size_t most_detailed_mip, size_t miplevel);
		HRESULT cast_SRV_cube_array(device_ptr& cp, SRV_ptr& rtv, const texture2D_ptr& t, size_t most_detailed_mip, size_t miplevel, size_t array_start, size_t array_count);

		HRESULT cast_SRV(device_ptr& cp, SRV_ptr& rtv, const texture3D_ptr& t, size_t most_detailed_mip, size_t miplevel);

		HRESULT cast_RTV(device_ptr& cp, RTV_ptr& rtv, const texture1D_ptr& t, size_t miplevel);
		HRESULT cast_RTV_array(device_ptr& cp, RTV_ptr& rtv, const texture1D_ptr& t, size_t miplevel, size_t array_start, size_t array_count);

		HRESULT cast_RTV(device_ptr& cp, RTV_ptr& rtv, const texture2D_ptr& t, size_t miplevel);
		HRESULT cast_RTV_array(device_ptr& cp, RTV_ptr& rtv, const texture2D_ptr& t, size_t miplevel, size_t array_start, size_t array_count);

		HRESULT cast_RTV_ms(device_ptr& cp, RTV_ptr& rtv, const texture2D_ptr& t);
		HRESULT cast_RTV_ms_array(device_ptr& cp, RTV_ptr& rtv, const texture2D_ptr& t, size_t array_start, size_t array_count);

		HRESULT cast_RTV(device_ptr& cp, RTV_ptr& rtv, const texture3D_ptr& t, size_t miplevel, size_t start_z, size_t z_count);

		HRESULT cast_DSV(device_ptr& cp, DSV_ptr& dsv, const texture1D_ptr& t, size_t miplevel);
		HRESULT cast_DSV_array(device_ptr& cp, DSV_ptr& dsv, const texture1D_ptr& t, size_t miplevel, size_t array_start, size_t array_count);

		HRESULT cast_DSV(device_ptr& cp, DSV_ptr& dsv, const texture2D_ptr& t, size_t miplevel);
		HRESULT cast_DSV_array(device_ptr& cp, DSV_ptr& dsv, const texture2D_ptr& t, size_t miplevel, size_t array_start, size_t array_count);

		HRESULT cast_DSV_ms(device_ptr& cp, DSV_ptr& dsv, const texture2D_ptr& t);
		HRESULT cast_DSV_ms_array(device_ptr& cp, DSV_ptr& dsv, const texture2D_ptr& t, size_t array_start, size_t array_count);


		HRESULT cast_DSV_readonly(device_ptr&, DSV_ptr& out_rtv, const texture1D_ptr& t, size_t miplevel, bool depth_f_stencil_t);
		HRESULT cast_DSV_array_readonly(device_ptr& cp, DSV_ptr& rtv, const texture1D_ptr& t, size_t miplevel, size_t array_start, size_t array_count, bool depth_f_stencil_t);

		HRESULT cast_DSV_readonly(device_ptr& cp, DSV_ptr& rtv, const texture2D_ptr& t, size_t miplevel, bool depth_false_stencil_true);
		HRESULT cast_DSV_array_readonly(device_ptr& cp, DSV_ptr& rtv,  const texture2D_ptr& t, size_t miplevel, size_t array_start, size_t array_count, bool depth_f_stencil_t);

		HRESULT cast_DSV_ms_readonly(device_ptr& cp, DSV_ptr& rtv, const texture2D_ptr& t, bool depth_f_stencil_t);
		HRESULT cast_DSV_ms_array_readonly(device_ptr& cp, DSV_ptr& rtv, const texture2D_ptr& t, size_t array_start, size_t array_count, bool depth_f_stencil_t);

		HRESULT cast_UAV(device_ptr& dp, UAV_ptr& up, const texture2D_ptr& tp, size_t mipslice);
		HRESULT cast_UAV(device_ptr& dp, UAV_ptr& up, const texture3D_ptr& tp, size_t mipslice, size_t zslicelocation, size_t zsize);

		void viewport_fill_texture(D3D11_VIEWPORT& DV, ID3D11Texture2D* t, float min_depth, float max_depth);
		void viewport_capture_texture(D3D11_VIEWPORT& DV, ID3D11Texture2D* t, float top_left_x_rate, float top_left_y_rate, float button_right_x_rate, float button_right_y_rate, float min_dapth, float max_depth);

		template<typename T, typename K = std::allocator<T>> struct dx11_res_array
		{
			std::vector<T*, K> ptr;
			UINT size() const { return static_cast<UINT>(ptr.size()); }
			T*const* data() const { return ptr.data(); }
			T** data() { return ptr.data(); }
			bool empty() const { return ptr.empty(); }
			void set(size_t solt, T* da)
			{
				if (ptr.size() <= solt)
					ptr.insert(ptr.end(), solt + 1 - ptr.size(), nullptr);
				auto& p = ptr[solt];
				if (p != nullptr) p->Release();
				p = da;
				if (p != nullptr) p->AddRef();
			}
			void clear() { for (auto ui : ptr) if (ui != nullptr) ui->Release(); ptr.clear(); }
			~dx11_res_array() { for (auto ui : ptr) if (ui != nullptr) ui->Release(); }
			dx11_res_array() {}
			dx11_res_array(const dx11_res_array& dre)
			{
				ptr = dre.ptr;
				for (auto ui : ptr)
					if (ui != nullptr) ui->AddRef();
			}
			dx11_res_array& operator=(const dx11_res_array& dra) {
				clear(); ptr = dre.ptr;
				for (auto ui : ptr)
					if (ui != nullptr) ui->AddRef();
				return *this;
			}
			dx11_res_array& operator=(const dx11_res_array&& dra) {
				clear(); ptr = std::move(dre.ptr);
				return *this;
			}
			auto begin() { return ptr.begin(); }
			auto end() { return ptr.end(); }
			decltype(auto) operator[](size_t s) { return ptr[s]; }
		};
		

		struct vertex
		{
			CComPtr<ID3D11Buffer> ptr;
			UINT offset;
			UINT element_size;
			size_t num;
			std::vector<D3D11_INPUT_ELEMENT_DESC> layout;
		};

		struct index
		{
			CComPtr<ID3D11Buffer> ptr;
			UINT offset;
			DXGI_FORMAT format;
			size_t num;
		};

		struct index_vertex
		{
			buffer_ptr ptr;
			UINT v_offset;
			UINT v_element_size;
			size_t v_num;
			std::vector<D3D11_INPUT_ELEMENT_DESC> v_layout;
			UINT i_offset;
			DXGI_FORMAT i_format;
			size_t i_num;
		};

		struct cbuffer
		{
			buffer_ptr ptr;
		};

		struct sbuffer
		{
			buffer_ptr ptr;
		};

		struct tbuffer
		{
			buffer_ptr ptr;
		};

		struct input_assember_d
		{
			dx11_res_array<ID3D11Buffer> vertex_array;
			std::vector<UINT> offset_array;
			std::vector<UINT> element_array;
			std::vector<D3D11_INPUT_ELEMENT_DESC> input_element;
			layout_ptr layout;
			D3D11_PRIMITIVE_TOPOLOGY primitive = D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

			buffer_ptr index_ptr;
			UINT offset;
			DXGI_FORMAT format;

			void set_vertex(size_t solt, const vertex& v);
			void set_index(const index& bp);
			void set_index_vertex(size_t solt, const index_vertex& iv);

			input_assember_d() = default;
			input_assember_d(const input_assember_d&) = default;
			input_assember_d(input_assember_d&& i);
			input_assember_d& operator= (input_assember_d&& i);
			input_assember_d& operator= (const input_assember_d&) = default;

			bool check() const { return !vertex_array.empty() && index_ptr!=nullptr; }
		};

		struct SRV
		{
			SRV_ptr ptr;
		};

		struct UAV
		{
			UAV_ptr ptr;
			UINT offset;
		};

		struct RTV
		{
			RTV_ptr ptr;
		};

		struct DSV
		{
			DSV_ptr ptr;
		};

		struct sample_s
		{
			D3D11_SAMPLER_DESC desc;
			sample_s();
		};

		struct sample_d
		{
			sample_state_ptr ptr;
		};

		struct shader_d
		{
			dx11_res_array<ID3D11Buffer> cbuffer_array;
			dx11_res_array<ID3D11ShaderResourceView> SRV_array;
			dx11_res_array<ID3D11SamplerState> sample_array;
			void set_cbuffer(size_t solt, const cbuffer&);
			void set_SRV(size_t solt, const SRV& ptr);
			void set_sample(size_t solt, const sample_d& sd);
		};

		struct vertex_shader_d : shader_d
		{
			binary code;
			vshader_ptr ptr;
		};

		struct pixel_shader_d : shader_d
		{
			pshader_ptr ptr;
		};

		struct raterizer_d
		{
			raterizer_state_ptr ptr;
			std::vector<D3D11_VIEWPORT> viewports;
			std::vector<D3D11_RECT> scissor;
			bool check() const { return !viewports.empty(); }
		};

		struct blend_d
		{
			blend_state_ptr ptr;
			std::array<float, 4> bind_factor;
			UINT sample_mask;
		};

		struct depth_stencil_d
		{
			depth_stencil_state_ptr ptr;
			UINT stencil_ref;
		};

		struct raterizer_s
		{
			D3D11_RASTERIZER_DESC desc;
			std::vector<D3D11_VIEWPORT> viewports;
			std::vector<D3D11_RECT> scissor;
			void view_fill_texture(size_t solt, const texture2D_ptr& t, float min_depth, float max_depth);
			raterizer_s();
			bool check() const { return !viewports.empty(); }
			raterizer_s& stop_cull() { desc.CullMode = D3D11_CULL_NONE; return *this; }
		};


		struct blend_s
		{
			D3D11_BLEND_DESC desc;
			std::array<float, 4> factor;
			UINT sample_mask;
			blend_s();
			template<typename T> blend_s& set_RT_blend_state(T&& t) { desc.IndependentBlendEnable = FALSE; t(desc.RenderTarget[0]); return *this; }
			template<typename T> blend_s& set_RT_blend_state(size_t RT_solt, T&& t) { desc.IndependentBlendEnable = TRUE; t(desc.RenderTarget[RT_solt]); return *this; }
			blend_s& unactive_blend() { desc.IndependentBlendEnable = FALSE; desc.RenderTarget[0].BlendEnable = FALSE; return *this; }
		};

		struct depth_stencil_s
		{
			D3D11_DEPTH_STENCIL_DESC desc;
			UINT stencil_ref;
			depth_stencil_s();
		};

		struct output_merge_d : shader_d
		{
			template<typename T, typename ...AT> using rt = std::enable_if_t<Tmp::is_one_of<T, AT...>::value>;

			dx11_res_array<ID3D11RenderTargetView> render_array;
			DSV_ptr depth;
			blend_d blend_state;
			depth_stencil_d depth_stencil_state;
			void set_RTV(size_t o, const RTV& rtv);
			void set_DSV(const DSV& dsv) { depth = dsv.ptr; }
			void set_state(blend_d bs) { blend_state = bs; }
			void set_state(depth_stencil_d dss) { depth_stencil_state = dss; }
			bool check() const { return !render_array.empty(); }
		};

		struct draw_range_d
		{
			struct vertex_d
			{
				UINT vertex_count;
				UINT start_vertex_location;
			};

			struct index_d
			{
				UINT index_count;
				UINT start_index_location;
				UINT base_vertex_location;
			};

			struct instance_d
			{
				UINT vertex_pre_instance;
				UINT instance_count;
				UINT start_vertex_location;
				UINT start_instance_location;
			};

			struct instance_index_d
			{
				UINT index_pre_instance;
				UINT instance_count;
				UINT start_index_location;
				UINT base_vertex_location;
				UINT start_instance_location;
			};

			struct dispatch_d
			{
				UINT X;
				UINT Y;
				UINT Z;
			};

			Tool::variant<vertex_d, index_d, instance_d, instance_index_d, dispatch_d> data;

			void set_vertex(UINT vertex_count, UINT start_vertex_location) { data = vertex_d{ vertex_count , start_vertex_location }; }
			void set_index(UINT index_count, UINT start_index_location, UINT base_vertex_location) { data = index_d{ index_count , start_index_location , base_vertex_location }; }
			void set_instance(UINT vertex_pre_instance, UINT instance_count, UINT start_vertex_location, UINT start_instance_location) {
				data = instance_d{ vertex_pre_instance, instance_count , start_vertex_location , start_instance_location };
			}
			void set_index_instance(UINT index_pre_instance, UINT instance_count,
				UINT start_index_location,UINT base_vertex_location,UINT start_instance_location) {
				data = instance_index_d{ index_pre_instance, instance_count , start_index_location , base_vertex_location,  start_instance_location };
			}
			void set_dispatch_d(UINT X, UINT Y, UINT Z) { data = dispatch_d{X, Y ,Z}; }
		};

		struct compute_d : shader_d
		{
			dx11_res_array<ID3D11UnorderedAccessView> UAV_array;
			std::vector<UINT> offset;
			cshader_ptr ptr;
			void set_UAV(size_t solt, const UAV& up);
		};
	}
}
