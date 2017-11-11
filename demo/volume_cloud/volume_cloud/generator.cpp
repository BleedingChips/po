#include "generator.h"
#include "DirectXTex.h"
#include "compute\volume_cloud_compute.h"


adapter_map generator::mapping(self& sel)
{
	return {
		make_member_adapter<defer_renderer_default>(this, &generator::init, &generator::tick)
	};
}

void generator::init(defer_renderer_default& dr, plugins& pl)
{
	CoInitialize(nullptr);
}

void generator::tick(defer_renderer_default& dr, duration da, plugins& pl, self& s)
{
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
				p.Lenght = 5.0;
			}
				<< [&](property_random_point_f3& prp)
			{
				prp.create_uniform_point(dr, point_count, { 1253,467424,456524 }, 0.0, 1.0);
			}
			;
			dr << ele;
			dr.insert_task([=](defer_renderer_default& dr) {
				DirectX::ScratchImage SI;
				DirectX::CaptureTexture(dr.dev, dr.get_context().imp->ptr, tiling_3d_worley_noise.ptr, SI);
				assert(SUCCEEDED(DirectX::SaveToDDSFile(SI.GetImages(), SI.GetImageCount(), SI.GetMetadata(), 0, L"tiling_3d_worley_noise.DDS")));
			});
		}

		tex3 SDF_3d;
		SDF_3d.create_unordered_access(dr, DXGI_FORMAT_R8G8B8A8_TYPELESS, {64, 64, 64});
		if (true)
		{
			element_compute ele;

		}


	});
	s.killmyself();
}