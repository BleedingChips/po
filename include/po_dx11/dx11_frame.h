#pragma once
#include <d3d11.h>
#include "../po_win32/win32_define.h"
#include <tuple>
#include "../po/tool/auto_adapter.h"
#include "../po_dx/dx_type.h"
namespace PO
{
	namespace Dx11
	{

		struct UINT2 { UINT x, y; };

		struct vertex
		{
			Win32::com_ptr<ID3D11Buffer> ptr;
			UINT offset;
			UINT element_size;
			UINT num;
			std::vector<D3D11_INPUT_ELEMENT_DESC> layout;

			std::tuple<vertex, size_t> operator[](size_t i) && {return std::tuple<vertex, size_t>{std::move(*this), i}; }
			std::tuple<const vertex&, size_t> operator[](size_t i) const& { return std::tuple<const vertex&, size_t>{*this, i}; }

		};

		struct index
		{
			Win32::com_ptr<ID3D11Buffer> ptr;
			UINT offset;
			DXGI_FORMAT format;
			UINT num;
		};

		struct index_vertex
		{
			Win32::com_ptr<ID3D11Buffer> ptr;
			UINT v_offset;
			UINT v_element_size;
			UINT v_num;
			std::vector<D3D11_INPUT_ELEMENT_DESC> v_layout;
			UINT i_offset;
			DXGI_FORMAT i_format;
			UINT i_num;
			std::tuple<index_vertex, size_t> operator[](size_t i) && {return std::tuple<index_vertex, size_t>{std::move(*this), i}; }
			std::tuple<const index_vertex&, size_t> operator[](size_t i) const& { return std::tuple<const index_vertex&, size_t>{*this, i}; }
		};

		struct tex1 { 
			Win32::com_ptr<ID3D11Texture1D> ptr; 
			operator bool() const { return ptr; } 
			uint32_t size() const;
		};
		struct tex2 { 
			Win32::com_ptr<ID3D11Texture2D> ptr; 
			operator bool() const { return ptr; }
			PO::Dx::uint32_t2 size() const;
		};
		struct tex3 { 
			Win32::com_ptr<ID3D11Texture3D> ptr; 
			operator bool() const { return ptr; } 
			PO::Dx::uint32_t3 size() const;
		};

		struct constant_buffer { 
			Win32::com_ptr<ID3D11Buffer> ptr; 
			std::tuple<constant_buffer, size_t> operator[](size_t i) && {return std::tuple<constant_buffer, size_t>{std::move(*this), i}; }
			std::tuple<const constant_buffer&, size_t> operator[](size_t i) const& {return std::tuple<const constant_buffer&, size_t>{std::move(*this), i}; }
			operator bool() const { return ptr; }
		};
		
		struct structured_buffer { 
			Win32::com_ptr<ID3D11Buffer> ptr;
			std::tuple<structured_buffer, size_t> operator[](size_t i) && {return std::tuple<structured_buffer, size_t>{std::move(*this), i}; }
			std::tuple<const structured_buffer&, size_t> operator[](size_t i) const& { return std::tuple<const structured_buffer&, size_t>{std::move(*this), i}; }
			operator bool() const { return ptr; }
		};
		
		struct readable_buffer { Win32::com_ptr<ID3D11Buffer> ptr; };

		struct shader_resource_view { 
			Win32::com_ptr<ID3D11ShaderResourceView> ptr;
			std::tuple<shader_resource_view, size_t> operator[](size_t i) && {return std::tuple<shader_resource_view, size_t>{std::move(*this), i}; }
			std::tuple<const shader_resource_view&, size_t> operator[](size_t i) const& { return std::tuple<const shader_resource_view&, size_t>{std::move(*this), i}; }
			operator bool() const { return ptr; }
		};

		struct unordered_access_view { 
			Win32::com_ptr<ID3D11UnorderedAccessView> ptr; UINT offset; 
			std::tuple<unordered_access_view, size_t> operator[](size_t i) && {return std::tuple<unordered_access_view, size_t>{std::move(*this), i}; }
			std::tuple<const unordered_access_view&, size_t> operator[](size_t i) const& { return std::tuple<const unordered_access_view&, size_t>{std::move(*this), i}; }
			operator bool() const { return ptr; }
		};

		struct render_target_view { 
			Win32::com_ptr<ID3D11RenderTargetView> ptr;
			std::tuple<render_target_view, size_t> operator[](size_t i) && {return std::tuple<render_target_view, size_t>{std::move(*this), i}; }
			std::tuple<const render_target_view&, size_t> operator[](size_t i) const& { return std::tuple<const render_target_view&, size_t>{std::move(*this), i}; }
			operator bool() const { return ptr; }
		};

		struct depth_stencil_view { 
			Win32::com_ptr<ID3D11DepthStencilView> ptr; 
			operator bool() const { return ptr; }
		};

		struct sample_state { 
			using description = D3D11_SAMPLER_DESC;
			static description default_description;
			operator bool() const { return ptr; }
			Win32::com_ptr<ID3D11SamplerState> ptr;
			std::tuple<sample_state, size_t> operator[](size_t i) && {return std::tuple<sample_state, size_t>{std::move(*this), i}; }
			std::tuple<const sample_state&, size_t> operator[](size_t i) const& { return std::tuple<const sample_state&, size_t>{std::move(*this), i}; }
		};

		struct viewports {
			std::vector<D3D11_VIEWPORT> views;
			std::vector<D3D11_RECT> scissor;
			viewports& set(const D3D11_VIEWPORT& port, size_t solt);
			viewports& fill_texture(size_t solt, const tex2& t, float min_depth = 0.0, float max_depth = 1.0);
			viewports& capture_texture(size_t solt, const tex2& t, float top_left_x_rate, float top_left_y_rate, float button_right_x_rate, float button_right_y_rate, float min_depth = 0.0, float max_depth = 1.0);
			std::tuple<viewports, size_t> operator[](size_t i) && {return std::tuple<viewports, size_t>{std::move(*this), i}; }
			std::tuple<const viewports&, size_t> operator[](size_t i) const& { return std::tuple<const viewports&, size_t>{std::move(*this), i}; }
		};

		struct raterizer_state {
			using description = D3D11_RASTERIZER_DESC;
			static description default_description;
			Win32::com_ptr<ID3D11RasterizerState> ptr;
			operator bool() const { return ptr; }
		};

		struct blend_state {
			using description = D3D11_BLEND_DESC;
			static description default_description;

			Win32::com_ptr<ID3D11BlendState> ptr;
			operator bool() const { return ptr; }
			std::array<float, 4> bind_factor = {1.0f, 1.0f, 1.0f, 1.0f};
			UINT sample_mask = 0xffffffff;
		};

		struct depth_stencil_state {
			using description = D3D11_DEPTH_STENCIL_DESC;
			static description default_description;

			Win32::com_ptr<ID3D11DepthStencilState> ptr;
			operator bool() const { return ptr; }
			UINT stencil_ref = 0;
		};

		struct shader_resource
		{
			Win32::com_vector<ID3D11Buffer> cbuffer_array;
			Win32::com_vector<ID3D11ShaderResourceView> shader_resource_view_array;
			Win32::com_vector<ID3D11SamplerState> sample_array;

			shader_resource& set(const constant_buffer& cb, size_t solt);
			shader_resource& set(const shader_resource_view& ptr, size_t solt);
			shader_resource& set(const sample_state& sd, size_t solt);

			shader_resource& operator<<(const std::tuple<constant_buffer, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }
			shader_resource& operator<<(const std::tuple<const constant_buffer&, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }
			shader_resource& operator<<(const std::tuple<shader_resource_view, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }
			shader_resource& operator<<(const std::tuple<const shader_resource_view&, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }
			shader_resource& operator<<(const std::tuple<sample_state, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }
			shader_resource& operator<<(const std::tuple<const sample_state&, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }

		};

		struct input_assember_stage
		{
			Win32::com_vector<ID3D11Buffer> vertex_array;
			std::vector<UINT> offset_array;
			std::vector<UINT> element_array;
			std::vector<D3D11_INPUT_ELEMENT_DESC> input_element;

			Win32::com_ptr<ID3D11InputLayout> layout;

			D3D11_PRIMITIVE_TOPOLOGY primitive = D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

			Win32::com_ptr<ID3D11Buffer> index_ptr;
			UINT offset;
			DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;

			//input_assember_stage& operator=(const input_assember_stage&) = default;
			//input_assember_stage& operator=(input_assember_stage&&) = default;

			input_assember_stage& operator<<(const std::tuple<vertex, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }
			input_assember_stage& operator<<(const std::tuple<const vertex&, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }
			input_assember_stage& operator<<(index bp) { return set(std::move(bp)); }

			input_assember_stage& operator<<(const std::tuple<index_vertex, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }
			input_assember_stage& operator<<(const std::tuple<const index_vertex&, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }

			input_assember_stage& operator<<(D3D11_PRIMITIVE_TOPOLOGY DPT) { return set(DPT); }

			input_assember_stage& set(const vertex& v, size_t solt = 0);
			input_assember_stage& set(index bp);
			input_assember_stage& set(const index_vertex& iv, size_t solt = 0);

			input_assember_stage& set(D3D11_PRIMITIVE_TOPOLOGY DPT) { primitive = DPT; return *this; }

			bool check() const;
		};

		struct vertex_shader
		{
			std::shared_ptr<PO::Dx::shader_binary> code;
			Win32::com_ptr<ID3D11VertexShader> ptr;
			operator bool() const { return ptr; }
		};

		struct vertex_resource : shader_resource
		{
			vertex_resource& set(const constant_buffer& cb, size_t solt) { shader_resource::set(cb, solt); return *this; }
			vertex_resource& set(const shader_resource_view& ptr, size_t solt) { shader_resource::set(ptr, solt); return *this; }
			vertex_resource& set(const sample_state& sd, size_t solt) { shader_resource::set(sd, solt); return *this; }

			shader_resource& operator<<(const std::tuple<constant_buffer, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }
			shader_resource& operator<<(const std::tuple<const constant_buffer&, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }
			shader_resource& operator<<(const std::tuple<shader_resource_view, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }
			shader_resource& operator<<(const std::tuple<const shader_resource_view&, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }
			shader_resource& operator<<(const std::tuple<sample_state, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }
			shader_resource& operator<<(const std::tuple<const sample_state&, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }
		};

		struct vertex_stage : vertex_resource
		{
			vertex_shader code;
			using vertex_resource::operator=;
			operator const vertex_shader&() const { return code; }

			vertex_stage& operator<<(vertex_shader vs) { return this->set(std::move(vs)); }
			vertex_stage& operator<<(const std::tuple<constant_buffer, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }
			vertex_stage& operator<<(const std::tuple<const constant_buffer&, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }
			vertex_stage& operator<<(const std::tuple<shader_resource_view, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }
			vertex_stage& operator<<(const std::tuple<const shader_resource_view&, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }
			vertex_stage& operator<<(const std::tuple<sample_state, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }
			vertex_stage& operator<<(const std::tuple<const sample_state&, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }
			//vertex_stage& operator<< (const constant_buffer& vs) { return this->set(std::move(vs)); }

			vertex_stage& set(vertex_shader vs) { code = std::move(vs); return *this; }
			vertex_stage& set(const constant_buffer& cb, size_t solt = 0) { vertex_resource::set(cb, solt); return *this; }
			vertex_stage& set(const shader_resource_view& ptr, size_t solt = 0) { vertex_resource::set(ptr, solt); return *this; }
			vertex_stage& set(const sample_state& sd, size_t solt = 0) { vertex_resource::set(sd, solt); return *this; }
			bool check() const;
		};

		struct pixel_shader
		{
			Win32::com_ptr<ID3D11PixelShader> ptr;
		};

		struct pixel_resource : shader_resource
		{
			pixel_resource& set(const constant_buffer& cb, size_t solt) { shader_resource::set(cb, solt); return *this; }
			pixel_resource& set(const shader_resource_view& ptr, size_t solt) { shader_resource::set(ptr, solt); return *this; }
			pixel_resource& set(const sample_state& sd, size_t solt) { shader_resource::set(sd, solt); return *this; }

			pixel_resource& operator<<(const std::tuple<constant_buffer, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }
			pixel_resource& operator<<(const std::tuple<const constant_buffer&, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }
			pixel_resource& operator<<(const std::tuple<shader_resource_view, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }
			pixel_resource& operator<<(const std::tuple<const shader_resource_view&, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }
			pixel_resource& operator<<(const std::tuple<sample_state, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }
			pixel_resource& operator<<(const std::tuple<const sample_state&, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }
		};

		struct pixel_stage : pixel_resource
		{
			pixel_shader code;

			pixel_stage& operator<<(pixel_shader ps) { return this->set(std::move(ps)); }
			pixel_stage& operator<<(const std::tuple<constant_buffer, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }
			pixel_stage& operator<<(const std::tuple<const constant_buffer&, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }
			pixel_stage& operator<<(const std::tuple<shader_resource_view, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }
			pixel_stage& operator<<(const std::tuple<const shader_resource_view&, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }
			pixel_stage& operator<<(const std::tuple<sample_state, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }
			pixel_stage& operator<<(const std::tuple<const sample_state&, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }

			pixel_stage& set(pixel_shader ps) { code = std::move(ps); return *this; }
			pixel_stage& set(const constant_buffer& cb, size_t solt = 0) { pixel_resource::set(cb, solt); return *this; }
			pixel_stage& set(const shader_resource_view& ptr, size_t solt = 0) { pixel_resource::set(ptr, solt); return *this; }
			pixel_stage& set(const sample_state& sd, size_t solt = 0) { pixel_resource::set(sd, solt); return *this; }
			bool check() const;
		};

		struct output_merge_stage
		{
			Win32::com_vector<ID3D11RenderTargetView> render_array;
			depth_stencil_view depth;
			output_merge_stage& set(const render_target_view& rtv, size_t o);
			output_merge_stage& set(depth_stencil_view dsv) { depth = std::move(dsv); return *this; }

			output_merge_stage& operator<< (const std::tuple<render_target_view, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }
			output_merge_stage& operator<< (const std::tuple<const render_target_view&, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }
			output_merge_stage& operator<< (depth_stencil_view dsv) { return set(std::move(dsv)); }

			bool check() const;
		};

		struct compute_shader
		{
			Win32::com_ptr<ID3D11ComputeShader> ptr;
		};

		struct compute_resource : shader_resource
		{

			Win32::com_vector<ID3D11UnorderedAccessView> UAV_array;
			std::vector<UINT> offset;

			compute_resource& set(const constant_buffer& cb, size_t solt) { shader_resource::set(cb, solt); return *this; }
			compute_resource& set(const shader_resource_view& ptr, size_t solt) { shader_resource::set(ptr, solt); return *this; }
			compute_resource& set(const sample_state& sd, size_t solt) { shader_resource::set(sd, solt); return *this; }

			compute_resource& operator<<(const std::tuple<constant_buffer, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }
			compute_resource& operator<<(const std::tuple<const constant_buffer&, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }
			compute_resource& operator<<(const std::tuple<shader_resource_view, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }
			compute_resource& operator<<(const std::tuple<const shader_resource_view&, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }
			compute_resource& operator<<(const std::tuple<sample_state, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }
			compute_resource& operator<<(const std::tuple<const sample_state&, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }

			compute_resource& set(const unordered_access_view& uav, size_t solt);
			compute_resource& operator<<(const std::tuple<unordered_access_view, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }
			compute_resource& operator<<(const std::tuple<const unordered_access_view&, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }
		};

		struct compute_stage : compute_resource
		{
			
			compute_shader code;
			compute_stage& operator<<(compute_shader cs) { return set(std::move(cs)); }

			compute_stage& operator<<(const std::tuple<constant_buffer, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }
			compute_stage& operator<<(const std::tuple<const constant_buffer&, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }
			compute_stage& operator<<(const std::tuple<shader_resource_view, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }
			compute_stage& operator<<(const std::tuple<const shader_resource_view&, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }
			compute_stage& operator<<(const std::tuple<sample_state, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }
			compute_stage& operator<<(const std::tuple<const sample_state&, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }
			compute_stage& operator<<(const std::tuple<unordered_access_view, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }
			compute_stage& operator<<(const std::tuple<const unordered_access_view&, size_t>& t) { return set(std::get<0>(t), std::get<1>(t)); }

			compute_stage& set(const unordered_access_view& uav, size_t solt) { compute_resource::set(uav, solt); return *this; }
			compute_stage& set(compute_shader cs) { code = std::move(cs); return *this; }
			compute_stage& set(const constant_buffer& cb, size_t solt = 0) { compute_resource::set(cb, solt); return *this; }
			compute_stage& set(const shader_resource_view& ptr, size_t solt = 0) { compute_resource::set(ptr, solt); return *this; }
			compute_stage& set(const sample_state& sd, size_t solt = 0) { compute_resource::set(sd, solt); return *this; }
			bool check() const;
		};

		enum class DST_format
		{
			D16,
			D24_UI8,
			F32,
			F32_UI8,
			UNKNOW
		};

		struct creator
		{
			Win32::com_ptr<ID3D11Device> dev;
			creator(Win32::com_ptr<ID3D11Device> d) : dev(std::move(d)) {}
			creator() {}
			creator(const creator&) = default;
			creator(creator&&) = default;
			void init(Win32::com_ptr<ID3D11Device> d) { dev = std::move(d); }
			operator bool() const { return dev; }

			static DXGI_FORMAT translate_depth_stencil_format_to_dxgi_format(DST_format dsf);
			static UINT creator::translate_usage_to_cpu_flag(D3D11_USAGE DU);

			sample_state create_sample_state(const sample_state::description& scri = sample_state::default_description);
			raterizer_state create_raterizer_state(const raterizer_state::description& scri = raterizer_state::default_description);
			blend_state create_blend_state(const blend_state::description& scri = blend_state::default_description, std::array<float, 4> bind_factor = { 1.0f, 1.0f, 1.0f, 1.0f }, UINT sample_mask = 0xffffffff);
			depth_stencil_state create_depth_stencil_state(const depth_stencil_state::description& scri = depth_stencil_state::default_description, UINT stencil_ref = 0);

			void update_layout(input_assember_stage& ia, const vertex_shader& vd);

			Win32::com_ptr<ID3D11Buffer> create_buffer_implement(UINT width, D3D11_USAGE DU, UINT BIND, UINT misc_flag, UINT struct_byte,  const void* data);

			vertex create_vertex(const void* data, UINT ele, UINT num, std::vector<D3D11_INPUT_ELEMENT_DESC> layout, bool write_able = false);
			template<typename T, typename K> vertex create_vertex(const std::vector<T, K>& v, std::vector<D3D11_INPUT_ELEMENT_DESC> layout, bool write_able = false) {
				return create_vertex(v.data(), static_cast<UINT>(sizeof(T)), static_cast<UINT>(v.size()), std::move(layout), write_able);
			}

			template<typename T, size_t i> vertex create_vertex(const std::array<T, i>& v, std::vector<D3D11_INPUT_ELEMENT_DESC> layout, bool write_able = false) {
				return create_vertex(v.data(), static_cast<UINT>(sizeof(T)), static_cast<UINT>(i), std::move(layout), write_able);
			}

			index create_index(const void* data, UINT size, UINT num, DXGI_FORMAT DF, bool write_able = false);
			template<typename T, typename K> index create_index(const std::vector<T, K>& v, bool write_able = false) {
				return create_index(v.data(), static_cast<UINT>(sizeof(T)), static_cast<UINT>(v.size()), DXGI::data_format<T>::format, write_able);
			}
			template<typename T, size_t i> index create_index(const std::array<T, i>& v, bool write_able = false) {
				return create_index(v.data(), static_cast<UINT>(sizeof(T)), static_cast<UINT>(i), DXGI::data_format<T>::format, write_able);
			}

			constant_buffer create_constant_buffer(UINT width, const void* data = nullptr, bool write_enable = true);
			template<typename T> constant_buffer create_constant_buffer(const T* data, bool write_enable = true) { return create_constant_buffer(sizeof(T), data, write_enable); }
			inline constant_buffer create_constant_buffer_with_size(size_t size, bool write_enable = true) { return create_constant_buffer(static_cast<UINT>(size), nullptr, write_enable); }

			structured_buffer create_structured_buffer(UINT element_size, UINT element_num, const void* data = nullptr, bool write_enable = true) {
				structured_buffer sb;
				sb.ptr = create_buffer_implement(element_size * element_num, (write_enable ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE), D3D11_BIND_SHADER_RESOURCE, D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, element_size, data);
				return sb;
			}
			template<typename T, typename K> structured_buffer create_structured_buffer(const std::vector<T, K>& v, bool write_enable = true) {
				return create_structured_buffer(static_cast<UINT>(sizeof(T)), static_cast<UINT>(v.size()), v.data(), write_enable);
			}
			template<typename T, size_t i> structured_buffer create_structured_buffer(const std::array<T, i>& v, bool write_enable = true) {
				return create_structured_buffer(static_cast<UINT>(sizeof(T)), static_cast<UINT>(i), v.data(), write_enable);
			}
			structured_buffer create_structured_buffer_unorder_access(UINT element_size, UINT element_num, const void* data = nullptr) {
				structured_buffer sb;
				sb.ptr = create_buffer_implement(element_size * element_num, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS, D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, element_size, data);
				return sb;
			}

			vertex_shader create_vertex_shader(std::shared_ptr<PO::Dx::shader_binary> b);
			pixel_shader create_pixel_shader(const PO::Dx::shader_binary&);
			compute_shader create_compute_shader(const PO::Dx::shader_binary&);

			tex1 create_tex1_implement(DXGI_FORMAT DF, UINT length, UINT miplevel, UINT count, D3D11_USAGE DU, UINT BIND, UINT misc, void** data);
			tex2 create_tex2_implement(DXGI_FORMAT DF, UINT width, UINT height, UINT miplevel, UINT count, UINT sample_num, UINT sample_quality, D3D11_USAGE usage, UINT bind, UINT mis, void** data, UINT* line);
			tex3 create_tex3_implement(DXGI_FORMAT DF, UINT width, UINT height, UINT depth, UINT miplevel, D3D11_USAGE usage, UINT bind, UINT mis, void* data, UINT line, UINT slice);

			tex1 create_tex1(DXGI_FORMAT DF, UINT length, Tool::optional<UINT> miplevel = {}, Tool::optional<UINT> count = {}, D3D11_USAGE DU = D3D11_USAGE_DYNAMIC, void** data = nullptr) {
				return create_tex1_implement(DF, length, (miplevel ? *miplevel : 1), (count ? *count : 1), DU, D3D11_BIND_SHADER_RESOURCE, 0, data);
			}
			tex1 create_tex1_unordered_access(DXGI_FORMAT DF, UINT length, Tool::optional<UINT> miplevel = {}, Tool::optional<UINT> count = {}, void** data = nullptr) {
				return create_tex1_implement(DF, length, (miplevel ? *miplevel : 1), (count ? *count : 1), D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS, 0, data);
			}
			tex1 create_tex1_render_target(DXGI_FORMAT DF, UINT length, Tool::optional<UINT> miplevel = {}, Tool::optional<UINT> count = {}, void** data = nullptr) {
				return create_tex1_implement(DF, length, (miplevel ? *miplevel : 1), (count ? *count : 1), D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET, 0, data);
			}
			tex1 create_tex1_depth_stencil(DST_format DF, UINT length, Tool::optional<UINT> miplevel = {}, Tool::optional<UINT> count = {}, void** data = nullptr) {
				return create_tex1_implement(translate_depth_stencil_format_to_dxgi_format(DF), length, (miplevel ? *miplevel : 1), (count ? *count : 1), D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET, 0, data);
			}

			tex2 create_tex2(DXGI_FORMAT DF, UINT width, UINT height, Tool::variant<UINT, UINT2> miplevel = {}, Tool::optional<UINT> count = {}, D3D11_USAGE usage = D3D11_USAGE_DYNAMIC, void** data = nullptr, UINT* line = nullptr) {
				UINT2 sample = miplevel.able_cast<UINT2>() ? miplevel.cast<UINT2>() : UINT2{ 1, 0 };
				return create_tex2_implement(DF, width, height, (miplevel.able_cast<UINT>() ? miplevel.cast<UINT>() : 1), (count ? *count : 1), sample.x, sample.y, usage, D3D11_BIND_SHADER_RESOURCE, 0, data, line);
			}

			tex2 create_tex2(DXGI_FORMAT DF, const tex2& t, D3D11_USAGE usage = D3D11_USAGE_DYNAMIC, void** data = nullptr, UINT* line = nullptr);

			tex2 create_tex2_unordered_access(DXGI_FORMAT DF, UINT width, UINT height, Tool::variant<UINT, UINT2> miplevel = {}, Tool::optional<UINT> count = {}, void** data = nullptr, UINT* line = nullptr) {
				UINT2 sample = miplevel.able_cast<UINT2>() ? miplevel.cast<UINT2>() : UINT2{ 1, 0 };
				return create_tex2_implement(DF, width, height, (miplevel.able_cast<UINT>() ? miplevel.cast<UINT>() : 1), (count ? *count : 1), sample.x, sample.y, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS, 0, data, line);
			}
			tex2 create_tex2_render_target(DXGI_FORMAT DF, UINT width, UINT height, Tool::variant<UINT, UINT2> miplevel = {}, Tool::optional<UINT> count = {}, void** data = nullptr, UINT* line = nullptr) {
				UINT2 sample = miplevel.able_cast<UINT2>() ? miplevel.cast<UINT2>() : UINT2{ 1, 0 };
				return create_tex2_implement(DF, width, height, (miplevel.able_cast<UINT>() ? miplevel.cast<UINT>() : 1), (count ? *count : 1), sample.x, sample.y, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET, 0, data, line);
			}
			tex2 create_tex2_render_target(DXGI_FORMAT DF, const tex2& t);
			tex2 create_tex2_depth_stencil(DST_format DF, UINT width, UINT height, Tool::variant<UINT, UINT2> miplevel = {}, Tool::optional<UINT> count = {}, void** data = nullptr, UINT* line = nullptr) {
				UINT2 sample = miplevel.able_cast<UINT2>() ? miplevel.cast<UINT2>() : UINT2{ 1, 0 };
				return create_tex2_implement(translate_depth_stencil_format_to_dxgi_format(DF), width, height, (miplevel.able_cast<UINT>() ? miplevel.cast<UINT>() : 1), (count ? *count : 1), sample.x, sample.y, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET, 0, data, line);
			}
			tex2 create_tex2_depth_stencil(DST_format DF, const tex2& t, void** data = nullptr, UINT* line = nullptr);

			tex2 create_tex_cube(DXGI_FORMAT DF, UINT width, UINT height, Tool::optional<UINT> miplevel = {}, Tool::optional<UINT> count = {}, void** data = nullptr, UINT* line = nullptr) {
				return create_tex2_implement(DF, width, height, (miplevel ? *miplevel : 1), (count ? *count * 6 : 6), 1, 0, D3D11_USAGE_DYNAMIC, D3D11_BIND_SHADER_RESOURCE, D3D11_RESOURCE_MISC_TEXTURECUBE, data, line);
			}
			tex2 create_tex_cube_unorder_access(DXGI_FORMAT DF, UINT width, UINT height, Tool::optional<UINT> miplevel = {}, Tool::optional<UINT> count = {}, void** data = nullptr, UINT* line = nullptr) {
				return create_tex2_implement(DF, width, height, (miplevel ? *miplevel : 1), (count ? *count * 6 : 6), 1, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS, D3D11_RESOURCE_MISC_TEXTURECUBE, data, line);
			}

			tex3 create_tex3(DXGI_FORMAT DF, UINT width, UINT height, UINT depth, Tool::optional<UINT> miplevel = {}, D3D11_USAGE usage = D3D11_USAGE_DYNAMIC, void* data = nullptr, UINT line = 0, UINT slice = 0){
				return create_tex3_implement(DF, width, height, depth, (miplevel ? *miplevel : 1), usage, D3D11_BIND_SHADER_RESOURCE, 0, data, line, slice);
			}
			tex3 create_tex3_unordered_access(DXGI_FORMAT DF, UINT width, UINT height, UINT depth, Tool::optional<UINT> miplevel = {}, void* data = nullptr, UINT line = 0, UINT slice = 0) {
				return create_tex3_implement(DF, width, height, depth, (miplevel ? *miplevel : 1), D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS, 0, data, line, slice);
			}

			shader_resource_view cast_shader_resource_view(const structured_buffer& t, Tool::optional<UINT2> range = {});

			shader_resource_view cast_shader_resource_view(const tex1& t, Tool::optional<UINT2> mip_range = {});
			shader_resource_view cast_shader_resource_view(const tex2& t, Tool::optional<UINT2> mip_range = {});

			shader_resource_view cast_shader_resource_view_array(const tex1& t, Tool::optional<UINT2> mip_range = {}, Tool::optional<UINT2> array = {});
			shader_resource_view cast_shader_resource_view_array(const tex2& t, Tool::optional<UINT2> mip_range = {}, Tool::optional<UINT2> array = {});

			shader_resource_view cast_shader_resource_view_ms(const tex2& t);
			shader_resource_view cast_shader_resource_view_ms_array(const tex2& t, Tool::optional<UINT2> array = {});

			shader_resource_view cast_shader_resource_view_cube(const tex2& t, Tool::optional<UINT2> mip_range = {});
			shader_resource_view cast_shader_resource_view_cube_array(const tex2& t, Tool::optional<UINT2> mip_range = {}, Tool::optional<UINT2> array = {});

			shader_resource_view cast_shader_resource_view(const tex3& t, Tool::optional<UINT2> mip_range = {});

			render_target_view cast_render_target_view(const tex1& t, UINT miplevel = 0);
			render_target_view cast_render_target_view(const tex2& t, UINT miplevel = 0);
			render_target_view cast_render_target_view(const tex3& t, UINT miplevel = 0, Tool::optional<UINT2> z_range = {});

			render_target_view cast_render_target_view_array(const tex1& t, UINT miplevel = 0, Tool::optional<UINT2> array_range = {});
			render_target_view cast_render_target_view_array(const tex2& t, UINT miplevel = 0, Tool::optional<UINT2> array_range = {});

			render_target_view cast_render_target_view_ms(const tex2& t);
			render_target_view cast_render_target_view_ms_array(const tex2& t, Tool::optional<UINT2> array_range = {});

			depth_stencil_view cast_depth_setncil_view(const tex1& t, UINT miplevel = 0, Tool::optional<bool> depth_read_only = {});
			depth_stencil_view cast_depth_setncil_view(const tex2& t, UINT miplevel = 0, Tool::optional<bool> depth_read_only = {});
			depth_stencil_view cast_depth_setncil_view_array(const tex1& t, UINT miplevel = 0, Tool::optional<UINT2> array_range = {}, Tool::optional<bool> depth_read_only = {});
			depth_stencil_view cast_depth_setncil_view_array(const tex2& t, UINT miplevel = 0, Tool::optional<UINT2> array_range = {}, Tool::optional<bool> depth_read_only = {});
			depth_stencil_view cast_depth_setncil_view_ms(const tex2& t, Tool::optional<bool> depth_read_only = {});
			depth_stencil_view cast_depth_setncil_view_ms_array(const tex2& t, Tool::optional<UINT2> array_range = {}, Tool::optional<bool> depth_read_only = {});

			unordered_access_view cast_unordered_access_view(const structured_buffer& tp, Tool::optional<UINT2> elemnt_start_and_count = {}, Tool::optional<UINT> offset = {}, bool is_append_or_consume = false);
			unordered_access_view cast_unordered_access_view(const tex1& tp, UINT mipslice = 0);
			unordered_access_view cast_unordered_access_view_array(const tex1& tp, UINT mipslice = 0, Tool::optional<UINT2> array = {});
			unordered_access_view cast_unordered_access_view(const tex2& tp, UINT mipslice = 0);
			unordered_access_view cast_unordered_access_view_array(const tex2& tp, UINT mipslice = 0, Tool::optional<UINT2> array = {});
			unordered_access_view cast_unordered_access_view(const tex3& tp, UINT mipslice = 0, Tool::optional<UINT2> z_range = {});
		};

		namespace Implement
		{

			struct shader_context_t
			{
				UINT srv_count = 0;
				UINT cb_count = 0;
				UINT sample_count = 0;


				void bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const shader_resource& id, 
					void (ID3D11DeviceContext::* cb_f)(UINT, UINT, ID3D11Buffer* const *),
					void (ID3D11DeviceContext::* srv_f)(UINT, UINT, ID3D11ShaderResourceView* const *),
					void (ID3D11DeviceContext::* sample_f)(UINT, UINT, ID3D11SamplerState* const *)
				);

				void unbind(Win32::com_ptr<ID3D11DeviceContext>& cp,
					void (ID3D11DeviceContext::* cb_f)(UINT, UINT, ID3D11Buffer* const *),
					void (ID3D11DeviceContext::* srv_f)(UINT, UINT, ID3D11ShaderResourceView* const *),
					void (ID3D11DeviceContext::* sample_f)(UINT, UINT, ID3D11SamplerState* const *)
				);

				void extract(Win32::com_ptr<ID3D11DeviceContext>& cp, shader_resource& id,
					void (ID3D11DeviceContext::* cb_f)(UINT, UINT, ID3D11Buffer**),
					void (ID3D11DeviceContext::* srv_f)(UINT, UINT, ID3D11ShaderResourceView**),
					void (ID3D11DeviceContext::* sample_f)(UINT, UINT, ID3D11SamplerState**)
				);
			};

			struct input_assember_context_t
			{
				UINT vb_count = 0;
				void bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const input_assember_stage& id);
				void extract(Win32::com_ptr<ID3D11DeviceContext>& cp, input_assember_stage& id);
				void unbind(Win32::com_ptr<ID3D11DeviceContext>& cp);
				void clear(Win32::com_ptr<ID3D11DeviceContext>& cp);
			};

			struct vertex_shader_context_t : shader_context_t
			{
				//size_t max_shader_resource_view = 0;
				//size_t max_sample = 0;
				void bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const vertex_stage& vs) { bind(cp, static_cast<const vertex_resource&>(vs)); bind(cp, vs.code); }
				void bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const vertex_resource&);
				void bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const vertex_shader& vs) { cp->VSSetShader(vs.ptr, nullptr, 0); }

				void extract(Win32::com_ptr<ID3D11DeviceContext>& cp, vertex_stage& vs) { extract(cp, static_cast<vertex_resource&>(vs)); extract(cp, vs.code); }
				void extract(Win32::com_ptr<ID3D11DeviceContext>& cp, vertex_resource& v);
				void extract(Win32::com_ptr<ID3D11DeviceContext>& cp, vertex_shader& vs);

				void unbind(Win32::com_ptr<ID3D11DeviceContext>& cp);
				void clear(Win32::com_ptr<ID3D11DeviceContext>& cp);
			};

			struct raterizer_context_t
			{
				//binding_count view_count;

				void bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const raterizer_state& rs);
				void bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const viewports& rs);

				void extract(Win32::com_ptr<ID3D11DeviceContext>& cp, raterizer_state& rs);
				void extract(Win32::com_ptr<ID3D11DeviceContext>& cp, viewports& rs);

				void unbind(Win32::com_ptr<ID3D11DeviceContext>& cp);
				void clear(Win32::com_ptr<ID3D11DeviceContext>& cp);
			};

			struct pixel_shader_context_t : shader_context_t
			{
				void bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const pixel_stage& ps) { bind(cp, static_cast<const pixel_resource&>(ps)); bind(cp, ps.code); }
				void bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const pixel_resource&);
				void bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const pixel_shader& ps) { cp->PSSetShader(ps.ptr, nullptr, 0); }
				
				void extract(Win32::com_ptr<ID3D11DeviceContext>& cp, pixel_stage& ps) { extract(cp, static_cast<pixel_stage&>(ps)); extract(cp, ps.code); }
				void extract(Win32::com_ptr<ID3D11DeviceContext>& cp, pixel_resource& ps);
				void extract(Win32::com_ptr<ID3D11DeviceContext>& cp, pixel_shader& ps);

				void unbind(Win32::com_ptr<ID3D11DeviceContext>& cp);
				void clear(Win32::com_ptr<ID3D11DeviceContext>& cp);
			};

			struct output_merge_context_t
			{
				UINT max_render_target = 0;
				void clear_render_target(Win32::com_ptr<ID3D11DeviceContext>& cp, output_merge_stage& omd, size_t solt, const std::array<float, 4>& color);
				void clear_render_target(Win32::com_ptr<ID3D11DeviceContext>& cp, output_merge_stage& omd, const std::array<float, 4>& color);
				void clear_depth(Win32::com_ptr<ID3D11DeviceContext>& cp, output_merge_stage& omd, float depth);
				void clear_stencil(Win32::com_ptr<ID3D11DeviceContext>& cp, output_merge_stage& omd, uint8_t ref);
				void clear_depth_stencil(Win32::com_ptr<ID3D11DeviceContext>& cp, output_merge_stage& omd, float depth, uint8_t ref);

				void extract(Win32::com_ptr<ID3D11DeviceContext>& cp, output_merge_stage& ps);

				void bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const output_merge_stage&);
				void bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const blend_state&);
				void bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const depth_stencil_state&);
				void unbind(Win32::com_ptr<ID3D11DeviceContext>& cp);
				void clear(Win32::com_ptr<ID3D11DeviceContext>& cp);
			};

			struct compute_shader_context_t : shader_context_t
			{
				UINT count = 0;
				void bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const compute_stage& cd) { bind(cp, static_cast<const compute_resource&>(cd)); bind(cp, cd.code); }
				void bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const compute_resource& cd);
				void bind(Win32::com_ptr<ID3D11DeviceContext>& cp, const compute_shader& cd) { cp->CSSetShader(cd.ptr, nullptr, 0); }

				void extract(Win32::com_ptr<ID3D11DeviceContext>& cp, compute_stage& ps) { extract(cp, static_cast<compute_resource&>(ps)); extract(cp, ps.code); }
				void extract(Win32::com_ptr<ID3D11DeviceContext>& cp, compute_resource& ps);
				void extract(Win32::com_ptr<ID3D11DeviceContext>& cp, compute_shader& ps);

				void unbind(Win32::com_ptr<ID3D11DeviceContext>& cp);
				void clear(Win32::com_ptr<ID3D11DeviceContext>& cp);
			};
		}

		struct pipeline
		{
			Win32::com_ptr<ID3D11DeviceContext> ptr;
			creator c;
			Implement::input_assember_context_t IA;
			Implement::vertex_shader_context_t VS;
			Implement::raterizer_context_t RA;
			Implement::pixel_shader_context_t PS;
			Implement::output_merge_context_t OM;

			Implement::compute_shader_context_t CS;

			pipeline(Win32::com_ptr<ID3D11DeviceContext> cp);
			pipeline() {}
			void init(Win32::com_ptr<ID3D11DeviceContext> d);
			operator bool() const { return ptr; }
			~pipeline() { clear(); }
			void clear();

			creator& get_creator() { return c; };

			enum DrawMode
			{
				PIPELINE,
				COMPLUTE,
				NONE
			};

			DrawMode last_mode = DrawMode::NONE;

			void dispatch(UINT x, UINT y, UINT z);

			void draw_vertex(UINT count, UINT start);
			void draw_index(UINT index_count, UINT index_start, UINT vertex_start);
			void draw_vertex_instance(UINT vertex_pre_instance, UINT instance_count, UINT vertex_start, UINT instance_start);
			void draw_index_instance(UINT index_pre_instance, UINT index_count, UINT index_start, UINT vertex_start, UINT instance_start);

			void unbind();

			pipeline& bind(const input_assember_stage& d) { IA.bind(ptr, d); return *this; }
			pipeline& bind(const vertex_stage& d) { VS.bind(ptr, d); return *this; }
			pipeline& bind(const vertex_resource& d) { VS.bind(ptr, d); return *this; }
			pipeline& bind(const vertex_shader& d) { VS.bind(ptr, d); return *this; }
			pipeline& bind(const pixel_stage& d) { PS.bind(ptr, d); return *this; }
			pipeline& bind(const pixel_resource& d) { PS.bind(ptr, d); return *this; }
			pipeline& bind(const pixel_shader& d) { PS.bind(ptr, d); return *this; }
			pipeline& bind(const output_merge_stage& d) { OM.bind(ptr, d); return *this; }
			pipeline& bind(const raterizer_state& rs) { RA.bind(ptr, rs); return *this; }
			pipeline& bind(const compute_stage& cd) { CS.bind(ptr, cd); return *this; }
			pipeline& bind(const compute_resource& cd) { CS.bind(ptr, cd); return *this; }
			pipeline& bind(const compute_shader& cd) { CS.bind(ptr, cd); return *this; }
			pipeline& bind(const viewports& vp) { RA.bind(ptr, vp); return *this; }
			pipeline& bind(const blend_state& bs) { OM.bind(ptr, bs); return *this; }
			pipeline& bind(const depth_stencil_state& dss) { OM.bind(ptr, dss); return *this; }

			template<typename T> pipeline& operator<<(const T& t) { return bind(t); }

			pipeline& clear_render_target(output_merge_stage& omd, size_t solt, const std::array<float, 4>& color) { OM.clear_render_target(ptr, omd, solt, color); return *this; }
			pipeline& clear_render_target(output_merge_stage& omd, const std::array<float, 4>& color) { OM.clear_render_target(ptr, omd, color); return *this;}
			pipeline& clear_depth(output_merge_stage& omd, float depth) { OM.clear_depth(ptr, omd, depth); return *this;}
			pipeline& clear_stencil(output_merge_stage& omd, uint8_t ref) { OM.clear_stencil(ptr, omd, ref); return *this;}
			pipeline& clear_depth_stencil(output_merge_stage& omd, float depth, uint8_t ref) { OM.clear_depth_stencil(ptr, omd, depth, ref); return *this;}

			template<typename T> bool write_constant_buffer(constant_buffer& b, T&& t)
			{
				if (b.ptr == nullptr) return false;
				D3D11_MAPPED_SUBRESOURCE DMS;
				if (SUCCEEDED(ptr->Map(b.ptr, 0, D3D11_MAP_WRITE_DISCARD, 0, &DMS)))
				{
					Tool::at_scope_exit ate([&, this]() { ptr->Unmap(b.ptr, 0); });
					return Tool::auto_adapter_unorder(t, DMS.pData, DMS.RowPitch, DMS.DepthPitch), true;
				}
				return false;
			}

			template<typename T> bool write_constant_buffer(shader_resource& b, size_t o, T&& t)
			{
				if (b.cbuffer_array.size() <= o) return false;
				D3D11_MAPPED_SUBRESOURCE DMS;
				auto ptr_buffer = b.cbuffer_array[o];
				if (SUCCEEDED(ptr->Map(ptr_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &DMS)))
				{
					Tool::at_scope_exit ate([&, this]() { ptr->Unmap(ptr_buffer, 0); });
					return Tool::auto_adapter_unorder(t, DMS.pData, DMS.RowPitch, DMS.DepthPitch), true;
				}
				return false;
			}
		};
	}
}