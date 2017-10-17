#include "new_intime.h"
#include "compute\volume_cloud_compute.h"
#include "material\volume_cloud_material.h"
#include "../DirectXTex/DirectXTex.h"
#include "geometry\ue4_geometry.h"
#include <random>
using namespace std;
using namespace PO::Dx;

adapter_map new_intime::mapping(self& sel)
{
	return {
		make_member_adapter<defer_renderer_default>(this, &new_intime::init, &new_intime::tick)
	};
}

void new_intime::init(defer_renderer_default& dr, plugins& pl)
{
	pl.find_extension([&, this](stage_instance_extension& sie) {
		screen << sie.create_geometry<geometry_cube>()
			<< sie.create_placement<placement_static_viewport_static>()
			<< sie.create_material<in_time_material>()
			<< [&](property_local_transfer& plt) {
			transfer3D ts;
			ts.poi = float3(0.0, 0.0, 2.0);
			plt.set_local_to_world(dr, ts, ts.inverse_float4x4());
		};
	});
}
void new_intime::tick(defer_renderer_default& dr, duration da, plugins& pl)
{
	dr.pipeline_opaque() << screen;
}