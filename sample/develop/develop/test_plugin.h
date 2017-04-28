#pragma once
#include "form_define.h"
#include "../../../gui/dx11/dx11_pipeline.h"
#include <DirectXTex.h>
#include "../../../tool/mail.h"
#include "../../../gui/dx/move_style.h"
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

	PO::raw_scene scene;

	texture3D_ptr vt;
	texture3D_ptr vt_shadow;

	input_assember_d cube_ia_d;
	vertex_shader_d cube_vs_d;

	raterizer_d cube_ra_d;
	pixel_shader_d cube_ps_d;
	
	output_merge_d cube_m;

	input_assember_d frame_cube_ia_d;
	pixel_shader_d frame_cube_ps_d;

	alignas(alignof(movement_interpolation)) movement_interpolation inter;

	float angle_a = 0.0, angle_b = 0.0, dis = 2.0;

	test_plugin();


	PO::Respond respond(conveyer& c);
	void tick(ticker& t);

	void init(ticker& t);
};