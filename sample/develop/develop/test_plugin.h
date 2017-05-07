#pragma once
#include <iostream>
#include "form_define.h"
#include "../../../gui/dx11/dx11_frame.h"
#include <DirectXTex.h>
#include "../../../tool/mail.h"
#include "../../../gui/dx/move_style.h"
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

struct alignas(alignof(movement_interpolation)) test_plugin
{

	PO::raw_scene scene;

	tex3 vt;
	tex3 vt_shadow;
	tex3 t;

	input_assember_stage cube_ia_d;
	vertex_stage cube_vs_d;

	raterizer_state cube_ra_d;
	pixel_stage cube_ps_d;
	
	output_merge_stage cube_m;

	input_assember_stage frame_cube_ia_d;
	pixel_stage frame_cube_ps_d;

	 movement_interpolation inter;

	float angle_a = 0.0, angle_b = 0.0, dis = 2.0;

	test_plugin(self_depute<Dx11_ticker> p);

	PO::Respond respond(event& c);
	void tick(self_depute<Dx11_ticker> t, duration da);
	void init(self_depute<Dx11_ticker> t);
	~test_plugin() { std::cout << "~test_plugin" << std::endl; }
};