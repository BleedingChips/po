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
#include "../../frame/define.h"
#include "../dx/dx_math.h"
namespace PO
{
	namespace Dx11
	{
		using namespace Dx;
		namespace Implement
		{
			using resource_ptr = CComPtr<ID3D11Device>;
			using context_ptr = CComPtr<ID3D11DeviceContext>;
			using chain_ptr = CComPtr<IDXGISwapChain>;
			using buffer_ptr = CComPtr<ID3D11Buffer>;
			using vshader_ptr = CComPtr<ID3D11VertexShader>;
			using pshader_ptr = CComPtr<ID3D11PixelShader>;
			using layout_ptr = CComPtr<ID3D11InputLayout>;
			using texture2D_ptr = CComPtr<ID3D11Texture2D>;
			using texture1D_ptr = CComPtr<ID3D11Texture1D>;
			using texture3D_ptr = CComPtr<ID3D11Texture3D>;

			using unorder_view_ptr = CComPtr<ID3D11UnorderedAccessView>;
			using render_view_ptr = CComPtr<ID3D11RenderTargetView>;
			using depth_stencil_view_ptr = CComPtr<ID3D11DepthStencilView>;
			using resource_view_ptr = CComPtr<ID3D11ShaderResourceView>;

			using sample_state_ptr = CComPtr<ID3D11SamplerState>;

			using cshader_ptr = CComPtr<ID3D11ComputeShader>;
			using gshader_ptr = CComPtr<ID3D11GeometryShader>;
			using dshader_ptr = CComPtr<ID3D11DomainShader>;

			using raterizer_state_ptr = CComPtr<ID3D11RasterizerState>;
			using blend_state_ptr = CComPtr<ID3D11BlendState>;
			using depth_stencil_state_ptr = CComPtr<ID3D11DepthStencilState>;

			HRESULT create_texture_implement(resource_ptr& cp, texture1D_ptr& t, size_t w, size_t s, void* data, DXGI_FORMAT DF, size_t miplevel, D3D11_USAGE DU, UINT BIND, bool cpu_w, UINT mis);
			HRESULT create_texture_implement(resource_ptr& cp, texture2D_ptr& t, size_t w, size_t h, size_t s, void* data, DXGI_FORMAT DF, size_t miplevel, D3D11_USAGE DU, UINT BIND, bool cpu_w, UINT mis);
			bool avalible_depth_texture_format(DXGI_FORMAT DF);
		}

		

		struct buffer_base
		{
			Implement::buffer_ptr ptr;
		};

		struct vertex_scr
		{
			UINT offset;
			UINT element_size;
			size_t num;
			std::vector<D3D11_INPUT_ELEMENT_DESC> layout;
		};

		struct index_scr
		{
			UINT offset;
			DXGI_FORMAT format;
			size_t num;
		};

		struct buffer_unorder_scr
		{
			Implement::unorder_view_ptr view;
			buffer_unorder_scr() = default; buffer_unorder_scr(const buffer_unorder_scr&) = default;
			buffer_unorder_scr(buffer_unorder_scr&& us) : view(us.view) { us.view = nullptr; }
		};

		struct buffer_resource_scr
		{
			Implement::resource_view_ptr view;
		};

		template<typename ...AT> struct buffer : buffer_base
		{
			static_assert(!Tmp::is_repeat<AT...>::value, "buffer scription can not repeat");
			template<typename T> using is_one_of = Tmp::is_one_of<T, AT...>;
			std::tuple<AT...> scription;
		};

		struct cbuffer
		{
			Implement::buffer_ptr ptr;
		};

		struct tbuffer
		{
			Implement::buffer_ptr ptr;
		};

		struct input_assember_d
		{
			std::vector<ID3D11Buffer*> vertex_array;
			std::vector<UINT> offset_array;
			std::vector<UINT> element_array;
			std::vector<D3D11_INPUT_ELEMENT_DESC> input_element;
			Implement::layout_ptr layout;
			D3D11_PRIMITIVE_TOPOLOGY primitive = D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

			Implement::buffer_ptr index_ptr;
			UINT offset;
			DXGI_FORMAT format;

			void set_vertex_implement(Implement::buffer_ptr bp, size_t solt, const vertex_scr& vs);
			void set_index_implement(Implement::buffer_ptr bp, const index_scr& is);

			template<typename ...T> auto set_index(const buffer<T...>& b) -> std::enable_if_t<Tmp::is_one_of<index_scr, T...>::value>{
				set_index_implement(b.ptr, std::get<index_scr>(b.scription));
			}

			template<typename ...T> auto set_vertex(const buffer<T...>& b, size_t solt) -> std::enable_if_t<Tmp::is_one_of<vertex_scr, T...>::value> {
				set_vertex_implement(b.ptr, solt, std::get<vertex_scr>(b.scription));
			}

			input_assember_d() = default;
			input_assember_d(const input_assember_d&);
			input_assember_d(input_assember_d&& i);
			input_assember_d& operator= (input_assember_d&& i);
			input_assember_d& operator= (const input_assember_d&) = default;
			~input_assember_d();
		};

		namespace Implement
		{

			template<size_t i> struct texture_ptr_type { static_assert(i > 0 && i < 4, "texture only receive 1,2,3"); };
			template<> struct texture_ptr_type<1> 
			{ 
				using type = Implement::texture1D_ptr;
				//using des_type = D3D11_TEXTURE1D_DESC;
				static bool create_RTV(Implement::resource_ptr& cp, Implement::render_view_ptr& rvp, const type& t, size_t mipslice, size_t array_start, size_t array_count, bool all_range);
				static bool create_DSV(Implement::resource_ptr& cp, Implement::depth_stencil_view_ptr& dsv, const type& t, bool dr, bool sr, size_t mipslice, size_t array_start, size_t array_count, bool all_range);
				static bool create(Implement::resource_ptr& cp, type& t, size_t w, size_t h, size_t d)
			};
			template<> struct texture_ptr_type<2> 
			{ 
				using type = Implement::texture2D_ptr; 
				static bool create_RTV(Implement::resource_ptr& cp, Implement::render_view_ptr& rvp, const type& t, size_t mipslice, size_t array_start, size_t array_count, bool all_range);
				static bool create_ms_RTV(Implement::resource_ptr& cp, Implement::render_view_ptr& rvp, const type& t, size_t array_start, size_t array_count, bool all_range);
				static bool create_DSV(Implement::resource_ptr& cp, Implement::depth_stencil_view_ptr& dsv, const type& t, bool dr, bool sr, size_t mipslice, size_t array_start, size_t array_count, bool all_range);
				static bool create_ms_DSV(Implement::resource_ptr& cp, Implement::depth_stencil_view_ptr& dsv, const type& t, bool dr, bool sr, size_t array_start, size_t array_count, bool all_range);
			};
			template<> struct texture_ptr_type<3>
			{ 
				using type = Implement::texture3D_ptr; 
				static bool create_RTV(Implement::resource_ptr& cp, Implement::render_view_ptr& rvp, const type& t, size_t mipslice, size_t array_start, size_t array_count, bool all_range);
			};
			template<size_t i> using texture_ptr_t = typename texture_ptr_type<i>::type;
		}

		using texture1 = Implement::texture1D_ptr;
		using texture2 = Implement::texture2D_ptr;
		using texture3 = Implement::texture3D_ptr;

		using render_target_v = Implement::render_view_ptr;
		using depth_stencil_v = Implement::depth_stencil_view_ptr;

		struct shader_d
		{
			
		};

		struct vertex_shader_d : shader_d
		{
			binary code;
			Implement::vshader_ptr ptr;
		};

		struct pixel_shader_d : shader_d
		{
			Implement::pshader_ptr ptr;
		};

		struct output_merge_d
		{
			template<typename T, typename ...AT> using rt = std::enable_if_t<Tmp::is_one_of<T, AT...>::value>;

			std::vector<ID3D11RenderTargetView*> render_array;
			Implement::depth_stencil_view_ptr depth;

			void set_render_implement(size_t o, Implement::render_view_ptr rv);
			template<size_t i, typename ...AT>
			auto set_depth(const texture<i, AT...>& t) ->rt<texture_depth_scr, AT...> { depth = std::get<texture_depth_scr>(t.scription).view; }

			template<size_t i, typename ...AT>
			auto set_render(size_t o, const texture<i, AT...>& t) ->rt<texture_render_scr, AT...> { set_render_implement(std::get<texture_render_scr>(t.scription).view, o); }
			output_merge_d() {}
			~output_merge_d();
		};

		struct draw_range_d
		{
			struct range
			{
				int32_t start;
				int32_t end;
				operator bool() const { return end > start; }
				UINT count() const { return static_cast<UINT>(end - start); }
				UINT at() const { return static_cast<UINT>(start); }
			};

			range vertex;
			range index;
			range instance;
		};

		

		/*
		struct shader_d
		{
			std::vector<ID3D11ShaderResourceView*> 
		};
		*/

		namespace Implement
		{
			struct input_assember_resource_t
			{
				Implement::resource_ptr res;
				
				using vertex_t = buffer<vertex_scr>;
				using index_t = buffer<index_scr>;
				
				using index_vertex_t = buffer<index_scr, vertex_scr>;

				bool create_vertex_implement(Implement::buffer_ptr& bp, vertex_scr& scr, void* data, size_t ele, size_t num, std::vector<D3D11_INPUT_ELEMENT_DESC> layout, bool cpu_write);
				bool create_index_implement(Implement::buffer_ptr& bp, index_scr& scr, void* data, size_t size, size_t num, DXGI_FORMAT DF, bool cpu_write);

				input_assember_resource_t(Implement::resource_ptr r) :res(r) {}
				bool create_vertex(vertex_t& b, void* data, size_t ele, size_t num, std::vector<D3D11_INPUT_ELEMENT_DESC> layout, bool cpu_write = false) {
					return create_vertex_implement(b.ptr, std::get<vertex_scr>(b.scription), data, ele, num, std::move(layout), cpu_write);
				}
				template<typename K> bool create_vertex(vertex_t& b, K* data, size_t num, std::vector<D3D11_INPUT_ELEMENT_DESC> layout, bool cpu_write = false) {
					return create_vertex_implement(b.ptr, std::get<vertex_scr>(b.scription), data, sizeof(K), num, std::move(layout), cpu_write);
				}
				template<typename K, typename A> auto create_vertex(vertex_t& b, const std::vector<K, A>& v, std::vector<D3D11_INPUT_ELEMENT_DESC> layout, bool cpu_write = false) {
					return create_vertex_implement(b.ptr, std::get<vertex_scr>(b.scription), v.data(), sizeof(K), v.size(), std::move(layout), cpu_write);
				}

				bool create_index(index_t& b, void* data, size_t elemnt_size, size_t num, DXGI_FORMAT DF, bool cpu_write = false) {
					return create_index_implement(b.ptr, std::get<index_scr>(b.scription), data, elemnt_size, num, DF, cpu_write);
				}
				template<typename K> bool create_index(index_t& b, K* data, size_t num, bool cpu_write = false) {
					return create_index_implement(b.ptr, std::get<index_scr>(b.scription), data, sizeof(K), num, DXGI::data_format<K>::format, cpu_write);
				}
				template<typename K, typename A> auto create_index(index_t& b, const std::vector<K, A>& v, bool cpu_write = false) {
					return create_index_implement(b.ptr, std::get<index_scr>(b.scription), v.data(), sizeof(K), v.size(), DF, cpu_write);
				}

				template<typename T, typename K>
				bool create_vertex(input_assember_d& iad, size_t solt, const std::vector<T, K>& v, std::vector<D3D11_INPUT_ELEMENT_DESC> layout, bool cpu_werite = false){
					vertex_t b;
					if (create_vertex(b, v, std::move(layout), cpu_werite);)
						return (iad.set_vertex(b, solt), true);
					return false;
				}

				template<typename T>
				bool create_vertex(input_assember_d& iad, size_t solt, T* data, size_t num, std::vector<D3D11_INPUT_ELEMENT_DESC> layout, bool cpu_werite = false) {
					vertex_t b;
					if (create_vertex(b, data, num, std::move(layout), cpu_werite))
						return (iad.set_vertex(b, solt), true);
					return false;
				}

				template<typename T, typename K>
				bool create_index(input_assember_d& iad, const std::vector<T, K>& v, bool cpu_werite = false) {
					index_t b;
					if (create_index(b, v, cpu_werite))
						return (iad.set_index(b), true);
					return false;
				}

				template<typename T>
				bool create_index(input_assember_d& iad, T* v, size_t num, bool cpu_werite = false) {
					index_t b;
					if (create_index(b, v, num, cpu_werite))
						return (iad.set_index(b), true);
					return false;
				}

				bool update_layout(input_assember_d& iad, const vertex_shader_d& vs);
			};

			struct shader_resource_t
			{
				Implement::resource_ptr res;
				shader_resource_t(Implement::resource_ptr r) :res(r) {}
				bool create_shader(vertex_shader_d& vsd, binary b);
				bool create_shader(pixel_shader_d& psd, const binary& b);
			};

			struct output_merge_resource_t
			{
				Implement::resource_ptr res;
				output_merge_resource_t(Implement::resource_ptr r) :res(r) {}

				using rview_ptr = Implement::render_view_ptr;

				struct RTV_capture
				{
					struct MS
					{
						Tool::optional<std::pair<size_t, size_t>> pair;
					};
					size_t mipslice;
					Tool::optional<std::pair<size_t, size_t>> range;
				};

				struct DSV_capture
				{
					enum class ReadOnly
					{
						NONE,
						DEPTH,
						STENCIL
					};
					struct MS
					{
						Tool::optional<std::pair<size_t, size_t>> range;
						ReadOnly read_only = ReadOnly::NONE;
					};
					size_t mipslice;
					Tool::optional<std::pair<size_t, size_t>> range;
					ReadOnly read_only = ReadOnly::NONE;
				};

				bool create_render_target_view(render_target_v& rtv, const texture1&, const RTV_capture&);
				bool create_render_target_view(render_target_v& rtv, const texture2&, const RTV_capture&);
				bool create_render_target_view_ms(render_target_v& rtv, const texture2&, const RTV_capture::MS&);
				bool create_render_target_view(render_target_v& rtv, const texture3&, const RTV_capture&);

				bool create_depth_stencil_view(render_target_v& rtv, const texture1&, const RTV_capture&);
				bool create_depth_stencil_view(render_target_v& rtv, const texture2&, const RTV_capture&);
				bool create_depth_stencil_view_ms(render_target_v& rtv, const texture2&, const RTV_capture::MS&);

				/*
				template<size_t i>
				bool create_render_view(texture<i, texture_render_scr>& t, const texture_ptr_t<i>& p, size_t mipslice = 0)
				{
					if (texture_ptr_type<i>::create_RTV(res, std::get<texture_render_scr>(t.scription).view, p, mipslice, 0, 0, true))
						return (t.ptr = p, true);
					return false;
				}

				template<size_t i>
				bool create_render_view(texture<i, texture_render_scr>& t, const texture_ptr_t<i>& p, size_t mipslice, size_t array_strat, size_t array_count)
				{
					if (texture_ptr_type<i>::create_RTV(res, std::get<texture_render_scr>(t.scription).view, p, mipslice, array_strat, array_count, false))
						return (t.ptr = p, true);
					return false;
				}

				bool create_render_view_ms(texture<2, texture_render_scr>& t, const texture_ptr_t<2>& p, size_t array_strat, size_t array_count);
				bool create_render_view_ms(texture<2, texture_render_scr>& t, const texture_ptr_t<2>& p);

				template<size_t i>
				bool create_depth_view(texture<i, texture_depth_scr>& t, const texture_ptr_t<i>& p, bool dr = false, bool sr = false, size_t mipslice = 0)
				{
					if (texture_ptr_type<i>::create_DSV(res, std::get<texture_depth_scr>(t.scription).view, p, dr, sr, mipslice, 0, 0, true))
						return (t.ptr = p, true);
					return false;
				}

				template<size_t i>
				bool create_render_view(texture<i, texture_depth_scr>& t, const texture_ptr_t<i>& p, bool dr, bool sr, size_t mipslice, size_t array_strat, size_t array_count)
				{
					if (texture_ptr_type<i>::create_DSV(res, std::get<texture_depth_scr>(t.scription).view, p, dr, sr, mipslice, array_strat, array_count, false))
						return (t.ptr = p, true);
					return false;
				}

				bool create_depth_view_ms(texture<2, texture_depth_scr>& t, const texture_ptr_t<2>& p, bool dr, bool sr, size_t array_strat, size_t array_count);
				bool create_depth_view_ms(texture<2, texture_depth_scr>& t, const texture_ptr_t<2>& p, bool dr = false, bool sr = false);
				*/
				/*
				template<size_t i>
				bool create_RT_texture(texture<2, texture_render_scr>& t, size_t w, size_t h, size_t h, )
				*/
			};
		};

		struct resource
		{
			Implement::input_assember_resource_t IA;
			Implement::shader_resource_t SR;
			resource(Implement::resource_ptr rp) : IA(rp), SR(rp) {}
		};

		namespace Implement
		{
			struct input_assember_context_t
			{
				Implement::context_ptr cp;
				input_assember_context_t(Implement::context_ptr c) : cp(c) {}
				size_t max_buffer_solt = 0;
				bool bind(input_assember_d& id);
				bool rebind_layout(Implement::layout_ptr lp);
				void unbind();
			};

			struct vertex_shader_context_t
			{
				Implement::context_ptr cp;
				vertex_shader_context_t(Implement::context_ptr c) : cp(c) { }
				bool bind(const vertex_shader_d&);
				void unbind();
			};

			struct pixel_shader_context_t
			{
				Implement::context_ptr cp;
				pixel_shader_context_t(Implement::context_ptr c) : cp(c) {}
				bool bind(const pixel_shader_d&);
				void unbind();
			};

			struct output_merge_context_t
			{
				size_t max_size = 0;
				Implement::context_ptr cp;
				output_merge_context_t(Implement::context_ptr c) : cp(c) {}
				bool bind(const output_merge_d&);
				void unbind();
			};

			struct draw_range_context_t
			{
				Implement::context_ptr cp;
				draw_range_context_t(Implement::context_ptr c) : cp(c) {}
				bool draw(const draw_range_d& d);
				
			};
		}

		struct pipe_line
		{
			using IA_t = Implement::input_assember_context_t; IA_t IA;
			using VS_t = Implement::vertex_shader_context_t; VS_t VS;
			using PS_t = Implement::pixel_shader_context_t; PS_t PS;
			using OM_t = Implement::output_merge_context_t;	OM_t OM;
			using DR_t = Implement::draw_range_context_t; DR_t DR;
	
			template<typename T>
			void bind_render(T&& t)
			{
				t(IA, VS, PS, OM, DR);
				IA.unbind(); VS.unbind(); PS.unbind(); OM.unbind();
			}
			pipe_line(Implement::context_ptr cp) : IA(cp), VS(cp), PS(cp), OM(cp), DR(cp) {}
		};


		/*
		struct rasterizar_setting
		{
			D3D11_RASTERIZER_DESC DRD = { D3D11_FILL_SOLID, D3D11_CULL_NONE, false, 0, 0.0f, 0.0f, true, false, false, false };
		};

		struct rasterizar_state
		{
			Implement::raterizer_state_ptr ptr;
		};

		struct blend_setting
		{
			D3D11_BLEND_DESC DBD{ false, false, { D3D11_RENDER_TARGET_BLEND_DESC 
			{
				true,
				D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA ,
				D3D11_BLEND_OP_ADD,
				D3D11_BLEND_ONE , D3D11_BLEND_ZERO ,
				D3D11_BLEND_OP_ADD ,
				D3D11_COLOR_WRITE_ENABLE_ALL
			}}};
		};

		struct blend_state
		{
			Implement::blend_state_ptr ptr;
			std::array<float, 4> blend_factor = { 0.0, 0.0, 0.0, 0.0 };
			UINT sample_mask = 0xffffffff;
		};

		struct depth_setting
		{
			D3D11_DEPTH_STENCIL_DESC DDSD{ false, D3D11_DEPTH_WRITE_MASK_ALL , D3D11_COMPARISON_LESS , false, 0, 0, D3D11_DEPTH_STENCILOP_DESC{ D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS } };
		};

		struct depth_state
		{
			Implement::depth_stencil_state_ptr dsp;
			UINT stencil_ref = 0;
		};

		HRESULT create_buffer(Implement::resource_ptr& rp, Implement::buffer_ptr& bp, D3D11_USAGE usage, UINT cpu_flag, UINT bind_flag, const void* data, size_t data_size, UINT misc_flag, size_t StructureByteStrides);

		class vertex_pool
		{
			std::vector<ID3D11Buffer*> vertex_array;
			std::vector<UINT> offset_array;
			std::vector<UINT> element_array;
			std::vector<D3D11_INPUT_ELEMENT_DESC> input_element;
			
			Implement::buffer_ptr index_ptr;
			UINT index_offset;
			DXGI_FORMAT index_format;
			D3D11_PRIMITIVE_TOPOLOGY primitive = D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		public:
			vertex_pool() {}
			vertex_pool(const vertex_pool&);
			vertex_pool(vertex_pool&&) = default;
			vertex_pool& operator=(const vertex_pool&);
			vertex_pool& operator=(vertex_pool&&) = default;

			bool create_index_const(void* data, size_t buffer_size, DXGI_FORMAT DF);
			bool create_index(void* data, size_t buffer_size, DXGI_FORMAT DF);
			template<typename T> bool create_index_const(T* data, size_t ver_size) { return create_index_const(data, sizeof(T) * ver_size, DXGI::data_format<T>::format); }
			template<typename T> bool create_index(T* data, size_t ver_size) { return create_index(data, sizeof(T) * ver_size, DXGI::data_format<T>::format); }
			template<typename T, typename P> bool create_index_const(const std::vector<T, P>& p) { return create_index_const(p.data(), p.size()); }
			template<typename T, typename P> bool create_index(const std::vector<T, P>& p) { return create_index(p.data(), p.size()); }
		};

		struct pipe_line
		{
			struct base_stage
			{
				Implement::resource_ptr res;
				//bool create_const_buffer(void* data, size_t size, )
			};
		};



		struct compute_stage
		{
			Implement::resource_ptr
		};

		struct stage
		{
			Implement::resource_ptr res;
			//struct 
			stage()
			{
				Implement::context_ptr cp;
			}
		};

		struct context_stage
		{

		};
		*/


		/*
		struct vertex_factor
		{
			Implement::resource_ptr rp;

			std::vector<ID3D11Buffer*> vertex_ptr;
			std::vector<UINT> vertex_offset;
			std::vector<UINT> vertex_element;
			std::vector<D3D11_INPUT_ELEMENT_DESC> vertex_input_element;
			Implement::layout_ptr vertex_layout;

			Implement::buffer_ptr index_ptr;
			UINT index_offset;
			DXGI_FORMAT index_DF;

			bool state = false;
			binary vshader_binary;

			Implement::vshader_ptr vshader;
			Implement::gshader_ptr gshader;
			
			D3D11_PRIMITIVE_TOPOLOGY primitive = D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

			bool create_index(void* data, size_t s, DXGI_FORMAT DF,  D3D11_USAGE DU, UINT flag);
		public:
			bool bind(Implement::resource_ptr& r);
			~vertex_factor();
			template<typename T>
			bool create_const_index(T* d, size_t s)
			{
				if (rp == nullptr)return false;
				Implement::buffer_ptr ptr;
				if(create_buffer())
			}
		};
		*/

		


		/*
		struct buffer
		{
			Implement::buffer_ptr ptr;
			//uint64_t vision;
			size_t buffer_size;
			buffer& operator= (const buffer& b)
			{
				ptr = b.ptr;
				buffer_size = b.buffer_size;
				return *this;
			}
			void clear() { ptr = nullptr; }
			bool create(Implement::resource_ptr& rp, D3D11_USAGE usage, UINT cpu_flag, UINT bind_flag, const void* data, size_t data_size, UINT misc_flag, size_t StructureByteStride);
			template<typename T> auto write(Implement::context_ptr& cp, T& t) -> Tool::optional_t<decltype(t(nullptr, 0))>
			{
				if (ptr != nullptr)
				{
					D3D11_BUFFER_DESC DBD;
					ptr->GetDesc(&DBD);
					if (
						(DBD.Usage == D3D11_USAGE::D3D11_USAGE_DYNAMIC || DBD.Usage == D3D11_USAGE::D3D11_USAGE_STAGING)
						&& ((DBD.CPUAccessFlags & D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE) == D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE)
						)
					{
						D3D11_MAPPED_SUBRESOURCE DMS;
						HRESULT re = cp->Map(ptr, 0, D3D11_MAP_WRITE_DISCARD, 0, &DMS);
						if (re != S_OK)
							return{};
						Tool::at_scope_exit tem([&,this]()
						{
							cp->Unmap(ptr, 0);
						});
						return{ Tool::return_optional_t(t , DMS.pData, DBD.ByteWidth) };
					}
				}
				return{};
			}
			operator bool() const { return ptr != nullptr; }
		};
		*/

		/*
		struct index : buffer
		{
			size_t offset;
			DXGI_FORMAT format;
			bool create_index(Implement::resource_ptr& rp, Purpose::purpose bp, const void* data, size_t buffer_size, DXGI_FORMAT DF)
			{
				if (buffer::create(rp, bp.usage, bp.cpu_flag, bp.additional_bind | D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER, data, buffer_size, 0, 0))
				{
					offset = 0;
					format = DF;
					return true;
				}
				return false;
			}
			template<typename T>
			bool create_index(Implement::resource_ptr& rp, Purpose::purpose bp, T* data, size_t size)
			{
				return create_index(rp, bp, data, sizeof(T)*size, DXGI::data_format<T>::format);
			}
		};

		struct vertex : buffer
		{
			size_t offset;
			size_t element_size;
			std::vector<D3D11_INPUT_ELEMENT_DESC> desc;
			void clear() { buffer::clear(); desc.clear(); }
			bool create_vertex(Implement::resource_ptr& rp, Purpose::purpose bp, const void* data, size_t element_size, size_t buffer_size, std::vector<D3D11_INPUT_ELEMENT_DESC> des)
			{
				if (buffer::create(rp, bp.usage, bp.cpu_flag, bp.additional_bind | D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER, data, buffer_size, 0, 0))
				{
					offset = 0;
					this->element_size = element_size;
					desc = std::move(des);
					return true;
				}
				return false;
			}

			template<typename T>
			bool create_vertex(Implement::resource_ptr& rp, Purpose::purpose bp, T* data, size_t size, std::vector<D3D11_INPUT_ELEMENT_DESC> des)
			{
				return create_vertex(rp, bp, data, sizeof(T), sizeof(T) * size, std::move(des));
			}
		};

		bool create_index_vertex_buffer(Implement::resource_ptr& rp, Purpose::purpose bp,
			const void* data,  size_t buffer_size,
			index& ind, vertex& ver,
			size_t index_offset, DXGI_FORMAT format,
			size_t vertex_offset, size_t element_size, std::vector<D3D11_INPUT_ELEMENT_DESC> layout
		);

		struct constant_value : buffer
		{
			bool create_value(Implement::resource_ptr& rp, Purpose::purpose bp, const void* data, size_t buffer_size)
			{
				return buffer::create(rp, bp.usage, bp.cpu_flag, bp.additional_bind | D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER, data, buffer_size, 0, 0);
			}
			template<typename T>
			bool create_value(Implement::resource_ptr& rp, Purpose::purpose bp, T* data)
			{
				return create_value(rp, bp, data, sizeof(T));
			}
		};

		struct structured_value : buffer
		{
			bool create_value(Implement::resource_ptr& rp, Purpose::purpose bp, const void* data,  size_t element_size, size_t buffer_size)
			{
				return buffer::create(rp, bp.usage, bp.cpu_flag, bp.additional_bind | D3D11_BIND_FLAG::D3D11_BIND_UNORDERED_ACCESS, data, buffer_size, D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, element_size);
			}
			template<typename T>
			bool create_value(Implement::resource_ptr& rp, Purpose::purpose bp, T* data, size_t size)
			{
				return create_value(rp, bp, data, sizeof(T), sizeof(T) * size);
			}
		};*/

		Implement::resource_view_ptr cast_resource(Implement::resource_ptr& rp, const Implement::texture2D_ptr& pt);
		Implement::resource_view_ptr cast_resource(Implement::resource_ptr& rp, const Implement::texture1D_ptr& pt);
		Implement::resource_view_ptr cast_resource(Implement::resource_ptr& rp, const Implement::texture3D_ptr& pt);
		DXGI_FORMAT adjust_texture_format(DXGI_FORMAT DF);

		Implement::texture2D_ptr create_render_target(Implement::resource_ptr& rp, size_t w, size_t e, DXGI_FORMAT DF);
		Implement::render_view_ptr cast_render_view(Implement::resource_ptr& rp, Implement::texture2D_ptr tp);
		//Implement::texture2D_ptr create_depth_s(Implement::resource_ptr& rp, size_t w, size_t e, DXGI_FORMAT DF);
		

		

		inline bool is_resource_available_for_context(Implement::resource_ptr rp, Implement::context_ptr cp)
		{
			if (rp == nullptr || cp == nullptr) return false;
			Implement::resource_ptr r;
			cp->GetDevice(&r);
			return rp.IsEqualObject(r);
		}

		

		/*
		struct pixel
		{
			struct pixel_solt_control
			{

			};
			Implement::resource_ptr rp;

			ID3D11Buffer* buffer_array[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
			UINT offset[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
			UINT element[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
			std::vector<std::shared_ptr<pixel_solt_control>> control;

			ID3D11Buffer* index;
			UINT offset;
			DXGI_FORMAT format;

			Implement::vshader_ptr vshader;
			binary v_binary;

			Implement::gshader_ptr gshader;


			bool update = false;
		};
		*/
		/*
		struct pixel_creater
		{
			Implement::resource_ptr rp;

			vertex vec[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
			bool update_flag = false;
			binary vshader_binary;

			index ind;
			
			Implement::vshader_ptr vshader;
			Implement::gshader_ptr gshader;
			
			Implement::layout_ptr layout;
			D3D11_PRIMITIVE_TOPOLOGY primitive = D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

			bool bind(Implement::resource_ptr& rp);
			bool load_vshader(std::u16string path);
			bool load_gshader(std::u16string path);

			template<typename T, typename K>
			bool create_vertex(size_t solt, T* ver_data, size_t ver_size, K&& fun, Purpose::purpose p = Purpose::constant)
			{
				if (rp == nullptr) return false;
				update_flag = true;
				return vec[solt].create_vertex(rp, p, ver_data, ver_size, fun(solt));
			}
			template<typename T, typename A, typename K>
			bool create_vertex(size_t solt, const std::vector<T,A>& v, K&& fun, Purpose::purpose p = Purpose::constant)
			{
				return this->create_vertex(solt, v.data(), v.size(), std::move(fun), p);
			}

			template<typename T>
			bool create_index(T* index_data, size_t index_size, Purpose::purpose p = Purpose::constant)
			{
				if (rp == nullptr) return false;
				return ind.create_index(rp, p, index_data, index_size);
			}

			template<typename T, typename A, typename K>
			bool create_index(const std::vector<T, A>& v, Purpose::purpose p = Purpose::constant)
			{
				return this->create_index(v.data(), v.size, p);
			}

			bool update_layout();
			bool create_index_vertex(size_t count, Implement::resource_ptr& rp, Purpose::purpose bp,
				const void* data, size_t buffer_size,
				size_t index_offset, DXGI_FORMAT format,
				size_t vertex_offset, size_t element_size, std::vector<D3D11_INPUT_ELEMENT_DESC> layout
			)
			{
				update_flag = true;
				return create_index_vertex_buffer(rp, bp, data, buffer_size, ind, vec[count], index_offset, format, vertex_offset, element_size, std::move(layout));
			}
			struct range { UINT start, count; }index_r, vertex_r, instance_r;
			void set_index_range(size_t s, size_t e) { index_r = range{ static_cast<UINT>(s), static_cast<UINT>(e) }; }
			void set_vertex_range(size_t s, size_t e) { vertex_r = range{ static_cast<UINT>(s), static_cast<UINT>(e) }; }
			void set_instance_range(size_t s, size_t e) { instance_r = range{ static_cast<UINT>(s), static_cast<UINT>(e) }; }
			bool apply(Implement::context_ptr& cp);
			bool draw(Implement::context_ptr& cp);
		};*/
		/*
		struct material_state
		{
			Implement::resource_ptr rp;

			D3D11_DEPTH_STENCIL_DESC DDSD{ false, D3D11_DEPTH_WRITE_MASK_ALL , D3D11_COMPARISON_LESS , false, 0, 0, D3D11_DEPTH_STENCILOP_DESC{ D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS } };
			Implement::depth_stencil_state_ptr dsp;
			bool depth_stencil_update = true;
			UINT stencil_ref = 0;

			D3D11_BLEND_DESC DBD{ 
				false, 
				false, 
				{
					D3D11_RENDER_TARGET_BLEND_DESC
					{
						true, 
						D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA , 
						D3D11_BLEND_OP_ADD, 
						D3D11_BLEND_ONE , D3D11_BLEND_ZERO , 
						D3D11_BLEND_OP_ADD , 
						D3D11_COLOR_WRITE_ENABLE_ALL
					}
				} 
			};
			Implement::blend_state_ptr bsp;
			bool blend_update = true;
			std::array<float, 4> blend_factor = { 0.0, 0.0, 0.0, 0.0 };
			UINT sample_mask = 0xffffffff;
			
			void bind(Implement::resource_ptr& rp);
			bool apply(Implement::context_ptr& cp);
			bool update();
			material_state& set_blend(const D3D11_RENDER_TARGET_BLEND_DESC& des, size_t index = 0)
			{
				blend_update = true;
				DBD.RenderTarget[index] = des;
				return *this;
			}
			material_state& set_blend_factor(float r, float g, float b, float a)
			{
				blend_factor[0] = r;
				blend_factor[1] = g;
				blend_factor[2] = b;
				blend_factor[3] = a;
				return *this;
			}
		};*/

		/*
		struct material
		{
			Implement::resource_ptr rp;
			Implement::pshader_ptr pshader;
		public:
			void bind(Implement::resource_ptr& r);
			bool load_p( std::u16string path)
			{
				if (rp == nullptr) return false;
				pshader = nullptr;
				binary tem;
				if (tem.load_file(path))
					return rp->CreatePixelShader(tem, tem.size(), nullptr, &pshader) == S_OK;
				return false;
			}
			bool apply(Implement::context_ptr& cp)
			{
				if (!is_resource_available_for_context(rp, cp)) return false;
				cp->PSSetShader(pshader, nullptr, 0);
				return true;
			}
		};
		*/

		/*
		struct constant_value
		{
			std::vector<ID3D11Buffer*> buffer;
			Implement::resource_ptr rp;

			bool create_implement(void* data, size_t buffer_size, D3D11_USAGE usage, UINT cpu_flag);


		public:

			size_t size() const { return buffer.size(); }

			template<typename T>
			bool create(T* data)
			{

			}


			void clear() noexcept
			{
				std::for_each(buffer.begin(), buffer.end(), [](ID3D11Buffer* B) {if(B!=nullptr) B->Release(); });
				buffer.clear();
			}
			~constant_value()
			{
				clear();
			}
		};
		*/

		/*
		struct sample_state
		{
			std::array<ID3D11SamplerState*, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT> array;
			bool apply_ps(Implement::context_ptr& cp)
			{
				if(cp == nullptr) return false;
				cp->PSSetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT£¬ array.data())
			}
		};
		*/
	}
}
