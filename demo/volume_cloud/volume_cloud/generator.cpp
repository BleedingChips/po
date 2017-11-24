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

	pl.find_extension([&, this](stage_instance_extension& sie) {
		
		tex3 tiling_3d_worley_noise;
		tiling_3d_worley_noise.create_unordered_access(dr, DXGI_FORMAT_R16_FLOAT, { 512, 512, 512 });
		if (false)
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

		if (true)
		{
			element_compute compute;
			uint32_t3 block = {10, 10, 10};
			uint WorleyCount = 200;
			tex3 output;
			output.create_unordered_access(dr, DXGI_FORMAT::DXGI_FORMAT_R16_FLOAT, { 256, 256, 256 });
			compute << sie.create_compute<generator_perlin_worley_3D_tiless_noise>()
				<< [&](generator_perlin_worley_3D_tiless_noise::property& p) {
				p.input = output.cast_unordered_access_view(dr);
				p.size = output.size();
				p.block = block;
				p.worley_count = WorleyCount;
				p.Length = 5.0f;
			}
				<< [&](indexed_property<property_random_point_f3, 0>& p) {
				p.create_uniform_point(dr, block.x * block.y * block.z, { 2343,3557231,35653 }, 0.0, 3.141592653f * 2.0f);
			}
				<< [&](indexed_property<property_random_point_f3, 1>& p) {
				p.create_uniform_point(dr, WorleyCount, { 2334443,355237231,3565323 }, 0.0, 1.0);
			}
			;
			dr << compute;

			tex2 FinalOutput;
			FinalOutput.create_unordered_access(dr, DXGI_FORMAT_R8G8B8A8_TYPELESS, { 256 * 8, 256 * 8 });
			element_compute ele2;
			ele2 << sie.create_compute<compute_format_tex3_f4_to_2d_u8_4>()
				<< [&](compute_format_tex3_f4_to_2d_u8_4::property& p) {
				p.output = FinalOutput.cast_unordered_access_view_as_format(dr, DXGI_FORMAT_R8G8B8A8_UINT);
				p.srv = output.cast_shader_resource_view(dr);
				p.value_factor = float4(1.0, 0.0, 0.0, 0.0);
				p.texture_size = FinalOutput.size();
				p.simulate_size = uint32_t4{256, 256, 8, 8};
			};
			dr << ele2;



			dr.insert_task([=](defer_renderer_default& dfd) {

				{
					DirectX::ScratchImage SI;
					HRESULT re = DirectX::CaptureTexture(dfd.dev, dfd.get_context().imp->ptr, output.ptr, SI);
					assert(SUCCEEDED(DirectX::SaveToDDSFile(SI.GetImages(), SI.GetImageCount(), SI.GetMetadata(), 0, L"Output_pre.DDS")));
				}
				{
					DirectX::ScratchImage SI;
					HRESULT re = DirectX::CaptureTexture(dfd.dev, dfd.get_context().imp->ptr, FinalOutput.ptr, SI);
					assert(SI.OverrideFormat(DXGI_FORMAT_R8G8B8A8_UNORM));
					assert(SUCCEEDED(DirectX::SaveToDDSFile(SI.GetImages(), SI.GetImageCount(), SI.GetMetadata(), 0, L"Output.DDS")));
					assert(SUCCEEDED(DirectX::SaveToTGAFile(*SI.GetImages(), L"Output.TGA")));
				}
			});
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
				std::cout << "[ " << p.input_start << " : " << p.input_end << std::endl;
				dr << SDF_ELE;
			}
			else {
				p.final_call = 1;
				dr << SDF_ELE;
				dr.insert_task([this](defer_renderer_default& drt) {
					{
						DirectX::ScratchImage SI;
						HRESULT re = DirectX::CaptureTexture(drt.dev, drt.get_context().imp->ptr, SDF_3d_Inside.ptr, SI);
						assert(SUCCEEDED(DirectX::SaveToDDSFile(SI.GetImages(), SI.GetImageCount(), SI.GetMetadata(), 0, L"SDF_3d_Inside.DDS")));
					}
					{
						DirectX::ScratchImage SI;
						HRESULT re = DirectX::CaptureTexture(drt.dev, drt.get_context().imp->ptr, SDF_3d_Outside.ptr, SI);
						assert(SUCCEEDED(DirectX::SaveToDDSFile(SI.GetImages(), SI.GetImageCount(), SI.GetMetadata(), 0, L"SDF_3d_Outside.DDS")));
					}
					std::cout << "Finsih" << std::endl;
				});
				s.killmyself();
			}
		};
	}
	
}