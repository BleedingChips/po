#include "dx11_renderer.h"
namespace PO
{
	namespace Dx11
	{

		sub_viewport_perspective::sub_viewport_perspective(float2 view_size, float2 view_left_top, float angle, float2 far_near_plane, float2 avalible_depth)
			: projection_property{  view_size.x / view_size.y, angle, far_near_plane.x, far_near_plane.y },
				view{ view_left_top, view_left_top + view_size, avalible_depth }
		{
			matrix ma = DirectX::XMMatrixPerspectiveFovLH(angle, projection_property.x, far_near_plane.x, far_near_plane.y);
			DirectX::XMStoreFloat4x4(&projection, ma);
		}


		renderer_default::renderer_default(value_table& vt) :
			stage_context(vt.get<stage_context>()), back_buffer(vt.get<tex2>()),
			/*instance(*this),*/ main_view(vt.get<tex2>().size_f())
		{
			om << cast_render_target_view(back_buffer)[0];
			view = viewport{ back_buffer.size_f() };
		}

		void renderer_default::pre_tick(duration da)
		{
			context() << view << om;
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