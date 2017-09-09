#include "volume_cloud_compute.h"
#include <random>
void property_worley_noise_3d_point::update(renderer_data& rd, stage_context& sc)
{
	if (!rd.m_cb || need_update)
	{
		need_update = false;
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
		if (!rd.m_cb)
		{
			rd.m_cb.create_pod(sc, as, true);
		}else
			sc.write_constant_buffer(rd.m_cb, [&](void* data) {
			*static_cast<shader_storage<aligned_array<float3, 300>>*>(data) = as;
		});
	}
}

void property_output_tex2::set_texture(creator& c, const tex2& texture, float s, uint32_t4 simulate)
{
	m_uav = texture.cast_unordered_access_view(c);
	tex_size = texture.size();
	shader_storage<uint32_t4, uint32_t2, float> ss{ simulate , tex_size, s };
	m_cb.create_pod(c, ss);
}


const std::set<std::type_index>& compute_worley_noise_tex2_3d::compute_requirement() { return make_property_info_set<property_output_tex2, property_worley_noise_3d_point>{}; }
bool compute_worley_noise_tex2_3d::compute_update(stage_context& sc, property_interface& pi)
{
	return pi.cast([&](property_output_tex2::renderer_data& pot) {
		sc << dispatch_call{ pot.tex_size.x / 32, pot.tex_size.y / 32, 1 };
		sc.CS() << pot.m_uav[0] << pot.m_cb[1];
	}) || pi.cast([&](property_worley_noise_3d_point::renderer_data& pwn) {
		sc.CS() << pwn.m_cb[0];
	});
}
const char16_t* compute_worley_noise_tex2_3d::compute_shader_patch_cs() { return u"volume_cloud_compute_worley_noise_tex2_3d_cs.cso"; }


void property_perline_worley_noise_3d_point::update(renderer_data& rd, stage_context& sc)
{
	if (!rd.m_cb || need_update)
	{
		need_update = false;
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

		if (!rd.m_cb)
			rd.m_cb.create_pod(sc, as);
		else {
			sc.write_constant_buffer(rd.m_cb, [&](void* data) {
				*static_cast<shader_storage<float3, float3, float3, float3, aligned_array<float3, 100>, aligned_array<float3, 200>, aligned_array<float3, 400>>*>(data)
					= as;
			});
		}
	}
}


const char16_t* compute_perlin_worley_noise_tex2_3d::compute_shader_patch_cs()
{
	return u"volume_cloud_compute_perlin_worley_noise_tex2_3d_cs.cso";
}

const std::set<std::type_index>& compute_perlin_worley_noise_tex2_3d::compute_requirement()
{
	return make_property_info_set<property_perline_worley_noise_3d_point, property_output_tex2>{};
}

bool compute_perlin_worley_noise_tex2_3d::compute_update(stage_context& sc, property_interface& pi)
{
	return pi.cast([&](property_perline_worley_noise_3d_point::renderer_data& p) {
		sc.CS() << p.m_cb[0];
	}) || pi.cast([&](property_output_tex2::renderer_data& p) {
		sc << dispatch_call{ p.tex_size.x, p.tex_size.y, 1 };
		sc.CS() << p.m_uav[0] << p.m_cb[1];
	});
}


property_output_texture_2d_simulate_3d& property_output_texture_2d_simulate_3d::set_mark_blend(float4 ma, float4 bl) 
{ 
	mark = ma; 
	blend = bl; 
	need_update = true; 
	return *this; 
}

property_output_texture_2d_simulate_3d& property_output_texture_2d_simulate_3d::set_texture(creator& c, const tex2& te, const uint32_t4& simulate) {
	uav = te.cast_unordered_access_view_as_format(c, DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UINT);
	uint32_t2 size = te.size();
	texture_size = uint32_t2{ size.x , size.y };
	simulate_size = simulate;
	assert(texture_size.x == simulate.x * simulate.z && texture_size.y == simulate.y * simulate.w);
	need_update = true;
	return *this;
}

void property_output_texture_2d_simulate_3d::push(property_output_texture_2d_simulate_3d& pot, creator& c)
{
	if (need_update)
	{
		need_update = false;
		pot.need_update = true;
		pot.uav = uav;
		pot.texture_size = texture_size;
		pot.simulate_size = simulate_size;
		pot.mark = mark;
		pot.blend = blend;
	}
}


void property_output_texture_2d_simulate_3d::update(renderer_data& rd, stage_context& sc)
{
	if (need_update && !rd.cb)
	{
		rd.uav = uav;
		need_update = false;
		rd.texture_size = texture_size;
		shader_storage<uint32_t2, uint32_t4, float4, float4> ss{ texture_size, simulate_size, mark, blend };
		if (!rd.cb)
		{
			rd.cb.create_pod(sc, ss, true);
		}else
			sc.write_constant_buffer(rd.cb, [&](void* da) {
			*static_cast<shader_storage<uint32_t2, uint32_t4, float4, float4>*>(da) = ss;
		});
	}
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
	data.get<0>() = count;
	data.get<1>() = 1;
	data.get<2>() = float4{ min, max, 0.0, 0.0 };
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
	data.get<0>() = count;
	data.get<1>() = 2;
	data.get<2>() = float4{ mean, stddev, 0.0, 0.0 };
}


void property_random_point_f::push(property_random_point_f& ppn, creator& c)
{
	data.push(ppn.data, c);
	ppn.srv = srv;
	ppn.toatl_count = toatl_count;
}

void property_random_point_f::update(renderer_data& rd, stage_context& sc)
{
	data.update(rd.cb, sc);
	rd.srv = srv;
	rd.toatl_count = toatl_count;
}

const char16_t* compute_perlin_noise_for_2d_rbga_uint8::compute_shader_patch_cs()
{
	return u"volume_cloud_compute_generate_perlin_noise_rgba_uint8.cso";
}

const std::set<std::type_index>& compute_perlin_noise_for_2d_rbga_uint8::compute_requirement() {
	return make_property_info_set<property_random_point_f, property_output_texture_2d_simulate_3d>{};
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


bool compute_perlin_noise_for_2d_rbga_uint8::compute_update(stage_context& sc, property_interface& rd)
{
	return rd.cast([&](property_random_point_f::renderer_data& rd) {
		sc.CS() << rd.cb[0] << rd.srv[0];
	}) || rd.cast([&](property_output_texture_2d_simulate_3d::renderer_data& rd) {
		sc.CS() << rd.uav[0] << rd.cb[1];
		sc << dispatch_call{ rd.texture_size.x, rd.texture_size.y, 1 };
	});
}