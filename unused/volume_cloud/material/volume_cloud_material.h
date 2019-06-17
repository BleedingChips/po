#pragma once
#include "po_dx11\dx11_buildin_element.h"
#include "po_dx11\dx11_renderer.h"
using namespace PO::Dx;
using namespace PO::Dx11;
using namespace PO;

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
		buffer_constant debug;
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

struct mateballshower : public material_resource
{
	depth_stencil_state dss;
public:
	struct property
	{
		float4 value;
		struct renderer_data
		{
			buffer_constant bc;
		};
		void update(creator& c, renderer_data& rd)
		{
			shader_storage<float4> ss(value);
			rd.bc.create_pod(c, ss);
		}
	};
	mateballshower(creator& c);
	const element_requirement& requirement() const;
	const depth_stencil_state& replace_depth_stencil_state(const depth_stencil_state&) {
		return dss;
	}
};

struct DepthShadow : public material_resource
{
	depth_stencil_state dss;
public:
	struct property
	{
		shader_resource_view<tex2> DensityMap;
		shader_resource_view<tex2> DetailMap;
		sample_state::description ss_des = sample_state::default_description;
		sample_state::description ss_des2 = sample_state::default_description;
		uint4 DensitySimulateSize = { 1, 1, 1, 1 };
		uint4 DetailSimulateSize = { 1, 1, 1, 1 };
		struct renderer_data_append
		{
			sample_state ss;
			sample_state ss2;
			buffer_constant bc;
		};
		void update(creator& c, renderer_data_append& rd)
		{
			rd.ss.create(c, ss_des);
			rd.ss2.create(c, ss_des2);
			shader_storage<uint4, uint4> ss{ DensitySimulateSize , DetailSimulateSize };
			rd.bc.create_pod(c, ss);
		}
	};
	DepthShadow(creator& c);
	const element_requirement& requirement() const;
	const depth_stencil_state& replace_depth_stencil_state(const depth_stencil_state&) {
		return dss;
	}
};

struct Tex3dSimulateDebuger : public material_resource
{
	depth_stencil_state dss;
public:
	struct property
	{
		shader_resource_view<tex3> Target;
		shader_resource_view<tex2> Simulate;
		shader_resource_view<tex2> Demo;
		sample_state::description ss_des = sample_state::default_description;
		float4 InputValue;
		struct renderer_data_append
		{
			sample_state ss;
			buffer_constant bc;
		};
		void update(creator& c, renderer_data_append& rd)
		{
			rd.ss.create(c, ss_des);
			shader_storage<float4> ss(InputValue);
			rd.bc.create_pod(c, ss);
		}
	};
	Tex3dSimulateDebuger(creator& c);
	const element_requirement& requirement() const;
	const depth_stencil_state& replace_depth_stencil_state(const depth_stencil_state&) {
		return dss;
	}
};

struct AdvantureRenderer : public material_resource
{
	depth_stencil_state dss;
public:
	struct property
	{
		shader_resource_view<tex2> DensityMap;
		shader_resource_view<tex2> DetailMap;
		sample_state::description ss_des = sample_state::default_description;
		sample_state::description ss_des2 = sample_state::default_description;
		uint4 DensitySimulateSize = { 1, 1, 1, 1 };
		uint4 DetailSimulateSize = { 1, 1, 1, 1 };
		struct renderer_data_append
		{
			sample_state ss;
			sample_state ss2;
			buffer_constant bc;
		};
		void update(creator& c, renderer_data_append& rd)
		{
			rd.ss.create(c, ss_des);
			rd.ss2.create(c, ss_des2);
			shader_storage<uint4, uint4> ss{ DensitySimulateSize , DetailSimulateSize };
			rd.bc.create_pod(c, ss);
		}
	};
	AdvantureRenderer(creator& c);
	const element_requirement& requirement() const;
	const depth_stencil_state& replace_depth_stencil_state(const depth_stencil_state&) {
		return dss;
	}
};
