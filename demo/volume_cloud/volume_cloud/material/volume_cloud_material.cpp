#include "volume_cloud_material.h"

void property_render_2d_for_3d::push(property_render_2d_for_3d& prf, creator& c)
{
	prf.m_srv = m_srv;
	prf.m_ss = m_ss;
	if (data_neeed_update)
	{
		data_neeed_update = false;
		prf.set_option(min_width, max_width, light, density);
	}
}

void property_render_2d_for_3d::update(renderer_data& rd, stage_context& sc)
{
	rd.m_srv = m_srv;
	rd.m_ss = m_ss;
	if (!rd.m_cb || data_neeed_update)
	{
		data_neeed_update = false;
		using type = shader_storage<float3, float3, float3, float>;
		type ss{ min_width, max_width, light, density };
		if (!rd.m_cb)
			rd.m_cb.create_pod(sc, ss, true);
		else
			sc.write_constant_buffer(rd.m_cb, [&](void* da) { 
			*static_cast<type*>(da) = ss;
		});
	}
}

const char16_t* material_transparent_render_2d_for_3d_64::material_shader_patch_ps()
{
	return u"volume_cloud_material_transparent_render_2d_for_3d_64_ps.cso";
}

const std::set<std::type_index>& material_transparent_render_2d_for_3d_64::material_requirement()
{
	return make_property_info_set < property_render_2d_for_3d, property_local_transfer, property_gbuffer_default, property_viewport_transfer >{};
}

bool material_transparent_render_2d_for_3d_64::material_update(stage_context& sc, property_interface& pi)
{
	return pi.cast([&](property_render_2d_for_3d::renderer_data& pp) {
		sc.PS() << pp.m_srv[0] << pp.m_cb[0];
	}) || pi.cast([&](property_local_transfer::renderer_data& pp) {
		sc.PS() << pp.transfer[1];
	}) || pi.cast([&](property_gbuffer_default::renderer_data& pp) {
		sc.PS() << pp.linear_z[1] << pp.ss[0];
	}) || pi.cast([&](property_viewport_transfer::renderer_data& pp) {
		sc.PS()<< pp.viewport[2];
	});
}

const char16_t* material_transparent_2d_for_3d_64_without_perlin::material_shader_patch_ps()
{
	return u"volume_cloud_material_transparent_2d_for_3d_64_without_perlin_ps.cso";
}

const std::set<std::type_index>& material_transparent_2d_for_3d_64_without_perlin::material_requirement()
{
	return make_property_info_set<property_render_2d_for_3d, property_local_transfer, property_gbuffer_default, property_viewport_transfer>{};
}

bool material_transparent_2d_for_3d_64_without_perlin::material_update(stage_context& sc, property_interface& pi)
{
	return pi.cast([&](property_render_2d_for_3d::renderer_data& pp) {
		sc.PS() << pp.m_srv[0] << pp.m_cb[0] << pp.m_ss[0];
	}) || pi.cast([&](property_local_transfer::renderer_data& pp) {
		sc.PS() << pp.transfer[1];
	}) || pi.cast([&](property_gbuffer_default::renderer_data& pp) {
		sc.PS() << pp.linear_z[1] << pp.ss[1];
	}) || pi.cast([&](property_viewport_transfer::renderer_data& pp) {
		sc.PS() << pp.viewport[2];
	});
}