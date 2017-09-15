#pragma once
#include "po_dx11_defer_renderer\defer_element.h"
using namespace PO::Dx;
using namespace PO::Dx11;

class property_worley_noise_3d_point : public property_resource
{
	buffer_constant m_cb;
public:

	struct renderer_data
	{
		buffer_constant m_cb;
	};

	void set_seed(creator& c, uint32_t3 s);
	void update(creator& rd, renderer_data& sc);
};

class property_output_tex2 : public property_resource
{
	unordered_access_view<tex2> m_uav;
	uint32_t2 tex_size;
	buffer_constant m_cb;

public:

	struct renderer_data
	{
		unordered_access_view<tex2> m_uav;
		uint32_t2 tex_size;
		buffer_constant m_cb;
	};

	void set_texture(creator& c, const tex2& texture, float step, uint32_t4 simulate = uint32_t4{0, 0, 0, 0});
	void update(creator& rd, renderer_data& sc)
	{
		sc.m_uav = m_uav;
		sc.tex_size = tex_size;
		sc.m_cb = m_cb;
	}

};

class compute_worley_noise_tex2_3d : public compute_resource
{
public:
	compute_worley_noise_tex2_3d(creator&);
	const element_requirement& requirement() const;
};

class property_perline_worley_noise_3d_point : public property_resource
{
	buffer_constant m_cb;
public:

	struct renderer_data
	{
		buffer_constant m_cb;
	};

	void set_seed(creator& c, uint32_t3 seed);
	void update(creator& c, renderer_data& rd) { rd.m_cb = m_cb; }
};

class compute_perlin_worley_noise_tex2_3d : public compute_resource
{
public:
	compute_perlin_worley_noise_tex2_3d(creator& c);
	const element_requirement& requirement() const;
};

struct property_random_point_f : public property_resource
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

struct property_random_point_f3 : public property_resource
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
	void create_uniform_point(creator& c, uint32_t count, uint32_t3 seed = { 0, 0, 0 }, float min = 0.0, float max = 1.0);
	void update(creator& c, renderer_data& rd);
};

struct property_output_tex2_2d_simulate_3d : public property_resource
{
	uint32_t2 texture_size;
	uint32_t4 simulate_size;
	unordered_access_view<tex2> output_texture_uav;
	struct renderer_data
	{
		uint32_t2 texture_size;
		unordered_access_view<tex2> output_texture_uav;
		buffer_constant cb;
	};
	void set_output_texture(unordered_access_view<tex2> output_texture, uint32_t2 texture_size, uint32_t4 simulate_size);
	void update(creator& c, renderer_data& rd);
};

class compute_generate_perlin_noise_uint8_4_2d_simulate_3d : public compute_resource
{
public:
	static uint32_t max_count();
	compute_generate_perlin_noise_uint8_4_2d_simulate_3d(creator& c);
	const element_requirement& requirement() const;
};

class property_generate_worley_noise_float4_2d_simulate_3d : public property_resource
{
	unordered_access_view<tex2> output_texture_uav;
	float radius = 0.01f;
	uint32_t2 texture_size;
	uint32_t4 simulate_size;
public:
	struct renderer_data
	{
		unordered_access_view<tex2> output_texture_uav;
		buffer_constant cb;
		uint32_t2 texture_size;
	};
	void set_peorperty(unordered_access_view<tex2> output_texture, uint32_t2 texture_size, uint32_t4 simulate_size, float radio = 1.0f);
	void update(creator& c, renderer_data& rd);
};

class compute_generate_worley_noise_float4_2d_simulate_3d : public compute_resource
{
public:
	static uint32_t max_count();
	compute_generate_worley_noise_float4_2d_simulate_3d(creator& c);
	const element_requirement& requirement() const;
};

struct property_merga_noise_float4_2d_simulate_3d : public property_resource
{
	uint32_t2 texture_size;
	shader_resource_view<tex2> noise_1;
	shader_resource_view<tex2> noise_2;
	unordered_access_view<tex2> output;
	struct renderer_data
	{
		uint32_t2 texture_size;
		shader_resource_view<tex2> noise_1;
		shader_resource_view<tex2> noise_2;
		unordered_access_view<tex2> output;
		buffer_constant cb;
	};
	void set_output_texture_uint8(unordered_access_view<tex2> output, uint32_t2 texture_size) { this->output = std::move(output); this->texture_size = texture_size; need_update(); }
	void set_input_noise(shader_resource_view<tex2> noise1, shader_resource_view<tex2> noise2) { this->noise_1 = std::move(noise1); this->noise_2 = std::move(noise2); need_update();}
	void update(creator& c, renderer_data& rd);
};

class compute_merga_noise_float4_2d_simulate_3d : public compute_resource
{
public:
	compute_merga_noise_float4_2d_simulate_3d(creator& c);
	const element_requirement& requirement() const;
};

class compute_generate_cube_mark_2d_simulate_3d : public compute_resource
{
public:
	compute_generate_cube_mark_2d_simulate_3d(creator& c);
	const element_requirement& requirement() const;

	struct property : public property_resource
	{
		uint32_t2 texture_size;
		uint32_t4 simulate_size;
		float3 cube_min;
		float3 cube_max;
		unordered_access_view<tex2> texturef;
		struct renderer_data
		{
			uint32_t2 texture_size;
			unordered_access_view<tex2> texture_f;
			buffer_constant cb;
		};

		void update(creator& c, renderer_data& rd);
		void set_output_texture_f(unordered_access_view<tex2> uac, uint32_t2 texture_size, uint32_t4 simulate_size, float3 min, float3 max)
		{
			texturef = std::move(uac);
			this->texture_size = texture_size;
			this->simulate_size = simulate_size;
			cube_min = min;
			cube_max = max;
			need_update();
		}
	};
};

class compute_merga_4_f1_to_f4 : public compute_resource
{
public:
	compute_merga_4_f1_to_f4(creator& c);
	const element_requirement& requirement() const;
	struct property : property_resource
	{
		uint32_t2 texture_size;
		float4 fator = float4{ 1.0, 1.0, 1.0, 1.0 };
		float4 shift = float4{0.0, 0.0, 0.0, 0.0};
		shader_resource_view<tex2> input1;
		shader_resource_view<tex2> input2;
		shader_resource_view<tex2> input3;
		shader_resource_view<tex2> input4;
		unordered_access_view<tex2> output;
		struct renderer_data
		{
			uint32_t2 texture_size;
			shader_resource_view<tex2> input1;
			shader_resource_view<tex2> input2;
			shader_resource_view<tex2> input3;
			shader_resource_view<tex2> input4;
			unordered_access_view<tex2> output;
			buffer_constant cb;
		};
		void update(creator& sc, renderer_data& rd);
		void set_texture(unordered_access_view<tex2> out, uint32_t2 texture_size, shader_resource_view<tex2> i1, shader_resource_view<tex2> i2, shader_resource_view<tex2> i3, shader_resource_view<tex2> i4)
		{
			input1 = std::move(i1);
			input2 = std::move(i2);
			input3 = std::move(i3);
			input4 = std::move(i4);
			output = std::move(out);
			need_update();
		}
		void set_property(float4 fator = { 1.0, 1.0, 1.0, 1.0 }, float4 shift = float4{ 0.0, 0.0, 0.0, 0.0 })
		{
			this->fator = fator;
			this->shift = shift;
			need_update();
		}
	};
};

//struct property_resource
/*
class compute_generate_perlin_noise_float_2d : public compute_resource
{
public:
	uint32_t point_count();
	compute_generate_perlin_noise_float_2d(creator& c);
};
*/