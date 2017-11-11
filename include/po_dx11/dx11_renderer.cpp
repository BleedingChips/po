#include "dx11_renderer.h"

PO::Dx11::property_proxy_map& operator>>(PO::Dx11::property_proxy_map& ppm, const PO::Dx11::sub_viewport_perspective& svp)
{
	ppm << [&](PO::Dx11::property_viewport_transfer& pvt) {
		PO::Dx::matrix tem = DirectX::XMLoadFloat4x4(&svp.eye);
		PO::Dx::matrix inv = DirectX::XMMatrixInverse(nullptr, tem);
		PO::Dx::float4x4 tem2;
		DirectX::XMStoreFloat4x4(&tem2, inv);
		pvt.world_to_eye = svp.eye;
		pvt.eye_to_world = tem2;
		PO::Dx::matrix tem3 = DirectX::XMLoadFloat4x4(&svp.projection);
		tem = DirectX::XMMatrixMultiply(tem, tem3);
		inv = DirectX::XMMatrixInverse(nullptr, tem);
		PO::Dx::float4x4 tem4;
		DirectX::XMStoreFloat4x4(&tem2, tem);
		DirectX::XMStoreFloat4x4(&tem4, inv);
		pvt.world_to_camera = tem2;
		pvt.camera_to_world = tem4;
		pvt.set_surface(svp.projection_property.z, svp.projection_property.w, svp.view.view.MinDepth, svp.view.view.MaxDepth);
	};
	return ppm;
}

namespace PO
{
	namespace Dx11
	{

		sub_viewport_perspective::sub_viewport_perspective(float2 view_size, float2 view_left_top, float angle, float2 far_near_plane, float2 avalible_depth)
			: projection_property{  view_size.x / view_size.y, angle, far_near_plane.x, far_near_plane.y },
				view{ view_size, view_left_top, avalible_depth }
		{
			matrix ma = DirectX::XMMatrixPerspectiveFovLH(angle, projection_property.x, far_near_plane.x, far_near_plane.y);
			DirectX::XMStoreFloat4x4(&projection, ma);
			DirectX::XMStoreFloat4x4(&eye, DirectX::XMMatrixIdentity());
		}

		renderer_default::renderer_default(Dx11_frame_initializer& DFI)
			: creator(DFI.cre), back_buffer(DFI.bac), m_context(DFI.sta), swap(DFI.swa), main_view(DFI.bac.size_f()) 
		{
			om << back_buffer.cast_render_target_view(*this);
			view = viewport{ back_buffer.size_f() };
		}

		void renderer_default::pre_tick(duration da)
		{
			context() << view << om;
		}

		void renderer_default::pos_tick(duration da)
		{
			compute_storage.logic_to_swap(*this);
			compute_storage.swap_to_renderer(m_context);
			for (auto& ite : compute_storage.renderer.dispatch_request)
				ite.dispatch(m_context);

			draw_storage.logic_to_swap(*this);
			draw_storage.swap_to_renderer(m_context);
			for (auto& ite : draw_storage.renderer.draw_request)
				ite.draw(m_context);
			swap->Present(0, 0);
		}


		defer_renderer_default::defer_renderer_default(Dx11_frame_initializer& DFi) : 
			creator(DFi.cre), context(DFi.sta), swap(DFi.swa), 
			view(DFi.bac.size_f()), final_back_buffer(DFi.bac), ins(*this), total_time(0)
		{
			tex2 gbuffer_color;
			gbuffer_color.create_render_target(*this, DXGI_FORMAT_R16G16B16A16_FLOAT, DFi.bac.size());

			tex2 opaque_depth;
			opaque_depth.create_depth_stencil(*this, DST_format::D24_UI8, DFi.bac.size());

			opaque_output_merga << gbuffer_color.cast_render_target_view(*this) << opaque_depth.cast_depth_stencil_view(*this);

			opaque_mapping >> view;

			PO::Dx11::depth_stencil_state::description dss_defer_des = PO::Dx11::depth_stencil_state::description{
				TRUE, D3D11_DEPTH_WRITE_MASK_ALL, D3D11_COMPARISON_LESS, FALSE, D3D11_DEFAULT_STENCIL_READ_MASK, D3D11_DEFAULT_STENCIL_WRITE_MASK,
				D3D11_DEPTH_STENCILOP_DESC{ D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS },
				D3D11_DEPTH_STENCILOP_DESC{ D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS }
			};
			opaque_depth_stencil_state.create(*this, dss_defer_des);
			blend_state::description one_to_zero = blend_state::description{
				FALSE, FALSE, D3D11_RENDER_TARGET_BLEND_DESC{ TRUE, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL }
			};
			qpaque_blend.create(*this, one_to_zero);

			tex2 linearize_z_tex;
			linearize_z_tex.create_unordered_access(*this, DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT, DFi.bac.size());
			element_linear_z.compute = ins.create_compute<compute_linearize_z>();
			linear_z_maping << [&](property_linearize_z_output& plz)
			{
				plz.set_taregt_f(opaque_depth.cast_shader_resource_view(*this), linearize_z_tex.cast_unordered_access_view(*this), DFi.bac.size());
			};
			linear_z_maping.logic_to_renderer(*this);
			element_linear_z.mapping = linear_z_maping.map();

			element_merga.placemenet = ins.create_placement<placement_direct>();
			element_merga.geometry = ins.create_geometry<geometry_screen>();
			element_merga.material = ins.create_material<material_merga_gbuffer>();
			merga_map << [&](property_gbuffer& pg)
			{
				pg.srv = gbuffer_color.cast_shader_resource_view(*this);
			};
			merga_map.logic_to_renderer(*this);
			element_merga.mapping = merga_map.map();

			transparent_mapping << [&](property_linear_z& pl)
			{
				pl.z_buffer = linearize_z_tex.cast_shader_resource_view(*this);
			};
			transparent_mapping.logic_to_renderer(*this);
			final_output << final_back_buffer.cast_render_target_view(*this);
		}

		void defer_renderer_default::pre_tick(duration da)
		{
			total_time += da;
		}

		void defer_renderer_default::pos_tick(duration da)
		{
			opaque_mapping << [this](property_viewport_transfer& pvt)
			{
				pvt.set_time(static_cast<float>(total_time.count()));
			};
			opaque_mapping.logic_to_renderer(*this);
			Tool::stack_list<Implement::property_map> map_list{ *opaque_mapping.map() };

			compute.logic_to_swap(*this);
			compute.swap_to_renderer(context);
			for (auto& ite : compute.renderer.dispatch_request)
			{
				ite.dispatch(context, &map_list);
				context.unbind();
			}
				

			context.clear_render_target(opaque_output_merga, { 0.0, 0.0, 1.0, 0.0 });
			context.clear_depth_stencil(opaque_output_merga, 1.0, 0);
			context << view.view;

			opaque.logic_to_swap(*this);
			opaque.swap_to_renderer(context);
			for (auto& ite : opaque.renderer.draw_request)
			{
				context << opaque_output_merga;
				ite.draw(context, opaque_depth_stencil_state, &map_list);
				context.unbind();
			}

			output_merge_stage om;
			context << om;
			element_linear_z.dispatch(*this, &map_list);
			context.unbind();

			Tool::stack_list<Implement::property_map> transparent_map_list{ *transparent_mapping.map(), &map_list };

			//context << opaque_output_merga;
			transparent.logic_to_swap(*this);
			transparent.swap_to_renderer(context);
			for (auto& ite : transparent.renderer.draw_request)
			{
				context << opaque_output_merga;
				ite.draw(context, transparent_depth_stencil_state, &transparent_map_list);
				context.unbind();
			}

			context << final_output;
			element_merga.draw(context, {}, &transparent_map_list);
			context.unbind();
			
			if (!pos_task.empty())
			{
				for (auto& ite : pos_task)
				{
					ite(*this);
				}
				pos_task.clear();
			}
			
			swap->Present(0, 0);



		}

		defer_renderer_default::material_merga_gbuffer::material_merga_gbuffer(creator& c) :
			material_resource(c, u"build_in_material_merge_gbuffer_ps.cso") {}

		const element_requirement& defer_renderer_default::material_merga_gbuffer::requirement() const
		{
			return make_element_requirement(
				[](stage_context& sc, property_wrapper_t<property_gbuffer>& pgd) {
				sc.PS() << pgd.srv[0] << pgd.ss[0];
			}
			);
		}
		
		void defer_renderer_default::property_linearize_z_output::set_taregt_f(shader_resource_view<tex2> input, unordered_access_view<tex2> output_f, uint32_t2 output_size) {
			input_depth = std::move(input);
			output_depth = std::move(output_f);
			size = output_size;
		}

		defer_renderer_default::compute_linearize_z::compute_linearize_z(creator& c) :
			compute_resource(c, u"build_in_compute_linearize_z_cs.cso")
		{}

		const element_requirement& defer_renderer_default::compute_linearize_z::requirement() const
		{
			return make_element_requirement(
				[](stage_context& sc, property_wrapper_t<property_linearize_z_output>& plz) {
				sc.CS() << plz.input_depth[0] << plz.output_depth[0];
				sc << dispatch_call{ plz.size.x, plz.size.y , 1 };
			},
				[](stage_context& sc, property_wrapper_t<property_viewport_transfer>& pvt) {
				sc.CS() << pvt.viewport[0];
			}
			);
		}

	}
}