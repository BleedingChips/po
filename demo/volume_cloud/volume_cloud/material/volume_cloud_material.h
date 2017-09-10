#pragma once
#include "po_dx11\dx11_buildin_element.h"
#include "po_dx11\dx11_renderer.h"
using namespace PO::Dx;
using namespace PO::Dx11;
class property_render_2d_for_3d : public property_resource
{
	float3 min_width, max_width, light;
	float density;
	shader_resource_view<tex2> m_srv;
	sample_state m_ss;

public:
	
	struct renderer_data
	{
		shader_resource_view<tex2> m_srv;
		sample_state m_ss;
		constant_buffer m_cb;
	};


	void set_option(float3 min_width, float3 max_width, float3 light, float density) 
	{
		this->min_width = min_width;
		this->max_width = max_width;
		this->light = light;
		this->density = density;
		need_update();
	}
	void set_texture(creator& c, const tex2& t, sample_state::description de = sample_state::default_description) 
	{ 
		m_srv = t.cast_shader_resource_view(c); m_ss.create(c, de);;
		need_update();
	}
	void update(creator& c, renderer_data& rd);
};

class material_transparent_render_2d_for_3d_64 :public material_transparent_resource
{
public:
	material_transparent_render_2d_for_3d_64(creator&);
	const element_requirement& requirement() const;
};

class material_transparent_2d_for_3d_64_without_perlin :public material_transparent_resource
{
public:
	material_transparent_2d_for_3d_64_without_perlin(creator& c);
	const element_requirement& requirement() const;
};