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
		
		tex3 tiling_3d_worley_noise;
		tiling_3d_worley_noise.create_unordered_access(dr, DXGI_FORMAT_R16_FLOAT, { 256, 256, 256 });
		if (true)
		{
			element_compute ele;
			uint32_t point_count = 200;
			ele << sie.create_compute<compute_3D_tiled>()
				<< [&](compute_3D_tiled::property& p) {
				p.output = tiling_3d_worley_noise.cast_unordered_access_view(dr);
				p.output_size = tiling_3d_worley_noise.size();
				p.count = point_count;
				p.Lenght = 10.0f;
			}
				<< [&](property_random_point_f3& prp)
			{
				prp.create_uniform_point(dr, point_count, { 1253,467424,456524 }, 0.0, 1.0);
			}
			;
			dr << ele;
			dr.insert_task([=](auto& dtr) {
				assert(SaveToDDS(dtr, tiling_3d_worley_noise, u"DebugOutput.DDS"));
			});
			
			try {
				uint32_t3 target_size = { 64, 64, 64 };
				std::vector<float4> att(target_size.x * target_size.y * target_size.z, float4(10.0, 10.0, 10.0, 10.0));
				tex3_source tex{ att.data(), target_size.x * sizeof(float4), target_size.x * target_size.y * sizeof(float4) };

				SDF_3d_Inside.create_unordered_access(dr, DXGI_FORMAT_R32G32B32A32_FLOAT, target_size, 1, &tex);
				SDF_3d_Outside.create_unordered_access(dr, DXGI_FORMAT_R32G32B32A32_FLOAT, target_size, 1, &tex);
				if (true)
				{
					SDF_ELE << sie.create_compute<SDF_3dGenerator>()
						<< [&](SDF_3dGenerator::property& p) {
						p.step_add = uint3{ 40, 40, 40 };
						p.EdgeValue = 0.3f;
						p.output_size = SDF_3d_Inside.size();
						p.InsideTexture = SDF_3d_Inside.cast_unordered_access_view(dr);
						p.OutsideTexture = SDF_3d_Outside.cast_unordered_access_view(dr);
						p.Input = tiling_3d_worley_noise.cast_shader_resource_view(dr);
						p.input_size = tiling_3d_worley_noise.size();
						p.DistanceMulity = float3{ 2, 2, 2 };
					};
				}
			}
			catch (...)
			{
				__debugbreak();
			}
		}

	});

}

void generator::tick(defer_renderer_default& dr, duration da, plugins& pl, self& s)
{
	if (SDF_ELE.ptr->compute)
	{
		SDF_ELE << [&](SDF_3dGenerator::property& p) {
			if (p.next())
			{
				std::cout << "[ " << p.input_start << " : " << p.input_end << "]" << std::endl;
				dr << SDF_ELE;
			}
			else {
				p.final_call = 1;
				dr << SDF_ELE;
				dr.insert_task([this](defer_renderer_default& drt) {
						assert(SaveToDDS(drt, SDF_3d_Inside, u"SDFInside.DDS"));
					std::cout << "Finsih" << std::endl;
				});
				s.killmyself();
			}
		};
	}
	
}