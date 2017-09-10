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
			om << back_buffer.cast_render_target_view(*this);
			view = viewport{ back_buffer.size_f() };
		}

		void renderer_default::pre_tick(duration da)
		{
			context << view << om;
		}
		void renderer_default::pos_tick(duration da)
		{
			els.logic_to_swap(esb, *this);
			ers.swap_to_renderer(esb, *this);

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
			g_buffer.create_render_target(c, DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT, size);
			depth.create_depth_stencil(c, DST_format::D24_UI8, size);
			rtv = g_buffer.cast_render_target_view(c);
			dsv = depth.cast_depth_stencil_view(c);
			om.clear();
			om << rtv << dsv;
			PO::Dx11::depth_stencil_state::description dss_defer_des = PO::Dx11::depth_stencil_state::description{
				TRUE, D3D11_DEPTH_WRITE_MASK_ALL, D3D11_COMPARISON_LESS, FALSE, D3D11_DEFAULT_STENCIL_READ_MASK, D3D11_DEFAULT_STENCIL_WRITE_MASK,
				D3D11_DEPTH_STENCILOP_DESC{ D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS },
				D3D11_DEPTH_STENCILOP_DESC{ D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS }
			};
			dss.create(c, dss_defer_des);
			blend_state::description one_to_zero = blend_state::description{
				FALSE, FALSE, D3D11_RENDER_TARGET_BLEND_DESC{ TRUE, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL }
			};
			bs.create(c, one_to_zero);
		}

		material_merga_gbuffer_default::material_merga_gbuffer_default(creator& c) :
			material_resource(c, u"build_in_material_merge_gbuffer_ps.cso") {}

		const element_requirement& material_merga_gbuffer_default::requirement() const
		{
			return make_element_requirement(
				[](stage_context& sc, property_gbuffer_default::renderer_data& pgd) {
				sc.PS() << pgd.srv[0] << pgd.ss[0];
			}
			);
		}

		material_opaque_testing::material_opaque_testing(creator& c) :
			material_opaque_resource(c, u"material_test_ps.cso")
		{}

		material_qpaque_texture_coord::material_qpaque_texture_coord(creator& c) :
			material_opaque_resource(c, u"build_in_material_defer_render_texcoord_ps.cso") {}
		
		void property_linearize_z::set_taregt_f(shader_resource_view<tex2> input, unordered_access_view<tex2> output_f, uint32_t2 output_size) {
			input_depth = std::move(input);
			output_depth = std::move(output_f);
			size = output_size;
			need_update();
		}

		compute_linearize_z::compute_linearize_z(creator& c) :
			compute_resource(c, u"build_in_compute_linearize_z_cs.cso")
		{}

		const element_requirement& compute_linearize_z::requirement() const
		{
			return make_element_requirement(
				[](stage_context& sc, property_linearize_z::renderer_data& plz) {
				sc.CS() << plz.input_depth[0] << plz.output_depth[0];
				sc << dispatch_call{ plz.size.x, plz.size.y , 1 };
			},
				[](stage_context& sc, property_viewport_transfer::renderer_data& pvt) {
				sc.CS() << pvt.viewport[0];
			}
			);
		}

		pipeline_transparent_default::pipeline_transparent_default(): pipeline_interface(typeid(decltype(*this))){}
		void pipeline_transparent_default::set(creator& c)
		{
			blend_state::description s_alpha_to_inv_s_alpha = blend_state::description{
				FALSE, FALSE, D3D11_RENDER_TARGET_BLEND_DESC{ TRUE, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL }
			};
			bs.create(c, s_alpha_to_inv_s_alpha);
			PO::Dx11::depth_stencil_state::description dss_transparent_des = PO::Dx11::depth_stencil_state::description{
				TRUE, D3D11_DEPTH_WRITE_MASK_ZERO, D3D11_COMPARISON_LESS, FALSE, D3D11_DEFAULT_STENCIL_READ_MASK, D3D11_DEFAULT_STENCIL_WRITE_MASK,
				D3D11_DEPTH_STENCILOP_DESC{ D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS },
				D3D11_DEPTH_STENCILOP_DESC{ D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS }
			}; 
			dss.create(c, dss_transparent_des);
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

		material_transparent_testing::material_transparent_testing(creator& c)
			: material_transparent_resource(c, u"material_test_ps.cso")
		{}

		defer_renderer_default::defer_renderer_default(value_table& vt) :
			creator(vt.get<creator>()),
			context(vt.get<stage_context>()), back_buffer(vt.get<tex2>()),
			/*instance(*this),*/ view(vt.get<tex2>().size_f()), ins(*this)
		{
			om << back_buffer.cast_render_target_view(*this);
			mapping >> view;
			opaque_pipeline.set(*this, back_buffer.size());

			linear_z_buffer.create_unordered_access(*this, DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT, back_buffer.size());

			decltype(dss)::description des = decltype(dss)::default_description;
			des.DepthEnable = false;
			dss.create(*this, des);

			merga << ins.create_geometry<geometry_screen>() << ins.create_placement<placement_direct>() << ins.create_material<material_merga_gbuffer_default>();
			post_mapping << [this](property_gbuffer_default& pgd) {
				pgd.set_gbuffer(*this, opaque_pipeline.g_buffer, linear_z_buffer);
			};
			
			//merga.ptr->mapping.logic_to_renderer(*this);
			//merga.ptr->geometry->update_layout(*(merga.ptr->placement), *this);

			//post_mapping.logic_to_renderer(*this);

			linear_z << ins.create_compute<compute_linearize_z>() << [&, this](property_linearize_z& plz) {
				plz.set_taregt_f(
					opaque_pipeline.depth.cast_shader_resource_view(*this), 
					linear_z_buffer.cast_unordered_access_view(*this),
					linear_z_buffer.size()
				);
			};
			linear_z.ptr->mapping.logic_to_renderer(*this);
			transparent_pipeline.set(*this);
		}

		void defer_renderer_default::pre_tick(duration da)
		{

		}

		void defer_renderer_default::pos_tick(duration da)
		{
			context << view;
			els.logic_to_swap(esb, *this);
			ers.swap_to_renderer(esb, context);
			mapping.logic_to_renderer(*this);
			Tool::stack_list<Implement::property_map> tem{ *mapping.map() };
			compute_pipeline.execute(context, ers, &tem);
			opaque_pipeline.execute(context, ers, &tem);
			context.unbind();
			Tool::stack_list<Implement::property_map> tem2{ *post_mapping.map(), &tem };
			Implement::element_dispatch_request temxx2{ linear_z.ptr->compute[0], linear_z.ptr->mapping.map() };
			temxx2.dispatch(context, &tem2);
			context.unbind();

			output_merge_stage om_tem;
			om_tem << opaque_pipeline.rtv << opaque_pipeline.dsv;
			context << om_tem;

			transparent_pipeline.execute(context, ers, &tem2);

			context << om << dss;

			

			
			
			Implement::element_draw_request temxx{ merga.ptr->placement, merga.ptr->geometry, merga.ptr->material[typeid(void)], merga.ptr->mapping.map() };
			temxx.draw(context, &tem2);

			context.unbind();

		}


		material_opaque_tex2_viewer::material_opaque_tex2_viewer(creator& c) :
			material_opaque_resource(c, u"build_in_material_tex2_view.cso")
		{}

		const element_requirement& material_opaque_tex2_viewer::requirement() const
		{
			return make_element_requirement(
				[](stage_context& sc, property_tex2::renderer_data& rd) {
				sc.PS() << rd.srv[0] << rd.ss[0];
			}
			);
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