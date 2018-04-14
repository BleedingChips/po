#include "dx11_renderer.h"

PO::Dx11::property_proxy_map& operator>>(PO::Dx11::property_proxy_map& ppm, const PO::Dx11::sub_viewport_perspective& svp)
{
	ppm << [&](PO::Dx11::property_viewport_transfer& pvt) {
		PO::matrix tem = DirectX::XMLoadFloat4x4(&svp.eye);
		PO::matrix inv = DirectX::XMMatrixInverse(nullptr, tem);
		PO::float4x4 tem2;
		DirectX::XMStoreFloat4x4(&tem2, inv);
		pvt.world_to_eye = svp.eye;
		pvt.eye_to_world = tem2;
		PO::matrix tem3 = DirectX::XMLoadFloat4x4(&svp.projection);
		tem = DirectX::XMMatrixMultiply(tem, tem3);
		inv = DirectX::XMMatrixInverse(nullptr, tem);
		PO::float4x4 tem4;
		DirectX::XMStoreFloat4x4(&tem2, tem);
		DirectX::XMStoreFloat4x4(&tem4, inv);
		pvt.world_to_camera = tem2;
		pvt.camera_to_world = tem4;
		pvt.set_surface(svp.projection_property.z, svp.projection_property.w, svp.view.view.MinDepth, svp.view.view.MaxDepth);
	};
	return ppm;
}
/*
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


		stage_instance& stage_instance_instance() noexcept
		{
			static stage_instance si;
			return si;
		}

		void defer_renderer_default::init(context& c, device_ptr& dp, tex2& back_buffer)
		{
			total_count = duration(0);
			stage_instance_instance().tool = dp;
			view.set(back_buffer.size_f());
			tex2 gbuffer_color;
			gbuffer_color.create_render_target(dp, DXGI_FORMAT_R16G16B16A16_FLOAT, back_buffer.size());
			tex2 opaque_depth;
			opaque_depth.create_depth_stencil(dp, DST_format::D24_UI8, back_buffer.size());
			opaque_output_merga << gbuffer_color.cast_render_target_view(dp) << opaque_depth.cast_depth_stencil_view(dp);
			opaque_mapping >> view;
			PO::Dx11::depth_stencil_state::description dss_defer_des = PO::Dx11::depth_stencil_state::description{
				TRUE, D3D11_DEPTH_WRITE_MASK_ALL, D3D11_COMPARISON_LESS, FALSE, D3D11_DEFAULT_STENCIL_READ_MASK, D3D11_DEFAULT_STENCIL_WRITE_MASK,
				D3D11_DEPTH_STENCILOP_DESC{ D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS },
				D3D11_DEPTH_STENCILOP_DESC{ D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS }
			};
			opaque_depth_stencil_state.create(dp, dss_defer_des);
			blend_state::description one_to_zero = blend_state::description{
				FALSE, FALSE, D3D11_RENDER_TARGET_BLEND_DESC{ TRUE, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL }
			};
			qpaque_blend.create(dp, one_to_zero);
			tex2 linearize_z_tex;
			linearize_z_tex.create_unordered_access(dp, DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT, back_buffer.size());
			element_linear_z.compute = stage_instance_instance().create_compute<compute_linearize_z>();
			linear_z_maping << [&](property_linearize_z_output& plz)
			{
				plz.set_taregt_f(opaque_depth.cast_shader_resource_view(dp), linearize_z_tex.cast_unordered_access_view(dp), back_buffer.size());
			};
			linear_z_maping.logic_to_renderer(dp);
			element_linear_z.mapping = linear_z_maping.map();

			element_merga.placemenet = stage_instance_instance().create_placement<placement_direct>();
			element_merga.geometry = stage_instance_instance().create_geometry<geometry_screen>();
			element_merga.material = stage_instance_instance().create_material<material_merga_gbuffer>();
			merga_map << [&](property_gbuffer& pg)
			{
				pg.srv = gbuffer_color.cast_shader_resource_view(dp);
			};
			merga_map.logic_to_renderer(dp);
			element_merga.mapping = merga_map.map();

			transparent_mapping << [&](property_linear_z& pl)
			{
				pl.z_buffer = linearize_z_tex.cast_shader_resource_view(dp);
			};
			transparent_mapping.logic_to_renderer(dp);
			final_output << back_buffer.cast_render_target_view(dp);
			dev = dp;
			sp = std::make_shared<Implement::defer_renderer_default_holder>();
			c.create_system<capture_system>(sp, dev);
			c.create_system<capture2_system>(sp, dev);
		}

		void defer_renderer_default::render_frame(duration d, context_ptr& cp, tex2& bb)
		{
			total_count += d;
			stage_con.set(cp, dev);
			assert(sp);
			opaque_mapping << [&, this](property_viewport_transfer& pvt)
			{
				pvt.set_time(static_cast<float>(total_count.count()));
			};
			opaque_mapping.logic_to_renderer(dev);
			Tool::stack_list<Implement::property_map> map_list{ *opaque_mapping.map() };

			sp->compute.swap_to_renderer(stage_con);

			for (auto ite = sp->compute.renderer.dispatch_request.rbegin(); ite != sp->compute.renderer.dispatch_request.rend(); ++ite)
			{
				ite->dispatch(stage_con, &map_list);
				stage_con.unbind();
			}

			stage_con.clear_render_target(opaque_output_merga, { 0.0, 0.0, 0.0, 0.0 });
			stage_con.clear_depth_stencil(opaque_output_merga, 1.0, 0);
			stage_con << view.view;

			//cp->opaque.logic_to_swap(*this);
			sp->opaque.swap_to_renderer(stage_con);


			for (auto ite = sp->opaque.renderer.draw_request.rbegin(); ite != sp->opaque.renderer.draw_request.rend(); ++ite)
			{
				stage_con << opaque_output_merga;
				ite->draw(stage_con, opaque_depth_stencil_state, &map_list);
				stage_con.unbind();
			}

			output_merge_stage om;
			stage_con << om;
			element_linear_z.dispatch(stage_con, &map_list);
			stage_con.unbind();

			Tool::stack_list<Implement::property_map> transparent_map_list{ *transparent_mapping.map(), &map_list };

			//context << opaque_output_merga;
			//sp->transparent.logic_to_swap(stage_con);
			sp->transparent.swap_to_renderer(stage_con);
			for (auto& ite : sp->transparent.renderer.draw_request)
			{
				stage_con << opaque_output_merga;
				ite.draw(stage_con, transparent_depth_stencil_state, &transparent_map_list);
				stage_con.unbind();
			}

			stage_con << final_output;
			element_merga.draw(stage_con, {}, &transparent_map_list);
			stage_con.unbind();
		}

		defer_renderer_default::material_merga_gbuffer::material_merga_gbuffer(device_ptr& c) :
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

		defer_renderer_default::compute_linearize_z::compute_linearize_z(device_ptr& c) :
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
*/