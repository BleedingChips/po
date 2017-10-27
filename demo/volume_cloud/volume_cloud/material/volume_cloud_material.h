#pragma once
#include "po_dx11\dx11_buildin_element.h"
#include "po_dx11\dx11_renderer.h"
using namespace PO::Dx;
using namespace PO::Dx11;


class property_volume_cloud_tex : public property_resource
{
	shader_resource_view<tex2> BaseShapeTex;
	sample_state BaseShapeSampler;

	shader_resource_view<tex2> MoveMaskTex;
	sample_state MoveMaskSampler;

	shader_resource_view<tex2> MaskTex;
	sample_state MaskSampler;

public:

	struct renderer_data
	{
		shader_resource_view<tex2> BaseShapeTex;
		sample_state BaseShapeSampler;

		shader_resource_view<tex2> MoveMaskTex;
		sample_state MoveMaskSampler;

		shader_resource_view<tex2> MaskTex;
		sample_state MaskSampler;
	};

	void set_base_shape(shader_resource_view<tex2> tex, sample_state sampler)
	{
		BaseShapeTex = std::move(tex);
		BaseShapeSampler = std::move(sampler);
		need_update();
	}

	void set_move_mask(shader_resource_view<tex2> tex, sample_state sampler)
	{
		MoveMaskTex = std::move(tex);
		MoveMaskSampler = std::move(sampler);
		need_update();
	}

	void set_mask(shader_resource_view<tex2> tex, sample_state sampler)
	{
		MaskTex = std::move(tex);
		MaskSampler = std::move(sampler);
		need_update();
	}

	void update(creator& c, renderer_data& rd)
	{
		rd.BaseShapeSampler = BaseShapeSampler;
		rd.BaseShapeTex = BaseShapeTex;
		rd.MaskSampler = MaskSampler;
		rd.MaskTex = MaskTex;
		rd.MoveMaskSampler = MoveMaskSampler;
		rd.MoveMaskTex = MoveMaskTex;
	}

};



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
		buffer_constant m_cb;
	};
	void set_option(float3 min_width, float3 max_width, float3 light, float density) 
	{
		this->min_width = min_width;
		this->max_width = max_width;
		this->light = light;
		this->density = density;
		need_update();
	}
	void set_texture(shader_resource_view<tex2> srv,  sample_state ss) 
	{ 
		m_srv = std::move(srv); m_ss = std::move(ss);
		need_update();
	}
	void update(creator& c, renderer_data& rd);
};

class material_transparent_2d_for_3d_64_without_perlin :public material_resource
{
	depth_stencil_state dss;
public:
	material_transparent_2d_for_3d_64_without_perlin(creator& c);
	const element_requirement& requirement() const;
	const depth_stencil_state& replace_depth_stencil_state(const depth_stencil_state&) { 
		return dss; 
	}
};

struct in_time_material : public material_resource
{
	in_time_material(creator& c);

	struct data : property_resource
	{
		float Scale;
		float Multy;
		struct renderer_data
		{
			buffer_constant bc;
		};
		void update(creator& c, renderer_data& rd)
		{
			shader_storage<float, float> ss{ Scale, Multy };
			rd.bc.create_pod(c, ss);
		}
		void set(float s, float m) { Scale = s; Multy = m; need_update(); }
	};
	const element_requirement& requirement() const;
};

struct new_material : public material_resource
{
public:

	struct property : public property_resource
	{
		shader_resource_view<tex3> BaseShapeTex;
		shader_resource_view<tex3> BaseShapeTex2;
		sample_state ss;
		float Density = 1.0;
		float4 Value = { 0.0f, 0.0f, 0.0f, 0.0f };
		struct renderer_data
		{
			shader_resource_view<tex3> BaseShapeTex;
			shader_resource_view<tex3> BaseShapeTex2;
			sample_state ss;
			buffer_constant b;
		};
		void set(shader_resource_view<tex3> t, shader_resource_view<tex3> t2, sample_state de = sample_state{}) {
			if (false)
			{
				sample_state::description{
					D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_MIRROR, D3D11_TEXTURE_ADDRESS_MIRROR, D3D11_TEXTURE_ADDRESS_MIRROR, 0.0f, 1,
					D3D11_COMPARISON_NEVER,{ 1.0f,1.0f,1.0f,1.0f }, -FLT_MAX, FLT_MAX };
			}
			ss = de;
			BaseShapeTex2 = std::move(t2);
			BaseShapeTex = std::move(t); 
			need_update(); 
		}
		void set(float D, float4 V = float4{ 1.0, 1.0,1.0,1.0 }) { Density = D;  Value = V; need_update(); }
		void update(creator& c, renderer_data& rd)
		{
			rd.BaseShapeTex = BaseShapeTex;
			rd.BaseShapeTex2 = BaseShapeTex2;
			rd.ss = ss;
			shader_storage<float, float4> ss(Density, Value);
			rd.b.create_pod(c, ss);
		}
	};

	new_material(creator& c);
	const element_requirement& requirement() const;
	depth_stencil_state dss;
	const depth_stencil_state& replace_depth_stencil_state(const depth_stencil_state&) {
		return dss;
	}
};