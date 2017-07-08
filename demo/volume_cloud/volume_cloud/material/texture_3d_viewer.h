#pragma once
#include "po_dx11_defer_renderer\element\interface.h"

class texture_3d_viewer : public PO::Dx11::material_interface
{ 
	
public:


	class input_property : public PO::Dx11::property_interface
	{

		PO::Dx11::sample_state ss;
		PO::Dx11::shader_resource_view srv;
		PO::Dx11::constant_buffer cb;

		PO::Dx11::constant_buffer debug_buffer;

		PO::Dx11::tex3 t_buffer;

		float layer;
		PO::Dx::float4 filter;
		bool layer_change;

		friend class texture_3d_viewer;
	public:
		virtual void update(PO::Dx11::pipeline& p) override;
		void set_texture(const PO::Dx11::tex3& t) { t_buffer = t; }
		void set_layer(float t) { layer = t; layer_change = true; }
		void set_filter(float r, float g, float b, float a) { filter = PO::Dx::float4{ r, g,b,a };  layer_change = true; }
		input_property();
	};


	texture_3d_viewer();

	virtual bool update(PO::Dx11::property_interface&, PO::Dx11::pipeline&) override;
	virtual const std::set<std::type_index>& acceptance() const override;
	virtual void init(PO::Dx11::creator&) override;
};