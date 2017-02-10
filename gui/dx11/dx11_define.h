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
namespace PO
{
	namespace Dx11
	{

		using float2 = DirectX::XMFLOAT2;
		using float3 = DirectX::XMFLOAT3;
		using float4 = DirectX::XMFLOAT4;
		using matrix4 = DirectX::XMMATRIX;

		namespace Implement
		{
			using resource = CComPtr<ID3D11Device>;
			using context = CComPtr<ID3D11DeviceContext>;
			using chain = CComPtr<IDXGISwapChain>;
			using buffer = CComPtr<ID3D11Buffer>;
			using vshader = CComPtr<ID3D11VertexShader>;
			using pshader = CComPtr<ID3D11PixelShader>;
			using layout = CComPtr<ID3D11InputLayout>;
			class data
			{
				struct scri
				{
					size_t size = 0;
					size_t s_ref = 0;
				};
				char* d;
			public:
				data() : d(nullptr) {}
				data(size_t da) : d(new char[da + sizeof(scri)])
				{
					auto scr = reinterpret_cast<scri*>(d);
					scr->size = da;
					scr->s_ref = 1;
				}
				data(const data& da) :d(da.d)
				{
					reinterpret_cast<scri*>(d)->s_ref += 1;
				}
				data(data&& da) :d(da.d)
				{
					da.d = nullptr;
				}
				data& operator=(const data& da)
				{
					data tem(da);
					reset();
					d = tem.d;
					tem.d = nullptr;
				}
				data& operator=(data&& da)
				{
					data tem(std::move(da));
					reset();
					d = tem.d;
					tem.d = nullptr;
				}
				void reset()
				{
					if (d != nullptr)
					{
						auto scr = reinterpret_cast<scri*>(d);
						scr->size -= 1;
						if (scr->size == 0)
						{
							delete scr;
						}
						d = nullptr;
					}
				}
				~data()
				{
					reset();
				}
				operator void*() const { return static_cast<void*>(d + sizeof(scri)); }
				operator char*() const { return (d + sizeof(scri)); }
				size_t size() const 
				{
					return (d != nullptr) ? reinterpret_cast<scri*>(d)->size : 0;
				}
			};
		}

		class vertex_const
		{
			Implement::buffer data;
			HRESULT create_buffer(const Implement::resource& re, void* da, size_t t);
		public:
			operator bool() const { return data != nullptr; }
			vertex_const(const vertex_const&) = default;
			vertex_const& operator=(const vertex_const&) = default;
			vertex_const() {}
			template<typename D>
			vertex_const(const Implement::resource& re, D* da, size_t s) 
			{
				create(re, da, s);
			}
			template<typename D> HRESULT create(const Implement::resource& re, D* da, size_t s)
			{
				data = nullptr;
				return create_buffer(re, static_cast<void*>(da), sizeof(D) * s);
			}
		};

		class shader_v
		{
			Implement::data da;
			Implement::vshader shader;
			
		public:

			shader_v(const Implement::resource& re, Implement::data da)
			{
				load_binary(re, da);
			}
			shader_v() {}
			shader_v(const shader_v&) = default;
			shader_v(shader_v&&) = default;
			shader_v& operator=(shader_v&&) = default;
			shader_v& operator=(const shader_v&) = default;

			operator bool() const { return shader != nullptr; }

			HRESULT load_binary(const Implement::resource& re, Implement::data da);
		};
	}
}
