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
			rd.m_cb = sc.create_constant_buffer(&as, true);
		}else
			sc.write_constant_buffer(rd.m_cb, [&](void* data) {
			*static_cast<shader_storage<aligned_array<float3, 300>>*>(data) = as;
		});
	}
}

void property_output_tex2::set_texture(creator& c, const tex2& texture, float s, uint32_t4 simulate)
{
	m_uav = c.cast_unordered_access_view(texture);
	tex_size = texture.size();
	shader_storage<uint32_t4, uint32_t2, float> ss{ simulate , tex_size, s };
	m_cb = c.create_constant_buffer(&ss);
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

		if(!rd.m_cb)
			rd.m_cb = sc.create_constant_buffer(&as);
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