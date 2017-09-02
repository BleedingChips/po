#include "dx11_renderer.h"

PO::Dx11::property_proxy_map& operator>>(PO::Dx11::property_proxy_map& ppm, const PO::Dx11::sub_viewport_perspective& svp)
{
	ppm << [&](PO::Dx11::property_viewport_transfer& pvt) {
		PO::Dx::matrix tem = DirectX::XMLoadFloat4x4(&svp.eye);
		PO::Dx::matrix inv = DirectX::XMMatrixInverse(nullptr, tem);
		PO::Dx::float4x4 tem2;
		DirectX::XMStoreFloat4x4(&tem2, inv);
		pvt.set_world_eye(svp.eye, tem2);
		PO::Dx::matrix tem3 = DirectX::XMLoadFloat4x4(&svp.projection);
		tem = DirectX::XMMatrixMultiply(tem, tem3);
		inv = DirectX::XMMatrixInverse(nullptr, tem);
		PO::Dx::float4x4 tem4;
		DirectX::XMStoreFloat4x4(&tem2, tem);
		DirectX::XMStoreFloat4x4(&tem4, inv);
		pvt.set_world_camera(tem2, tem4);
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


		renderer_default::renderer_default(value_table& vt) :
			creator(vt.get<creator>()), 
			context(vt.get<stage_context>()), back_buffer(vt.get<tex2>()),
			/*instance(*this),*/ main_view(vt.get<tex2>().size_f()), ins(*this)
		{
			om << cast_render_target_view(back_buffer)[0];
			view = viewport{ back_buffer.size_f() };
		}

		void renderer_default::pre_tick(duration da)
		{
			context << view << om;
		}
		void renderer_default::pos_tick(duration da)
		{
			els.push(esb, *this);
			ers.get(esb, *this);

			for (auto& ite : ers.draw_request)
			{
				for (auto& ite2 : ite.second)
					ite2.draw(*this);
			}
		}


		pipeline_compute_default::pipeline_compute_default() : pipeline_interface(typeid(decltype(*this))) {}
		void pipeline_compute_default::execute_implement(stage_context& sc, element_renderer_storage& storage, Tool::stack_list<Implement::property_map>* ptr)
		{
			for (auto& ite : storage.dispatch_request)
				ite.dispatch(sc, ptr);
		}

		pipeline_opaque_default::pipeline_opaque_default() : pipeline_interface(typeid(decltype(*this))) {}
		void pipeline_opaque_default::execute_implement(stage_context& sc, element_renderer_storage& storage, Tool::stack_list<Implement::property_map>* ptr)
		{
			sc.clear_render_target(om, { 0.0, 0.0, 0.0, 0.0 });
			sc.clear_depth_stencil(om, 1.0, 0);
			sc << om << dss << bs;
			auto ite = storage.draw_request.find(typeid(decltype(*this)));
			if (ite != storage.draw_request.end())
			{
				for (auto& ite2 : ite->second)
					ite2.draw(sc, ptr);
			}
			sc.unbind();
		}

		void pipeline_opaque_default::set(creator& c, uint32_t2 size)
		{

			g_buffer = c.create_tex2_render_target(DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT, size.x, size.y);
			depth = c.create_tex2_depth_stencil(DST_format::D24_UI8, g_buffer);
			rtv = c.cast_render_target_view(g_buffer);
			dsv = c.cast_depth_setncil_view(depth);
			om << rtv[0] << dsv;
			PO::Dx11::depth_stencil_state::description dss_defer_des = PO::Dx11::depth_stencil_state::description{
				TRUE, D3D11_DEPTH_WRITE_MASK_ALL, D3D11_COMPARISON_LESS, FALSE, D3D11_DEFAULT_STENCIL_READ_MASK, D3D11_DEFAULT_STENCIL_WRITE_MASK,
				D3D11_DEPTH_STENCILOP_DESC{ D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS },
				D3D11_DEPTH_STENCILOP_DESC{ D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS }
			};
			dss = c.create_depth_stencil_state(dss_defer_des);
			blend_state::description one_to_zero = blend_state::description{
				FALSE, FALSE, D3D11_RENDER_TARGET_BLEND_DESC{ TRUE, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL }
			};
			bs = c.create_blend_state(one_to_zero);
		}

		const char16_t* material_opaque_testing::material_shader_patch_ps()
		{
			return u"material_test_ps.cso";
		}

		std::type_index material_opaque_testing::pipeline_id()
		{
			return typeid(pipeline_opaque_default);
		}

		const char16_t* material_merga_gbuffer_default::material_shader_patch_ps()
		{
			return u"build_in_material_merge_gbuffer_ps.cso";
		}

		void material_merga_gbuffer_default::material_apply(stage_context&) {}
		bool material_merga_gbuffer_default::material_update(stage_context& sc, property_interface& pi)
		{
			return pi.cast([&](property_gbuffer_default::renderer_data& pgd) {
				sc.PS() << pgd.srv[0] << pgd.ss[0];
			});
		}
		const std::set<std::type_index>& material_merga_gbuffer_default::material_requirement() const
		{
			return make_property_info_set<property_gbuffer_default>{};
		}

		pipeline_transparent_default::pipeline_transparent_default(): pipeline_interface(typeid(decltype(*this))){}
		void pipeline_transparent_default::set(creator& c)
		{
			blend_state::description s_alpha_to_inv_s_alpha = blend_state::description{
				FALSE, FALSE, D3D11_RENDER_TARGET_BLEND_DESC{ TRUE, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL }
			};
			bs = c.create_blend_state(s_alpha_to_inv_s_alpha);
			PO::Dx11::depth_stencil_state::description dss_transparent_des = PO::Dx11::depth_stencil_state::description{
				TRUE, D3D11_DEPTH_WRITE_MASK_ZERO, D3D11_COMPARISON_LESS, FALSE, D3D11_DEFAULT_STENCIL_READ_MASK, D3D11_DEFAULT_STENCIL_WRITE_MASK,
				D3D11_DEPTH_STENCILOP_DESC{ D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS },
				D3D11_DEPTH_STENCILOP_DESC{ D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS }
			}; 
			dss = c.create_depth_stencil_state(dss_transparent_des);
		}
		void pipeline_transparent_default::execute_implement(stage_context& sc, element_renderer_storage& storage, Tool::stack_list<Implement::property_map>* ptr)
		{
			sc << dss << bs;
			auto find = storage.draw_request.find(typeid(decltype(*this)));
			if (find != storage.draw_request.end())
			{
				for (auto& ite : find->second)
				{
					ite.draw(sc, ptr);
				}
			}
			sc.unbind();
		}

		const char16_t* material_transparent_testing::material_shader_patch_ps()
		{
			return u"material_test_ps.cso";
		}

		std::type_index material_transparent_testing::pipeline_id()
		{
			return typeid(pipeline_transparent_default);
		}


		defer_renderer_default::defer_renderer_default(value_table& vt) :
			creator(vt.get<creator>()),
			context(vt.get<stage_context>()), back_buffer(vt.get<tex2>()),
			/*instance(*this),*/ view(vt.get<tex2>().size_f()), ins(*this)
		{
			om << cast_render_target_view(back_buffer)[0];
			mapping >> view;
			opaque_pipeline.set(*this, back_buffer.size());

			linear_z_buffer = create_tex2_unordered_access(DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT, back_buffer);

			decltype(dss)::description des = decltype(dss)::default_description;
			des.DepthEnable = false;
			dss = create_depth_stencil_state(des);

			merga << ins.create_geometry<geometry_screen>() << ins.create_placement<placement_direct>() << ins.create_material<material_merga_gbuffer_default>();
			post_mapping << [this](property_gbuffer_default& pgd) {
				pgd.set_gbuffer(*this, opaque_pipeline.g_buffer, linear_z_buffer);
			};
			
			merga.ptr->mapping.push(*this);
			merga.ptr->mapping.inside_map->update(context);
			merga.ptr->geometry->update_layout(*(merga.ptr->placement), *this);

			post_mapping.push(*this);
			post_mapping.inside_map->update(context);

			linear_z << ins.create_compute<compute_linearize_z>() << [&, this](property_linearize_z& plz) {
				plz.set_taregt(*this, opaque_pipeline.depth, linear_z_buffer);
			};
			linear_z.ptr->mapping.push(*this);
			linear_z.ptr->mapping.inside_map->update(context);
			transparent_pipeline.set(*this);
		}

		void defer_renderer_default::pre_tick(duration da)
		{

		}

		void defer_renderer_default::pos_tick(duration da)
		{
			context << view;
			els.push(esb, *this);
			ers.get(esb, context);
			mapping.push(*this);
			mapping.inside_map->update(context);
			Tool::stack_list<Implement::property_map> tem{ *mapping.inside_map };
			compute_pipeline.execute(context, ers, &tem);
			opaque_pipeline.execute(context, ers, &tem);
			context.unbind();
			Tool::stack_list<Implement::property_map> tem2{ *post_mapping.inside_map, &tem };
			Implement::element_dispatch_request temxx2{ linear_z.ptr->compute[0], linear_z.ptr->mapping.inside_map };
			temxx2.update(context);
			temxx2.dispatch(context, &tem2);
			context.unbind();

			output_merge_stage om_tem;
			om_tem << opaque_pipeline.rtv[0] << opaque_pipeline.dsv;
			context << om_tem;

			transparent_pipeline.execute(context, ers, &tem2);

			context << om << dss;

			

			
			
			Implement::element_draw_request temxx{ merga.ptr->placement, merga.ptr->geometry, merga.ptr->material[typeid(void)], merga.ptr->mapping.inside_map };
			temxx.update(context);
			temxx.draw(context, &tem2);

			context.unbind();

			
		}





		/*
		defer_render_pipeline_default::defer_render_pipeline_default() : Implement::pipeline_interface(typeid(decltype(*this))) {}
		defer_render_pipeline_default::defer_render_pipeline_default(const std::type_index& ti) : Implement::pipeline_interface(ti) {}

		void defer_render_pipeline_default::set_back_buffer_size(creator& p, tex2 t)
		{
			G_buffer = p.create_tex2_render_target(DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT, t);
			depth_stencial = p.create_tex2_depth_stencil(DST_format::D24_UI8, t);
			liner_z = p.create_tex2_unordered_access(DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT, t);
			oms << p.cast_render_target_view(G_buffer)[0] << p.cast_depth_setncil_view(depth_stencial);
		}
		*/






		/*
		proxy simple_renderer::mapping(std::type_index ti, adapter_interface& ai)
		{
			if (ti == typeid(simple_renderer))
				return make_proxy<simple_renderer>(ai, *this);
			return {};
		}*/


	}
}