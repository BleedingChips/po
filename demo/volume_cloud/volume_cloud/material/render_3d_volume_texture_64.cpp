#include "render_3d_volume_texture_64.h"
#include "po_dx11_defer_renderer\element\property.h"
/*
namespace Implement
{
	render_3d_volume_texture_64_property::render_3d_volume_texture_64_property() : property_constructor(typeid(render_3d_volume_texture_64_property)) {}

	void render_3d_volume_texture_64_property::set_light(float3 t)
	{
		light = t;
	}

	void render_3d_volume_texture_64_property::push(creator& c)
	{
		aligned_storage<float3, float, float3, float3> bp{ light, max_density, min_size, max_size };

		if (!cb)
		{
			cb = c.create_buffer_constant(&bp, true);
			if (need_update)
				need_update = false;
		}

		if (texture)
		{
			srv = c.cast_shader_resource_view(texture);
			texture = tex3{};
		}

		if (!ss)
		{
			auto des = sample_state::default_description;
			des.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			des.AddressU = decltype(des.AddressU)::D3D11_TEXTURE_ADDRESS_MIRROR;
			des.AddressV = decltype(des.AddressV)::D3D11_TEXTURE_ADDRESS_MIRROR;
			des.AddressW = decltype(des.AddressW)::D3D11_TEXTURE_ADDRESS_MIRROR;
			ss = c.create_sample_state(des);
		}
	}

	void render_3d_volume_texture_64_property::update(pipeline& p)
	{
		if (texture)
		{
			srv = p.get_creator().cast_shader_resource_view(texture);
			texture = tex3{};
		}
		if (need_update)
		{
			aligned_storage<float3, float, float3, float3> bp{ light, max_density, min_size, max_size };
			p.write_buffer_constant(cb, [&](void *data) {
				*reinterpret_cast<std::decay_t<decltype(bp)>*> (data) = (bp);
			});
			need_update = false;
		}
			
	}
}

render_3d_volume_texture_64::render_3d_volume_texture_64() : material_interface(typeid(render_3d_volume_texture_64), render_order::Post) {}

const std::set<std::type_index>& render_3d_volume_texture_64::acceptance() const
{
	static const std::set<std::type_index>& acc = {typeid(property), typeid(Property::transfer_3d_static), typeid(Property::renderer_3d)};
	return acc;
}

bool render_3d_volume_texture_64::update(property_constructor& pi, pipeline& p)
{
	return pi.cast([this](property& po) {
		ps << po.srv[0] << po.ss[0] << po.cb[2];
	}) || pi.cast([this](Property::transfer_3d_static& ts) {
		ps << ts.cb[1];
	}) || pi.cast([this](Property::renderer_3d& r3) {
		ps << r3.cb[0];
	});
}

void render_3d_volume_texture_64::init(creator& c)
{
	if (!load_ps(u"render_3d_volume_texture_64_ps.cso", c)) throw 1;
	auto des = blend_state::default_description;
	des.RenderTarget[0].BlendEnable = TRUE;
	des.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	des.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	des.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	bs = c.create_blend_state(des);
}
*/

