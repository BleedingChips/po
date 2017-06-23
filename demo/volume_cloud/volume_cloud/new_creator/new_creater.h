#pragma once
#include <iostream>
#include "po/tool/mail.h"
#include "dx/movement.h"
#include "DirectXTex.h"
#include "dx11/dx11_renderer.h"
#include "po/plugin.h"
#include "po/tool/scene.h"
using namespace PO::Dx;
using namespace PO::Dx11;
using namespace PO;

struct control_input {
	alignas(16) float4 filter = float4{ 1.0, 1.0, 1.0, 1.0 };
	alignas(16) float layout = 0;
};


struct new_creator
{
	raw_scene rs;

	tex3 texture;

	input_assember_stage ias;
	pixel_stage ps;
	vertex_stage vs;
	depth_stencil_state dss;
	output_merge_stage oms;

	control_input con;

	const UINT max_layer = 256;

	new_creator();

	adapter_map mapping(self& s);

	void init(simple_renderer& s);
	void tick(simple_renderer& s);
	Respond respond(event& e);
};