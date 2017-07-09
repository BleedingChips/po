#include "texture_3d_viewer.h"
#include <random>
#undef max
using namespace PO::Dx;
using namespace PO::Dx11; 
using namespace PO;
texture_3d_viewer::texture_3d_viewer() : material_interface(typeid(texture_3d_viewer), render_order::Post) {}
const std::set<std::type_index>& texture_3d_viewer::acceptance() const
{
	static const std::set<std::type_index> acc = {typeid(input_property)};
	return acc;
}

void texture_3d_viewer::init(PO::Dx11::creator& c)
{
	if (!load_ps(u"texture_3d_viewer_ps.cso", c)) throw 1;
}

bool texture_3d_viewer::update(PO::Dx11::property_interface& pi, PO::Dx11::pipeline& p)
{
	if (pi.is<input_property>())
	{
		auto& ref = pi.cast<input_property>();
		ps << ref.ss[0] << ref.srv[0] << ref.cb[0];
		return true;
	}
	return false;
}



void texture_3d_viewer::input_property::update(PO::Dx11::pipeline& p)
{
	auto& c = p.get_creator();
	if (!ss)
		ss = c.create_sample_state();

	if (layer_change)
	{

		struct layout_t
		{
			alignas(16) float layer;
			alignas(16) float4 filter;
		}lt{layer, filter};

		if (!cb)
			cb = c.create_constant_buffer(&lt, true);
		else
			p.write_constant_buffer(cb, [&, this](void* data) {
				*static_cast<layout_t*>(data) = lt;
			});

		layer_change = false;
	}

	if (t_buffer)
	{
		srv = c.cast_shader_resource_view(t_buffer);
		t_buffer = PO::Dx11::tex3{};
	}
		
}

texture_3d_viewer::input_property::input_property() : property_interface(typeid(input_property)), layer_change(false), layer(0.0f), filter(float4{ 1.0f, 1.0f, 1.0f, 1.0f }) {}