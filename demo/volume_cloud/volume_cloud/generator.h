#pragma once
#include "../../po/include/po/plugin.h"
#include "../../po/include/po_dx11/dx11_form.h"
#include "../../po/include/po_dx11/dx11_renderer.h"
using namespace PO;
using namespace PO::Dx11;

struct generator
{
	element_compute SDF_ELE;
	tex3 SDF_3d_Inside;
	tex3 SDF_3d_Outside;
	adapter_map mapping(self& sel);
	void init(defer_renderer_default& dr, plugins& pl);
	void tick(defer_renderer_default& dr, duration da, plugins& pl, self&);
};