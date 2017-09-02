#pragma once
#include "po_dx11\dx11_buildin_element.h"
#include "po_dx11\dx11_renderer.h"
using namespace PO::Dx;
using namespace PO::Dx11;
class property_render_2d_for_3d
{
	
	bool data_neeed_update = true;

	float3 min_width, max_width, light;
	float density;
	shader_resource_view m_srv;
	sample_state m_ss;

public:
	
	struct renderer_data
	{
		shader_resource_view m_srv;
		sample_state m_ss;
		constant_buffer m_cb;
	};


	void set_option(float3 min_width, float3 max_width, float3 light, float density) 
	{
		this->min_width = min_width;
		this->max_width = max_width;
		this->light = light;
		this->density = density;
		data_neeed_update = true;
	}
	void set_texture(creator& c, const tex2& t, sample_state::description de = sample_state::default_description) 
	{ 
		m_srv = c.cast_shader_resource_view(t); m_ss = c.create_sample_state(de); 
	}

	void push(property_render_2d_for_3d& prf, creator& c);
	void update(renderer_data& rd, stage_context& sc);
};

class material_transparent_render_2d_for_3d_64 :public  material_default
{
public:
	static std::type_index pipeline_id() { return typeid(pipeline_transparent_default); }
	material_transparent_render_2d_for_3d_64(creator&) {}
	static const char16_t* material_shader_patch_ps();
	static const std::set<std::type_index>& material_requirement();
	static bool material_update(stage_context& sc, property_interface& pi);
};

class material_transparent_2d_for_3d_64_without_perlin :public material_default
{
public:
	static std::type_index pipeline_id() { return typeid(pipeline_transparent_default); }
	material_transparent_2d_for_3d_64_without_perlin(creator& c) {}
	static const char16_t* material_shader_patch_ps();
	static const std::set<std::type_index>& material_requirement();
	static bool material_update(stage_context& sc, property_interface& pi);
};