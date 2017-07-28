#pragma once
#include "po_dx11_defer_renderer\defer_renderer.h"
#include "po_dx\controller.h"
#include "po\plugin.h"
using namespace PO;
using namespace PO::Dx11;

struct new_plugin
{
	int swith_state = 0;
	tex2 worley;
	defer_element compute;
	defer_element output_volume_cube;
	defer_element back_ground;
	//defer_element viewer;
	transfer3D ts1;
	transfer3D ts2;
	showcase s;
	float layer = 0.5;
	float scale = 1.0;
	float max_denstiy;
	adapter_map mapping(self& sel);
	void init(defer_renderer& dr);
	void tick(defer_renderer& dr, duration da);
	Respond respond(event& e);
};
