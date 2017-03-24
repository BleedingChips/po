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
namespace PO
{
	namespace Dx11
	{
		using float2 = DirectX::XMFLOAT2;
		using float3 = DirectX::XMFLOAT3;
		using float4 = DirectX::XMFLOAT4;
		using matrix4 = DirectX::XMFLOAT4X4;

		template<typename store> struct store2
		{
			store x;
			store y;
		};

		template<typename store> struct store3
		{
			store x;
			store y;
			store z;
		};
		template<typename store> struct store4
		{
			store x;
			store y;
			store z;
			store w;
		};

		using int2 = store2<int32_t>;
		using int3 = store3<int32_t>;
		using int4 = store4<int32_t>;

		using uint2 = store2<uint32_t>;
		using uint3 = store3<uint32_t>;
		using uint4 = store4<uint32_t>;
	}

	namespace DXGI
	{
		template<typename T> struct data_format;
		template<> struct data_format<Dx11::float2>
		{
			static constexpr DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT;
		};
		template<> struct data_format<Dx11::float3>
		{
			static constexpr DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT;
		};
		template<> struct data_format<Dx11::float4>
		{
			static constexpr DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT;
		};
		template<> struct data_format<Dx11::int2>
		{
			static constexpr DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_R32G32_SINT;
		};
		template<> struct data_format<Dx11::int3>
		{
			static constexpr DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32_SINT;
		};
	}

	namespace Dx11
	{
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
			using render_view_ptr = CComPtr<ID3D11RenderTargetView>;
			using depth_stencil_view_ptr = CComPtr<ID3D11DepthStencilView>;
			using resource_view_ptr = CComPtr<ID3D11ShaderResourceView>;
			using sample_state_ptr = CComPtr<ID3D11SamplerState>;
			using cshader_ptr = CComPtr<ID3D11ComputeShader>;
			using gshader_ptr = CComPtr<ID3D11GeometryShader>;

			using raterizer_state_ptr = CComPtr<ID3D11RasterizerState>;
			using blend_state_ptr = CComPtr<ID3D11BlendState>;
			using depth_stencil_state_ptr = CComPtr<ID3D11DepthStencilState>;

		}

		namespace Purpose
		{
			struct purpose
			{
				D3D11_USAGE usage;
				UINT cpu_flag;
				UINT additional_bind;
			};
			extern purpose input;
			extern purpose output;
			extern purpose constant;
			extern purpose transfer;
		}

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
			template<typename T> auto write(Implement::context_ptr& cp, T& t) -> Tool::optional<decltype(t(static_cast<void*>(nullptr), static_cast<size_t>(0)))>
			{
				if (ptr != nullptr)
				{
					D3D11_BUFFER_DESC DBD;
					ptr->GetDesc(&DBD);
					if (
						(DBD.Usage == D3D11_USAGE::D3D11_USAGE_DYNAMIC || DBD.Usage == D3D11_USAGE::D3D11_USAGE_STAGING)
						&& (DBD.CPUAccessFlags & D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE == D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE)
						)
					{
						D3D11_MAPPED_SUBRESOURCE DMS;
						if (cp->Map(ptr, 0, D3D11_MAP_WRITE, &DMS) != S_OK)
							return{};
						Tool::at_scope_exit tem([&,this]()
						{
							cp->Unmap(ptr, 0);
						});
						return{ t(DMS.pData, DBD.ByteWidth) };
					}
				}
				return{};
			}
			operator bool() const { return ptr != nullptr; }
		};

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
		};

		Implement::resource_view_ptr cast_resource(Implement::resource_ptr& rp, const Implement::texture2D_ptr& pt);

		Implement::resource_view_ptr cast_resource(Implement::resource_ptr& rp, const Implement::texture1D_ptr& pt);

		Implement::resource_view_ptr cast_resource(Implement::resource_ptr& rp, const Implement::texture3D_ptr& pt);

		Implement::texture2D_ptr create_render_target(Implement::resource_ptr& rp, size_t w, size_t e, DXGI_FORMAT DF);
		Implement::render_view_ptr cast_render_view(Implement::resource_ptr& rp, Implement::texture2D_ptr tp);
		//Implement::texture2D_ptr create_depth_s(Implement::resource_ptr& rp, size_t w, size_t e, DXGI_FORMAT DF);

		

		DXGI_FORMAT adjust_texture_format(DXGI_FORMAT DF);

		inline bool is_resource_available_for_context(Implement::resource_ptr rp, Implement::context_ptr cp)
		{
			if (rp == nullptr || cp == nullptr) return false;
			Implement::resource_ptr r;
			cp->GetDevice(&r);
			return rp.IsEqualObject(r);
		}

		struct pixel_state
		{
			D3D11_RASTERIZER_DESC DRD = { D3D11_FILL_SOLID, D3D11_CULL_NONE, false, 0, 0.0f, 0.0f, true, false, false, false };
			Implement::resource_ptr rp;
			Implement::raterizer_state_ptr rsp;
			bool need_update = true;

			void bind(Implement::resource_ptr& r)
			{
				rsp = nullptr;
				rp = r;
				need_update = true;
			}
			bool apply(Implement::context_ptr& cp);
			bool update();
		};

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
			void set_index_range(UINT s, UINT e) { index_r = range{ s, e }; }
			void set_vertex_range(UINT s, UINT e) { vertex_r = range{ s, e }; }
			void set_instance_range(UINT s, UINT e) { instance_r = range{ s, e }; }
			bool apply(Implement::context_ptr& cp);
			bool draw(Implement::context_ptr& cp);
		};

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
		};

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
