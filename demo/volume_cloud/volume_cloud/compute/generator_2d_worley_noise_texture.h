#pragma once
#include "po_dx11_defer_renderer\defer_element.h"

using namespace PO::Dx;
using namespace PO::Dx11;
class worley_noise_2d_property : public property_interface
{
	uint32_t3 seed;
	constant_buffer cb;
public:
	void set_seed(creator& c, uint32_t3 s);
	worley_noise_2d_property();
	void update(pipeline& p);
};

class worley_noise_2d_compute : public compute_interface
{
public:
	worley_noise_2d_compute();
	virtual void dispath(pipeline& p) override;
	virtual auto acceptance() const -> const acceptance_t& override;
	virtual void init(creator&) override;
	virtual bool update_imp(property_interface&, pipeline&) override;
};
/*
class generator_2d_worley_noise_texture;
namespace Property_t
{
	struct generator_2d_worley_noise_texture_property : public PO::Dx11::property_constructor
	{
		std::array<PO::Dx::float3, 300> worley_noise_point;
		PO::Dx11::tex2 texture;
		PO::Dx::uint32_t2 texture_size;
		PO::Dx::uint32_t4 target_size;
		float step = 1.0;

		PO::Dx11::constant_buffer cb;
		PO::Dx11::unordered_access_view uav;
	public:
		void set_texture(PO::Dx11::tex2);
		void set_random_seed(uint32_t x, uint32_t y, uint32_t z);
		void set_target_size(uint32_t x, uint32_t y, uint32_t zx, uint32_t zy) { target_size = { x, y, zx, zy }; need_push(); }
		void set_step(float t) { step = t; need_push(); }
		virtual void push(PO::Dx11::creator& c) override;
		virtual void update(PO::Dx11::pipeline& p) override;
		generator_2d_worley_noise_texture_property();
	};
}

class generator_2d_worley_noise_texture : public PO::Dx11::compute_interface
{
	PO::Dx11::uint32_t2 draw_call;
public:
	using property = Property_t::generator_2d_worley_noise_texture_property;
	generator_2d_worley_noise_texture();
	virtual bool update(PO::Dx11::property_constructor&, PO::Dx11::pipeline&) override;
	virtual const std::set<std::type_index>& acceptance() const override;
	virtual bool draw(PO::Dx11::pipeline& c) override;
	virtual void init(PO::Dx11::creator&) override;
};
*/