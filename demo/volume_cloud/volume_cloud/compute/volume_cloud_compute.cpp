#include "volume_cloud_compute.h"
#include <random>

// property_random_point_f ***********************************************************
void property_random_point_f::create_uniform_point(creator& c, uint32_t count, uint32_t seed, float min, float max)
{
	random_vector.clear();
	random_vector.reserve(count);
	std::mt19937 mtx(seed);
	std::uniform_real_distribution<float> nd2(min, max);
	for (uint32_t i = 0; i < count; ++i)
		random_vector.push_back(nd2(mtx));
	style = 1;
	parameter1 = float4{ min, max, 0.0, 0.0 };
}

void property_random_point_f::create_normal_point(creator& c, uint32_t count, uint32_t seed, float mean, float stddev)
{
	random_vector.clear();
	random_vector.reserve(count);
	std::mt19937 mtx(seed);
	std::normal_distribution<float> nd(mean, stddev);
	for (uint32_t i = 0; i < count; ++i)
		random_vector.push_back(nd(mtx));
	style = 2;
	parameter1 = float4{ mean, stddev, 0.0, 0.0 };
}

void property_random_point_f::update(creator& c, renderer_data& rd)
{
	rd.toatl_count = static_cast<uint32_t>(random_vector.size());
	rd.style = style;
	rd.parameter1 = parameter1;
	rd.parameter2 = parameter2;
	buffer_structured sb;
	sb.create(c, random_vector);
	rd.srv = sb.cast_shader_resource_view(c);
	shader_storage<uint32_t, uint32_t, float4, float4> ss{ rd.toatl_count, style, parameter1, parameter2 };
	rd.cb.create_pod(c, ss);
}

// property_random_point_f3 ***********************************************************
void property_random_point_f3::create_uniform_point(creator& c, uint32_t count, uint32_t3 seed, float min, float max)
{
	random_vector.clear();
	random_vector.reserve(count);
	std::mt19937 mtx1(seed.x);
	std::mt19937 mtx2(seed.y);
	std::mt19937 mtx3(seed.z);
	std::uniform_real_distribution<float> nd(min, max);
	for (uint32_t i = 0; i < count; ++i)
		random_vector.push_back(float3{ nd(mtx1), nd(mtx2), nd(mtx3) });
	style = 1;
	parameter1 = float4{ min, max, 0.0, 0.0 };
}

void property_random_point_f3::craate_custom(creator& c, uint32_t count, uint32_t3 seed)
{
	random_vector.clear();
	random_vector.reserve(count);
	std::mt19937 mtx1(seed.x);
	std::mt19937 mtx2(seed.y);
	std::mt19937 mtx3(seed.z);
	std::uniform_real_distribution<float> ndz(0.35, 0.65);
	std::normal_distribution<float> ndx_y(0.5, 0.15);
	for (uint32_t i = 0; i < count; ++i)
		random_vector.push_back(float3{ ndx_y(mtx1), ndx_y(mtx2), ndz(mtx3) });
	style = 1;
	parameter1 = float4{ 0.5f, 0.01f, 0.0, 0.0 };
}

void property_random_point_f3::create_normal_point(creator& c, uint32_t count, uint32_t3 seed, float mean, float stddev)
{
	random_vector.clear();
	random_vector.reserve(count);
	std::mt19937 mtx1(seed.x);
	std::mt19937 mtx2(seed.y);
	std::mt19937 mtx3(seed.z);
	std::normal_distribution<float> nd(mean, stddev);
	for (uint32_t i = 0; i < count; ++i)
		random_vector.push_back(float3{ nd(mtx1), nd(mtx2), nd(mtx3) });
	style = 2;
	parameter1 = float4{ mean, stddev, 0.0, 0.0 };
}

void property_random_point_f3::update(creator& c, renderer_data& rd)
{
	rd.toatl_count = static_cast<uint32_t>(random_vector.size());
	rd.style = style;
	rd.parameter1 = parameter1;
	rd.parameter2 = parameter2;
	buffer_structured sb;
	sb.create(c, random_vector);
	rd.srv = sb.cast_shader_resource_view(c);
	shader_storage<uint32_t, uint32_t, float4, float4> ss{ rd.toatl_count, style, parameter1, parameter2 };
	rd.cb.create_pod(c, ss);
}

// compute_generate_perlin_noise_tex3_3d_f1 ***********************************************************
compute_generate_perlin_noise_tex3_3d_f1::compute_generate_perlin_noise_tex3_3d_f1(creator& c)
	:compute_resource(c, u"volume_cloud_compute_generate_perlin_noise_tex3_3d_f1.cso")
{}

const element_requirement& compute_generate_perlin_noise_tex3_3d_f1::requirement() const
{
	return make_element_requirement(
		[](stage_context& sc, property_wrapper_t<property_random_point_f>& rd) {
		sc.CS() << rd.cb[0] << rd.srv[0];
	},
		[](stage_context& sc, property_wrapper_t<property>& rd) {
		sc.CS() << rd.output_texture[0] << rd.size_cb[1];
		sc << dispatch_call{ rd.size.x, rd.size.y, rd.size.z };
	}
	);
}

uint32_t compute_generate_perlin_noise_tex3_3d_f1::max_count(uint32_t4 sample)
{
	auto count_all = [](uint32_t i)->uint32_t {return (i + 1) * (i + 1) * (i + 1); };
	return count_all(sample.x) + count_all(sample.y) + count_all(sample.z) + count_all(sample.w);
}

// compute_generate_worley_noise_tex3_3d_f4 ***********************************************************
uint32_t compute_generate_worley_noise_tex3_3d_f4::max_count() { return 400; }

compute_generate_worley_noise_tex3_3d_f4::compute_generate_worley_noise_tex3_3d_f4(creator& c) :
	compute_resource(c, u"volume_cloud_compute_generate_worley_noise_tex3_3d_f4.cso")
{}

const element_requirement& compute_generate_worley_noise_tex3_3d_f4::requirement() const
{
	return make_element_requirement(
		[](stage_context& sc, property_wrapper_t<property_random_point_f3>& rd) {
		sc.CS() << rd.cb[0] << rd.srv[0];
	},
		[](stage_context& sc, property_wrapper_t<property>& rd) {
		sc.CS() << rd.output_texture_uav[0] << rd.cb[1];
		sc << dispatch_call{ rd.texture_size.x / 32 + ((rd.texture_size.x % 32) == 0 ? 0 : 1) , rd.texture_size.y / 32 + ((rd.texture_size.y % 32) == 0 ? 0 : 1), rd.texture_size.z };
	}
	);
}

void compute_generate_worley_noise_tex3_3d_f4::property::update(creator& c, renderer_data& rd)
{
	rd.output_texture_uav = output_texture_uav;
	rd.texture_size = texture_size;
	shader_storage<uint32_t3, float> ss{ texture_size, radius };
	rd.cb.create_pod(c, ss);
}


SDF_2dGenerator::SDF_2dGenerator(creator& c) : compute_resource(c, u"sdf_2d_generator.cso") {

}

const element_requirement& SDF_2dGenerator::requirement()
{
	return make_element_requirement(
		[](stage_context& sg, property_wrapper_t<property>& rd) {
		sg.CS() << rd.s1[0] << rd.t1[0] << rd.ss[0] << rd.bc[0];
		sg << dispatch_call{ rd.size.x / 32 + (rd.size.x % 32 == 0 ? 0 : 1), rd.size.y / 32 + (rd.size.y % 32 == 0 ? 0 : 1), 1 };
	}
	);
}


SDF_3dGenerator::SDF_3dGenerator(creator& c) : compute_resource(c, u"sdf_3d_generator.cso") {}


bool SDF_3dGenerator::property::next()
{
	if (input_end == uint32_t3{ 0, 0, 0 })
	{
		input_end = min(step_add, input_size);
	}
	else {
		if (input_end.z >= input_size.z)
		{
			if (input_end.y >= input_size.y)
			{
				if (input_end.x >= input_size.x)
					return false;
				else {
					input_start.z = 0;
					input_end.z = step_add.z;
					input_start.y = 0;
					input_end.y = step_add.y;
					input_start.x = input_end.x;
					input_end.x = input_start.x + step_add.x;
					input_end = min(input_end, input_size);
				}
			}
			else {
				input_start.z = 0;
				input_end.z = step_add.z;
				input_start.y = input_end.y;
				input_end.y = input_start.y + step_add.y;
				input_end = min(input_size, input_end);
			}
		}
		else {
			input_start.z = input_end.z;
			input_end.z = input_start.z + step_add.z;
			input_end = min(input_size, input_end);
		}
	}
	return true;
}


const element_requirement& SDF_3dGenerator::requirement()
{
	return make_element_requirement(
		[](stage_context& sc, property_wrapper_t<property>& re) {
		sc.CS() << re.bc[0] << re.Input[0] << re.InsideTexture[0] << re.OutsideTexture[1];
		sc << dispatch_call{ re.output_size.x / 32 + (re.output_size.x % 32 == 0 ? 0 : 1), re.output_size.y / 32 + (re.output_size.y % 32 == 0 ? 0 : 1), re.output_size.z };
	}
	);
}


full_1::full_1(creator& c) : compute_resource(c, u"full_1.cso") {}

const element_requirement& full_1::requirement()
{
	return make_element_requirement(
		[](stage_context& sc, property_wrapper_t<property>& p)
	{
		sc.CS() << p.input[0];
		sc << dispatch_call{p.size.x, p.size.y, p.size.z};
	}
	);
}

generator_perlin_worley_3D_tiless_noise::generator_perlin_worley_3D_tiless_noise(creator& c) : compute_resource(c, u"generator_perlin_worley_3D_tiless_noise.cso") 
{
}

const element_requirement& generator_perlin_worley_3D_tiless_noise::requirement()
{
	return make_element_requirement(
		[](stage_context& sc, property_wrapper_t<property>& p) {
		sc.CS() << p.input[0] << p.bc[0];
		sc << dispatch_call{p.size.x / 32 + (p.size.x % 32 != 0), p.size.y / 32 + (p.size.y % 32 != 0), p.size.z};
	},
		[](stage_context& sc, property_wrapper_t<indexed_property<property_random_point_f3, 0>>& p) {
		sc.CS() << p.srv[0] << p.cb[1];
	},
		[](stage_context& sc, property_wrapper_t<indexed_property<property_random_point_f3, 1>>& p) {
		sc.CS() << p.srv[1] << p.cb[2];
	}
	);
}






// compute_generate_worley_noise_tex3_3d_f4 ***********************************************************


/*
void property_merga_noise_float4_2d_simulate_3d::update(creator& c, renderer_data& rd)
{
	rd.texture_size = texture_size;
	rd.noise_1 = noise_1;
	rd.noise_2 = noise_2;
	rd.output = output;
	shader_storage<uint32_t2> ss{ texture_size };
	rd.cb.create_pod(c, ss);
}

compute_merga_noise_float4_2d_simulate_3d::compute_merga_noise_float4_2d_simulate_3d(creator& c) :
	compute_resource(c, u"volume_cloud_compute_merga_noise_float4_3d_simutale_3d.cso") {}

const element_requirement& compute_merga_noise_float4_2d_simulate_3d::requirement() const
{
	return make_element_requirement(
		[](stage_context& sg, property_merga_noise_float4_2d_simulate_3d::renderer_data& ps) {
		sg.CS() << ps.noise_1[0] << ps.noise_2[1] << ps.output[0] << ps.cb[0];
		sg << dispatch_call{ ps.texture_size.x, ps.texture_size.y, 1 };
	}
	);
}

void compute_generate_cube_mark_2d_simulate_3d::property::update(creator& c, renderer_data& rd)
{
	rd.texture_size = texture_size;
	rd.texture_f = texturef;
	shader_storage<uint32_t2, uint32_t4, float3, float3> ss{ texture_size, simulate_size, cube_min, cube_max };
	rd.cb.create_pod(c, ss);
}

compute_generate_cube_mark_2d_simulate_3d::compute_generate_cube_mark_2d_simulate_3d(creator& c) :
	compute_resource(c, u"volume_cloud_compute_generate_cube_mark_2d_simulate_3d.cso")
{

}

const element_requirement& compute_generate_cube_mark_2d_simulate_3d::requirement() const
{
	return make_element_requirement(
		[](stage_context& sc, property::renderer_data& p) {
		sc.CS() << p.cb[0] << p.texture_f[0];
		sc << dispatch_call{p.texture_size.x, p.texture_size.y, 1};
	});
}

compute_merga_4_f1_to_f4::compute_merga_4_f1_to_f4(creator& c) :
	compute_resource(c, u"volume_cloud_compute_merga_4_f1_to_f4") {}

void compute_merga_4_f1_to_f4::property::update(creator& sc, renderer_data& rd)
{
	rd.input1 = input1;
	rd.input2 = input2;
	rd.input3 = input3;
	rd.input4 = input4;
	rd.output = output;
	shader_storage<float4, float4> ss;
	rd.cb.create_pod(sc, ss);
	rd.texture_size = texture_size;
}

const element_requirement& compute_merga_4_f1_to_f4::requirement() const
{
	return make_element_requirement(
		[](stage_context& c, property::renderer_data& rd) {
		c.CS() << rd.input1[0] << rd.input2[0] << rd.input3[0] << rd.input4[0] << rd.output[0] << rd.cb[0];
		c << dispatch_call{ rd.texture_size.x , rd.texture_size.y, 1 };
	});
}*/

