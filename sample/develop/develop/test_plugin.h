#pragma once
#include "form_define.h"
#include "../../../gui/dx11/dx11_define.h"
#include <DirectXTex.h>
using ticker = PO::ticker<PO::Dx11::Dx11_ticker>;
using conveyer = PO::conveyer<PO::Dx11::Dx11_ticker>;
using namespace PO::Dx11;

/*
PO::Tool::variant<Implement::texture1D_ptr, Implement::texture2D_ptr, Implement::texture3D_ptr>
load_dds(Implement::resource_ptr& rs, std::u16string path, PO::Dx11::Purpose::purpose bp = PO::Dx11::Purpose::constant);
Implement::resource_view_ptr cast_resource(Implement::resource_ptr& rp,
	const PO::Tool::variant<Implement::texture1D_ptr, Implement::texture2D_ptr, Implement::texture3D_ptr>& v
);
*/

struct test_plugin
{
	/*
	pixel_creater pc;
	material ma;
	Implement::texture2D_ptr tp;
	Implement::resource_view_ptr rvp;
	Implement::sample_state_ptr ss;
	constant_value cv;
	*/

	PO::raw_scene rs;
	PO::Dx11::input_assember_d iad;
	PO::Dx11::vertex_shader_d vs;
	PO::Dx11::pixel_shader_d ps;

	test_plugin()
	{
		rs.pre_load(
			typeid(PO::binary), 
			{
				u"base_vshader.cso",
				u"base_pshader.cso"
			}
		);
	}

	void tick(ticker& t);

	void init(ticker& t);
};