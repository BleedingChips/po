#pragma once
/*
#include "po_dx11_defer_renderer\element\interface.h"

class generator_3d_volume_texture;

namespace SelfProperty
{
	class generator_3d_volume_texture_property : public PO::Dx11::property_interface
	{
		std::array<PO::Dx::float3, 200> wise_nosie_point;
		std::array<PO::Dx::float3, 4> perlin_noise_factor;
		PO::Dx11::tex3 texture;
		PO::Dx::uint32_t3 size;

		PO::Dx11::unordered_access_view output_texture;
		PO::Dx11::constant_buffer cb;
		
		friend class generator_3d_volume_texture;

	public:

		void set_texture(PO::Dx11::tex3 output_texture);
		void set_random(uint32_t random_seed1, uint32_t random_seed2, uint32_t random_seed3);

		virtual void push(PO::Dx11::creator& p) override;
		generator_3d_volume_texture_property();
	};
}


class generator_3d_volume_texture : public PO::Dx11::compute_interface
{

	PO::Dx::uint32_t3 size = PO::Dx::uint32_t3{0, 0, 0};

public:

	using input_property = SelfProperty::generator_3d_volume_texture_property;

	generator_3d_volume_texture();

	virtual bool update(PO::Dx11::property_interface&, PO::Dx11::pipeline&) override;
	virtual const std::set<std::type_index>& acceptance() const override;

	virtual bool draw(PO::Dx11::pipeline& c) override;
	virtual void init(PO::Dx11::creator&) override;
};
*/