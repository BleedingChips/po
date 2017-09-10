#pragma once
#include "po_dx11_defer_renderer\defer_element.h"
using namespace PO::Dx;
using namespace PO::Dx11;

class property_worley_noise_3d_point : public property_resource
{
	constant_buffer m_cb;
public:

	struct renderer_data
	{
		constant_buffer m_cb;
	};

	void set_seed(creator& c, uint32_t3 s);
	void update(creator& rd, renderer_data& sc);
};

class property_output_tex2 : public property_resource
{
	unordered_access_view<tex2> m_uav;
	uint32_t2 tex_size;
	constant_buffer m_cb;

public:

	struct renderer_data
	{
		unordered_access_view<tex2> m_uav;
		uint32_t2 tex_size;
		constant_buffer m_cb;
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
	constant_buffer m_cb;
public:

	struct renderer_data
	{
		constant_buffer m_cb;
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

struct property_output_texture_2d_simulate_3d : public property_resource
{
	unordered_access_view<tex2> uav;
	constant_buffer cb;
	uint32_t2 texture_size;

	struct renderer_data
	{
		constant_buffer cb;
		unordered_access_view<tex2> uav;
		uint32_t2 texture_size;
	};

	void set_output_texture_rgba_u8(
		creator& c,
		unordered_access_view<tex2> uav,
		uint32_t2 texture_size,
		uint32_t4 simulate_size,
		float4 mark = float4(1.0f, 1.0f, 1.0f, 1.0f),
		float4 blend = float4(0.0f, 0.0f, 0.0f, 0.0f)
	);
	void update(creator& c, renderer_data& rd)
	{
		rd.uav = uav;
		rd.cb = cb;
		rd.texture_size = texture_size;
	}
};

struct property_random_point_f : public property_resource
{
	std::vector<float> random_vector;
	// 数量，类型，参数1, 参数2
	uint32_t toatl_count = 0;
	shader_resource_view<structured_buffer> srv;
	constant_buffer cb;

	struct renderer_data
	{
		constant_buffer cb;
		shader_resource_view<structured_buffer> srv;
		uint32_t toatl_count = 1;
	};

	void create_normal_point(creator& c, uint32_t count, uint32_t seed = 0, float mean = 0.0, float stddev = 1.0);
	void create_uniform_point(creator& c, uint32_t count, uint32_t seed = 0, float min = 0.0, float max = 1.0);
	void update(creator& c, renderer_data& rd)
	{
		rd.cb = cb;
		rd.srv = srv;
		rd.toatl_count = toatl_count;
	}
};

class compute_perlin_noise_for_2d_rbga_uint8 : public compute_resource
{
public:
	static uint32_t max_count();
	compute_perlin_noise_for_2d_rbga_uint8(creator& c);
	const element_requirement& requirement() const;
};

struct property_worley_noise_setting
{

	//struct 

};

class compute_worley_noise_for_2d_rgba_uint8
{
public:

};
