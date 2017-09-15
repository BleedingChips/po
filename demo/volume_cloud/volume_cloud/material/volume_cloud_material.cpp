#include "volume_cloud_material.h"

void property_render_2d_for_3d::update(creator& c, renderer_data& rd)
{
	rd.m_srv = m_srv;
	rd.m_ss = m_ss;
	using type = shader_storage<float3, float3, float3, float>;
	type ss{ min_width, max_width, light, density };
	rd.m_cb.create_pod(c, ss);
}

material_transparent_render_2d_for_3d_64::material_transparent_render_2d_for_3d_64(creator& c)
	:material_transparent_resource(c, u"volume_cloud_material_transparent_render_2d_for_3d_64_ps.cso")
{}

const element_requirement& material_transparent_render_2d_for_3d_64::requirement() const
{
	return make_element_requirement(
		[](stage_context& sc, property_render_2d_for_3d::renderer_data& pp) {
		sc.PS() << pp.m_srv[0] << pp.m_cb[0];
	},
		[](stage_context& sc, property_local_transfer::renderer_data& pp) {
		sc.PS() << pp.transfer[1];
	},
		[](stage_context& sc, property_viewport_transfer::renderer_data& pp) {
		sc.PS() << pp.viewport[2];
	},
		[](stage_context& sc, property_gbuffer_default::renderer_data& pp) {
		sc.PS() << pp.linear_z[1] << pp.ss[0];
	}
	);
}

material_transparent_2d_for_3d_64_without_perlin::material_transparent_2d_for_3d_64_without_perlin(creator& c)
	: material_transparent_resource(c, u"volume_cloud_material_transparent_2d_for_3d_64_without_perlin_ps.cso", blend_state::description{
	FALSE, FALSE, D3D11_RENDER_TARGET_BLEND_DESC{ TRUE, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL }
})
{}

const element_requirement& material_transparent_2d_for_3d_64_without_perlin::requirement() const
{
	return make_element_requirement(
		[](stage_context& sc, property_render_2d_for_3d::renderer_data& pp) {
		sc.PS() << pp.m_srv[0] << pp.m_cb[0] << pp.m_ss[0];
	},
		[](stage_context& sc, property_local_transfer::renderer_data& pp) {
		sc.PS() << pp.transfer[1];
	},
		[](stage_context& sc, property_gbuffer_default::renderer_data& pp) {
		sc.PS() << pp.linear_z[1] << pp.ss[1];
	},
		[](stage_context& sc, property_viewport_transfer::renderer_data& pp) {
		sc.PS() << pp.viewport[2];
	}
	);
}