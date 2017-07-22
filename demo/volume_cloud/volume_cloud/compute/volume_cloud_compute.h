#pragma once
#include "po_dx11_defer_renderer\defer_element.h"
using namespace PO::Dx;
using namespace PO::Dx11;

class property_worley_noise_3d_point : public property_interface
{
	constant_buffer m_cb;
public:
	void set_seed(creator& c, uint32_t3 s);
	const constant_buffer& cb() const { return m_cb; }
	property_worley_noise_3d_point();
};

class property_output_tex2 : public property_interface
{
	unordered_access_view m_uav;
	constant_buffer m_cb;
	uint32_t2 tex_size;
public:
	property_output_tex2();
	uint32_t2 size() { return tex_size; }
	const constant_buffer& cb() const { return m_cb; }
	const unordered_access_view& uav() const { return m_uav; }
	void set_texture(creator& c, const tex2& texture, float step, uint32_t4 simulate = uint32_t4{0, 0, 0, 0});
};

class compute_worley_noise_tex2_3d : public compute_interface
{
	uint32_t2 size;
public:
	compute_worley_noise_tex2_3d();
	virtual void dispath(pipeline& p) override;
	virtual auto acceptance() const -> const acceptance_t& override;
	virtual void init(creator&) override;
	virtual bool update(property_interface&, pipeline&) override;
};
