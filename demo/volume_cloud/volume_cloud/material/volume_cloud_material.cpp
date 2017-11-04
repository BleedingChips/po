#include "volume_cloud_material.h"

void property_render_2d_for_3d::update(creator& c, renderer_data& rd)
{
	rd.m_srv = m_srv;
	rd.m_ss = m_ss;
	using type = shader_storage<float3, float3, float3, float>;
	type ss{ min_width, max_width, light, density };
	rd.m_cb.create_pod(c, ss);
}

material_transparent_2d_for_3d_64_without_perlin::material_transparent_2d_for_3d_64_without_perlin(creator& c)
	: material_resource(c, u"volume_cloud_material_transparent_2d_for_3d_64_without_perlin_ps.cso", blend_state::description{
	FALSE, FALSE, D3D11_RENDER_TARGET_BLEND_DESC{ TRUE, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL }
})
{
	depth_stencil_state::description des = depth_stencil_state::default_description;
	des.DepthEnable = FALSE;
	dss.create(c, des);
}

const element_requirement& material_transparent_2d_for_3d_64_without_perlin::requirement() const
{
	return make_element_requirement(
		[](stage_context& sc, property_render_2d_for_3d::renderer_data& pp) {
		sc.PS() << pp.m_cb[3];
	},
		[](stage_context& sc, property_local_transfer::renderer_data& pp) {
		sc.PS() << pp.transfer[4];
	},
		[](stage_context& sc, defer_renderer_default::property_linear_z::renderer_data& pp) {
		sc.PS() << pp.z_buffer[3] << pp.ss[3];
	},
		[](stage_context& sc, property_viewport_transfer::renderer_data& pp) {
		sc.PS() << pp.viewport[5];
	},
		[](stage_context& sc, property_volume_cloud_tex::renderer_data& rd) {
		sc.PS() << rd.BaseShapeTex[0] << rd.BaseShapeSampler[0] << rd.MaskSampler[2] << rd.MaskTex[2] << rd.MoveMaskSampler[1] << rd.MoveMaskTex[1];
	}
	);
}


const element_requirement& in_time_material::requirement() const
{
	return make_element_requirement(
		[](stage_context& sc, property_viewport_transfer::renderer_data& pvt) {
		sc.PS() << pvt.viewport[2];
	}, [](stage_context& sc, data::renderer_data& rd) {
		sc.PS() << rd.bc[0];
	}, [](stage_context& sc, defer_renderer_default::property_linear_z::renderer_data& pp)
	{
		sc.PS() << pp.ss[0] << pp.z_buffer[0];
	}, [](stage_context& sc, property_local_transfer::renderer_data& rd)
	{
		sc.PS() << rd.transfer[1];
	}
		);
}

in_time_material::in_time_material(creator& c) : material_resource(c, u"colume_cloud_material_in_time_generator.cso" , blend_state::description{
	FALSE, FALSE, D3D11_RENDER_TARGET_BLEND_DESC{ TRUE, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL }
}) {}

new_material ::new_material(creator& c) : material_resource(c, u"new_material.cso", blend_state::description{
	FALSE, FALSE, D3D11_RENDER_TARGET_BLEND_DESC{ TRUE, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL }
}) {

	depth_stencil_state::description des = depth_stencil_state::default_description;
	des.DepthEnable = FALSE;
	dss.create(c, des);
}

const element_requirement& new_material::requirement() const
{
	return make_element_requirement(
		[](stage_context& sc, property_viewport_transfer::renderer_data& pvt) {
		sc.PS() << pvt.viewport[2];
	}, [](stage_context& sc, property::renderer_data& rd) {
		sc.PS() << rd.b[0] << rd.BaseShapeTex[0] << rd.ss[0] << rd.BaseShapeTex2[1] << rd.ss[1] << rd.BaseShapeTex22[3] << rd.BaseShapeTex222[4];
	}, [](stage_context& sc, defer_renderer_default::property_linear_z::renderer_data& pp)
	{
		sc.PS() << pp.ss[2] << pp.z_buffer[2];
	}, [](stage_context& sc, property_local_transfer::renderer_data& rd)
	{
		sc.PS() << rd.transfer[1];
	}
	);
}

new_new_material::new_new_material(creator& c) : material_resource(c, u"new_new_material.cso", blend_state::description{
	FALSE, FALSE, D3D11_RENDER_TARGET_BLEND_DESC{ TRUE, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL }
}) {

	depth_stencil_state::description des = depth_stencil_state::default_description;
	des.DepthEnable = FALSE;
	dss.create(c, des);
}

const element_requirement& new_new_material::requirement() const
{
	return make_element_requirement(
		[](stage_context& sc, property_viewport_transfer::renderer_data& pvt) {
		sc.PS() << pvt.viewport[2];
	}, [](stage_context& sc, property::renderer_data& rd) {
		sc.PS() << rd.b[0] << rd.BaseShapeTex[0] << rd.ss[0];
	}, [](stage_context& sc, defer_renderer_default::property_linear_z::renderer_data& pp)
	{
		sc.PS() << pp.ss[2] << pp.z_buffer[2];
	}, [](stage_context& sc, property_local_transfer::renderer_data& rd)
	{
		sc.PS() << rd.transfer[1];
	}
	);
}

new_new_new_material::new_new_new_material(creator& c) : material_resource(c, u"new_new_new_material.cso", blend_state::description{
	FALSE, FALSE, D3D11_RENDER_TARGET_BLEND_DESC{ TRUE, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL }
}) {

	depth_stencil_state::description des = depth_stencil_state::default_description;
	des.DepthEnable = FALSE;
	dss.create(c, des);
}

const element_requirement& new_new_new_material::requirement() const
{
	return make_element_requirement(
		[](stage_context& sc, property_viewport_transfer::renderer_data& pvt) {
		sc.PS() << pvt.viewport[2];
	}, [](stage_context& sc, property::renderer_data& rd) {
		sc.PS() << rd.b[0] << rd.BaseShapeTex[0] << rd.ss[0] << rd.BaseShapeTex2[1];
	}, [](stage_context& sc, defer_renderer_default::property_linear_z::renderer_data& pp)
	{
		sc.PS() << pp.ss[2] << pp.z_buffer[2];
	}, [](stage_context& sc, property_local_transfer::renderer_data& rd)
	{
		sc.PS() << rd.transfer[1];
	}
	);
}