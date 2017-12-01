#pragma once
#include "po_dx11\dx11_buildin_element.h"
#include "po_dx11\dx11_renderer.h"
using namespace PO::Dx;
using namespace PO::Dx11;

struct property_volumecloud_debug_value
{
	float4 InputValue = { 0.0, 0.0, 0.0, 0.0 };
	float Density = 0.0;
	uint32_t3 XYZSizeOfCube = { 1, 1, 1 };
	struct renderer_data
	{
		buffer_constant bc;
	};
	void update(creator& c, renderer_data& rd)
	{
		shader_storage<float4, float, uint32_t3> ss{ InputValue ,Density, XYZSizeOfCube };
		rd.bc.create_pod(c, ss);
	}
};

struct HeightMap2D : public material_resource
{
public:
	struct property
	{
		shader_resource_view<tex2> BaseShapeTex;
		sample_state::description ss_des = sample_state::default_description;
		struct renderer_data_append
		{
			sample_state ss;
		};

		void update(creator& c, renderer_data_append& rd)
		{
			rd.ss.create(c, ss_des);
		}
	};

	HeightMap2D(creator& c);
	const element_requirement& requirement() const;
	depth_stencil_state dss;
	const depth_stencil_state& replace_depth_stencil_state(const depth_stencil_state&) {
		return dss;
	}
};

struct SignedDistanceField3D : public material_resource
{
	depth_stencil_state dss;
public:
	struct property
	{
		shader_resource_view<tex3> tex;
		shader_resource_view<tex2> tex_dif;
		sample_state::description ss_des = sample_state::default_description;
		struct renderer_data_append
		{
			sample_state ss;
		};
		void update(creator& c, renderer_data_append& rd)
		{
			rd.ss.create(c, ss_des);
		}
	};
	SignedDistanceField3D(creator& c);
	const element_requirement& requirement() const;
	const depth_stencil_state& replace_depth_stencil_state(const depth_stencil_state&) {
		return dss;
	}
};

struct Density3DEdge2DDensity2D : public material_resource
{
	depth_stencil_state dss;
public:
	struct property
	{
		shader_resource_view<tex2> Edge;
		shader_resource_view<tex2> Height;
		shader_resource_view<tex3> DensityMap;
		sample_state::description ss_des = sample_state::default_description;
		struct renderer_data_append
		{
			sample_state ss;
		};
		void update(creator& c, renderer_data_append& rd)
		{
			rd.ss.create(c, ss_des);
		}
	};
	Density3DEdge2DDensity2D(creator& c);
	const element_requirement& requirement() const;
	const depth_stencil_state& replace_depth_stencil_state(const depth_stencil_state&) {
		return dss;
	}
};

struct DensityMap3D : public material_resource
{
	depth_stencil_state dss;
public:
	struct property
	{
		shader_resource_view<tex3> DensityMap;
		sample_state::description ss_des = sample_state::default_description;
		struct renderer_data_append
		{
			sample_state ss;
		};
		void update(creator& c, renderer_data_append& rd)
		{
			rd.ss.create(c, ss_des);
		}
	};
	DensityMap3D(creator& c);
	const element_requirement& requirement() const;
	const depth_stencil_state& replace_depth_stencil_state(const depth_stencil_state&) {
		return dss;
	}
};