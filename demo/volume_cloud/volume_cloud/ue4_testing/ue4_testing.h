#pragma once
#include <iostream>
#include "dx11/dx11_frame.h"
#include "po/tool/mail.h"
#include "dx/movement.h"
#include "dx11/dx11_renderer.h"
#include "po/plugin.h"
#include "po/tool/scene.h"
#include <wincodec.h>
#pragma comment(lib, "Windowscodecs.lib" )

using namespace PO::Dx;
using namespace PO::Dx11;
using namespace PO;

struct UE4_testing
{
	tex2 volume_texture[20];

	size_t current_view = 0;
	

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

	UE4_testing();
	adapter_map mapping(self&);
	void init(simple_renderer& p);
	void tick(simple_renderer& p, duration da);
	Respond respond(event& ev);
};