#pragma once
#include "po_dx11\dx11_renderer.h"
#include "po_dx\controller.h"
#include "po\plugin.h"
using namespace PO;
using namespace PO::Dx11;

struct new_plugin
{
	int swith_state = 0;

	tex2 final_perlin_noise;
	tex2 final_worley_noise;
	tex2 cube_mask;

	tex3 debug_tex;
	tex3 debug_tex2;
	tex3 debug_tex3;

	tex2 final_tex0;
	tex2 final_tex1;
	

	element_draw back_ground;
	element_draw frame;
	element_draw output_volume_cube;
	element_draw output_volume_cube_frame;

	//defer_element viewer;
	transfer3D ts1;
	transfer3D ts2;
	showcase s;

	size_t index = 0;
	float4 Value;

	tex2 TiledWorly;
	tex3 TiledWorly3D;

	float layer = 0.5;
	float scale = 1.0;
	float max_denstiy;
	adapter_map mapping(self& sel);
	void init(defer_renderer_default& dr, plugins& pl);
	void tick(defer_renderer_default& dr, duration da, plugins& pl);
	Respond respond(const event& e);
};