#include "generator_3d_volume_texture.h"
#include <random>
#include <array>
/*
using namespace PO;
using namespace PO::Dx;
using namespace PO::Dx11;
#undef max

namespace SelfProperty
{
	generator_3d_volume_texture_property::generator_3d_volume_texture_property()
		: PO::Dx11::property_interface(typeid(generator_3d_volume_texture_property)){}

	void generator_3d_volume_texture_property::set_texture(PO::Dx11::tex3 t)
	{
		texture = std::move(t);
		size = texture.size();
		need_push();
	}

	void generator_3d_volume_texture_property::set_random(uint32_t random_seed1, uint32_t random_seed2, uint32_t random_seed3)
	{
		std::mt19937 mtx(random_seed1);
		std::mt19937 mty(random_seed2);
		std::mt19937 mtz(random_seed3);
		std::normal_distribution<double> nd(0.0f, 0.2f);
		std::uniform_real_distribution<float> nd2(-1.0, 1.0);
		
		for (auto& p : perlin_noise_factor)
		{
			p = float3{
				mtz() / static_cast<float>(std::mt19937::max()),
				mtx() / static_cast<float>(std::mt19937::max()),
				mty() / static_cast<float>(std::mt19937::max())
			};
		}

		for (auto& p : wise_nosie_point)
		{
			p = float3{
				nd2(mtx), nd2(mty), nd2(mtz)
			};
		}

		need_push();
	}

	void generator_3d_volume_texture_property::push(creator& p)
	{
		output_texture = p.cast_unordered_access_view(texture);
		aligned_storage<uint32_t3, aligned_array<float3, 200>, aligned_array<float3, 4>> storage;
		storage.get<0>() = size;
		storage.get<1>() = wise_nosie_point;
		storage.get<2>() = perlin_noise_factor;
		cb = p.create_constant_buffer(&storage);
	}
}












generator_3d_volume_texture::generator_3d_volume_texture() : compute_interface(typeid(generator_3d_volume_texture)) {}

bool generator_3d_volume_texture::update(property_interface& pi, pipeline& p)
{
	if (pi.is<input_property>())
	{
		auto& ref = pi.cast<input_property>();
		if (ref.cb && ref.output_texture)
		{
			cs << ref.cb[0] << ref.output_texture[0];
			size = ref.size;
			return true;
		}
	}
	return false;
}

const std::set<std::type_index>& generator_3d_volume_texture::acceptance() const
{
	static const std::set<std::type_index> acc = { typeid(input_property) };
	return acc;
}

bool generator_3d_volume_texture::draw(pipeline& c)
{
	c << cs;
	if (static_cast<UINT>(size.x) % 32 != 0 || static_cast<UINT>(size.y) % 32 != 0) return false;
	c.dispatch(static_cast<UINT>(size.x) / 32, static_cast<UINT>(size.y) / 32, static_cast<UINT>(size.z));
	return true;
}

void generator_3d_volume_texture::init(creator& c)
{
	if (!load_cs(u"compute_generator_3d_volume_texture_cs.cso", c))
		throw 1;
}
*/