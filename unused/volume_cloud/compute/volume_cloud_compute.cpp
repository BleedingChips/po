#include "volume_cloud_compute.h"
#include <random>
using namespace PO;
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

property_custom_random_point_f3& property_custom_random_point_f3::set_normal(uint32_t index, uint32_t Seed, float mean, float stddev) {
	assert(index <= 3);
	data[index].data = uint32_t4(Seed, 0,0, 0);
	data[index].Parameter = float4(mean, stddev, 0, 0);
	return *this;
}

property_custom_random_point_f3& property_custom_random_point_f3::set_uniform(uint32_t index, uint32_t Seed, float min, float max)
{
	assert(index <= 3);
	data[index].data = uint32_t4(Seed, 1, 0, 0);
	data[index].Parameter = float4(min, max, 0, 0);
	return *this;
}

void property_custom_random_point_f3::update(creator& c, renderer_data& rd)
{
	std::vector<float3> buffer(count, float3(0, 0, 0));
	//std::array<std::mt19937, 3> mt({ data[0].data.x, data[1].data.x, data[2].data.x});
	std::mt19937 mt[3] = { std::mt19937(data[0].data.x), std::mt19937(data[1].data.x), std::mt19937(data[2].data.x) };
	for (size_t i = 0; i < count; ++i)
	{
		float Result[3];
		for (size_t i = 0; i < 3; ++i)
		{
			if (data[i].data.y == 0)
			{
				std::normal_distribution<float> nd(data[i].Parameter.x, data[i].Parameter.y);
				Result[i] = nd(mt[i]);
			}
			else if (data[i].data.y == 1)
			{
				std::uniform_real_distribution<float> nd(data[i].Parameter.x, data[i].Parameter.y);
				Result[i] = nd(mt[i]);
			}
		}
		float3 FinalResult(Result[0], Result[1], Result[2]);
		buffer[i] = FinalResult;
	}
	buffer_structured bs;
	bs.create(c, buffer);
	rd.rand_buffer = bs.cast_shader_resource_view(c);
	shader_storage<uint32_t, uint32_t4, float4, uint32_t4, float4, uint32_t4, float4> ss(count, data[0].data, data[0].Parameter, data[1].data, data[1].Parameter, data[1].data, data[1].Parameter);
	rd.parameter.create_pod(c, ss);
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

#undef min

uint32_t3 min(uint32_t3 i, uint32_t3 k)
{
	return {
		i.x < k.x ? i.x : k.x,
		i.y < k.y ? i.y : k.y,
		i.z < k.z ? i.z : k.z,
	};
}

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

DensityMap3DGenerator::DensityMap3DGenerator(creator& c) : compute_resource(c, u"DensityMap3DGenerator.cso") {}

const element_requirement& DensityMap3DGenerator::requirement()
{
	return make_element_requirement(
		[](stage_context& sc, property_wrapper_t<property>& p) {
		sc.CS() << p.bc[0] << p.output[0];
		sc << dispatch_call{ p.output_size, { 32, 32, 1 } };
	},
		[](stage_context& sc, property_wrapper_t<property_random_point_f3>& p){
		sc.CS() << p.srv[0] << p.cb[1];
	}
	);
}

Simulate3DFloatWith2DUnorm4::Simulate3DFloatWith2DUnorm4(creator& c) : compute_resource(c, u"Simulate3DFloatWith2DUnorm4.cso")
{

}

uint32_t2 Simulate3DFloatWith2DUnorm4::calculate_texture_size(uint32_t4 input)
{
	return uint32_t2{ (input.x + 2) * input.z, (input.y + 2) * input.w };
}


const element_requirement& Simulate3DFloatWith2DUnorm4::requirement()
{
	return make_element_requirement(
		[](stage_context& sc, property_wrapper_t<property>& p) {
		sc.CS() << p.bc[0] << p.input[0] << p.output[0];
		sc << dispatch_call{ p.output_size, { 32, 32, 1 } };
	}
	);
}

SignedDistanceField3DGenerator::SignedDistanceField3DGenerator(creator& c) : compute_resource(c, u"SignedDistanceField3DGenerator.cso") {}
const element_requirement& SignedDistanceField3DGenerator::requirement()
{
	return make_element_requirement(
		[](stage_context& sc, property_wrapper_t<property>& pp) {
		sc.CS() << pp.bc[0] << pp.inputTexture[0] << pp.outputTexture[0] << pp.bufferTexture[1];
		sc << dispatch_call{ pp.output_size, { 32, 32, 1 } };
	}
	);
}


CenterNoiseGenerator::CenterNoiseGenerator(creator& c) : compute_resource(c, u"CenterNoiseGenerator.cso") {}

const element_requirement& CenterNoiseGenerator::requirement()
{
	return make_element_requirement(
		[](stage_context& sc, property_wrapper_t<property>& pp) {
		sc.CS() << pp.bc[0] << pp.output[0] << pp.ss[0] << pp.XY[1] << pp.YZ[2] << pp.XZ[3];
		sc << dispatch_call{ pp.output_size, { 32, 32, 1 } };
	},
		[](stage_context& sc, property_wrapper_t<property_custom_random_point_f3>& p)
	{
		sc.CS() << p.parameter[1] << p.rand_buffer[0];
	}
	);
}

GeneratorMetaBall3D::GeneratorMetaBall3D(creator& c) : compute_resource(c, u"GeneratorMetaBall3D.cso")
{
	
}

const element_requirement& GeneratorMetaBall3D::requirement()
{
	return make_element_requirement(
		[](stage_context& sc, property_wrapper_t<property>& p) {
		sc.CS() << p.original_point[1] << p.shape_output[0] << p.area[0];
		sc << dispatch_call{ p.texture_size, {32, 32, 1} };
	}
		/*
		,
		[](stage_context& sc, property_wrapper_t<property_custom_random_point_f3>& p) {
		sc.CS() << p.rand_buffer[1];
	}*/
	);
}

PointFilter::PointFilter(creator& c) : compute_resource(c, u"PointFilter.cso") {}
const element_requirement& PointFilter::requirement()
{
	return make_element_requirement(
		[](stage_context& sc, property_wrapper_t<property>& p) {
		sc.CS() << p.input[0] << p.output[1] << p.Edge[0] << p.ss[0];
		sc << dispatch_call{ uint32_t3{uint32_t(p.input_size), 1, 1}, {32, 1, 1} };
	}
	);
}

fbmgenerator::fbmgenerator(creator& c) : compute_resource(c, u"fbmgenerator.cso") {}
const element_requirement& fbmgenerator::requirement()
{
	return make_element_requirement(
		[](stage_context& sc, property_wrapper_t<property>& p) {
		sc.CS() << p.random_point[0] << p.output[0] << p.input[1] << p.ss[0] << p.bc[0];
		sc << dispatch_call{ p.input_size,{ 32, 32, 1 } };
	}
	);
}

fbmWorleyNoiseGenerator::fbmWorleyNoiseGenerator(creator& c) : compute_resource(c, u"fbmWorleyNoiseGenerator.cso") {}

const element_requirement&  fbmWorleyNoiseGenerator::requirement()
{
	return make_element_requirement(
		[](stage_context& sc, property_wrapper_t<property>& p) {
		sc.CS() << p.bc[0] << p.fbmOutput[0];
		sc << dispatch_call{ p.texture_size,{ 32, 32, 1 } };
	}
	);
}