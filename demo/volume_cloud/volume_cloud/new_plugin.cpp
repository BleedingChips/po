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
	max_denstiy = 2.0f;
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
				if (max_denstiy < 0.05)
					max_denstiy = 0.05f;
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

void new_plugin::init(defer_renderer_default& dr)
{
	s.set_translation_speed(10.0);
	
	//ts1.sca = float3(1.0, 0.5, 1.0);

	{
		worley.create_unordered_access(dr, DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT, { 256 * 16, 256 * 16 });
		element worley_output;
		worley_output << dr.ins.create_compute<compute_perlin_worley_noise_tex2_3d>()
			<< [&](property_output_tex2& pot) {
			pot.set_texture(dr, worley, 2.0, { 256, 256, 16, 16 });
		} << [&](property_perline_worley_noise_3d_point& pot) {
			pot.set_seed(dr, { 123, 456, 789 });
		};
		//dr << worley_output;
	}
	

	{
		perlin_out.create_unordered_access(dr, DXGI_FORMAT_R8G8B8A8_TYPELESS, { 256 * 16, 256 * 16 });
		element perlin_execute;
		perlin_execute << dr.ins.create_compute<compute_generate_perlin_noise_uint8_4_2d_simulate_3d>()
				<< [&](property_output_tex2_2d_simulate_3d& cgl) {
				cgl.set_output_texture(perlin_out.cast_unordered_access_view_as_format(dr, DXGI_FORMAT_R8G8B8A8_UINT), perlin_out.size(), { 256, 256, 16, 16 });
			} << [&](property_random_point_f& prp)
			{
				prp.create_uniform_point(dr, compute_generate_perlin_noise_uint8_4_2d_simulate_3d::max_count(), { 2234535 }, -0.2f, 1.2f);
			};
		dr << perlin_execute;
	}

	{
		cube_mask_texture.create_unordered_access(dr, DXGI_FORMAT_R16_FLOAT, { 64 * 8, 64 * 8 });
		element mask_cube;
		mask_cube << dr.ins.create_compute<compute_generate_cube_mark_2d_simulate_3d>()
			<< [&](compute_generate_cube_mark_2d_simulate_3d::property& p) {
			p.set_output_texture_f(cube_mask_texture.cast_unordered_access_view(dr), cube_mask_texture.size(), { 64, 64, 8, 8 }, float3{ 0.3f,0.3f,0.0f }, float3{ 0.7f, 0.7f, 1.0f });
		};

		//dr << mask_cube;
	}


	{
		worley_tex.create_unordered_access(dr, DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT, { 256 * 16, 256 * 16 });
		worley_generator << dr.ins.create_compute<compute_generate_worley_noise_float4_2d_simulate_3d>()
			<< [&](property_generate_worley_noise_float4_2d_simulate_3d& po) {
			po.set_peorperty(
				worley_tex.cast_unordered_access_view(dr),
				worley_tex.size(),
				{ 256, 256, 16, 16 },
				2.0
			);
		} << [&](property_random_point_f& purp) {
			purp.create_uniform_point(dr, compute_generate_worley_noise_float4_2d_simulate_3d::max_count(), 3456783, -0.2f, 1.2f);
		};
		//dr << worley_generator;
	}

	{
		ts2.poi = float3(0.0, 0.0, 6.0f);
		back_ground << dr.ins.create_geometry<geometry_cube>()
			<< dr.ins.create_placement<placement_static_viewport_static>()
			<< dr.ins.create_material<material_opaque_tex2_viewer>()
			<< [&](property_local_transfer& pt) {
			pt.set_local_to_world(dr, ts2, ts2.inverse_float4x4());
		} << [&](property_tex2& pt) {
			pt.set_texture(dr, perlin_out.cast_shader_resource_view_as_format(dr, DXGI_FORMAT_R8G8B8A8_UNORM));
		};
	}

	{
		ts1.poi = float3(0.0, 0.0, 5.0);
		ts1.sca = float3(0.02f, 0.02f, 0.02f);
		output_volume_cube << dr.ins.create_geometry<UE4_cube_static>()
			<< dr.ins.create_placement<placement_static_viewport_static>()
			<< dr.ins.create_material<material_transparent_2d_for_3d_64_without_perlin>()
			<< [&](property_render_2d_for_3d& prf) {
			sample_state::description des = sample_state::default_description;
			des.Filter = decltype(des.Filter)::D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
			des.AddressU = decltype(des.AddressU)::D3D11_TEXTURE_ADDRESS_MIRROR;
			des.AddressV = decltype(des.AddressV)::D3D11_TEXTURE_ADDRESS_MIRROR;
			sample_state ss;
			ss.create(dr, des);
			prf.set_texture(worley_tex.cast_shader_resource_view(dr), ss);
			prf.set_option(float3{ -50.0, -50.0, -50.0 }, float3{ 50.0, 50.0, 50.0 }, float3{ 0.0, -1.0, 0.0 }, max_denstiy);
		}
			<< [&](property_local_transfer& tlt) {
			tlt.set_local_to_world(dr, ts1, ts1.inverse_float4x4());
		};
	}
	
}

static int count__ = 0;

void new_plugin::tick(defer_renderer_default& dr, duration da)
{
	count__++;
	if (count__ == 10)
	{

		CoInitialize(nullptr);
		if(false)
		{
			DirectX::ScratchImage SI;
			DirectX::CaptureTexture(dr.dev, dr.context.imp->ptr, worley_tex.ptr, SI);
			if(false)
				if (!
					SUCCEEDED(DirectX::SaveToDDSFile(SI.GetImages(), SI.GetImageCount(), SI.GetMetadata(), 0, L"NEW_IMAGE.DDS"))
					) __debugbreak();
			if(false)
				if (!
					SUCCEEDED(DirectX::SaveToWICFile(*SI.GetImages(), DirectX::WIC_FLAGS_FORCE_RGB, DirectX::GetWICCodec(DirectX::WIC_CODEC_TIFF), L"NEW_IMAGE.tif"))
					) __debugbreak();
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
	
	dr << back_ground;
	//dr << output_volume_cube;
}