#include "generator.h"
#include "DirectXTex.h"
#include "compute\volume_cloud_compute.h"


std::atomic_uint generator::count = 0;

adapter_map generator::mapping(self& sel)
{
	return {
		make_member_adapter<defer_renderer_default>(this, &generator::init, &generator::tick)
	};
}

void generator::init(defer_renderer_default& dr, plugins& pl)
{
	if (count != 0) return;
	count = 1;
	pl.find_extension([&, this](stage_instance_extension& sie) {
		{
			uint32_t4 sample_scale = { 20, 40, 80, 160 };
			perlin_noise.create_unordered_access(dr, DXGI_FORMAT::DXGI_FORMAT_R16_FLOAT, { 256, 256, 256 });
			element_compute perlin_noise_output;
			perlin_noise_output << sie.create_compute<compute_generate_perlin_noise_tex3_3d_f1>()
				<< [&](compute_generate_perlin_noise_tex3_3d_f1::property& pot) {
				pot.set_output_f(perlin_noise.cast_unordered_access_view(dr), perlin_noise.size(), sample_scale, { 0.5, 0.25, 0.125, 0.125 });
			} << [&](property_random_point_f& pot) {
				pot.create_uniform_point(dr, compute_generate_perlin_noise_tex3_3d_f1::max_count(sample_scale), { 3456 });
			};
			dr << perlin_noise_output;
		}

		{
			final_perlin_noise.create_unordered_access(dr, DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_TYPELESS, { 256 * 8, 256 * 8 });
			element_compute final_perlin_noise_output;
			final_perlin_noise_output << sie.create_compute<compute_format_tex3_f4_to_2d_u8_4>()
				<< [&](compute_format_tex3_f4_to_2d_u8_4::property& p)
			{
				sample_state ss;
				ss.create(dr);
				p.set(perlin_noise.cast_shader_resource_view(dr), ss, final_perlin_noise.cast_unordered_access_view_as_format(dr, DXGI_FORMAT_R8G8B8A8_UINT), final_perlin_noise.size(), { 256, 256, 8, 8 }, { 1.0f, 0.0, 0.0, 0.0 });
			};
			dr << final_perlin_noise_output;
		}

		{
			worley_noise.create_unordered_access(dr, DXGI_FORMAT_R16G16B16A16_FLOAT, { 256, 256, 256 });
			element_compute perlin_noise_output;
			perlin_noise_output << sie.create_compute<compute_generate_worley_noise_tex3_3d_f4>()
				<< [&](compute_generate_worley_noise_tex3_3d_f4::property& p) {
				p.set_peorperty(worley_noise.cast_unordered_access_view(dr), worley_noise.size(), 2.0);
			}
				<< [&](property_random_point_f3& rpf) {
				rpf.create_uniform_point(dr, compute_generate_worley_noise_tex3_3d_f4::max_count(), { 24567, 345653, 3455 }, -0.2, 1.2);
			};
			dr << perlin_noise_output;
		}

		{
			final_worley_noise_1.create_unordered_access(dr, DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_TYPELESS, { 256 * 8, 256 * 8 });
			element_compute final_worley_noise_output;
			final_worley_noise_output << sie.create_compute<compute_format_tex3_f4_to_2d_u8_4>()
				<< [&](compute_format_tex3_f4_to_2d_u8_4::property& p)
			{
				sample_state ss;
				ss.create(dr);
				p.set(worley_noise.cast_shader_resource_view(dr), ss, final_worley_noise_1.cast_unordered_access_view_as_format(dr, DXGI_FORMAT_R8G8B8A8_UINT), final_worley_noise_1.size(), { 256, 256, 8, 8 }, { 1.0f, 0.0, 0.0, 0.0 });
			};
			dr << final_worley_noise_output;
		}

		{
			final_worley_noise_2.create_unordered_access(dr, DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_TYPELESS, { 256 * 8, 256 * 8 });
			element_compute final_worley_noise_output;
			final_worley_noise_output << sie.create_compute<compute_format_tex3_f4_to_2d_u8_4>()
				<< [&](compute_format_tex3_f4_to_2d_u8_4::property& p)
			{
				sample_state ss;
				ss.create(dr);
				p.set(worley_noise.cast_shader_resource_view(dr), ss, final_worley_noise_2.cast_unordered_access_view_as_format(dr, DXGI_FORMAT_R8G8B8A8_UINT), final_worley_noise_2.size(), { 256, 256, 8, 8 }, { 0.0f, 1.0, 0.0, 0.0 });
			};
			dr << final_worley_noise_output;
		}

		{
			final_worley_noise_3.create_unordered_access(dr, DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_TYPELESS, { 256 * 8, 256 * 8 });
			element_compute final_worley_noise_output;
			final_worley_noise_output << sie.create_compute<compute_format_tex3_f4_to_2d_u8_4>()
				<< [&](compute_format_tex3_f4_to_2d_u8_4::property& p)
			{
				sample_state ss;
				ss.create(dr);
				p.set(worley_noise.cast_shader_resource_view(dr), ss, final_worley_noise_3.cast_unordered_access_view_as_format(dr, DXGI_FORMAT_R8G8B8A8_UINT), final_worley_noise_3.size(), { 256, 256, 8, 8 }, { 0.0f, 0.0, 1.0, 0.0 });
			};
			dr << final_worley_noise_output;
		}

		{
			final_worley_noise_4.create_unordered_access(dr, DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_TYPELESS, { 256 * 8, 256 * 8 });
			element_compute final_worley_noise_output;
			final_worley_noise_output << sie.create_compute<compute_format_tex3_f4_to_2d_u8_4>()
				<< [&](compute_format_tex3_f4_to_2d_u8_4::property& p)
			{
				sample_state ss;
				ss.create(dr);
				p.set(worley_noise.cast_shader_resource_view(dr), ss, final_worley_noise_4.cast_unordered_access_view_as_format(dr, DXGI_FORMAT_R8G8B8A8_UINT), final_worley_noise_4.size(), { 256, 256, 8, 8 }, { 0.0f, 0.0, 0.0, 1.0 });
			};
			dr << final_worley_noise_output;
		}

		{
			cube_mask.create_unordered_access(dr, DXGI_FORMAT_R16_FLOAT, { 64, 64, 64 });
			element_compute cube_mask_output;
			cube_mask_output << sie.create_compute<compute_generate_cube_mask_tex3_f>()
				<< [&](compute_generate_cube_mask_tex3_f::property& p) {
				p.set(cube_mask.cast_unordered_access_view(dr), { 64, 64, 64 });
			};
			dr << cube_mask_output;
		}

		{
			final_cube_mask.create_unordered_access(dr, DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_TYPELESS, { 64 * 4, 64 * 4 });
			element_compute final_cube_mask_output;
			final_cube_mask_output << sie.create_compute<compute_format_tex3_f4_to_2d_u8_4>()
				<< [&](compute_format_tex3_f4_to_2d_u8_4::property& p)
			{
				sample_state ss;
				ss.create(dr);
				p.set(cube_mask.cast_shader_resource_view(dr), ss, final_cube_mask.cast_unordered_access_view_as_format(dr, DXGI_FORMAT_R8G8B8A8_UINT), final_cube_mask.size(), { 64, 64, 4, 4 }, { 1.0f, 0.0, 0.0, 0.0 });
			};
			dr << final_cube_mask_output;
		}
	});

}

static int count__ = 0;

void generator::tick(defer_renderer_default& dr, duration da, plugins& pl)
{
	count__++;
	if (count__ == 2)
	{
		CoInitialize(nullptr);

		if (true)
		{
			DirectX::ScratchImage SI;
			DirectX::CaptureTexture(dr.dev, dr.get_context().imp->ptr, perlin_noise.ptr, SI);
			assert(SUCCEEDED(DirectX::SaveToDDSFile(SI.GetImages(), SI.GetImageCount(), SI.GetMetadata(), 0, L"perlin_noise.DDS")));
		}

		if (true)
		{
			DirectX::ScratchImage SI;
			DirectX::CaptureTexture(dr.dev, dr.get_context().imp->ptr, final_perlin_noise.ptr, SI);
			assert(SI.OverrideFormat(DXGI_FORMAT_R8G8B8A8_UNORM));
			assert(SUCCEEDED(DirectX::SaveToDDSFile(SI.GetImages(), SI.GetImageCount(), SI.GetMetadata(), 0, L"final_perlin_noise.DDS")));
			assert(SUCCEEDED(DirectX::SaveToTGAFile(*SI.GetImages(), L"final_perlin_out.tga")));
		}

		if (true)
		{
			DirectX::ScratchImage SI;
			DirectX::CaptureTexture(dr.dev, dr.get_context().imp->ptr, worley_noise.ptr, SI);
			assert(SUCCEEDED(DirectX::SaveToDDSFile(SI.GetImages(), SI.GetImageCount(), SI.GetMetadata(), 0, L"worley_noise.DDS")));
		}

		if (true)
		{
			DirectX::ScratchImage SI;
			DirectX::CaptureTexture(dr.dev, dr.get_context().imp->ptr, final_worley_noise_1.ptr, SI);
			assert(SI.OverrideFormat(DXGI_FORMAT_R8G8B8A8_UNORM));
			assert(SUCCEEDED(DirectX::SaveToDDSFile(SI.GetImages(), SI.GetImageCount(), SI.GetMetadata(), 0, L"final_worley_noise_1.DDS")));
			assert(SUCCEEDED(DirectX::SaveToTGAFile(*SI.GetImages(), L"final_worley_noise_1.tga")));
		}
		if (true)
		{
			DirectX::ScratchImage SI;
			DirectX::CaptureTexture(dr.dev, dr.get_context().imp->ptr, final_worley_noise_2.ptr, SI);
			assert(SI.OverrideFormat(DXGI_FORMAT_R8G8B8A8_UNORM));
			assert(SUCCEEDED(DirectX::SaveToDDSFile(SI.GetImages(), SI.GetImageCount(), SI.GetMetadata(), 0, L"final_worley_noise_2.DDS")));
			assert(SUCCEEDED(DirectX::SaveToTGAFile(*SI.GetImages(), L"final_worley_noise_2.tga")));
		}
		if (true)
		{
			DirectX::ScratchImage SI;
			DirectX::CaptureTexture(dr.dev, dr.get_context().imp->ptr, final_worley_noise_3.ptr, SI);
			assert(SI.OverrideFormat(DXGI_FORMAT_R8G8B8A8_UNORM));
			assert(SUCCEEDED(DirectX::SaveToDDSFile(SI.GetImages(), SI.GetImageCount(), SI.GetMetadata(), 0, L"final_worley_noise_3.DDS")));
			assert(SUCCEEDED(DirectX::SaveToTGAFile(*SI.GetImages(), L"final_worley_noise_3.tga")));
		}
		if (true)
		{
			DirectX::ScratchImage SI;
			DirectX::CaptureTexture(dr.dev, dr.get_context().imp->ptr, final_worley_noise_4.ptr, SI);
			assert(SI.OverrideFormat(DXGI_FORMAT_R8G8B8A8_UNORM));
			assert(SUCCEEDED(DirectX::SaveToDDSFile(SI.GetImages(), SI.GetImageCount(), SI.GetMetadata(), 0, L"final_worley_noise_4.DDS")));
			assert(SUCCEEDED(DirectX::SaveToTGAFile(*SI.GetImages(), L"final_worley_noise_4.tga")));
		}
		if (true)
		{
			DirectX::ScratchImage SI;
			DirectX::CaptureTexture(dr.dev, dr.get_context().imp->ptr, cube_mask.ptr, SI);
			assert(SUCCEEDED(DirectX::SaveToDDSFile(SI.GetImages(), SI.GetImageCount(), SI.GetMetadata(), 0, L"cube_mask.DDS")));
		}
		if (true)
		{
			DirectX::ScratchImage SI;
			DirectX::CaptureTexture(dr.dev, dr.get_context().imp->ptr, final_cube_mask.ptr, SI);
			assert(SI.OverrideFormat(DXGI_FORMAT_R8G8B8A8_UNORM));
			assert(SUCCEEDED(DirectX::SaveToDDSFile(SI.GetImages(), SI.GetImageCount(), SI.GetMetadata(), 0, L"final_cube_mask.DDS")));
			assert(SUCCEEDED(DirectX::SaveToTGAFile(*SI.GetImages(), L"final_cube_mask.tga")));
		}
	}
}