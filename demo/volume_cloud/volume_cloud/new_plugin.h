#pragma once
#include "po_dx11_defer_renderer\defer_renderer.h"
#include "po_dx\controller.h"
#include "po\plugin.h"
using namespace PO;
using namespace PO::Dx11;

struct new_plugin
{
	tex2 testing;
	element ele;
	element ele2;
	transfer3D ts;
	showcase s;
	float layer = 0.5;
	adapter_map mapping(self& sel);
	void init(defer_renderer& dr);
	void tick(defer_renderer& dr, duration da);
	Respond respond(event& e);
};