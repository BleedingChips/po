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
	std::uniform_real_distribution<float> ndz(0.35f, 0.65f);
	std::normal_distribution<float> ndx_y(0.5f, 0.15f);
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

