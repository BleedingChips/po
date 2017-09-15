#include "volume_cloud_compute.h"
#include <random>

void property_worley_noise_3d_point::set_seed(creator& c, uint32_t3 seed) 
{ 
	std::mt19937 mtx(seed.x);
	std::mt19937 mty(seed.y);
	std::mt19937 mtz(seed.z);
	std::uniform_real_distribution<float> nd2(0.0f, 1.0f);
	shader_storage<aligned_array<float3, 300>> as;
	auto& ui = std::get<0>(as);
	for (size_t i = 0; i < 300; ++i)
	{
		ui[i] = float3{ nd2(mtx), nd2(mty), nd2(mtz) };
	}
	m_cb.create_pod(c, as);
	need_update();
}

void property_worley_noise_3d_point::update(creator& rd, renderer_data& sc)
{
	sc.m_cb = m_cb;
}



void property_output_tex2::set_texture(creator& c, const tex2& texture, float s, uint32_t4 simulate)
{
	m_uav = texture.cast_unordered_access_view(c);
	tex_size = texture.size();
	shader_storage<uint32_t4, uint32_t2, float> ss{ simulate , tex_size, s };
	m_cb.create_pod(c, ss);
	need_update();
}


compute_worley_noise_tex2_3d::compute_worley_noise_tex2_3d(creator& c) :
	compute_resource(c, u"volume_cloud_compute_worley_noise_tex2_3d_cs.cso")
{}
const element_requirement& compute_worley_noise_tex2_3d::requirement() const
{
	return make_element_requirement(
		[](stage_context& sc, property_output_tex2::renderer_data& pot) {
		sc << dispatch_call{ pot.tex_size.x / 32, pot.tex_size.y / 32, 1 };
		sc.CS() << pot.m_uav[0] << pot.m_cb[1];
	},
		[](stage_context& sc, property_worley_noise_3d_point::renderer_data& pwn) {
		sc.CS() << pwn.m_cb[0];
	}
	);
}

void property_perline_worley_noise_3d_point::set_seed(creator& c, uint32_t3 seed)
{
	std::mt19937 mtx(seed.x);
	std::mt19937 mty(seed.y);
	std::mt19937 mtz(seed.z);
	std::uniform_real_distribution<float> nd2(0.0f, 1.0f);
	shader_storage<float3, float3, float3, float3, aligned_array<float3, 100>, aligned_array<float3, 200>, aligned_array<float3, 400>> as{
		float3{ nd2(mtx), nd2(mty),nd2(mtz) },
		float3{ nd2(mtx), nd2(mty),nd2(mtz) },
		float3{ nd2(mtx), nd2(mty),nd2(mtz) },
		float3{ nd2(mtx), nd2(mty),nd2(mtz) }
	};
	auto& ref = std::get<4>(as);
	auto& ref2 = std::get<5>(as);
	auto& ref3 = std::get<6>(as);
	for (size_t i = 0; i < 100; ++i)
	{
		ref[i] = float3{ nd2(mtx), nd2(mty), nd2(mtz) };
	}
	for (size_t i = 0; i < 200; ++i)
	{
		ref2[i] = float3{ nd2(mtx), nd2(mty), nd2(mtz) };
	}
	for (size_t i = 0; i < 400; ++i)
	{
		ref3[i] = float3{ nd2(mtx), nd2(mty), nd2(mtz) };
	}
	m_cb.create_pod(c, as);
	need_update();
}

compute_perlin_worley_noise_tex2_3d::compute_perlin_worley_noise_tex2_3d(creator& c) :
	compute_resource(c, u"volume_cloud_compute_perlin_worley_noise_tex2_3d_cs.cso")
{}

const element_requirement& compute_perlin_worley_noise_tex2_3d::requirement() const
{
	return make_element_requirement(
		[](stage_context& sc, property_perline_worley_noise_3d_point::renderer_data& p) {
		sc.CS() << p.m_cb[0];
	},
		[](stage_context& sc, property_output_tex2::renderer_data& p) {
		sc << dispatch_call{ p.tex_size.x, p.tex_size.y, 1 };
		sc.CS() << p.m_uav[0] << p.m_cb[1];
	}
	);
}


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
	need_update();
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
	need_update();
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
	need_update();
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
	need_update();
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

void property_output_tex2_2d_simulate_3d::set_output_texture(unordered_access_view<tex2> output_texture, uint32_t2 texture_size, uint32_t4 simulate_size)
{
	output_texture_uav = std::move(output_texture);
	this->texture_size = texture_size;
	this->simulate_size = simulate_size;
	need_update();
}

void property_output_tex2_2d_simulate_3d::update(creator& c, renderer_data& rd)
{
	rd.texture_size = texture_size;
	rd.output_texture_uav = std::move(output_texture_uav);
	shader_storage<uint32_t2, uint32_t4> ss{ texture_size , simulate_size };
	rd.cb.create_pod(c, ss);
}

compute_generate_perlin_noise_uint8_4_2d_simulate_3d::compute_generate_perlin_noise_uint8_4_2d_simulate_3d(creator& c)
	:compute_resource(c, u"volume_cloud_compute_generate_perlin_noise_uint8_4_2d_simulate_3d.cso")
{}

const element_requirement& compute_generate_perlin_noise_uint8_4_2d_simulate_3d::requirement() const
{
	return make_element_requirement(
		[](stage_context& sc, property_random_point_f::renderer_data& rd) {
		sc.CS() << rd.cb[0] << rd.srv[0];
	},
		[](stage_context& sc, property_output_tex2_2d_simulate_3d::renderer_data& rd) {
		sc.CS() << rd.output_texture_uav[0] << rd.cb[1];
		sc << dispatch_call{ rd.texture_size.x, rd.texture_size.y, 1 };
	}
	);
}

uint count_all(uint i)
{
	return (i + 1) * (i + 1) * (i + 1);
}

uint32_t compute_generate_perlin_noise_uint8_4_2d_simulate_3d::max_count()
{
	uint32_t layout1 = 5;
	uint32_t layout2 = layout1 * 2;
	uint32_t layout3 = layout2 * 2;
	uint32_t layout4 = layout3 * 2;
	return (count_all(layout1) + count_all(layout2) + count_all(layout3) + count_all(layout4)) * 4;
}

void property_generate_worley_noise_float4_2d_simulate_3d::set_peorperty(unordered_access_view<tex2> output_texture, uint32_t2 texture_size, uint32_t4 simulate_size, float radio)
{
	output_texture_uav = std::move(output_texture);
	this->radius = radio;
	this->texture_size = texture_size;
	this->simulate_size = simulate_size;
	need_update();
}

void property_generate_worley_noise_float4_2d_simulate_3d::update(creator& c, renderer_data& rd)
{
	rd.output_texture_uav = std::move(output_texture_uav);
	rd.texture_size = texture_size;
	shader_storage<float, uint32_t2, uint32_t4> ss{ radius,  texture_size ,simulate_size };
	rd.cb.create_pod(c, ss);
}

uint32_t compute_generate_worley_noise_float4_2d_simulate_3d::max_count() { return 400 * 3; }

compute_generate_worley_noise_float4_2d_simulate_3d::compute_generate_worley_noise_float4_2d_simulate_3d(creator& c) :
	compute_resource(c, u"volume_cloud_compute_generate_worley_noise_float4_2d_simulate_3d.cso")
{}

const element_requirement& compute_generate_worley_noise_float4_2d_simulate_3d::requirement() const
{
	return make_element_requirement(
		[](stage_context& sc, property_random_point_f::renderer_data& rd) {
		sc.CS() << rd.cb[0] << rd.srv[0];
	},
		[](stage_context& sc, property_generate_worley_noise_float4_2d_simulate_3d::renderer_data& rd) {
		sc.CS() << rd.output_texture_uav[0] << rd.cb[1];
		sc << dispatch_call{ rd.texture_size.x / 32 + 1 , rd.texture_size.y / 32 + 1, 1 };
	}
	);
}

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
}

