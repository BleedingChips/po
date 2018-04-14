#include "defer_renderer.h"
#include "build_in_element.h"
namespace {
	PO::Dx11::depth_stencil_state::description dss_defer_des = PO::Dx11::depth_stencil_state::description{
			TRUE, D3D11_DEPTH_WRITE_MASK_ALL, D3D11_COMPARISON_LESS, FALSE, D3D11_DEFAULT_STENCIL_READ_MASK, D3D11_DEFAULT_STENCIL_WRITE_MASK,
			D3D11_DEPTH_STENCILOP_DESC{ D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS },
			D3D11_DEPTH_STENCILOP_DESC{ D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS }
	};

	PO::Dx11::depth_stencil_state::description dss_transparent_des = PO::Dx11::depth_stencil_state::description{
		TRUE, D3D11_DEPTH_WRITE_MASK_ZERO, D3D11_COMPARISON_LESS, FALSE, D3D11_DEFAULT_STENCIL_READ_MASK, D3D11_DEFAULT_STENCIL_WRITE_MASK,
		D3D11_DEPTH_STENCILOP_DESC{ D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS },
		D3D11_DEPTH_STENCILOP_DESC{ D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS }
	};

	PO::Dx11::depth_stencil_state::description dss_post_des = PO::Dx11::depth_stencil_state::description{
		TRUE, D3D11_DEPTH_WRITE_MASK_ALL, D3D11_COMPARISON_ALWAYS, FALSE, D3D11_DEFAULT_STENCIL_READ_MASK, D3D11_DEFAULT_STENCIL_WRITE_MASK,
		D3D11_DEPTH_STENCILOP_DESC{ D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS },
		D3D11_DEPTH_STENCILOP_DESC{ D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS }
	};
}

namespace PO
{
	namespace Dx11
	{

		defer_renderer::defer_renderer(value_table& vt) : simple_renderer(vt), near_plane(0.01f), far_plane(1000.0f), xy_rate(1024.0f / 768.0f), view_angel(3.1415926f / 4.0f), time(0.0f) 
		{
			depth_stencial = create_tex2_depth_stencil(DST_format::D24_UI8, simple_renderer::back_buffer);
			color_bufer = create_tex2_render_target(DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT, simple_renderer::back_buffer);
			liner_z = create_tex2_unordered_access(DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT, simple_renderer::back_buffer);

			oms << cast_render_target_view(color_bufer)[0] << cast_depth_setncil_view(depth_stencial);

			using namespace DirectX;

			XMStoreFloat4x4(&view, XMMatrixLookAtLH({ 0.0, 0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0, 1.0 }, { 0.0, 1.0, 0.0, 0.0 }));

			XMStoreFloat4x4(&projection, XMMatrixPerspectiveFovLH(3.1415926f / 4.0f, xy_rate, near_plane, far_plane));

			dss_defer = create_depth_stencil_state(::dss_defer_des);
			dss_transparent = create_depth_stencil_state(::dss_transparent_des);
			dss_post = create_depth_stencil_state(::dss_post_des);

			XMMATRIX view_ = XMLoadFloat4x4(&view);
			XMMATRIX mat = XMMatrixMultiply(view_, XMMatrixPerspectiveFovLH(3.1415926f / 4.0f, 1024.0f / 768.0f, 0.01f, 1000.0f));
			XMMATRIX mat_inv = XMMatrixInverse(nullptr, mat);
			float4x4 p;
			float4x4 p_inv;
			XMStoreFloat4x4(&p, mat);
			XMStoreFloat4x4(&p_inv, mat_inv);



			float4x4 temporary;
			DirectX::XMStoreFloat4x4(&temporary, DirectX::XMMatrixIdentity());

			all_element.mapping.make_interface([&, this](property_screen& sp) {
				sp.set(*this, view, p, p_inv, time);
			});
			all_element.mapping.make_interface([&, this](property_screen_static& sp) {
				sp.set(*this, projection, near_plane, far_plane, xy_rate, float2{ 0.0, 0.0 }, float2{ 1024, 768 }, float2{ 0.0, 1.0 });
			});

			all_interface.make_geometry_and_placement(*this, merga, [](geometry_square_static&, placement_direct& pss) {});
			all_interface.make_material(*this, merga, [](material_merge_gbuffer&) {});
			all_element.mapping.make_interface([this](property_gbuffer& gp) {
				gp.set_gbuffer(*this, color_bufer, liner_z);
			});

			all_interface.make_compute(*this, linearize_z, [](compute_linearize_z&) {});
			linearize_z.make_interface([this](property_linearize_z& plz) {
				plz.set_depth_stencil_texture(*this, depth_stencial, liner_z);
			});
		}

		void defer_renderer::pre_tick(duration da)
		{
			simple_renderer::pre_tick(da);
			using namespace DirectX;
			XMMATRIX view_ = XMLoadFloat4x4(&view);
			XMMATRIX mat = XMMatrixMultiply(view_, XMMatrixPerspectiveFovLH(3.1415926f / 4.0f, 1024.0f / 768.0f, 0.01f, 1000.0f));
			XMMATRIX mat_inv = XMMatrixInverse(nullptr, mat);
			float4x4 p;
			float4x4 p_inv;
			XMStoreFloat4x4(&p, mat);
			XMStoreFloat4x4(&p_inv, mat_inv);
			time += da.count();
			all_element.mapping.make_interface([&, this](property_screen& sp) {
				sp.set(*this, view, p, p_inv, time);
			});
			pipeline::clear_render_target(oms, { 0.0, 0.0, 0.0, 1.0 });
			pipeline::clear_depth_stencil(oms, 1.0, 0);
			unbind();
			all_element.draw(render_order::NotSet, *this);

			*this << oms << dss_defer;
		}

		void defer_renderer::pos_tick(duration da)
		{
			
			all_element.draw(render_order::Defer, *this);
			*this << dss_transparent;
			unbind();
			all_element.draw(linearize_z, *this);
			unbind();
			*this << oms;
			all_element.draw(render_order::Transparent, *this);
			unbind();
			pipeline::clear_render_target(om, { 0.0, 0.0, 0.0, 1.0 });
			*this << om << dss_post;
			all_element.draw(merga, *this);
			all_element.draw(render_order::Post, *this);
			unbind();
			all_element.clear();
		}

		/*
		void defer_renderer::push_element(const element& ptr)
		{
			
			storage_elemnt.push_back(ptr, *this);
			
		}
		*/

	}
}