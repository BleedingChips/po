#include "generator.h"
#include "DirectXTex.h"
#include "compute\volume_cloud_compute.h"
#include "imagefileio.h"

adapter_map generator::mapping(self& sel)
{
	return {
		//make_member_adapter<defer_renderer_default>(this, &generator::init, &generator::tick)
	};
}

void generator::init(defer_renderer_default& dr, plugins& pl)
{
	CoInitialize(nullptr);

	pl.find_extension([&, this](stage_instance_extension& sie) {

		tex3 DensityMap;
		DensityMap.create_unordered_access(dr, DXGI_FORMAT_R16_FLOAT, { 64, 64, 64 });
		element_compute ele;
		ele << sie.create_compute<DensityMap3DGenerator>()
			<< [&](DensityMap3DGenerator::property& p) {
			p.Length = 5.0;
			p.output = DensityMap.cast_unordered_access_view(dr);
			p.output_size = DensityMap.size();
			p.block = { 3, 3, 3 };
		}
			<< [&](property_random_point_f3& rpf)
		{
			rpf.create_uniform_point(dr, 200, { 96564563,346345,4557567 }, 0.0, 1.0);
		}
		;
		dr << ele;
		dr.insert_task([=](defer_renderer_default& dr) {
			assert(SaveToDDS(dr, DensityMap, u"GOGOG.DDS"));
		});

		SDF_Output.create_unordered_access(dr, DXGI_FORMAT_R16G16B16A16_FLOAT, { 64, 64, 64 });
		SDF << sie.create_compute<SignedDistanceField3DGenerator>()
			<< [&](SignedDistanceField3DGenerator::property& p) {
			p.EdgeValue = 0.3f;
			p.InputFactor = float4(1.0, 0.0, 0.0, 0.0);
			p.inputTexture = DensityMap.cast_shader_resource_view(dr);
			p.input_size = DensityMap.size();
			p.MaxCount = 4000;
			p.MaxDistance = 0.25f;
			p.outputTexture = SDF_Output.cast_unordered_access_view(dr);
			p.output_size = SDF_Output.size();
			p.reset();
		}
		;
	});

}

void generator::tick(defer_renderer_default& dr, duration da, plugins& pl, self& s)
{
	
	if (SDF.ptr->compute)
	{
		bool Finish = false;
		SDF << [&](SignedDistanceField3DGenerator::property& p) {
			if (p.need_next())
			{
				dr << SDF;
			}
			else {
				Finish = true;
				dr.insert_task([=](defer_renderer_default& dr) {
					assert(SaveToDDS(dr, SDF_Output, u"SDF_GOGOG.DDS"));
				});
			}
		}
		;
		if (Finish)
			SDF.ptr->compute.reset();
	}
}