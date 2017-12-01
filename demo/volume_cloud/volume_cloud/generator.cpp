#include "generator.h"
#include "DirectXTex.h"
#include "compute\volume_cloud_compute.h"
#include "imagefileio.h"

adapter_map generator::mapping(self& sel)
{
	return {
		make_member_adapter<defer_renderer_default>(this, &generator::init, &generator::tick)
	};
}

void generator::init(defer_renderer_default& dr, plugins& pl)
{
	CoInitialize(nullptr);

	pl.find_extension([&, this](stage_instance_extension& sie) {
		tex3 CenterTexture;
		CenterTexture.create_unordered_access(dr, DXGI_FORMAT_R16_FLOAT, { 256,256,256 });
		element_compute ele;
		ele << sie.create_compute<CenterNoiseGenerator>()
			<< [&](CenterNoiseGenerator::property& p){
			p.output = CenterTexture.cast_unordered_access_view(dr);
			p.output_size = CenterTexture.size();
			p.Distance = 10.0;
		}
			<< [&](property_custom_random_point_f3& pcrp)
		{
			pcrp.set_normal(0, 23543, 0.5, 0.15)
				.set_normal(1, 2342342, 0.5, 0.15)
				.set_normal(2, 67853, 0.5, 0.1)
				.set_count(50);
		}
		;
		dr << ele;
		dr.insert_task([=](defer_renderer_default& dr) {
			assert(SaveToDDS(dr, CenterTexture, u"Center.DDS"));
		});
	});

}

void generator::tick(defer_renderer_default& dr, duration da, plugins& pl, self& s)
{
}