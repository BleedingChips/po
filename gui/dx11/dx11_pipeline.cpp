#include "dx11_pipeline.h"
namespace PO
{
	namespace Dx11
	{
		namespace Implement
		{

			static std::array<ID3D11Buffer*, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT> input_assember_context_nullptr_array = {};
			static std::array<UINT, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT> input_assember_context_zero_array = {};

			/*****  input_assember_context_t   ******************************************************************************************/
			void input_assember_context_t::bind(context_ptr& cp, const input_assember_d& id)
			{
				cp->IASetInputLayout(id.layout);
				cp->IASetPrimitiveTopology(id.primitive);
				if (id.vertex_array.size() < max_buffer_solt)
					cp->IASetVertexBuffers(static_cast<UINT>(id.vertex_array.size() - 1), static_cast<UINT>(max_buffer_solt - id.vertex_array.size()), input_assember_context_nullptr_array.data(),
						input_assember_context_zero_array.data(), input_assember_context_zero_array.data());
				max_buffer_solt = id.vertex_array.size();
				cp->IASetVertexBuffers(0, static_cast<UINT>(id.vertex_array.size()), id.vertex_array.data(), id.element_array.data(), id.offset_array.data());
				cp->IASetIndexBuffer(id.index_ptr, id.format, id.offset);
			}

			void input_assember_context_t::unbind(context_ptr& cp)
			{
				cp->IASetInputLayout(nullptr);
				cp->IASetVertexBuffers(0, static_cast<UINT>(max_buffer_solt), input_assember_context_nullptr_array.data(), input_assember_context_zero_array.data(), input_assember_context_zero_array.data());
				cp->IASetIndexBuffer(nullptr, DXGI_FORMAT::DXGI_FORMAT_R16_UINT, 0);
				max_buffer_solt = 0;
			}

			static Tool::scope_lock < std::vector<ID3D11Buffer*> > nullptr_cbuffer;

			/*****  vertex_shader_context_t   ******************************************************************************************/
			void vertex_shader_context_t::bind(context_ptr& cp, const vertex_shader_d& vs)
			{
				cp->VSSetShader(vs.ptr, nullptr, 0);
				if (vs.cbuffer_array.size() < max_cbuffer)
					nullptr_cbuffer.lock([&, this](decltype(nullptr_cbuffer)::type& b) {
					b.insert(b.end(), max_cbuffer - vs.cbuffer_array.size(), nullptr);
					cp->VSSetConstantBuffers(
						static_cast<UINT>(vs.cbuffer_array.size()), static_cast<UINT>(max_cbuffer - vs.cbuffer_array.size()), b.data());
				});
				cp->VSSetConstantBuffers(0, static_cast<UINT>(vs.cbuffer_array.size()), vs.cbuffer_array.data());
				max_cbuffer = vs.cbuffer_array.size();
			}

			void vertex_shader_context_t::unbind(context_ptr& cp)
			{
				cp->VSSetShader(nullptr, nullptr, 0);
				nullptr_cbuffer.lock([&, this](decltype(nullptr_cbuffer)::type& b) {
					b.insert(b.end(), max_cbuffer, nullptr);
					cp->VSSetConstantBuffers(0, static_cast<UINT>(max_cbuffer), b.data());
				});
			}

			/*****  raterizer_context_t   ******************************************************************************************/
			void raterizer_context_t::bind(context_ptr& cp, const raterizer_d& rs)
			{
				cp->RSSetState(rs.ptr);
				cp->RSSetScissorRects(static_cast<UINT>(rs.scissor.size()), rs.scissor.data());
				cp->RSSetViewports(static_cast<UINT>(rs.viewports.size()), rs.viewports.data());
				//cp->PSS
			}

			void raterizer_context_t::unbind(context_ptr& cp)
			{
				cp->RSSetState(nullptr);
				cp->RSSetScissorRects(0, nullptr);
				cp->RSSetViewports(0, nullptr);
			}

			/*****  pixel_shader_context_t   ******************************************************************************************/
			void pixel_shader_context_t::bind(context_ptr& cp, const pixel_shader_d& vs)
			{
				cp->PSSetShader(vs.ptr, nullptr, 0);
				if (vs.cbuffer_array.size() < max_cbuffer)
					nullptr_cbuffer.lock([&, this](decltype(nullptr_cbuffer)::type& b) {
					b.insert(b.end(), max_cbuffer - vs.cbuffer_array.size(), nullptr);
					cp->PSSetConstantBuffers(
						static_cast<UINT>(vs.cbuffer_array.size()), static_cast<UINT>(max_cbuffer - vs.cbuffer_array.size()), b.data());
				});
				cp->PSSetConstantBuffers(0, static_cast<UINT>(vs.cbuffer_array.size()), vs.cbuffer_array.data());
				max_cbuffer = vs.cbuffer_array.size();
				cp->PSSetShaderResources(0, static_cast<UINT>(vs.SRV_array.size()), vs.SRV_array.data());
			}

			void pixel_shader_context_t::unbind(context_ptr& cp)
			{
				cp->PSSetShader(nullptr, nullptr, 0);
				nullptr_cbuffer.lock([&, this](decltype(nullptr_cbuffer)::type& b) {
					b.insert(b.end(), max_cbuffer, nullptr);
					cp->PSSetConstantBuffers(0, static_cast<UINT>(max_cbuffer), b.data());
				});
			}

			/*****  output_merge_context_t   ******************************************************************************************/
			void output_merge_context_t::bind(context_ptr& cp, const output_merge_d& od)
			{
				unbind(cp);
				cp->OMSetRenderTargets(static_cast<UINT>(od.render_array.size()), od.render_array.data(), od.depth);
				cp->OMSetBlendState(od.blend_state.ptr, od.blend_state.bind_factor.data(), od.blend_state.sample_mask);
				cp->OMSetDepthStencilState(od.depth_stencil_state.ptr, od.depth_stencil_state.stencil_ref);
			}

			void output_merge_context_t::clear_render_target(context_ptr& cp, output_merge_d& omd, size_t solt, float4 color)
			{
				if (omd.render_array.size() > solt)
					cp->ClearRenderTargetView(omd.render_array[solt], &color.x);
			}

			void output_merge_context_t::clear_render_target(context_ptr& cp, output_merge_d& omd, float4 color)
			{
				for (auto& ra : omd.render_array)
					if (ra != nullptr)
						cp->ClearRenderTargetView(ra, &color.x);
			}

			void output_merge_context_t::clear_depth(context_ptr& cp, output_merge_d& omd, float depth)
			{
				if (omd.depth != nullptr)
					cp->ClearDepthStencilView(omd.depth, D3D11_CLEAR_DEPTH, depth, 0);
			}

			void output_merge_context_t::clear_stencil(context_ptr& cp, output_merge_d& omd, uint8_t ref)
			{
				if (omd.depth != nullptr)
					cp->ClearDepthStencilView(omd.depth, D3D11_CLEAR_STENCIL, 0.0f, ref);
			}

			void output_merge_context_t::clear_depth_stencil(context_ptr& cp, output_merge_d& omd, float depth, uint8_t ref)
			{
				if (omd.depth != nullptr)
					cp->ClearDepthStencilView(omd.depth, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth, ref);
			}

			void output_merge_context_t::unbind(context_ptr& cp)
			{
				if (cp == nullptr) return;
				cp->OMSetRenderTargets(0, nullptr, nullptr);
				std::array<float, 4> factor = { 0.0f, 0.0f, 0.0f, 0.0f };
				UINT sample_mask = 0;
				cp->OMSetBlendState(nullptr, factor.data(), sample_mask);
				cp->OMSetDepthStencilState(nullptr, sample_mask);
			}

			/*****  draw_range_context_t   ******************************************************************************************/
			void draw_range_context_t::draw(context_ptr& cp, const draw_range_d& d)
			{
				auto& var = d.data;
				if (var.able_cast<draw_range_d::vertex_d>())
					return draw_type = type::RENDERER, cp->Draw(var.cast<draw_range_d::vertex_d>().vertex_count, var.cast<draw_range_d::vertex_d>().start_vertex_location);
				if (var.able_cast<draw_range_d::index_d>())
				{
					auto& ind = var.cast<draw_range_d::index_d>();
					return draw_type = type::RENDERER, cp->DrawIndexed(ind.index_count, ind.start_index_location, ind.base_vertex_location);
				}
				if (var.able_cast<draw_range_d::instance_d>())
				{
					auto& ins = var.cast<draw_range_d::instance_d>();
					return draw_type = type::RENDERER, cp->DrawInstanced(ins.vertex_pre_instance, ins.instance_count, ins.start_vertex_location, ins.start_instance_location);
				}
				if (var.able_cast<draw_range_d::instance_index_d>())
				{
					auto& in = var.cast<draw_range_d::instance_index_d>();
					return draw_type = type::RENDERER, cp->DrawIndexedInstanced(in.index_pre_instance, in.instance_count, in.start_index_location, in.base_vertex_location, in.start_instance_location);
				}
				if (var.able_cast<draw_range_d::dispatch_d>())
				{
					auto& in = var.cast<draw_range_d::dispatch_d>();
					return draw_type = type::COMPUTE, cp->Dispatch(in.X, in.Y, in.Z);
				}
			}

			auto draw_range_context_t::unbind(context_ptr& cp) -> type
			{
				type p = draw_type;
				draw_type = type::NONE;
				return p;
			}


			/*****  compute_shader_context_t   ******************************************************************************************/
			void compute_shader_context_t::bind(context_ptr& cp, const compute_d& cd)
			{
				cp->CSSetShader(cd.ptr, nullptr, 0);
				cp->CSSetUnorderedAccessViews(0, static_cast<UINT>(cd.UAV_array.size()), cd.UAV_array.data(), cd.offset.data());
				if(max_cbuffer > cd.cbuffer_array.size())
					nullptr_cbuffer.lock([&, this](decltype(nullptr_cbuffer)::type& b) {
					b.insert(b.end(), max_cbuffer - cd.cbuffer_array.size(), nullptr);
					cp->CSSetConstantBuffers(
						static_cast<UINT>(cd.cbuffer_array.size()), static_cast<UINT>(max_cbuffer - cd.cbuffer_array.size()), b.data());
				});
				max_cbuffer = cd.cbuffer_array.size();
				cp->CSSetConstantBuffers(0, static_cast<UINT>(cd.cbuffer_array.size()), cd.cbuffer_array.data());
				cp->CSSetShaderResources(0, static_cast<UINT>(cd.SRV_array.size()), cd.SRV_array.data());
			}

			void compute_shader_context_t::unbind(context_ptr& cp)
			{
				cp->CSSetShader(nullptr, nullptr, 0);
				ID3D11UnorderedAccessView* tem = nullptr;
				cp->CSSetUnorderedAccessViews(0, 1, &tem, nullptr);
				nullptr_cbuffer.lock([&, this](decltype(nullptr_cbuffer)::type& b) {
					b.insert(b.end(), max_cbuffer, nullptr);
					cp->CSSetConstantBuffers(0, static_cast<UINT>(max_cbuffer), b.data());
				});
				max_cbuffer = 0;
			}

		}
		/*****  pipe   ******************************************************************************************/
		void pipe_line::unbind() {
			switch (DR.unbind(ptr))
			{
			case Implement::draw_range_context_t::type::COMPUTE:
				CS.unbind(ptr);
				break;
			case Implement::draw_range_context_t::type::RENDERER:
				IA.unbind(ptr); VS.unbind(ptr); RA.unbind(ptr);
				PS.unbind(ptr); OM.unbind(ptr);
				break;
			default:
				break;
			}
		}

	}
}