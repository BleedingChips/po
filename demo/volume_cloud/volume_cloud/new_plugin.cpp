#include "new_plugin.h"
#include "compute\volume_cloud_compute.h"
#include "material\volume_cloud_material.h"
#include "../DirectXTex/DirectXTex.h"
#include "geometry\ue4_geometry.h"
#include <random>
using namespace std;
using namespace PO::Dx;


adapter_map new_plugin::mapping(self& sel)
{
	max_denstiy = 0.5f;
	s.binding({
		{ KeyValue::K_D, showcase::State::Y_CW },
		{ KeyValue::K_A, showcase::State::Y_ACW },
		{ KeyValue::K_S, showcase::State::X_CW },
		{ KeyValue::K_W, showcase::State::X_ACW },
		{ KeyValue::K_E, showcase::State::Z_CW },
		{ KeyValue::K_Q, showcase::State::Z_ACW },
		{ KeyValue::K_R, showcase::State::T_FR },
		{ KeyValue::K_F, showcase::State::T_BA }
	});
	sel.auto_bind_respond(&new_plugin::respond, this);
	return {
		make_member_adapter<defer_renderer_default>(this, &new_plugin::init, &new_plugin::tick)
	};
	s.set_translation_speed(10.0);
}

Respond new_plugin::respond(const event& e)
{
	Respond re = s.respond(e);
	if (re == Respond::Pass)
	{
		if (e.is_key() && e.key.is_down())
		{
			switch (e.key.value)
			{
			case PO::KeyValue::K_K:
				max_denstiy -= max_denstiy * 0.1f;
				if (max_denstiy < 0.005)
					max_denstiy = 0.005f;
				std::cout << "max_denstiy: " << max_denstiy << std::endl;
				return Respond::Truncation;
			case PO::KeyValue::K_L:
				max_denstiy += max_denstiy * 0.1f;
				std::cout << "max_denstiy: " << max_denstiy << std::endl;
				return Respond::Truncation;
			}
		}
		else if (e.is_key() && e.key.is_up())
		{
			switch (e.key.value)
			{
			case PO::KeyValue::K_1:
				swith_state = 0;
				return Respond::Truncation;
			case PO::KeyValue::K_2:
				swith_state = 1;
				return Respond::Truncation;
			}
		}
	}
	return Respond::Pass;
}

void new_plugin::init(defer_renderer_default& dr, plugins& pl)
{
	s.set_translation_speed(10.0);

	pl.find_extension([&, this](stage_instance_extension& sie) {
		{
			uint32_t4 sample_scale = { 20, 40, 80, 160 };
			perlin_noise.create_unordered_access(dr, DXGI_FORMAT::DXGI_FORMAT_R16_FLOAT, { 256, 256, 256 });
			element_compute perlin_noise_output;
			perlin_noise_output << sie.create_compute<compute_generate_perlin_noise_tex3_3d_f1>()
				<< [&](compute_generate_perlin_noise_tex3_3d_f1::property& pot) {
				pot.set_output_f(perlin_noise.cast_unordered_access_view(dr), perlin_noise.size(), sample_scale, {0.5, 0.25, 0.125, 0.125});
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
		/*
		{
			perlin_out.create_unordered_access(dr, DXGI_FORMAT_R8G8B8A8_TYPELESS, { 256 * 16, 256 * 16 });
			element_compute perlin_execute;
			perlin_execute << sie.create_compute<compute_generate_perlin_noise_uint8_4_2d_simulate_3d>()
				<< [&](property_output_tex2_2d_simulate_3d& cgl) {
				cgl.set_output_texture(perlin_out.cast_unordered_access_view_as_format(dr, DXGI_FORMAT_R8G8B8A8_UINT), perlin_out.size(), { 256, 256, 16, 16 });
			} << [&](property_random_point_f& prp)
			{
				prp.create_uniform_point(dr, compute_generate_perlin_noise_uint8_4_2d_simulate_3d::max_count(), { 2234535 }, -0.2f, 1.2f);
			};
			dr << perlin_execute;
		}
		*/

		/*
		{
			cube_mask_texture.create_unordered_access(dr, DXGI_FORMAT_R8G8B8A8_TYPELESS, { 64 * 8, 64 * 8 });
			element_compute mask_cube;
			mask_cube << sie.create_compute<compute_generate_cube_mark_2d_simulate_3d>()
				<< [&](compute_generate_cube_mark_2d_simulate_3d::property& p) {
				p.set_output_texture_f(cube_mask_texture.cast_unordered_access_view_as_format(dr, DXGI_FORMAT_R8G8B8A8_UINT), cube_mask_texture.size(), { 64, 64, 8, 8 }, float3{ 0.4f,0.1f,0.0f }, float3{ 0.6f, 0.9f, 1.0f });
			};
			dr << mask_cube;
		}

		{
			worley_tex.create_unordered_access(dr, DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_TYPELESS, { 256 * 16, 256 * 16 });
			element_compute worley_generator;
			worley_generator << sie.create_compute<compute_generate_worley_noise_uint8_4_2d_simulate_3d>()
				<< [&](property_generate_worley_noise_uint8_4_2d_simulate_3d& po) {
				po.set_peorperty(
					worley_tex.cast_unordered_access_view_as_format(dr, DXGI_FORMAT_R8G8B8A8_UINT),
					worley_tex.size(),
					{ 256, 256, 16, 16 },
					2.0
				);
			} << [&](property_random_point_f& purp) {
				purp.create_uniform_point(dr, compute_generate_worley_noise_uint8_4_2d_simulate_3d::max_count(), 3456783, -0.2f, 1.2f);
			};
			dr << worley_generator;
		}*/

		{
			ts2.poi = float3(0.0, 0.0, 6.0f);
			back_ground << sie.create_geometry<geometry_cube>()
				<< sie.create_placement<placement_static_viewport_static>()
				<< sie.create_material<material_testing>()
				<< [&](property_local_transfer& pt) {
				pt.set_local_to_world(dr, ts2, ts2.inverse_float4x4());
			} << [&](property_tex2& pt) {
				sample_state ss;
				ss.create(dr);
				pt.set_texture(final_perlin_noise.cast_shader_resource_view_as_format(dr, DXGI_FORMAT_R8G8B8A8_UNORM), ss);
			};
		}

		{
			ts1.poi = float3(0.0, 0.0, 5.0);
			ts1.sca = float3(0.02f, 0.02f, 0.02f);
			output_volume_cube << sie.create_geometry<UE4_cube_static>()
				<< sie.create_placement<placement_static_viewport_static>()
				<< sie.create_material<material_transparent_2d_for_3d_64_without_perlin>()
				<< [&](property_render_2d_for_3d& prf) {
				sample_state::description des = sample_state::default_description;
				des.Filter = decltype(des.Filter)::D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
				des.AddressU = decltype(des.AddressU)::D3D11_TEXTURE_ADDRESS_MIRROR;
				des.AddressV = decltype(des.AddressV)::D3D11_TEXTURE_ADDRESS_MIRROR;
				sample_state ss;
				ss.create(dr, des);
				prf.set_texture(final_worley_noise_3.cast_shader_resource_view_as_format(dr, DXGI_FORMAT_R8G8B8A8_UNORM), ss);
				prf.set_option(float3{ -50.0, -50.0, -50.0 }, float3{ 50.0, 50.0, 50.0 }, float3{ 0.0, -1.0, 0.0 }, max_denstiy);
			}
				<< [&](property_local_transfer& tlt) {
				tlt.set_local_to_world(dr, ts1, ts1.inverse_float4x4());
			} 
				<< [&](property_volume_cloud_tex& pvct) {
				sample_state ss;
				ss.create(dr);
				pvct.set_base_shape(final_perlin_noise.cast_shader_resource_view_as_format(dr, DXGI_FORMAT_R8G8B8A8_UNORM), ss);
				pvct.set_mask(final_cube_mask.cast_shader_resource_view_as_format(dr, DXGI_FORMAT_R8G8B8A8_UNORM), ss);
				pvct.set_move_mask(final_worley_noise_3.cast_shader_resource_view_as_format(dr, DXGI_FORMAT_R8G8B8A8_UNORM), ss);
			};
		}
	});
	
	//ts1.sca = float3(1.0, 0.5, 1.0);
	
}

static int count__ = 0;

void new_plugin::tick(defer_renderer_default& dr, duration da, plugins& pl)
{
	count__++;
	if (count__ == 10)
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

		/*
		if(true)
		{
			DirectX::ScratchImage SI;
			DirectX::CaptureTexture(dr.dev, dr.get_context().imp->ptr, worley_tex.ptr, SI);
			assert(SI.OverrideFormat(DXGI_FORMAT_R8G8B8A8_UNORM));
			assert(SUCCEEDED(DirectX::SaveToWICFile(*SI.GetImages(), DirectX::WIC_FLAGS_NONE, DirectX::GetWICCodec(DirectX::WIC_CODEC_PNG), L"worley_tex.png")));
		
			if(false)
				if (!
					SUCCEEDED(DirectX::SaveToDDSFile(SI.GetImages(), SI.GetImageCount(), SI.GetMetadata(), 0, L"worley_tex.DDS"))
					) __debugbreak();
			if(false)
				if (!
					SUCCEEDED(DirectX::SaveToWICFile(*SI.GetImages(), DirectX::WIC_FLAGS_FORCE_RGB, DirectX::GetWICCodec(DirectX::WIC_CODEC_TIFF), L"NEW_IMAGE.tif"))
					) __debugbreak();
		}

		if (true)
		{
			DirectX::ScratchImage SI;
			DirectX::CaptureTexture(dr.dev, dr.get_context().imp->ptr, perlin_out.ptr, SI);
			assert(SI.OverrideFormat(DXGI_FORMAT_R8G8B8A8_UNORM));
			assert(SUCCEEDED(DirectX::SaveToWICFile(*SI.GetImages(), DirectX::WIC_FLAGS_NONE, DirectX::GetWICCodec(DirectX::WIC_CODEC_PNG), L"perlin_out.png")));
		}

		if (true)
		{
			DirectX::ScratchImage SI;
			DirectX::CaptureTexture(dr.dev, dr.get_context().imp->ptr, cube_mask_texture.ptr, SI);
			assert(SI.OverrideFormat(DXGI_FORMAT_R8G8B8A8_UNORM));
			assert(SUCCEEDED(DirectX::SaveToWICFile(*SI.GetImages(), DirectX::WIC_FLAGS_NONE, DirectX::GetWICCodec(DirectX::WIC_CODEC_PNG), L"cube_mask_texture.png")));
		}
		
		if(false)
		{
			DirectX::ScratchImage SI;
			
			DirectX::CaptureTexture(dr.dev, dr.context.imp->ptr, perlin_out.ptr, SI);
			assert(SI.OverrideFormat(DXGI_FORMAT_R8G8B8A8_UNORM));
			assert(SUCCEEDED(DirectX::SaveToDDSFile(SI.GetImages(), SI.GetImageCount(), SI.GetMetadata(), 0, L"OUTPUT.DDS")));
		}

		if (true)
		{
			//DirectX::ScratchImage SI;
			//DirectX::CaptureTexture(dr.dev, dr.context.imp->ptr, merga_texture.ptr, SI);
			//assert(SI.OverrideFormat(DXGI_FORMAT_R8G8B8A8_UNORM));
			//assert(SUCCEEDED(DirectX::SaveToWICFile(*SI.GetImages(), DirectX::WIC_FLAGS_FORCE_RGB, DirectX::GetWICCodec(DirectX::WIC_CODEC_JPEG), L"merga_texture.JPEG")));
		}
		*/
		std::cout << "finish" << std::endl;
			
	}




	if (swith_state == 0)
	{
		if (s.apply(da, ts1))
		{
			
			output_volume_cube << [&, this](property_local_transfer& pt) {
				pt.set_local_to_world(dr, ts1, ts1.inverse_float4x4());
			};
			
		}
	}
	else if (swith_state == 1)
	{
		if (s.apply(da, ts2))
		{
			
			back_ground << [&](property_local_transfer& pt) {
				pt.set_local_to_world(dr, ts2, ts2.inverse_float4x4());
			};
			
		}
	}
	
	output_volume_cube << [&, this](property_render_2d_for_3d& pt) {
		pt.set_option(float3{ -50.0, -50.0, -50.0 }, float3{ 50.0, 50.0, 50.0 }, float3{ 0.0, -1.0, 0.0 }, max_denstiy);
	};
	
	dr.pipeline_opaque() << back_ground;
	dr.pipeline_transparent() << output_volume_cube;
}