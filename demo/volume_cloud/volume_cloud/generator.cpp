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
			//dr << perlin_noise_output;
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
			//dr << final_perlin_noise_output;
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
			//dr << perlin_noise_output;
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
			//dr << final_worley_noise_output;
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
			//dr << final_worley_noise_output;
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
			//dr << final_worley_noise_output;
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
			//dr << final_worley_noise_output;
		}

		{
			cube_mask.create_unordered_access(dr, DXGI_FORMAT_R16_FLOAT, { 64, 64, 64 });
			element_compute cube_mask_output;
			cube_mask_output << sie.create_compute<compute_generate_cube_mask_tex3_f>()
				<< [&](compute_generate_cube_mask_tex3_f::property& p) {
				p.set(cube_mask.cast_unordered_access_view(dr), { 64, 64, 64 });
			};
			//dr << cube_mask_output;
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
			//dr << final_cube_mask_output;
		}

		if(false)
		{
			std::array<unordered_access_view<tex3>, 5> da;
			for (size_t i = 0; i < 2; ++i)
			{
				new_perlin[i].create_unordered_access(dr, DXGI_FORMAT_R16_FLOAT, { 256, 256, 64 });
				da[i] = new_perlin[i].cast_unordered_access_view(dr);
			}
			//helpText.create_unordered_access(dr, DXGI_FORMAT_R16_FLOAT, {64, 64, 64});
			//da[4] = helpText.cast_unordered_access_view(dr);
			element_compute new_perlin_element;
			new_perlin_element << sie.create_compute<compute_generator>()
				<< [&, this](compute_generator::property& p)
			{
				p << da;
			}/* << [&, this](property_random_point_f3& rd) {
				//rd.craate_custom(dr, 800, { 123234,231254,6878 });
			}*/;
			dr << new_perlin_element;

			for (size_t i = 0; i < 2; ++i)
			{
				final_perlin_output[i].create_unordered_access(dr, DXGI_FORMAT_R8G8B8A8_TYPELESS, { 256 * 4, 256 * 4 });
				element_compute output;
				output << sie.create_compute<compute_format_tex3_f4_to_2d_u8_4>()
					<< [&](compute_format_tex3_f4_to_2d_u8_4::property& p)
				{
					sample_state ss;
					ss.create(dr);
					p.set(new_perlin[i].cast_shader_resource_view(dr), ss, final_perlin_output[i].cast_unordered_access_view_as_format(dr, DXGI_FORMAT_R8G8B8A8_UINT), final_perlin_output[i].size(), { 256, 256, 4, 4 }, { 1.0f, 0.0, 0.0, 0.0 });
				};
				dr << output;
			}
		}

		if (true)
		{
			std::array<unordered_access_view<tex2>, 2> tex;
			for (size_t i = 0; i < 2; ++i)
			{
				tiled_nose[i].create_unordered_access(dr, DXGI_FORMAT_R16G16_FLOAT, { 256, 256 });
				tex[i] = tiled_nose[i].cast_unordered_access_view(dr);
			}
			element_compute ele;
			ele << sie.create_compute<compute_2D_tiled>()
				<< [&](compute_2D_tiled::property& p)
			{
				p << tex;
			}
				<< [&](property_random_point_f& p)
			{
				p.create_uniform_point(dr, 200, { 234 });
			}
				<< [&](property_random_point_f3& p)
			{
				p.create_uniform_point(dr, 200, { 435,62245,352351 });
			}
			;
			dr << ele;

			{

			}

			tiled_worley.create_unordered_access(dr, DXGI_FORMAT_R16_FLOAT, { 32, 32, 32 });
			{
				element_compute ele;
				ele << sie.create_compute<compute_3D_tiled>()
					<< [&](compute_3D_tiled::property& p)
				{
					p << tiled_worley.cast_unordered_access_view(dr);
				}
					<< [&](property_random_point_f3& p)
				{
					p.create_uniform_point(dr, 200, { 435,62245,352351 });
				}
				;
				dr << ele;
			}
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
			{
				DirectX::ScratchImage SI;
				DirectX::CaptureTexture(dr.dev, dr.get_context().imp->ptr, tiled_nose[0].ptr, SI);
				assert(SUCCEEDED(DirectX::SaveToDDSFile(SI.GetImages(), SI.GetImageCount(), SI.GetMetadata(), 0, L"tiled_noise0.DDS")));
			}
			{
				DirectX::ScratchImage SI;
				DirectX::CaptureTexture(dr.dev, dr.get_context().imp->ptr, tiled_worley.ptr, SI);
				assert(SUCCEEDED(DirectX::SaveToDDSFile(SI.GetImages(), SI.GetImageCount(), SI.GetMetadata(), 0, L"tiled_noise2.DDS")));
			}
		}


		if (false)
		{
			{
				DirectX::ScratchImage SI;
				DirectX::CaptureTexture(dr.dev, dr.get_context().imp->ptr, new_perlin[0].ptr, SI);
				assert(SUCCEEDED(DirectX::SaveToDDSFile(SI.GetImages(), SI.GetImageCount(), SI.GetMetadata(), 0, L"new_perlin0.DDS")));
			}
			{
				DirectX::ScratchImage SI;
				DirectX::CaptureTexture(dr.dev, dr.get_context().imp->ptr, new_perlin[1].ptr, SI);
				assert(SUCCEEDED(DirectX::SaveToDDSFile(SI.GetImages(), SI.GetImageCount(), SI.GetMetadata(), 0, L"new_perlin1.DDS")));
			}

			{
				DirectX::ScratchImage SI;
				DirectX::CaptureTexture(dr.dev, dr.get_context().imp->ptr, final_perlin_output[0].ptr, SI);
				assert(SI.OverrideFormat(DXGI_FORMAT_R8G8B8A8_UNORM));
				assert(SUCCEEDED(DirectX::SaveToTGAFile(*SI.GetImages(), L"final_perlin_out0.tga")));
			}
			{
				DirectX::ScratchImage SI;
				DirectX::CaptureTexture(dr.dev, dr.get_context().imp->ptr, final_perlin_output[1].ptr, SI);
				assert(SI.OverrideFormat(DXGI_FORMAT_R8G8B8A8_UNORM));
				assert(SUCCEEDED(DirectX::SaveToTGAFile(*SI.GetImages(), L"final_perlin_out1.tga")));
			}
			/*
			{
				DirectX::ScratchImage SI;
				DirectX::CaptureTexture(dr.dev, dr.get_context().imp->ptr, new_perlin[2].ptr, SI);
				assert(SUCCEEDED(DirectX::SaveToDDSFile(SI.GetImages(), SI.GetImageCount(), SI.GetMetadata(), 0, L"new_perlin2.DDS")));
			}
			{
				DirectX::ScratchImage SI;
				DirectX::CaptureTexture(dr.dev, dr.get_context().imp->ptr, new_perlin[3].ptr, SI);
				assert(SUCCEEDED(DirectX::SaveToDDSFile(SI.GetImages(), SI.GetImageCount(), SI.GetMetadata(), 0, L"new_perlin3.DDS")));
			}

			{
				DirectX::ScratchImage SI;
				DirectX::CaptureTexture(dr.dev, dr.get_context().imp->ptr, new_perlin[4].ptr, SI);
				assert(SUCCEEDED(DirectX::SaveToDDSFile(SI.GetImages(), SI.GetImageCount(), SI.GetMetadata(), 0, L"new_perlin4.DDS")));
			}*/
		}

		if (false)
		{
			DirectX::ScratchImage SI;
			DirectX::CaptureTexture(dr.dev, dr.get_context().imp->ptr, perlin_noise.ptr, SI);
			assert(SUCCEEDED(DirectX::SaveToDDSFile(SI.GetImages(), SI.GetImageCount(), SI.GetMetadata(), 0, L"perlin_noise.DDS")));
		}

		if (false)
		{
			DirectX::ScratchImage SI;
			DirectX::CaptureTexture(dr.dev, dr.get_context().imp->ptr, final_perlin_noise.ptr, SI);
			assert(SI.OverrideFormat(DXGI_FORMAT_R8G8B8A8_UNORM));
			assert(SUCCEEDED(DirectX::SaveToDDSFile(SI.GetImages(), SI.GetImageCount(), SI.GetMetadata(), 0, L"final_perlin_noise.DDS")));
			assert(SUCCEEDED(DirectX::SaveToTGAFile(*SI.GetImages(), L"final_perlin_out.tga")));
		}

		if (false)
		{
			DirectX::ScratchImage SI;
			DirectX::CaptureTexture(dr.dev, dr.get_context().imp->ptr, worley_noise.ptr, SI);
			assert(SUCCEEDED(DirectX::SaveToDDSFile(SI.GetImages(), SI.GetImageCount(), SI.GetMetadata(), 0, L"worley_noise.DDS")));
		}

		if (false)
		{
			DirectX::ScratchImage SI;
			DirectX::CaptureTexture(dr.dev, dr.get_context().imp->ptr, final_worley_noise_1.ptr, SI);
			assert(SI.OverrideFormat(DXGI_FORMAT_R8G8B8A8_UNORM));
			assert(SUCCEEDED(DirectX::SaveToDDSFile(SI.GetImages(), SI.GetImageCount(), SI.GetMetadata(), 0, L"final_worley_noise_1.DDS")));
			assert(SUCCEEDED(DirectX::SaveToTGAFile(*SI.GetImages(), L"final_worley_noise_1.tga")));
		}
		if (false)
		{
			DirectX::ScratchImage SI;
			DirectX::CaptureTexture(dr.dev, dr.get_context().imp->ptr, final_worley_noise_2.ptr, SI);
			assert(SI.OverrideFormat(DXGI_FORMAT_R8G8B8A8_UNORM));
			assert(SUCCEEDED(DirectX::SaveToDDSFile(SI.GetImages(), SI.GetImageCount(), SI.GetMetadata(), 0, L"final_worley_noise_2.DDS")));
			assert(SUCCEEDED(DirectX::SaveToTGAFile(*SI.GetImages(), L"final_worley_noise_2.tga")));
		}
		if (false)
		{
			DirectX::ScratchImage SI;
			DirectX::CaptureTexture(dr.dev, dr.get_context().imp->ptr, final_worley_noise_3.ptr, SI);
			assert(SI.OverrideFormat(DXGI_FORMAT_R8G8B8A8_UNORM));
			assert(SUCCEEDED(DirectX::SaveToDDSFile(SI.GetImages(), SI.GetImageCount(), SI.GetMetadata(), 0, L"final_worley_noise_3.DDS")));
			assert(SUCCEEDED(DirectX::SaveToTGAFile(*SI.GetImages(), L"final_worley_noise_3.tga")));
		}
		if (false)
		{
			DirectX::ScratchImage SI;
			DirectX::CaptureTexture(dr.dev, dr.get_context().imp->ptr, final_worley_noise_4.ptr, SI);
			assert(SI.OverrideFormat(DXGI_FORMAT_R8G8B8A8_UNORM));
			assert(SUCCEEDED(DirectX::SaveToDDSFile(SI.GetImages(), SI.GetImageCount(), SI.GetMetadata(), 0, L"final_worley_noise_4.DDS")));
			assert(SUCCEEDED(DirectX::SaveToTGAFile(*SI.GetImages(), L"final_worley_noise_4.tga")));
		}
		if (false)
		{
			DirectX::ScratchImage SI;
			DirectX::CaptureTexture(dr.dev, dr.get_context().imp->ptr, cube_mask.ptr, SI);
			assert(SUCCEEDED(DirectX::SaveToDDSFile(SI.GetImages(), SI.GetImageCount(), SI.GetMetadata(), 0, L"cube_mask.DDS")));
		}
		if (false)
		{
			DirectX::ScratchImage SI;
			DirectX::CaptureTexture(dr.dev, dr.get_context().imp->ptr, final_cube_mask.ptr, SI);
			assert(SI.OverrideFormat(DXGI_FORMAT_R8G8B8A8_UNORM));
			assert(SUCCEEDED(DirectX::SaveToDDSFile(SI.GetImages(), SI.GetImageCount(), SI.GetMetadata(), 0, L"final_cube_mask.DDS")));
			assert(SUCCEEDED(DirectX::SaveToTGAFile(*SI.GetImages(), L"final_cube_mask.tga")));
		}
	}
}