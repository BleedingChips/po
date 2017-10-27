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
	Value = float4(0.5f, 0.02f, 18.0f, 0.2f);
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
			case PO::KeyValue::K_T:
				Value.x += 0.1;
				std::cout << "Value.x:" << Value << std::endl;
				return Respond::Truncation;
			case PO::KeyValue::K_Y:
				Value.x -= 0.1;
				std::cout << "Value.x:" << Value << std::endl;
				return Respond::Truncation;
			case PO::KeyValue::K_G:
				Value.y += 0.01f;
				std::cout << "Value.y:" << Value << std::endl;
				return Respond::Truncation;
			case PO::KeyValue::K_H:
				Value.y -= 0.01f;
				std::cout << "Value.y:" << Value << std::endl;
				return Respond::Truncation;
			case PO::KeyValue::K_B:
				Value.z += 0.1;
				std::cout << "Value.z:" << Value << std::endl;
				return Respond::Truncation;
			case PO::KeyValue::K_N:
				Value.z -= 0.1;
				std::cout << "Value.z:" << Value << std::endl;
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

	if (true)
	{
		{
			DirectX::ScratchImage SI;
			assert(SUCCEEDED(DirectX::LoadFromDDSFile(L"new_perlin1.DDS", 0, nullptr, SI)));
			DirectX::TexMetadata MetaData = SI.GetMetadata();
			tex3_source tem{ SI.GetPixels(), static_cast<uint32_t>(SI.GetImages()->rowPitch), static_cast<uint32_t>(SI.GetImages()->slicePitch) };
			assert(debug_tex.create(dr, MetaData.format, { static_cast<uint32_t>(MetaData.width), static_cast<uint32_t>(MetaData.height), static_cast<uint32_t>(MetaData.depth) }, 1, false, &tem));
		}

		{
			DirectX::ScratchImage SI;
			assert(SUCCEEDED(DirectX::LoadFromDDSFile(L"new_perlin0.DDS", 0, nullptr, SI)));
			DirectX::TexMetadata MetaData = SI.GetMetadata();
			tex3_source tem{ SI.GetPixels(), static_cast<uint32_t>(SI.GetImages()->rowPitch), static_cast<uint32_t>(SI.GetImages()->slicePitch) };
			assert(debug_tex2.create(dr, MetaData.format, { static_cast<uint32_t>(MetaData.width), static_cast<uint32_t>(MetaData.height), static_cast<uint32_t>(MetaData.depth) }, 1, false, &tem));
		}

		{
			DirectX::ScratchImage SI;
			assert(SUCCEEDED(DirectX::LoadFromDDSFile(L"new_perlin4.DDS", 0, nullptr, SI)));
			DirectX::TexMetadata MetaData = SI.GetMetadata();
			tex3_source tem{ SI.GetPixels(), static_cast<uint32_t>(SI.GetImages()->rowPitch), static_cast<uint32_t>(SI.GetImages()->slicePitch) };
			assert(debug_tex3.create(dr, MetaData.format, { static_cast<uint32_t>(MetaData.width), static_cast<uint32_t>(MetaData.height), static_cast<uint32_t>(MetaData.depth) }, 1, false, &tem));
		}
	}

	if(false)
	{
		DirectX::ScratchImage SI;
		assert(SUCCEEDED(DirectX::LoadFromTGAFile(L"final_perlin_out.tga", nullptr, SI)));
		DirectX::TexMetadata MetaData = SI.GetMetadata();
		tex2_source tem{SI.GetPixels(), static_cast<uint32_t>(SI.GetImages()->rowPitch)};
		assert(final_perlin_noise.create(dr, MetaData.format, { static_cast<uint32_t>(MetaData.width), static_cast<uint32_t>(MetaData.height) }, 1, false, &tem));
	}
	if (false)
	{
		DirectX::ScratchImage SI;
		assert(SUCCEEDED(DirectX::LoadFromTGAFile(L"final_worley_noise_4.tga", nullptr, SI)));
		DirectX::TexMetadata MetaData = SI.GetMetadata();
		tex2_source tem{ SI.GetPixels(), static_cast<uint32_t>(SI.GetImages()->rowPitch) };
		assert(final_worley_noise.create(dr, MetaData.format, { static_cast<uint32_t>(MetaData.width), static_cast<uint32_t>(MetaData.height) }, 1, false, &tem));
	}
	if (false)
	{
		DirectX::ScratchImage SI;
		assert(SUCCEEDED(DirectX::LoadFromTGAFile(L"final_cube_mask.tga", nullptr, SI)));
		DirectX::TexMetadata MetaData = SI.GetMetadata();
		tex2_source tem{ SI.GetPixels(), static_cast<uint32_t>(SI.GetImages()->rowPitch) };
		assert(cube_mask.create(dr, MetaData.format, { static_cast<uint32_t>(MetaData.width), static_cast<uint32_t>(MetaData.height) }, 1, false, &tem));
	}
	
	pl.find_extension([&, this](stage_instance_extension& sie) {

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
				//pt.set_texture(final_perlin_noise.cast_shader_resource_view(dr), ss);
			}  << [&](CubeSimpleX::property& p)
			{
				p.set_index(0);
			};

		}

		{

			ts1.poi = float3(0.0, 0.0, 5.0);
			ts1.sca = float3(0.02f, 0.02f, 0.02f);

			

			
			output_volume_cube << sie.create_geometry<UE4_cubiods_static>()
				<< sie.create_placement<placement_static_viewport_static>()
				<< sie.create_material<new_material>()
				//<< sie.create_material<in_time_material>()
				//<< sie.create_material<material_transparent_2d_for_3d_64_without_perlin>()
				<< [&](property_render_2d_for_3d& prf) {
				sample_state::description des = sample_state::default_description;
				des.Filter = decltype(des.Filter)::D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
				des.AddressU = decltype(des.AddressU)::D3D11_TEXTURE_ADDRESS_MIRROR;
				des.AddressV = decltype(des.AddressV)::D3D11_TEXTURE_ADDRESS_MIRROR;
				sample_state ss;
				ss.create(dr, des);
				//prf.set_texture(final_worley_noise.cast_shader_resource_view(dr), ss);
				prf.set_option(float3{ -50.0, -50.0, -50.0 }, float3{ 50.0, 50.0, 50.0 }, float3{ 0.0, -1.0, 0.0 }, max_denstiy);
			}
				<< [&](property_local_transfer& tlt) {
				tlt.set_local_to_world(dr, ts1, ts1.inverse_float4x4());
			} 
				<< [&](property_volume_cloud_tex& pvct) {
				sample_state ss;
				ss.create(dr);
				//pvct.set_base_shape(final_perlin_noise.cast_shader_resource_view(dr), ss);
				//pvct.set_mask(cube_mask.cast_shader_resource_view(dr), ss);
				//pvct.set_move_mask(final_worley_noise.cast_shader_resource_view(dr), ss);
			} << [&](new_material::property& d) {
				d.set(max_denstiy);
				sample_state::description dr2{
					D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_MIRROR, D3D11_TEXTURE_ADDRESS_MIRROR, D3D11_TEXTURE_ADDRESS_MIRROR, 0.0f, 1,
					//D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_MIRROR, D3D11_TEXTURE_ADDRESS_MIRROR, D3D11_TEXTURE_ADDRESS_MIRROR, 0.0f, 1,
					D3D11_COMPARISON_NEVER,{ 1.0f,1.0f,1.0f,1.0f }, -FLT_MAX, FLT_MAX };
				sample_state ss;
				ss.create(dr, dr2);
				d.set(debug_tex.cast_shader_resource_view(dr), debug_tex2.cast_shader_resource_view(dr), ss);
			}
			;
		}
	});
	

	
	//ts1.sca = float3(1.0, 0.5, 1.0);
	
}

static int count__ = 0;

void new_plugin::tick(defer_renderer_default& dr, duration da, plugins& pl)
{
	
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
	} << [&](new_material::property& d) {
		d.set(max_denstiy, Value);
	};
	//dr.pipeline_opaque() << frame;
	dr.pipeline_opaque() << back_ground;
	
	dr.pipeline_transparent() << output_volume_cube;
}