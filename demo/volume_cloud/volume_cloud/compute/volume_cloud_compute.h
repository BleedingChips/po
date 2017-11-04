#pragma once
#include "po_dx11_defer_renderer\defer_element.h"
using namespace PO::Dx;
using namespace PO::Dx11;

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
	void craate_custom(creator& c, uint32_t count, uint32_t3 seed = { 0, 0, 0 });
	void create_uniform_point(creator& c, uint32_t count, uint32_t3 seed = { 0, 0, 0 }, float min = 0.0, float max = 1.0);
	void update(creator& c, renderer_data& rd);
};



class compute_generate_perlin_noise_tex3_3d_f1 : public compute_resource
{
public:

	struct property : public property_resource
	{
		unordered_access_view<tex3> output_texture;
		uint32_t3 size = {0, 0, 0};
		uint32_t4 sample_scale;
		float4 value_factor;
		struct renderer_data
		{
			unordered_access_view<tex3> output_texture;
			buffer_constant size_cb;
			uint32_t3 size;
		};
		void set_output_f(unordered_access_view<tex3> output, uint32_t3 size, uint32_t4 scale, float4 factor)
		{
			output_texture = std::move(output);
			this->size = size;
			sample_scale = scale;
			value_factor = factor;
			need_update();
		}
		void update(creator& c, renderer_data& rd)
		{
			rd.output_texture = output_texture;
			rd.size = size;
			shader_storage<uint32_t3, uint32_t4, float4> ss{size, sample_scale, value_factor};
			rd.size_cb.create_pod(c, ss);
		}
	};

	static uint32_t max_count(uint32_t4);
	compute_generate_perlin_noise_tex3_3d_f1(creator& c);
	const element_requirement& requirement() const;
};




class compute_generate_worley_noise_tex3_3d_f4 : public compute_resource
{
public:

	class property : public property_resource
	{
		unordered_access_view<tex3> output_texture_uav;
		float radius = 0.01f;
		uint32_t3 texture_size = {0, 0, 0};
	public:
		struct renderer_data
		{
			unordered_access_view<tex3> output_texture_uav;
			buffer_constant cb;
			uint32_t3 texture_size;
		};
		void set_peorperty(unordered_access_view<tex3> output_texture, uint32_t3 texture_size, float radio = 1.0f)
		{
			output_texture_uav = std::move(output_texture);
			radius = radio;
			this->texture_size = texture_size;
			need_update();
		}
		void update(creator& c, renderer_data& rd);
	};

	static uint32_t max_count();
	compute_generate_worley_noise_tex3_3d_f4(creator& c);
	const element_requirement& requirement() const;
};

class compute_format_tex3_f4_to_2d_u8_4 : public compute_resource
{
public:
	struct property : public property_resource
	{
		uint32_t2 texture_size = {0, 0};
		uint32_t4 simulate_size = {0 , 0, 0, 0};
		float4 value_factor = {0.0, 0.0, 0.0, 0.0};
		sample_state ss;
		shader_resource_view<tex3> srv;
		unordered_access_view<tex2> output;
		struct renderer_data
		{
			shader_resource_view<tex3> srv;
			sample_state ss;
			unordered_access_view<tex2> output;
			uint32_t2 texture_size;
			buffer_constant bc;
		};
		void set(shader_resource_view<tex3> input, sample_state input_ss, unordered_access_view<tex2> output, uint32_t2 size, uint32_t4 simulate, float4 factor)
		{
			srv = std::move(input);
			ss = std::move(input_ss);
			this->output = std::move(output);
			texture_size = size;
			simulate_size = simulate;
			value_factor = factor;
			need_update();
		}
		void update(creator& c, renderer_data& rd)
		{
			shader_storage<uint32_t2, uint32_t4, float4> ss_store(texture_size, simulate_size, value_factor);
			rd.bc.create_pod(c, ss_store);
			rd.ss = ss;
			rd.output = output;
			rd.srv = srv;
			rd.texture_size = texture_size;
		}
	};
	compute_format_tex3_f4_to_2d_u8_4(creator& c) : compute_resource(c, u"volume_cloud_compute_format_tex3_f4_to_2d_u8_4_cs.cso") {}
	const element_requirement& requirement()
	{
		return make_element_requirement(
			[](stage_context& sc, property::renderer_data& p) {
			sc.CS() << p.bc[0] << p.output[0] << p.srv[0] << p.ss[0];
			sc << dispatch_call{p.texture_size.x, p.texture_size.y, 1};
		}
		);
	}
};

class compute_generate_cube_mask_tex3_f : public compute_resource
{

public:
	struct property : public property_resource
	{
		unordered_access_view<tex3> output;
		uint32_t3 texture_size = {0, 0,0};
		struct renderer_data
		{
			unordered_access_view<tex3> output;
			buffer_constant bc;
			uint32_t3 texture_size;
		};
		void set(unordered_access_view<tex3> output_texture, uint32_t3 size)
		{
			output = std::move(output_texture);
			texture_size = size;
			need_update();
		}
		void update(creator& c, renderer_data& rd)
		{
			rd.output = output;
			rd.texture_size = texture_size;
			shader_storage<uint32_t3> ss{ texture_size };
			rd.bc.create_pod(c, ss);
		}
	};
	compute_generate_cube_mask_tex3_f(creator& c) : compute_resource(c, u"volume_cloud_compute_generate_cube_mark_2d_simulate_3d.cso") {}
	const element_requirement& requirement()
	{
		return make_element_requirement(
			[](stage_context& sc, property::renderer_data& p) {
			sc.CS() << p.bc[0] << p.output[0];
			sc << dispatch_call{ p.texture_size.x, p.texture_size.y, p.texture_size.z };
		}
		);
	}
};

class compute_generator : public compute_resource
{
public:

	struct property : public property_resource
	{
		std::array<unordered_access_view<tex3>, 5> output;
		struct renderer_data
		{
			std::array<unordered_access_view<tex3>, 5> output;
		};

		property& operator<<(std::array<unordered_access_view<tex3>, 5> t) { output = std::move(t); property_resource::need_update(); return *this; }
		void update(creator& sc, renderer_data& rd) { rd.output = output; }
	};

	compute_generator(creator& c) : compute_resource(c, u"volume_generator_tiling_3d_perlin_noise.cso") {}
	const element_requirement& requirement()
	{
		return make_element_requirement(
			[](stage_context& sc, property::renderer_data& p) {
			for (size_t i = 0; i < 5; ++i)
				sc.CS() << p.output[i][i];
			sc << dispatch_call{ 8, 8, 64 };
		}/*, [](stage_context& sc, property_random_point_f3::renderer_data& rd) {
			sc.CS() << rd.srv[0];
		}*/
		);
	}
};


class compute_2D_tiled : public compute_resource
{
public:
	struct property : public property_resource
	{
		std::array<unordered_access_view<tex2>, 2> output;
		unordered_access_view<tex3> output2;
		struct renderer_data
		{
			std::array<unordered_access_view<tex2>, 2> output;
			unordered_access_view<tex3> output2;
		};
		property& operator<<(std::array<unordered_access_view<tex2>, 2> t) { output = std::move(t); property_resource::need_update(); return *this; }
		property& operator<<(unordered_access_view<tex3> t) { output2 = std::move(t); property_resource::need_update(); return *this; }
		void update(creator& sc, renderer_data& rd) { rd.output = output; rd.output2 = output2; }
	};
	compute_2D_tiled(creator& c) : compute_resource(c, u"compute_2d_tiled.cso") {}
	const element_requirement& requirement()
	{
		return make_element_requirement(
			[](stage_context& sc, property::renderer_data& p) {
			for (size_t i = 0; i < 2; ++i)
				sc.CS() << p.output[i][i];
			sc.CS() << p.output2[2];
			sc << dispatch_call{ 8, 8, 1 };
		}, [](stage_context& sc, property_random_point_f::renderer_data& rd) {
		 sc.CS() << rd.srv[0];
		 }, [](stage_context& sc, property_random_point_f3::renderer_data& rd) {
			 sc.CS() << rd.srv[1];
		 }
		 );
	}
};


class compute_3D_tiled : public compute_resource
{
public:
	struct property : public property_resource
	{
		unordered_access_view<tex3> output2;
		struct renderer_data
		{
			unordered_access_view<tex3> output2;
		};
		property& operator<<(unordered_access_view<tex3> t) { output2 = std::move(t); property_resource::need_update(); return *this; }
		void update(creator& sc, renderer_data& rd) { rd.output2 = output2; }
	};
	compute_3D_tiled(creator& c) : compute_resource(c, u"compute_3d_tiled.cso") {}
	const element_requirement& requirement()
	{
		return make_element_requirement(
			[](stage_context& sc, property::renderer_data& p) {
			sc.CS() << p.output2[0];
			sc << dispatch_call{ 1, 1, 32 };
		}, [](stage_context& sc, property_random_point_f3::renderer_data& rd) {
			sc.CS() << rd.srv[0];
		}
		);
	}
};





/*

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
*/
//struct property_resource
/*
class compute_generate_perlin_noise_float_2d : public compute_resource
{
public:
	uint32_t point_count();
	compute_generate_perlin_noise_float_2d(creator& c);
};
*/