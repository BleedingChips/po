#pragma once
#include "new_plugin.h"
#include "compute\volume_cloud_compute.h"
#include "material\volume_cloud_material.h"
#include "../DirectXTex/DirectXTex.h"
#include "geometry\ue4_geometry.h"
#include <random>
using namespace std;
using namespace PO::Dx;

struct new_intime
{
	element_draw screen;
	adapter_map mapping(self& sel);
	void init(defer_renderer_default& dr, plugins& pl);
	void tick(defer_renderer_default& dr, duration da, plugins& pl);
};