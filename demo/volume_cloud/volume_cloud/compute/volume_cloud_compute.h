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
	unordered_access_view m_uav;
	uint32_t2 tex_size;
	constant_buffer m_cb;

public:

	struct renderer_data
	{
		unordered_access_view m_uav;
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
