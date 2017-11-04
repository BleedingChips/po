#pragma once
#include "../../po/include/po/plugin.h"
#include "../../po/include/po_dx11/dx11_form.h"
#include "../../po/include/po_dx11/dx11_renderer.h"
using namespace PO;
using namespace PO::Dx11;

struct generator
{
	tex3 perlin_noise;
	tex2 final_perlin_noise;

	tex3 worley_noise;
	tex2 final_worley_noise_1;
	tex2 final_worley_noise_2;
	tex2 final_worley_noise_3;
	tex2 final_worley_noise_4;

	tex3 cube_mask;
	tex2 final_cube_mask;

	tex3 new_perlin[4];
	tex2 final_perlin_output[2];
	tex3 helpText;

	tex2 tiled_nose[2];
	tex3 tiled_worley;

	static std::atomic_uint count;
	static bool ready() { return count >= 2; }
	adapter_map mapping(self& sel);
	void init(defer_renderer_default& dr, plugins& pl);
	void tick(defer_renderer_default& dr, duration da, plugins& pl);
};