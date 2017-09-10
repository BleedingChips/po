#include "new_plugin.h"
#include "compute\volume_cloud_compute.h"
#include "material\volume_cloud_material.h"
#include "../DirectXTex/DirectXTex.h"
#include "geometry\ue4_geometry.h"
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

Respond new_plugin::respond(event& e)
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
	worley.create_unordered_access(dr, DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT, { 256 * 16, 256 * 16 });
	compute << dr.ins.create_compute<compute_perlin_worley_noise_tex2_3d>()
		<< [&](property_output_tex2& pot) {
		pot.set_texture(dr, worley, 2.0, { 256, 256, 16, 16 });
	} << [&](property_perline_worley_noise_3d_point& pot) {
		pot.set_seed(dr, { 123, 456, 789 });
	};


	dr << compute;

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
		prf.set_texture(dr, worley, des);
		prf.set_option(float3{ -50.0, -50.0, -50.0 }, float3{ 50.0, 50.0, 50.0 }, float3{ 0.0, -1.0, 0.0 }, max_denstiy);
	}
		<< [&](property_local_transfer& tlt) {
		tlt.set_local_to_world(dr, ts1, ts1.inverse_float4x4());
	};

	ts2.poi = float3(0.0, 0.0, 6.0f);


	std::array<float, 8 * 8 * 8> data;
	for (size_t o = 0; o < 8 * 8; ++o)
		data[o] = o / float(8 * 8);
	uint32_t d = 8;
	void* dat = data.data();

	perlin_out.create_unordered_access(dr, DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_TYPELESS, { 256 * 16, 256 * 16 });

	perlin_element << dr.ins.create_compute<compute_perlin_noise_for_2d_rbga_uint8>();
	perlin_element << [&](property_output_texture_2d_simulate_3d& po) {
		po.set_output_texture_rgba_u8(
			dr, perlin_out.cast_unordered_access_view_as_format(dr, DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UINT),
			perlin_out.size(),
			uint32_t4{ 256,256, 16,16 },
			float4(0.0, 1.0, 0.0, 0.0),
			float4(1.0, 0.0, 1.0, 1.0)
		);
	} << [&](property_random_point_f& purp) {
		purp.create_uniform_point(dr, compute_perlin_noise_for_2d_rbga_uint8::max_count(), 1234563, 0.0, 1.0);
	};

	//dr << perlin_element;

	back_ground << dr.ins.create_geometry<geometry_cube>()
		<< dr.ins.create_placement<placement_static_viewport_static>()
		<< dr.ins.create_material<material_opaque_tex2_viewer>()
		<< [&](property_local_transfer& pt) {
		pt.set_local_to_world(dr, ts2, ts2.inverse_float4x4());
	} << [&](property_tex2& pt) {
		pt.set_texture(dr, perlin_out.cast_shader_resource_view_as_format(dr, DXGI_FORMAT_R8G8B8A8_UNORM));
	}
	;


	/*
	auto ss2 = dr.lack_acceptance(compute);
	if (!ss2.empty())
	{
		std::vector < std::string > p;
		for (auto& i : ss2)
			p.push_back(i.name());
		__debugbreak();
	}


	auto ss = dr.lack_acceptance(output_volume_cube);
	if (!ss.empty())
	{
		std::vector < std::string > p;
		for (auto& i : ss)
			p.push_back(i.name());
		__debugbreak();
	}*/
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
			DirectX::CaptureTexture(dr.dev, dr.context.imp->ptr, perlin_out.ptr, SI);
			if(false)
				if (!
					SUCCEEDED(DirectX::SaveToDDSFile(SI.GetImages(), SI.GetImageCount(), SI.GetMetadata(), 0, L"NEW_IMAGE.DDS"))
					) __debugbreak();
			if(false)
				if (!
					SUCCEEDED(DirectX::SaveToWICFile(*SI.GetImages(), DirectX::WIC_FLAGS_NONE, DirectX::GetWICCodec(DirectX::WIC_CODEC_JPEG), L"NEW_IMAGE.JPEG"))
					) __debugbreak();
		}
		if(false)
		{
			DirectX::ScratchImage SI;
			DirectX::CaptureTexture(dr.dev, dr.context.imp->ptr, dr.linear_z_buffer.ptr, SI);
			;

			if (!
				SUCCEEDED(DirectX::SaveToDDSFile(SI.GetImages(), SI.GetImageCount(), SI.GetMetadata(), 0, L"DEPTH.DDS"))
				) __debugbreak();
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
	
	//dr << back_ground;
	//dr << output_volume_cube;
}