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
#include "../../tool/tmp.h"
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
			using raterizer_state = CComPtr<ID3D11RasterizerState>;
			using texture2D = CComPtr<ID3D11Texture2D>;
			using resource_view = CComPtr<ID3D11ShaderResourceView>;
			using sample_state = CComPtr<ID3D11SamplerState>;

			class data
			{
				std::shared_ptr<void> steam;
				static void deleter(void* p) { delete [] p; }
			public:

				class weak_ref
				{
					std::weak_ptr<void> steam;
				public:
					operator bool() const { return !steam.expired(); }
					weak_ref() {}
					weak_ref(const weak_ref&) = default;
					weak_ref(weak_ref&&) = default;
					weak_ref(const data& d) :steam(d.steam) {}
					weak_ref& operator= (const weak_ref&) = default;
					weak_ref& operator= (weak_ref&&) = default;
					weak_ref& operator= (const data& d)
					{
						steam = d.steam;
						return *this;
					}
					auto lock() const { return steam.lock(); }
				};

				data() {}
				data(size_t da) : steam(new char[da + sizeof(size_t)], deleter)
				{
					auto scr = reinterpret_cast<size_t*>(steam.get());
					*scr = da;
				}
				data(const data& da) = default;
				data(data&& da) = default;
				data& operator=(const data& da) = default;
				data& operator=(data&& da) = default;
				void reset()
				{
					steam.reset();
				}
				operator bool() const { return static_cast<bool>(steam); }
				~data() {}
				operator void*() const { return static_cast<void*>(static_cast<char*>(steam.get()) + sizeof(size_t)); }
				operator char*() const { return static_cast<char*>(steam.get()) + sizeof(size_t); }
				size_t size() const 
				{
					return steam ? *reinterpret_cast<size_t*>(steam.get()) : 0;
				}
			};
		}
		
		class vertex_buffer
		{
		protected:
			Implement::buffer data;
			size_t ver_size = 0;
			size_t ver_numb = 0;
			operator ID3D11Buffer *() { return data; }
			friend class pipe_line;
		public:
			operator bool() const {return data!=nullptr; }
			vertex_buffer() {}
			vertex_buffer(vertex_buffer&& vb) : data(std::move(vb.data)), ver_size(vb.ver_size), ver_numb(vb.ver_numb) 
			{
				vb.ver_size = 0;
				vb.ver_numb = 0;
			}
			vertex_buffer(Implement::buffer da, size_t vs, size_t vn) : data(da), ver_size(vs), ver_numb(vn) {}

			size_t size() const { return ver_size; }
			size_t num() const { return ver_numb; }
		};

		namespace Property
		{
			struct constant
			{
				static constexpr D3D11_USAGE USAGE = D3D11_USAGE::D3D11_USAGE_IMMUTABLE;
				static constexpr UINT CPU_ACCESS = 0;
			};

			struct input
			{
				static constexpr D3D11_USAGE USAGE = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
				static constexpr UINT CPU_ACCESS = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE;
			};

			struct readable
			{
				static constexpr D3D11_USAGE USAGE = D3D11_USAGE::D3D11_USAGE_STAGING;
				static constexpr UINT CPU_ACCESS = static_cast<D3D11_CPU_ACCESS_FLAG>(D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_READ);
			};
		}

		namespace Component
		{
			template<typename T> struct is_component : std::false_type {};

			struct shader
			{
				
			};

			struct index
			{

			};
		}

		struct buffer_data
		{
			Implement::buffer ptr;
			size_t size;
		};

		template<typename pro, typename bind_type> class buffer : public buffer_data
		{	
			Implement::buffer ptr;
			size_t size;
			typename bind_type::scription scri;
		public:
			buffer() {}
			operator bool() const { return ptr != nullptr; }
			template<typename ...AK>
			HRESULT create_buffer(Implement::resource& re, size_t *,AK&& ... ak)
			{
				re->CreateBuffer()
			}
		};

		namespace Implement
		{
			template<typename T> struct data_format;
			template<> struct data_format<float2>
			{
				static constexpr DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT;
			};
			template<> struct data_format<float3>
			{
				static constexpr DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT;
			};
			template<> struct data_format<float4>
			{
				static constexpr DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT;
			};
		}

		


		

		template<typename Property_t> class vertex : public vertex_buffer
		{
			void(*layout_creater)(std::vector<D3D11_INPUT_ELEMENT_DESC>& v, size_t slot);
		public:
			using vertex_buffer::vertex_buffer;
			template<typename D, typename layout> HRESULT create(const Implement::resource& re, D* da, size_t s, layout)
			{
				data.reset();
				static_assert(std::is_pod<D>::value, "vertex creating only receive pod struct");
				D3D11_BUFFER_DESC tem_b{
					sizeof(D) * s,
					Property_t::USAGE,
					static_cast<UINT>(D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER),
					Property_t::CPU_ACCESS,
					static_cast<UINT>(0x0L),
					0
				};
				D3D11_SUBRESOURCE_DATA tem_s{ da, 0, 0 };
				HRESULT re = re->CreateBuffer(&tem_b, &tem_s, &data);
				if (SUCCEEDED(re))
				{
					layout_creater = layout::create_input_element_desc;
					ver_size = sizeof(da);
					ver_numb = s;
				}
				return re;
			}
		};

		namespace Components
		{
			struct vertex_s
			{
				using store_type = Implement::vshader;
				static HRESULT LoadShader(store_type& s,Implement::resource& re, Implement::data& da)
				{
					return re->CreateVertexShader(da, da.size(), nullptr, &s);
				}
			};

			struct pixel_s
			{
				using store_type = Implement::pshader;
				static HRESULT LoadShader(store_type& s, Implement::resource& re, Implement::data& da)
				{
					return re->CreatePixelShader(da, da.size(), nullptr, &s);
				}
			};
		}

		template<typename shader_t>
		class shader
		{
			Implement::data da;
			typename shader_t::store_type sh;
			friend class pipe_line;
		public:
			shader(const shader&) = default;
			shader(shader&& s) = default;
			shader() = default;
			shader& operator=(const shader&) = default;
			shader& operator=(shader&&) = default;

			shader(const Implement::resource& re, Implement::data da)
			{
				load_binary(re, da);
			}

			operator bool() const { return shader != nullptr; }

			HRESULT load_binary(const Implement::resource& re, const Implement::data& d)
			{
				sh = nullptr;
				da = d;
				return shader_t::load_binary(sh, re, da);
			}
		};


		class pipe_line
		{
			using vs_t = shader<Components::vertex_s>;
			using ps_t = shader<Components::pixel_s>;

			vs_t vs;
			ps_t ps;

			static thread_local std::vector<D3D11_INPUT_ELEMENT_DESC> input_element_buffer;

		public:

			pipe_line() {}

			pipe_line(pipe_line&&) = default;
			pipe_line(const pipe_line&) = default;
			
			pipe_line& operator= (pipe_line&& pl) = default;
			pipe_line& operator= (const pipe_line& pl) = default;

			vs_t& v_shader() { return vs; }
			ps_t& p_shader() { return ps; }

			/*
			void draw(Implement::context& c, ID3D11RenderTargetView* pView, ID3D11DepthStencilView* pDepthView, const vertex_layout& vl, vertex_buffer& vb)
			{
				
				unsigned int stride = static_cast<UINT>(vb.size());
				unsigned int offset = 0;
				HRESULT last = GetLastError();
				
				c->IASetInputLayout(vl.lay);
				ID3D11Buffer * const adress[] = { vb };
				last = GetLastError();
				c->IASetVertexBuffers(0, 1, adress, &stride, &offset);
				last = GetLastError();
				c->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				last = GetLastError();
				c->VSSetShader(v.shader, nullptr, 0);
				last = GetLastError();
				c->PSSetShader(p.shader, nullptr, 0);
				last = GetLastError();
				c->Draw(6, 0);
				last = GetLastError();
			}
			*/
		};

	}
}
