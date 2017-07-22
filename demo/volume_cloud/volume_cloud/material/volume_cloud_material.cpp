#include "volume_cloud_material.h"
#include "po_dx11_defer_renderer\build_in_element.h"
property_render_2d_for_3d::property_render_2d_for_3d() : property_interface(typeid(property_render_2d_for_3d)) {}
void property_render_2d_for_3d::set_option(creator& c, float3 min_width, float3 max_width, float3 light, float density)
{
	if (!m_cb)
	{
		shader_storage<float3, float3, float3, float> ss{ min_width, max_width, light, density };
		m_cb = c.create_constant_buffer(&ss, true);
	}
	else {
		update_function = [min_width, max_width, light, density, this](pipeline& p) {
			shader_storage<float3, float3, float3, float> ss{ min_width, max_width, light, density };
			p.write_constant_buffer(m_cb, [&](void* data) {
				*static_cast<shader_storage<float3, float3, float3, float>*>(data) = ss;
			});
		};
	}
}

material_transparent_render_2d_for_3d_64::material_transparent_render_2d_for_3d_64() : defer_material_interface(typeid(material_transparent_render_2d_for_3d_64), render_order::Transparent){}
auto material_transparent_render_2d_for_3d_64::acceptance() const -> const acceptance_t&
{
	return make_acceptance<property_render_2d_for_3d, property_transfer, property_gbuffer, property_screen>{};
}

void material_transparent_render_2d_for_3d_64::init(creator& c)
{
	if (!load_ps(u"volume_cloud_material_transparent_render_2d_for_3d_64_ps.cso", c)) throw 1;
	stage_bs = c.create_blend_state(s_alpha_to_inv_s_alpha);
}

bool material_transparent_render_2d_for_3d_64::update(property_interface& pi, pipeline& p)
{
	return pi.cast([this](property_render_2d_for_3d& pp) {
		stage_ps << pp.srv()[0] << pp.cb()[0];
	}) || pi.cast([this](property_transfer& pp) {
		stage_ps << pp.cb()[1];
	}) || pi.cast([this](property_gbuffer& pp) {
		stage_ps << pp.linearization_z()[1] << pp.ss()[0];
	}) || pi.cast([this](property_screen& pp) {
		stage_ps << pp.cb()[2];
	});
}