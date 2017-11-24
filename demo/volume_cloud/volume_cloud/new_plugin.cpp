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
	Value = float4(0.0f, 0.0f, 0.0f, 0.0f);
	max_denstiy = 0.5f;
	//s.set_translation_speed(0.1);
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
				if (max_denstiy < 0.005f)
					max_denstiy = 0.005f;
				std::cout << "max_denstiy: " << max_denstiy << std::endl;
				return Respond::Truncation;
			case PO::KeyValue::K_L:
				max_denstiy += max_denstiy * 0.1f;
				std::cout << "max_denstiy: " << max_denstiy << std::endl;
				return Respond::Truncation;
			case PO::KeyValue::K_T:
				Value.x += 0.1f;
				std::cout << "Value.x:" << Value << std::endl;
				return Respond::Truncation;
			case PO::KeyValue::K_Y:
				Value.x -= 0.1f;
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
				Value.z += 0.1f;
				std::cout << "Value.z:" << Value << std::endl;
				return Respond::Truncation;
			case PO::KeyValue::K_N:
				Value.z -= 0.1f;
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

	CoInitialize(0);

	s.set_translation_speed(4.0);

	tex3 SDF;
	{
		DirectX::ScratchImage SI;
		assert(SUCCEEDED(DirectX::LoadFromDDSFile(L"SDF_3d_Inside.DDS", 0, nullptr, SI)));
		DirectX::TexMetadata MetaData = SI.GetMetadata();
		tex3_source tem{ SI.GetPixels(), static_cast<uint32_t>(SI.GetImages()->rowPitch), static_cast<uint32_t>(SI.GetImages()->slicePitch) };
		assert(SDF.create(dr, MetaData.format, { static_cast<uint32_t>(MetaData.width), static_cast<uint32_t>(MetaData.height), static_cast<uint32_t>(MetaData.depth) }, 1, false, &tem));
	}

	tex2 HeightMap;
	{
		DirectX::ScratchImage SI;
		assert(SUCCEEDED(DirectX::LoadFromTGAFile(L"ttt.tga", nullptr, SI)));
		DirectX::TexMetadata MetaData = SI.GetMetadata();
		tex2_source tem{ SI.GetPixels(), static_cast<uint32_t>(SI.GetImages()->rowPitch) };
		assert(HeightMap.create(dr, MetaData.format, { static_cast<uint32_t>(MetaData.width), static_cast<uint32_t>(MetaData.height) }, 1, false, &tem));
	}

	tex2 HeightDensity;
	{
		DirectX::ScratchImage SI;
		assert(SUCCEEDED(DirectX::LoadFromTGAFile(L"HeightDensity.tga", nullptr, SI)));
		DirectX::TexMetadata MetaData = SI.GetMetadata();
		tex2_source tem{ SI.GetPixels(), static_cast<uint32_t>(SI.GetImages()->rowPitch) };
		assert(HeightDensity.create(dr, MetaData.format, { static_cast<uint32_t>(MetaData.width), static_cast<uint32_t>(MetaData.height) }, 1, false, &tem));
	}

	tex2 EdgeDensity;
	{
		DirectX::ScratchImage SI;
		assert(SUCCEEDED(DirectX::LoadFromTGAFile(L"EdgeDensity.tga", nullptr, SI)));
		DirectX::TexMetadata MetaData = SI.GetMetadata();
		tex2_source tem{ SI.GetPixels(), static_cast<uint32_t>(SI.GetImages()->rowPitch) };
		assert(EdgeDensity.create(dr, MetaData.format, { static_cast<uint32_t>(MetaData.width), static_cast<uint32_t>(MetaData.height) }, 1, false, &tem));
	}

	tex2 DensityMap;
	{
		DirectX::ScratchImage SI;
		assert(SUCCEEDED(DirectX::LoadFromDDSFile(L"Output.DDS", 0, nullptr, SI)));
		DirectX::TexMetadata MetaData = SI.GetMetadata();
		tex2_source tem{ SI.GetPixels(), static_cast<uint32_t>(SI.GetImages()->rowPitch) };
		assert(DensityMap.create(dr, MetaData.format, { static_cast<uint32_t>(MetaData.width), static_cast<uint32_t>(MetaData.height)}, 1, false, &tem));
	}
	
	pl.find_extension([&, this](stage_instance_extension& sie) {

		{
			ts2.poi = float3(0.0, 0.0, 6.0f);
			
			back_ground << sie.create_geometry<geometry_cube>()
				<< sie.create_placement<placement_static_viewport_static>()
				<< sie.create_material<material_testing>()
				<< [&](property_local_transfer& pt) {
				pt.WorldToLocal = ts2;
				pt.LocalToWorld = ts2.inverse_float4x4();
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
			
			output_volume_cube << sie.create_geometry<UE4_cube_static>()
				<< sie.create_placement<placement_static_viewport_static>()
				<< sie.create_material<Heght2DEdge2DDensity2D>()
				//<< sie.create_material<in_time_material>()
				//<< sie.create_material<material_transparent_2d_for_3d_64_without_perlin>()
				<< [&](property_local_transfer& tlt) {
				tlt.LocalToWorld = ts1;
				tlt.WorldToLocal = ts1.inverse_float4x4();
				//tlt.set_local_to_world(dr, ts1, ts1.inverse_float4x4());
			} << [&](new_new_new_new_material::property& p)
			{
				p.tex = SDF.cast_shader_resource_view(dr);
				p.Value = Value;
				p.Density = max_denstiy;
				p.ss_des.AddressU = decltype(p.ss_des.AddressU)::D3D11_TEXTURE_ADDRESS_WRAP;
				p.ss_des.AddressV = decltype(p.ss_des.AddressU)::D3D11_TEXTURE_ADDRESS_WRAP;
				p.ss_des.AddressW = decltype(p.ss_des.AddressU)::D3D11_TEXTURE_ADDRESS_WRAP;
			} << [&](new_new_new_material::property& p)
			{
				p.Value = Value;
				p.Density = max_denstiy;
				p.BaseShapeTex = HeightMap.cast_shader_resource_view(dr);
			} << [&](Heght2DEdge2DDensity2D::property& p)
			{
				p.Value = Value;
				p.Density = max_denstiy;
				p.ss_des.AddressU = decltype(p.ss_des.AddressU)::D3D11_TEXTURE_ADDRESS_WRAP;
				p.ss_des.AddressV = decltype(p.ss_des.AddressU)::D3D11_TEXTURE_ADDRESS_WRAP;
				p.ss_des.AddressW = decltype(p.ss_des.AddressU)::D3D11_TEXTURE_ADDRESS_WRAP;
				p.ss_des.Filter = decltype(p.ss_des.Filter)::D3D11_FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR;
				p.Edge = EdgeDensity.cast_shader_resource_view(dr);
				p.Height = HeightDensity.cast_shader_resource_view(dr);
				p.DensityMap = DensityMap.cast_shader_resource_view(dr);
			}
			;
		}

		{
			output_volume_cube_frame << sie.create_geometry<UE4_cube_static_Frame>()
				<< sie.create_placement<placement_static_viewport_static>()
				<< sie.create_material<material_testing>();
			output_volume_cube.ptr->mapping.shared_property_to<property_local_transfer>(output_volume_cube_frame.ptr->mapping);
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
				pt.LocalToWorld = ts1;
				pt.WorldToLocal = ts1.inverse_float4x4();
			};
			
		}
	}
	else if (swith_state == 1)
	{
		if (s.apply(da, ts2))
		{

			back_ground << [&](property_local_transfer& pt) {
				pt.LocalToWorld = ts2;
				pt.WorldToLocal = ts2.inverse_float4x4();
			};
			
		}
	}
	
	output_volume_cube << [&, this](property_render_2d_for_3d& pt) {
		pt.set_option(float3{ -50.0, -50.0, -50.0 }, float3{ 50.0, 50.0, 50.0 }, float3{ 0.0, -1.0, 0.0 }, max_denstiy);
	} << [&](Heght2DEdge2DDensity2D::property& d) {
		d.Value = Value;
		d.Density = max_denstiy;
	};
	//dr.pipeline_opaque() << frame;
	dr.pipeline_opaque() << back_ground;
	
	dr.pipeline_transparent() << output_volume_cube;
	//dr.pipeline_transparent() << output_volume_cube_frame;
}