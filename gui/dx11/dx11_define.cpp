#include "dx11_define.h"
#include "../../frame/define.h"
#include <fstream>
#undef max
namespace PO
{
	namespace Dx11
	{

		namespace Purpose
		{
			purpose input{ D3D11_USAGE::D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE, 0 };
			purpose output{ D3D11_USAGE::D3D11_USAGE_DEFAULT,  0, D3D11_BIND_FLAG::D3D11_BIND_STREAM_OUTPUT | D3D11_BIND_FLAG::D3D11_BIND_UNORDERED_ACCESS };
			purpose constant{ D3D11_USAGE::D3D11_USAGE_IMMUTABLE, 0, 0 };
			purpose transfer{ D3D11_USAGE::D3D11_USAGE_STAGING, UINT(D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_READ) | (UINT)(D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE),  0 };
		}

		bool buffer::create(Implement::resource_ptr& rp, D3D11_USAGE usage, UINT cpu_flag, UINT bind_flag, const void* data, size_t data_size, UINT misc_flag, size_t StructureByteStride)
		{
			//++vision;
			ptr = nullptr;
			D3D11_BUFFER_DESC DBD
			{
				static_cast<UINT>(data_size),
				usage,
				bind_flag,
				cpu_flag,
				misc_flag,
				static_cast<UINT>(StructureByteStride)
			};
			D3D11_SUBRESOURCE_DATA DSD{ data, 0, 0 };
			return rp->CreateBuffer(&DBD, ((data == nullptr) ? nullptr : &DSD), &ptr) == S_OK;
		}

		bool create_index_vertex_buffer(Implement::resource_ptr& rp, Purpose::purpose bp,
			const void* data, size_t buffer_size,
			index& ind, vertex& ver,
			size_t index_offset, DXGI_FORMAT format,
			size_t vertex_offset, size_t element_size, std::vector<D3D11_INPUT_ELEMENT_DESC> layout
		)
		{
			buffer b;
			ind.ptr = nullptr;
			ver.ptr = nullptr;
			if (b.create(
				rp, bp.usage, bp.cpu_flag, bp.additional_bind | D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER,
				data, buffer_size, 0, 0
			))
			{
				static_cast<buffer&>(ind) = b;
				static_cast<buffer&>(ver) = b;
				ind.offset = index_offset;
				ind.format = format;
				ver.desc = std::move(layout);
				ver.element_size = element_size;
				ver.offset = vertex_offset;
				return true;
			}
			return false;
		}

		bool pixel_creater::update_layout()
		{
			static std::vector<D3D11_INPUT_ELEMENT_DESC> des_buffer;
			static std::mutex buffer_mutex;
			if (rp == nullptr) return false;
			if (update_flag)
			{
				update_flag = false;
				layout = nullptr;
				std::lock_guard<std::mutex> lg(buffer_mutex);
				des_buffer.clear();
				for (size_t i = 0; i < D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT; ++i)
				{
					auto ite = des_buffer.insert(des_buffer.end(), vec[i].desc.begin(), vec[i].desc.end());
					for (; ite != des_buffer.end(); ++ite)
						ite->InputSlot = static_cast<UINT>(i);
				}
				update_flag = (rp->CreateInputLayout(des_buffer.data(), static_cast<UINT>(des_buffer.size()), vshader_binary, static_cast<UINT>(vshader_binary.size()), &layout) != S_OK);
			}
			return rp != nullptr;
		}

		DXGI_FORMAT adjust_texture_format(DXGI_FORMAT DF)
		{
			switch (DF)
			{
			case DXGI_FORMAT_R24G8_TYPELESS:
				return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			case DXGI_FORMAT_R32_TYPELESS:
				return DXGI_FORMAT_R32_FLOAT;
			}
			return DF;
		}

		bool pixel_creater::bind(Implement::resource_ptr& r)
		{
			update_flag = true;
			for (size_t i = 0; i < D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT; ++i)
				vec[i].clear();
			ind.clear();
			layout = nullptr;
			vshader = nullptr;
			gshader = nullptr;
			rp = r;
			if (rp != nullptr)
			{
				if (vshader_binary)
					rp->CreateVertexShader(vshader_binary, vshader_binary.size(), nullptr, &vshader);
				return true;
			}
			return false;
		}

		bool pixel_creater::load_vshader(std::u16string path)
		{
			if (rp == nullptr) return false;
			vshader = nullptr;
			if (vshader_binary.load_file(path))
				if (rp->CreateVertexShader(vshader_binary, vshader_binary.size(), nullptr, &vshader) == S_OK)
				{
					update_flag = true;
					return true;
				}
			return false;
		}

		bool pixel_creater::load_gshader(std::u16string path)
		{
			if (rp == nullptr) return false;
			gshader = nullptr;
			binary tem;
			if (tem.load_file(path))
				return rp->CreateGeometryShader(tem, tem.size(), nullptr, &gshader) == S_OK;
			return false;
		}

		bool pixel_creater::apply(Implement::context_ptr& cp)
		{
			if (!is_resource_available_for_context(rp,cp) || !update_layout()) return false;
			cp->IASetInputLayout(layout);
			cp->IASetPrimitiveTopology(primitive);
			cp->VSSetShader(vshader, nullptr, 0);
			ID3D11Buffer* array[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
			UINT offset[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
			UINT element[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
			for (size_t i = 0; i < D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT; ++i)
			{
				if (vec[i])
				{
					array[i] = vec[i].ptr;
					offset[i] = static_cast<UINT>(vec[i].offset);
					element[i] = static_cast<UINT>(vec[i].element_size);
				}
				else {
					array[i] = nullptr;
					offset[i] = 0;
					element[i] = 0;
				}
			}
			cp->IASetVertexBuffers(0, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT, array, element, offset);
			cp->IASetIndexBuffer(ind.ptr, ind.format, static_cast<UINT>(ind.offset));
			return true;
		}

		bool pixel_creater::draw(Implement::context_ptr& cp)
		{
			if (!is_resource_available_for_context(rp, cp)) return false;
			if (ind)
			{
				if (instance_r.count == 0)
					cp->DrawIndexed(index_r.count, index_r.start, vertex_r.start);
				else
					cp->DrawIndexedInstanced(index_r.count, instance_r.count, index_r.start, vertex_r.start, instance_r.start);
			}
			else {
				if (instance_r.count != 0)
					cp->DrawInstanced(vertex_r.count, instance_r.count, vertex_r.start, instance_r.start);
				else
					cp->Draw(vertex_r.count, vertex_r.start);
			}
			return true;
		}

		Implement::resource_view_ptr cast_resource(Implement::resource_ptr& rp, const Implement::texture2D_ptr& pt)
		{
			Implement::resource_view_ptr ptr;
			if (pt != nullptr)
			{
				D3D11_TEXTURE2D_DESC tem;
				pt->GetDesc(&tem);
				D3D11_SHADER_RESOURCE_VIEW_DESC SRVD{ tem.Format };
				if ((tem.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE) == D3D11_RESOURCE_MISC_TEXTURECUBE)
				{
					SRVD.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
					SRVD.TextureCube = D3D11_TEXCUBE_SRV{ 0 ,  tem.MipLevels };
				}
				else if (tem.ArraySize > 1)
				{
					SRVD.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
					SRVD.Texture2DArray = D3D11_TEX2D_ARRAY_SRV{ 0,  tem.MipLevels , 0, tem.ArraySize };
				}
				else {
					SRVD.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
					SRVD.Texture2D = D3D11_TEX2D_SRV{ 0,  tem.MipLevels };
				}
				HRESULT re = rp->CreateShaderResourceView(pt, &SRVD, &ptr);
				volatile int i = 0;
			}
			return ptr;
		}

		Implement::resource_view_ptr cast_resource(Implement::resource_ptr& rp, const Implement::texture1D_ptr& pt)
		{
			Implement::resource_view_ptr ptr;
			if (pt != nullptr)
			{
				D3D11_TEXTURE1D_DESC tem;
				pt->GetDesc(&tem);
				D3D11_SHADER_RESOURCE_VIEW_DESC SRVD{ tem.Format };
				if (tem.ArraySize > 1)
				{
					SRVD.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1DARRAY;
					SRVD.Texture1DArray = D3D11_TEX1D_ARRAY_SRV{ 0,  tem.MipLevels , 0, tem.ArraySize };
				}
				else {
					SRVD.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
					SRVD.Texture1D = D3D11_TEX1D_SRV{ 0,  tem.MipLevels };
				}
				rp->CreateShaderResourceView(pt, &SRVD, &ptr);
			}
			return ptr;
		}

		Implement::resource_view_ptr cast_resource(Implement::resource_ptr& rp, const Implement::texture3D_ptr& pt)
		{
			Implement::resource_view_ptr ptr;
			if (pt != nullptr)
			{
				D3D11_TEXTURE3D_DESC tem;
				pt->GetDesc(&tem);
				D3D11_SHADER_RESOURCE_VIEW_DESC SRVD{ tem.Format };
				SRVD.ViewDimension = D3D11_SRV_DIMENSION::D3D10_1_SRV_DIMENSION_TEXTURE3D;
				SRVD.Texture3D = D3D11_TEX3D_SRV{ 0, tem.MipLevels };
				rp->CreateShaderResourceView(pt, &SRVD, &ptr);
			}
			return ptr;
		}

		void material::bind(Implement::resource_ptr& r)
		{
			pshader = nullptr;
			rp = r;
		}
		
		bool pixel_state::apply(Implement::context_ptr& cp)
		{
			if (!is_resource_available_for_context(rp, cp) || !update()) return false;
			cp->RSSetState(rsp);
			return true;
		}

		bool pixel_state::update()
		{
			if (need_update)
			{
				rsp = nullptr;
				if (rp->CreateRasterizerState(&DRD, &rsp) != S_OK) return false;
				need_update = false;
			}
			return true;
		}

		void material_state::bind(Implement::resource_ptr& r)
		{
			dsp = nullptr;
			depth_stencil_update = true;
			bsp = nullptr;
			blend_update = true;
			rp = r;
		}

		bool material_state::update()
		{
			if (rp == nullptr) return false;
			if (depth_stencil_update)
			{
				dsp = nullptr;
				if (rp->CreateDepthStencilState(&DDSD, &dsp) != S_OK) return false;
				depth_stencil_update = false;
			}
			if (blend_update)
			{
				bsp = nullptr;
				HRESULT re = rp->CreateBlendState(&DBD, &bsp);
				if (re != S_OK) return false;
				blend_update = false;
			}
			return true;
		}

		bool material_state::apply(Implement::context_ptr& cp)
		{
			if (!is_resource_available_for_context(rp, cp) || !update()) return false;
			cp->OMSetBlendState(bsp, blend_factor.data(), sample_mask);
			cp->OMSetDepthStencilState(dsp, stencil_ref);
			return true;
		}

		Implement::texture2D_ptr create_render_target(Implement::resource_ptr& rp, size_t w, size_t h, DXGI_FORMAT DF)
		{
			Implement::texture2D_ptr ptr;
			D3D11_TEXTURE2D_DESC DTD
			{
				static_cast<UINT>(w),
				static_cast<UINT>(h),
				1,
				1,
				DF,
				DXGI_SAMPLE_DESC{1, 0},
				D3D11_USAGE_DEFAULT,
				D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
				0,
				0
			};
			rp->CreateTexture2D(&DTD, nullptr, &ptr);
			return ptr;
		}

		Implement::render_view_ptr cast_render_view(Implement::resource_ptr& rp, Implement::texture2D_ptr tp)
		{
			Implement::render_view_ptr ptr;
			if (rp == nullptr || tp == nullptr) return ptr;
			D3D11_TEXTURE2D_DESC tem;
			tp->GetDesc(&tem);
			if (
				((tem.BindFlags & D3D11_BIND_RENDER_TARGET) != D3D11_BIND_RENDER_TARGET) ||
				((tem.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE) == D3D11_RESOURCE_MISC_TEXTURECUBE)
				) return ptr;
			D3D11_RENDER_TARGET_VIEW_DESC DRTVD{ tem.Format };
			if (tem.ArraySize > 1)
			{
				DRTVD.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
				DRTVD.Texture2DArray = D3D11_TEX2D_ARRAY_RTV{0, 0, tem.ArraySize };
			}
			else {
				DRTVD.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
				DRTVD.Texture2D = D3D11_TEX2D_RTV{ 0 };
			}
			rp->CreateRenderTargetView(tp, &DRTVD, &ptr);
			return ptr;
		}

		bool constant_value::create_implement(void* data, size_t buffer_size, D3D11_USAGE usage, UINT cpu_flag)
		{
			if (rp == nullptr) return false;
			ID3D11Buffer* ptr = nullptr;
			D3D11_BUFFER_DESC DBD
			{
				static_cast<UINT>(buffer_size),
				usage,
				D3D11_BIND_CONSTANT_BUFFER,
				cpu_flag,
				0,
				static_cast<UINT>(0)
			};
			D3D11_SUBRESOURCE_DATA DSD{ data, 0, 0 };
			if (rp->CreateBuffer(&DBD, ((data == nullptr) ? nullptr : &DSD), &ptr) == S_OK)
			{
				buffer.push_back(ptr);
				return true;
			}
			return false;
		}

	}

























	/*
	namespace Dx11
	{
		
		namespace Implement
		{
			bool vertex_data::create_buffer_implement(
				Implement::resource_ptr& rp, D3D11_USAGE usage, UINT cpu_flag, UINT additional_bind,
				const void* data, size_t vertex_size, size_t vertex_count, size_t layout_count, void(*scr)(D3D11_INPUT_ELEMENT_DESC*, size_t solt)
			)
			{
				D3D11_BUFFER_DESC DBD
				{
					static_cast<UINT>(vertex_size * vertex_count),
					usage,
					D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER | additional_bind,
					cpu_flag,
					0,
					0
				};
				D3D11_SUBRESOURCE_DATA DSD{ data, 0, 0 };
				ptr = nullptr;
				HRESULT re = rp->CreateBuffer(&DBD, &DSD, &ptr);
				if (re == S_OK)
				{
					input_layout_count = layout_count;
					scription = scr;
					vision++;
					return true;
				}
				return false;
			}



			bool geometry_store::create_buffer_implement
			(
				Implement::resource_ptr& rp, D3D11_USAGE usage, UINT cpu_flag, UINT additional_bind,
				const void* data, size_t index_offset, size_t index_size, DXGI_FORMAT index_format,
				size_t vertex_offset, size_t vertex_size, size_t layout_count, void(*scription)(D3D11_INPUT_ELEMENT_DESC*, size_t solt)
			)
			{
				ptr = nullptr;
				D3D11_BUFFER_DESC DBD
				{
					static_cast<UINT>(vertex_size),
					usage,
					((index_size == 0) ? 0 : D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER ) |
					((vertex_size == 0) ? 0 : D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER ) |
					additional_bind,
					cpu_flag,
					0,
					0
				};
				D3D11_SUBRESOURCE_DATA DSD{ data, 0, 0 };
				HRESULT re = rp->CreateBuffer(&DBD, &DSD, &ptr);
				if (re == S_OK)
				{
					//input_layout_count = layout_count;
					//scription = scr;
					//vision++;
					return true;
				}
				return false;
			}

			bool geometry_store::create_buffer_implement
			(
				Implement::resource_ptr& rp, D3D11_USAGE usage, UINT cpu_flag, UINT additional_bind,
				const void* index_data, size_t index_size, DXGI_FORMAT index_format,
				const void* vertex_data, size_t vertex_size, size_t layout_count, void(*scription)(D3D11_INPUT_ELEMENT_DESC*, size_t solt)
			)
			{
				static std::vector<char> temporary_buffer;
				static std::mutex temporary_buffer_mutex;
				std::lock_guard<std::mutex> lg(temporary_buffer_mutex);
				temporary_buffer.resize(index_size + vertex_size, 0);
				std::memcpy(temporary_buffer.data(), index_data, index_size);
				std::memcpy(temporary_buffer.data() + index_size, vertex_data, vertex_size);
				ptr = nullptr;
				D3D11_BUFFER_DESC DBD
				{
					static_cast<UINT>(temporary_buffer.size()),
					usage,
					((index_size == 0) ? 0 : D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER) |
					((vertex_size == 0) ? 0 : D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER) |
					additional_bind,
					cpu_flag,
					0,
					0
				};
				D3D11_SUBRESOURCE_DATA DSD{ temporary_buffer.data(), 0, 0 };
				HRESULT re = rp->CreateBuffer(&DBD, &DSD, &ptr);
				if (re == S_OK)
				{
					//input_layout_count = layout_count;
					//scription = scr;
					//vision++;
					return true;
				}
				return false;
			}

			bool instance_store::create_buffer_implement(
				resource_ptr& rp, D3D11_USAGE usage, UINT cpu_flag, UINT additional_bind,
				const void* instance_data, size_t instance_size, size_t layout_count, void(*scription)(D3D11_INPUT_ELEMENT_DESC*, size_t solt)
			)
			{
				ptr = nullptr;
				D3D11_BUFFER_DESC DBD
				{
					static_cast<UINT>(instance_size),
					usage,
					D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER | additional_bind,
					cpu_flag,
					0,
					0
				};
				D3D11_SUBRESOURCE_DATA DSD{ instance_data, 0, 0 };
				HRESULT re = rp->CreateBuffer(&DBD, &DSD, &ptr);
				if (re == S_OK)
				{
					//input_layout_count = layout_count;
					//scription = scr;
					//vision++;
					return true;
				}
				return false;
			}

		}

















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

		HRESULT vertex_pool::create_vertex(size_t solt, void* data, size_t type_size, size_t data_size, size_t vertex_size, size_t layout_count, void(*func)(D3D11_INPUT_ELEMENT_DESC*, size_t solt))
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

		bool pipe_line::draw(Implement::context_ptr& cp, const_buffer& cb, vertex_pool& vp, size_t vertex_num)
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
	*/
}