#include "volume_cloud_compute.h"
#include <random>
property_worley_noise_3d_point::property_worley_noise_3d_point() : property_interface(typeid(property_worley_noise_3d_point)) {}
void property_worley_noise_3d_point::set_seed(creator& c, uint32_t3 s)
{
	if (!m_cb) 
	{
		std::mt19937 mtx(s.x);
		std::mt19937 mty(s.y);
		std::mt19937 mtz(s.z);
		std::uniform_real_distribution<float> nd2(0.0f, 1.0f);
		shader_storage<aligned_array<float3, 300>> as;
		auto& ui = std::get<0>(as);
		for (size_t i = 0; i < 300; ++i)
		{
			ui[i] = float3{ nd2(mtx), nd2(mty), nd2(mtz) };
		}
		m_cb = c.create_constant_buffer(&as, true);
	}
	else {
		update_function = [s, this](pipeline& p) {
			std::mt19937 mtx(s.x);
			std::mt19937 mty(s.y);
			std::mt19937 mtz(s.z);
			std::uniform_real_distribution<float> nd2(0.0f, 1.0f);
			shader_storage<aligned_array<float3, 300>> as;
			auto& ui = std::get<0>(as);
			for (size_t i = 0; i < 300; ++i)
			{
				ui[i] = float3{ nd2(mtx), nd2(mty), nd2(mtz) };
			}
			p.write_constant_buffer(m_cb, [&](void* data) {
				*static_cast<shader_storage<aligned_array<float3, 300>> *>(data) = as;
			});
		};
	}
}

property_output_tex2::property_output_tex2() : property_interface(typeid(property_output_tex2)) {}
void property_output_tex2::set_texture(creator& c, const tex2& texture, float s, uint32_t4 simulate)
{
	m_uav = c.cast_unordered_access_view(texture);
	tex_size = texture.size();
	if (!m_cb)
	{
		shader_storage<uint32_t4, uint32_t2, float> ss{ simulate , tex_size, s };
		m_cb = c.create_constant_buffer(&ss, true);
	}
	else {
		update_function = [s, simulate, this](pipeline& p) {
			p.write_constant_buffer(m_cb, [&, this](void* data) {
				shader_storage<uint32_t4, uint32_t2, float> ss{ simulate , tex_size , s };
				*static_cast<shader_storage<uint32_t4, uint32_t2, float>*>(data) = ss;
			});
		};
	}
}


compute_worley_noise_tex2_3d::compute_worley_noise_tex2_3d() :compute_interface(typeid(compute_worley_noise_tex2_3d)) {}
void compute_worley_noise_tex2_3d::dispath(pipeline& p)
{
	p.dispatch(size.x / 32, size.y / 32, 1);
}
auto compute_worley_noise_tex2_3d::acceptance() const -> const acceptance_t&
{
	return make_acceptance<property_output_tex2, property_worley_noise_3d_point>{};
}
void compute_worley_noise_tex2_3d::init(creator& c)
{
	if (!load_cs(u"volume_cloud_compute_worley_noise_tex2_3d_cs.cso", c)) throw 1;
}
bool compute_worley_noise_tex2_3d::update(property_interface& pi, pipeline&)
{
	return pi.cast([this](property_output_tex2& pot) {
		size = pot.size();
		stage_cs << pot.uav()[0] << pot.cb()[1];
	}) || pi.cast([this](property_worley_noise_3d_point& pwn) {
		stage_cs << pwn.cb()[0];
	});
}

property_perline_worley_noise_3d_point::property_perline_worley_noise_3d_point() :property_interface(typeid(property_perline_worley_noise_3d_point)) {}
void property_perline_worley_noise_3d_point::set_seed(creator& c, uint32_t3 s)
{
	std::mt19937 mtx(s.x);
	std::mt19937 mty(s.y);
	std::mt19937 mtz(s.z);
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
		ref[i] = float3{nd2(mtx), nd2(mty), nd2(mtz) };
	}
	for (size_t i = 0; i < 200; ++i)
	{
		ref2[i] = float3{ nd2(mtx), nd2(mty), nd2(mtz) };
	}
	for (size_t i = 0; i < 400; ++i)
	{
		ref3[i] = float3{ nd2(mtx), nd2(mty), nd2(mtz) };
	}
	m_cb = c.create_constant_buffer(&as);
}

compute_perlin_worley_noise_tex2_3d::compute_perlin_worley_noise_tex2_3d() : compute_interface(typeid(compute_perlin_worley_noise_tex2_3d)) {}
void compute_perlin_worley_noise_tex2_3d::dispath(pipeline& p)
{
	p.dispatch(size.x, size.y, 1);
}

auto compute_perlin_worley_noise_tex2_3d::acceptance() const -> const acceptance_t&
{
	return make_acceptance<property_perline_worley_noise_3d_point, property_output_tex2>{};
}

void compute_perlin_worley_noise_tex2_3d::init(creator& c)
{
	if (!load_cs(u"volume_cloud_compute_perlin_worley_noise_tex2_3d_cs.cso", c)) throw 1;
}

bool compute_perlin_worley_noise_tex2_3d::update(property_interface& pi, pipeline&)
{
	return pi.cast([this](property_perline_worley_noise_3d_point& p) {
		stage_cs << p.cb()[0];
	}) || pi.cast([this](property_output_tex2& p) {
		size = p.size();
		stage_cs << p.uav()[0] << p.cb()[1];
	});
}