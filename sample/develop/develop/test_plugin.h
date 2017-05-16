#pragma once
#include <iostream>
#include "form_define.h"
#include "../../../gui/dx11/dx11_frame.h"
#include <DirectXTex.h>
#include "../../../tool/mail.h"
#include "../../../gui/dx/movement.h"
#include "../../../gui/dx11/dx11_form.h"
using namespace PO::Dx;
using namespace PO::Dx11;
using namespace PO;
/*
PO::Tool::variant<Implement::texture1D_ptr, Implement::texture2D_ptr, Implement::texture3D_ptr>
load_dds(Implement::resource_ptr& rs, std::u16string path, PO::Dx11::Purpose::purpose bp = PO::Dx11::Purpose::constant);
Implement::resource_view_ptr cast_resource(Implement::resource_ptr& rp,
	const PO::Tool::variant<Implement::texture1D_ptr, Implement::texture2D_ptr, Implement::texture3D_ptr>& v
);
*/

struct test_plugin
{

	PO::raw_scene scene;

	tex2 back;
	tex2 g_poi;
	tex2 g_col;

	tex3 da;

	tex3 volum;
	tex3 volum_shadow;

	input_assember_stage cube_ia;
	vertex_stage cube_vs;
	raterizer_state cube_ra;

	pixel_stage deffer_ps;
	output_merge_stage deffer_om;
	

	input_assember_stage frame_ia;
	pixel_stage frame_ps;
	blend_state frame_bs;
	depth_stencil_state frame_dss;

	pixel_stage volum_ps;
	blend_state volum_bs;
	depth_stencil_state volum_dss;

	input_assember_stage screen_ia;
	vertex_stage screen_vs;
	raterizer_state screen_ra;
	pixel_stage screen_ps;
	output_merge_stage screen_om;
	blend_state screen_bs;
	depth_stencil_state screen_dss;

	

	movement_free_object mfo;

	test_plugin(self_depute<Dx11_ticker> p);

	PO::Respond respond(event& c);
	void tick(self_depute<Dx11_ticker> t, duration da);
	void init(self_depute<Dx11_ticker> t);
	~test_plugin() { std::cout << "~test_plugin" << std::endl; }
};