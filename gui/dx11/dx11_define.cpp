#include "dx11_define.h"
#include "../../frame/define.h"
#include <fstream>
#undef max
namespace PO
{
	namespace Dx11
	{

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

		HRESULT vertex_pool::create_vertex(size_t solt, void* data, size_t data_size, size_t vertex_size, size_t layout_count, void(*func)(D3D11_INPUT_ELEMENT_DESC*, size_t solt))
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

		bool pipe_line::draw(Implement::context_ptr& cp, /*const_buffer& cb,*/ vertex_pool& vp, size_t vertex_num)
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
}