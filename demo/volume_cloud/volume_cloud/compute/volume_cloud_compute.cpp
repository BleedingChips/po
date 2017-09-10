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

void property_output_texture_2d_simulate_3d::set_output_texture_rgba_u8(
	creator& c,
	unordered_access_view<tex2> uav,
	uint32_t2 texture_size,
	uint32_t4 simulate_size,
	float4 mark,
	float4 blend
)
{
	shader_storage<uint32_t2, uint32_t4, float4, float4> ss{ texture_size, simulate_size, mark, blend };
	cb.create_pod(c, ss);
	this->uav = std::move(uav);
	texture_size = texture_size;
	assert(texture_size.x == simulate_size.x * simulate_size.z && texture_size.y == simulate_size.y * simulate_size.w);
	need_update();
}

void property_random_point_f::create_uniform_point(creator& c, uint32_t count, uint32_t seed, float min, float max)
{
	random_vector.clear();
	random_vector.reserve(count);
	std::mt19937 mtx(seed);
	std::uniform_real_distribution<float> nd2(min, max);
	for (uint32_t i = 0; i < count; ++i)
		random_vector.push_back(nd2(mtx));
	structured_buffer sb;
	sb.create(c, random_vector);
	srv = sb.cast_shader_resource_view(c);
	toatl_count = count;
	shader_storage<uint32_t, uint32_t, float4, float4> ss{ count, 1, float4{min, max, 0.0, 0.0}, float4{} };
	cb.create_pod(c, ss);
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
	structured_buffer sb;
	sb.create(c, random_vector);
	srv = sb.cast_shader_resource_view(c);
	toatl_count = count;
	shader_storage<uint32_t, uint32_t, float4, float4> ss{ count, 2, float4{ mean, stddev, 0.0, 0.0 }, float4{} };
	cb.create_pod(c, ss);
	need_update();
}

compute_perlin_noise_for_2d_rbga_uint8::compute_perlin_noise_for_2d_rbga_uint8(creator& c)
	:compute_resource(c, u"volume_cloud_compute_generate_perlin_noise_rgba_uint8.cso")
{}

const element_requirement& compute_perlin_noise_for_2d_rbga_uint8::requirement() const
{
	return make_element_requirement(
		[](stage_context& sc, property_random_point_f::renderer_data& rd) {
		sc.CS() << rd.cb[0] << rd.srv[0];
	},
		[](stage_context& sc, property_output_texture_2d_simulate_3d::renderer_data& rd) {
		sc.CS() << rd.uav[0] << rd.cb[1];
		sc << dispatch_call{ rd.texture_size.x, rd.texture_size.y, 1 };
	}
	);
}

uint count_all(uint i)
{
	return (i + 1) * (i + 1) * (i + 1);
}

uint32_t compute_perlin_noise_for_2d_rbga_uint8::max_count()
{
	uint32_t layout1 = 5;
	uint32_t layout2 = layout1 * 2;
	uint32_t layout3 = layout2 * 2;
	uint32_t layout4 = layout3 * 2;
	return count_all(layout1) + count_all(layout2) + count_all(layout3) + count_all(layout4);
}