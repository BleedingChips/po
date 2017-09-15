//#include "generator_2d_worley_noise_texture.h"
#include <random>
//using namespace PO::Dx;
//using namespace PO::Dx11;
/*
namespace Property_t
{
	generator_2d_worley_noise_texture_property::generator_2d_worley_noise_texture_property() : property_constructor(typeid(generator_2d_worley_noise_texture_property)) {}
	void generator_2d_worley_noise_texture_property::set_texture(PO::Dx11::tex2 t)
	{
		texture = std::move(t);
		texture_size = texture.size();
		need_push();
	}

	void generator_2d_worley_noise_texture_property::set_random_seed(uint32_t x, uint32_t y, uint32_t z)
	{
		std::mt19937 mtx(x);
		std::mt19937 mty(y);
		std::mt19937 mtz(z);
		std::uniform_real_distribution<float> nd2(0.0f, 1.0f);
		for (auto& p : worley_noise_point)
		{
			p = float3{
				nd2(mtx), nd2(mty), nd2(mtz)
			};
		}
		need_push();
	}

	void generator_2d_worley_noise_texture_property::push(PO::Dx11::creator& c)
	{
		aligned_storage<uint32_t4, float, aligned_array<float3, 300>> bp{ target_size, step, worley_noise_point };
		cb = c.create_buffer_constant(&bp);
		uav = c.cast_unordered_access_view(texture);
	}

	void generator_2d_worley_noise_texture_property::update(PO::Dx11::pipeline& p)
	{

	}
}

generator_2d_worley_noise_texture::generator_2d_worley_noise_texture() : compute_interface(typeid(generator_2d_worley_noise_texture)) {}
const std::set<std::type_index>& generator_2d_worley_noise_texture::acceptance() const
{
	static const std::set<std::type_index> acc = { typeid(property) };
	return acc;
}

void generator_2d_worley_noise_texture::init(PO::Dx11::creator& c)
{
	if (!load_cs(u"generator_2d_worley_noise_texture_cs.cso", c))
		throw 1;
}

bool generator_2d_worley_noise_texture::update(PO::Dx11::property_constructor& pi, PO::Dx11::pipeline& p)
{
	return pi.cast([this](property& p) {
		draw_call = p.texture_size;
		cs << p.cb[0] << p.uav[0];
	});
}

bool generator_2d_worley_noise_texture::draw(PO::Dx11::pipeline& c)
{
	c << cs;
	c.dispatch(draw_call.x / 32, draw_call.y / 32, 1);
	return true;
}
*/
