#include "defer_renderer.h"
#include "element\material.h"
#include "element\geometry.h"
#include "element\placement.h"
#include "element\property.h"
namespace PO
{
	namespace Dx11
	{
		proxy defer_renderer::mapping(std::type_index ti, adapter_interface& ai)
		{
			if (ti == typeid(defer_renderer))
				return make_proxy<defer_renderer>(ai, *this);
			return simple_renderer::mapping(ti, ai);
		}

		void defer_renderer::init(value_table& vt)
		{
			simple_renderer::init(vt);

			depth_stencial = create_tex2_depth_stencil(DST_format::D24_UI8, simple_renderer::back_buffer);
			color_bufer = create_tex2_render_target(DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT, simple_renderer::back_buffer);

			oms << cast_render_target_view(color_bufer)[0]
				<< cast_depth_setncil_view(depth_stencial);

			using namespace DirectX;

			XMStoreFloat4x4(&view, XMMatrixLookAtLH({ 0.0, 0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0, 1.0 }, {0.0, 1.0, 0.0, 0.0}));
			XMStoreFloat4x4(&projection, XMMatrixPerspectiveFovLH(3.1415926f / 4.0f, 1024.0f / 768.0f, 0.01f, 1000.0f));

			merga.get(material<Material::merga_gbuffer>{}, storage_inter, *this);
			merga.get(geometry<Geometry::screen_square>{}, storage_inter, *this);

			auto& ref = merga.create<Material::merga_gbuffer::gbuffer>();
			ref.srv = cast_shader_resource_view(color_bufer);
			ref.ss = create_sample_state();

			auto& transfer_ref = storage_elemnt.create<Property::renderer_3d>();
			transfer_ref.projection = projection;
			transfer_ref.view = view;
			transfer_ref.init(*this);

			auto des = depth_stencil_state::default_dscription;
			des.DepthEnable = TRUE;
			des.DepthFunc = decltype(des.DepthFunc)::D3D11_COMPARISON_LESS;
			des.DepthWriteMask = decltype(des.DepthWriteMask)::D3D11_DEPTH_WRITE_MASK_ALL;
			des.StencilEnable = FALSE;
			defer_dss = create_depth_stencil_state(des);

			des.DepthFunc = decltype(des.DepthFunc)::D3D11_COMPARISON_ALWAYS;
			des.DepthWriteMask = decltype(des.DepthWriteMask)::D3D11_DEPTH_WRITE_MASK_ZERO;
			des.StencilEnable = FALSE;
			merga_dss = create_depth_stencil_state(des);


			merga.init(*this);
		}

		void defer_renderer::pre_tick(duration da)
		{
			simple_renderer::pre_tick(da);
			storage_elemnt.find([this](Property::renderer_3d& r3) {
				r3.view = view;
				r3.projection = projection;
			});
			
			storage_elemnt.update(*this);

			pipeline::clear_render_target(oms, { 0.0, 0.0, 0.0, 1.0 });
			pipeline::clear_depth_stencil(oms, 1.0, 0);

			*this << oms << defer_dss;
			
		}

		void defer_renderer::pos_tick(duration da)
		{
			storage_elemnt.call(renderer_order::Defer, *this, *this);
			unbind();
			storage_elemnt.clear_element();
			pipeline::clear_render_target(om, { 0.0, 0.0, 0.0, 1.0 });
			*this << om << merga_dss;
			merga.direct_call(*this, *this);
			unbind();
		}

		void defer_renderer::push_element(const element& ptr)
		{
			storage_elemnt.push_back(ptr);
		}

	}
}