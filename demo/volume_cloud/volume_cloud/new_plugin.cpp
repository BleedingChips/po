#include "new_plugin.h"
#include "compute\volume_cloud_compute.h"
#include "material\volume_cloud_material.h"
#include "../DirectXTex/DirectXTex.h"
#include "geometry\ue4_geometry.h"
#include <random>
#include "imagefileio.h"
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
			};
		}

		{

			ts1.poi = float3(0.0, 0.0, 5.0);
			ts1.sca = float3(0.02f, 0.02f, 0.02f);

			tex3 CenterTexture;
			CenterTexture.create_unordered_access(dr, DXGI_FORMAT_R16_FLOAT, { 128,128, 128 });
			tex2 XY;
			assert(LoadFormTGA(dr, XY, u"XY.tga"));
			tex2 YZ;
			assert(LoadFormTGA(dr, YZ, u"YZ.tga"));
			tex2 XZ;
			assert(LoadFormTGA(dr, XZ, u"XZ.tga"));
			element_compute ele;
			ele << sie.create_compute<CenterNoiseGenerator>()
				<< [&](CenterNoiseGenerator::property& p) {
				p.output = CenterTexture.cast_unordered_access_view(dr);
				p.output_size = CenterTexture.size();
				p.Distance = 6.0;
				p.XY = XY.cast_shader_resource_view(dr);
				p.YZ = YZ.cast_shader_resource_view(dr);
				p.XZ = XZ.cast_shader_resource_view(dr);
			}
				<< [&](property_custom_random_point_f3& pcrp)
			{
				pcrp.set_normal(0, 23543, 0.0, 0.22f)
					.set_normal(1, 2342342, 0.0, 0.22f)
					.set_normal(2, 67853, 0.0, 0.15f)
					.set_count(400);
			}
			;
			dr << ele;
			dr.insert_task([=](defer_renderer_default& dr) {
				assert(SaveToDDS(dr, CenterTexture, u"Center.DDS"));
			});

			output_volume_cube << sie.create_geometry<UE4_cube_static>()
				<< sie.create_placement<placement_static_viewport_static>()
				<< [&](property_local_transfer& tlt) {
				tlt.LocalToWorld = ts1;
				tlt.WorldToLocal = ts1.inverse_float4x4();
			} << [&](property_volumecloud_debug_value& pvdv)
			{
				pvdv.Density = max_denstiy;
				pvdv.InputValue = Value;
				pvdv.XYZSizeOfCube = UE4_cube_static::size();
			} << sie.create_material<DensityMap3D>()
				<< [&](DensityMap3D::property& p) {
				p.DensityMap = CenterTexture.cast_shader_resource_view(dr);
				p.ss_des.Filter = decltype(p.ss_des.Filter)::D3D11_FILTER_MIN_MAG_MIP_LINEAR;
				p.ss_des.AddressU = decltype(p.ss_des.AddressU)::D3D11_TEXTURE_ADDRESS_WRAP;
				p.ss_des.AddressV = decltype(p.ss_des.AddressU)::D3D11_TEXTURE_ADDRESS_WRAP;
				p.ss_des.AddressW = decltype(p.ss_des.AddressU)::D3D11_TEXTURE_ADDRESS_WRAP;
			};
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
	
	output_volume_cube << [&](property_volumecloud_debug_value& pvdv)
	{
		pvdv.Density = max_denstiy;
		pvdv.InputValue = Value;
	}
	;
	//dr.pipeline_opaque() << frame;
	dr.pipeline_opaque() << back_ground;
	
	dr.pipeline_transparent() << output_volume_cube;
	dr.pipeline_transparent() << output_volume_cube_frame;
}