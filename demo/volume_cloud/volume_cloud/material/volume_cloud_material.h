#pragma once
#include "po_dx11_defer_renderer\defer_element.h"
using namespace PO::Dx;
using namespace PO::Dx11;
class property_render_2d_for_3d : public property_interface
{
	shader_resource_view m_srv;
	constant_buffer m_cb;
public:
	property_render_2d_for_3d();
	void set_option(creator& c, float3 min_width, float3 max_width, float3 light, float density);
	const shader_resource_view& srv() const { return m_srv; }
	const constant_buffer& cb() const { return m_cb; }
	void set_texture(creator& c, const tex2& t) { m_srv = c.cast_shader_resource_view(t); }
};

class material_transparent_render_2d_for_3d_64 : public defer_material_interface
{
public:
	material_transparent_render_2d_for_3d_64();
	virtual auto acceptance() const -> const acceptance_t& override;
	virtual void init(creator&) override;
	virtual bool update(property_interface&, pipeline&) override;
};

class material_transparent_2d_for_3d_64_without_perlin : public defer_material_interface
{
public:
	material_transparent_2d_for_3d_64_without_perlin();
	virtual auto acceptance() const -> const acceptance_t& override;
	virtual void init(creator&) override;
	virtual bool update(property_interface&, pipeline&) override;
};