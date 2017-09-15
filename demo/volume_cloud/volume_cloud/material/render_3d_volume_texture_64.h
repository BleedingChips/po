#pragma once
//#include "po_dx11_defer_renderer\element\interface.h"
/*
using namespace PO;
using namespace PO::Dx;
using namespace PO::Dx11;

struct render_3d_volume_texture_64;

namespace Implement
{
	class render_3d_volume_texture_64_property : public property_constructor
	{
		tex3 texture;
		
		float max_density = 1.0;
		float3 light;
		bool need_update = false;
		float3 min_size = float3(-1.0, -1.0, -1.0);
		float3 max_size = float3(1.0, 1.0, 1.0);

		sample_state ss;
		shader_resource_view srv;
		buffer_constant cb;

		friend struct render_3d_volume_texture_64;

	public:

		void set_texture(tex3 t) { texture = std::move(t); }
		void set_max_density(float t) { max_density = t; need_update = true; }
		void set_min_max_size(float3 t, float3 t2) { min_size = t; max_size = t2; need_update = true; }
		void set_light(float3 t);
		void push(creator&);
		void update(pipeline&);
		render_3d_volume_texture_64_property();
	};
}


struct render_3d_volume_texture_64 : material_interface
{
	using property = ::Implement::render_3d_volume_texture_64_property;

	virtual bool update(property_constructor&, pipeline&) override;
	virtual const std::set<std::type_index>& acceptance() const override;
	virtual void init(creator&) override;

	render_3d_volume_texture_64();
};
*/
