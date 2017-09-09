#pragma once
#include "po_dx11_defer_renderer\defer_element.h"
using namespace PO::Dx;
using namespace PO::Dx11;

class property_worley_noise_3d_point
{
	uint32_t3 seed = { 1, 2, 3 };
	bool need_update = true;
public:

	struct renderer_data
	{
		constant_buffer m_cb;
	};

	void set_seed(uint32_t3 s) { seed = s; need_update = true; }
	void push(property_worley_noise_3d_point& pwnp, creator& c)
	{
		if (need_update)
		{
			pwnp.set_seed(seed);
			need_update = false;
		}
	}

	void update(renderer_data& rd, stage_context& sc);
};

class property_output_tex2
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
	void push(property_output_tex2& pot, creator&)
	{
		pot = *this;
	}

	void update(renderer_data& rd, stage_context& sc)
	{
		rd.m_uav = m_uav;
		rd.tex_size = tex_size;
		rd.m_cb = m_cb;
	}

};

class compute_worley_noise_tex2_3d
{
public:
	compute_worley_noise_tex2_3d(creator&) {}
	static const char16_t* compute_shader_patch_cs();
	static const std::set<std::type_index>& compute_requirement();
	static void compute_apply() { }
	static bool compute_update(stage_context& sc, property_interface& pi);
};

class property_perline_worley_noise_3d_point
{
	uint32_t3 seed = { 1, 2, 3 };
	bool need_update = true;
public:

	struct renderer_data
	{
		constant_buffer m_cb;
	};

	void set_seed(uint32_t3 s) { seed = s; need_update = true; }
	void push(property_perline_worley_noise_3d_point& pwnp, creator& c)
	{
		if (need_update)
		{
			pwnp.set_seed(seed);
			need_update = false;
		}
	}

	void update(renderer_data& rd, stage_context& sc);
};

class compute_perlin_worley_noise_tex2_3d
{
public:
	compute_perlin_worley_noise_tex2_3d(creator& c) {}
	static const char16_t* compute_shader_patch_cs();
	static const std::set<std::type_index>& compute_requirement();
	static void compute_apply(stage_context& sc) {};
	static bool compute_update(stage_context& sc, property_interface&);
};

struct property_output_texture_2d_simulate_3d
{
	unordered_access_view<tex2> uav;
	uint32_t2 texture_size = uint32_t2{ 1, 1};
	uint32_t4 simulate_size = uint32_t4{ 1, 1, 1, 1 };
	float4 mark = float4(0.0, 0.0, 0.0, 0.0);
	float4 blend = float4(1.0, 1.0, 1.0, 1.0);
	bool need_update = true;


	struct renderer_data
	{
		constant_buffer cb;
		unordered_access_view<tex2> uav;
		uint32_t2 texture_size;
	};

	property_output_texture_2d_simulate_3d& set_mark_blend(float4 ma, float4 bl);
	property_output_texture_2d_simulate_3d& set_texture(creator& c, const tex2& te, const uint32_t4& simulate);
	void push(property_output_texture_2d_simulate_3d& pot, creator& c);
	void update(renderer_data& rd, stage_context& sc);
};

struct property_random_point_f
{
	std::vector<float> random_vector;
	// 数量，类型，参数1, 参数2
	uint32_t toatl_count = 0;
	using cb_type = property_contanst_buffer_write_able<uint32_t, uint32_t, float4, float4>;
	cb_type data;
	shader_resource_view<structured_buffer> srv;

	struct renderer_data
	{
		constant_buffer cb;
		shader_resource_view<structured_buffer> srv;
		uint32_t toatl_count = 1;
	};

	void create_normal_point(creator& c, uint32_t count, uint32_t seed = 0, float mean = 0.0, float stddev = 1.0);
	void create_uniform_point(creator& c, uint32_t count, uint32_t seed = 0, float min = 0.0, float max = 1.0);
	void push(property_random_point_f& ppn, creator& c);
	void update(renderer_data& rd, stage_context& sc);
};

class compute_perlin_noise_for_2d_rbga_uint8
{
public:
	static uint32_t max_count();
	compute_perlin_noise_for_2d_rbga_uint8(creator& c) {}
	static const char16_t* compute_shader_patch_cs();
	static const std::set<std::type_index>& compute_requirement();
	static void compute_apply(stage_context& sc) {}
	static bool compute_update(stage_context& sc, property_interface& rd);
};

struct property_worley_noise_setting
{

	//struct 

};

class compute_worley_noise_for_2d_rgba_uint8
{
public:

};
