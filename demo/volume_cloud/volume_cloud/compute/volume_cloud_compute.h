#pragma once
#include "po_dx11_defer_renderer\defer_element.h"
using namespace PO::Dx;
using namespace PO::Dx11;

struct property_random_point_f
{
	std::vector<float> random_vector;
	uint32_t style = 0;
	float4 parameter1;
	float4 parameter2;

	struct renderer_data
	{
		buffer_constant cb;
		shader_resource_view<buffer_structured> srv;
		uint32_t toatl_count = 1;
		uint32_t style;
		float4 parameter1;
		float4 parameter2;
	};

	void create_normal_point(creator& c, uint32_t count, uint32_t seed = 0, float mean = 0.0, float stddev = 1.0);
	void create_uniform_point(creator& c, uint32_t count, uint32_t seed = 0, float min = 0.0, float max = 1.0);
	void update(creator& c, renderer_data& rd);
};

struct property_random_point_f3
{
	std::vector<float3> random_vector;
	uint32_t style = 0;
	float4 parameter1;
	float4 parameter2;

	struct renderer_data
	{
		buffer_constant cb;
		shader_resource_view<buffer_structured> srv;
		uint32_t toatl_count = 1;
		uint32_t style;
		float4 parameter1;
		float4 parameter2;
	};

	void create_normal_point(creator& c, uint32_t count, uint32_t3 seed = { 0, 0, 0 }, float mean = 0.0, float stddev = 1.0);
	void craate_custom(creator& c, uint32_t count, uint32_t3 seed = { 0, 0, 0 });
	void create_uniform_point(creator& c, uint32_t count, uint32_t3 seed = { 0, 0, 0 }, float min = 0.0, float max = 1.0);
	void update(creator& c, renderer_data& rd);
};


class compute_2D_tiled : public compute_resource
{
public:
	struct property
	{
		std::array<unordered_access_view<tex2>, 2> output;
		unordered_access_view<tex3> output2;
		struct renderer_data
		{
			std::array<unordered_access_view<tex2>, 2> output;
			unordered_access_view<tex3> output2;
		};
		property& operator<<(std::array<unordered_access_view<tex2>, 2> t) { output = std::move(t); return *this; }
		property& operator<<(unordered_access_view<tex3> t) { output2 = std::move(t);  return *this; }
		void update(creator& sc, renderer_data& rd) { rd.output = output; rd.output2 = output2; }
	};
	compute_2D_tiled(creator& c) : compute_resource(c, u"compute_2d_tiled.cso") {}
	const element_requirement& requirement()
	{
		return make_element_requirement(
			[](stage_context& sc, property_wrapper_t<property>& p) {
			for (uint32_t i = 0; i < 2; ++i)
				sc.CS() << p.output[i][i];
			sc.CS() << p.output2[2];
			sc << dispatch_call{ 8, 8, 1 };
		}, [](stage_context& sc, property_wrapper_t<property_random_point_f>& rd) {
		 sc.CS() << rd.srv[0];
		 }, [](stage_context& sc, property_wrapper_t<property_random_point_f3>& rd) {
			 sc.CS() << rd.srv[1];
		 }
		 );
	}
};


class compute_3D_tiled : public compute_resource
{
public:
	struct property
	{
		unordered_access_view<tex3> output;
		uint32_t3 output_size = { 0, 0, 0 };
		float Lenght = 10.0f;
		uint32_t count = 100;
		struct renderer_data_append
		{
			buffer_constant bc;
		};
		void update(creator& sc, renderer_data_append& rd) { 
			shader_storage<uint32_t3, float, uint32_t> ss(output_size, Lenght, count);
			rd.bc.create_pod(sc, ss);
		}
	};
	compute_3D_tiled(creator& c) : compute_resource(c, u"compute_3d_tiled.cso") {}
	const element_requirement& requirement()
	{
		return make_element_requirement(
			[](stage_context& sc, property_wrapper_t<property>& p) {
			sc.CS() << p.output[0] << p.bc[0];
			sc << dispatch_call{ p.output_size.x / 32 + (p.output_size.x % 32 == 0 ? 0 : 1) , p.output_size.y / 32 + (p.output_size.y % 32 == 0 ? 0 : 1) , p.output_size.z };
		}, [](stage_context& sc, property_wrapper_t<property_random_point_f3>& rd) {
			sc.CS() << rd.srv[0];
		}
		);
	}
};

class SDF_2dGenerator : public compute_resource
{
public:
	struct property 
	{
		shader_resource_view<tex2> s1;
		unordered_access_view<tex2> t1;
		sample_state::description ss_des = sample_state::default_description;
		uint32_t2 size = {0, 0};
		uint32_t2 input_texture = {0, 0};
		float EdgeValue = 0.5f;
		float DistanceMulity = 4.0f;

		struct renderer_data_append
		{
			sample_state ss;
			buffer_constant bc;
		};
		void set(shader_resource_view<tex2> sv, unordered_access_view<tex2> uv) { s1 = std::move(sv); t1 = std::move(uv); }
		void set(const sample_state::description& sd) { ss_des = sd;}
		void update(creator& c, renderer_data_append& rd)
		{
			rd.ss.create(c, ss_des);
			shader_storage<uint32_t2, uint32_t2, float, float > ss(size, input_texture, EdgeValue, DistanceMulity);
			rd.bc.create_pod(c, ss);
		}
	};
	SDF_2dGenerator(creator& c);
	const element_requirement& requirement();
};

class SDF_3dGenerator : public compute_resource
{
public:
	struct property
	{
		shader_resource_view<tex3> Input;
		unordered_access_view<tex3> InsideTexture;
		unordered_access_view<tex3> OutsideTexture;
		uint32_t3 input_start = { 0, 0, 0 };
		uint32_t3 input_end = { 0, 0, 0 };
		uint32_t3 input_size = {0, 0, 0};
		uint32_t3 output_size = {0, 0, 0};
		uint32_t3 step_add = { 1, 1, 1 };
		float EdgeValue = 0.5f;
		float3 DistanceMulity = float3{ 4, 8, 8 };
		uint32_t final_call = 0;
		struct renderer_data_append
		{
			buffer_constant bc;
		};
		void clear();
		bool next();
		void update(creator& c, renderer_data_append& rda)
		{
			shader_storage<uint32_t3, uint32_t3, uint32_t3, uint32_t3, float, float3, uint32_t> ss(input_start, input_end, input_size, output_size, EdgeValue, DistanceMulity, final_call);
			rda.bc.create_pod(c, ss);
		}
	};
	SDF_3dGenerator(creator& c);
	const element_requirement& requirement();
};