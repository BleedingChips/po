#pragma once
#include <iostream>
#include "form_define.h"
#include "../../../gui/dx11/dx11_frame.h"
#include "../../../tool/mail.h"
#include "../../../gui/dx/movement.h"
#include "../../../gui/dx11/dx11_form.h"

using namespace PO::Dx;
using namespace PO::Dx11;
using namespace PO;

struct UE4_testing
{
	tex2 volume_texture;

	input_assember_stage ia;
	vertex_stage vs;
	pixel_stage ps;
	output_merge_stage om;
	blend_state bs;
	depth_stencil_state dss;

	raw_scene scene;

	opposite_direct od;
	opposite_direct left_right;
	opposite_direct up_down;
	opposite_direct pp2;

	movement_free_object mfo;

	UE4_testing(peek<Dx11_ticker> p);
	void init(self_depute<Dx11_ticker> p);
	void tick(self_depute<Dx11_ticker> p, duration da);
	Respond respond(event& ev);
};