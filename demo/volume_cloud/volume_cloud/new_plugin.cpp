#include "new_plugin.h"
#include "compute\volume_cloud_compute.h"
#include "material\volume_cloud_material.h"
#include "../DirectXTex/DirectXTex.h"
#include "po_dx11_defer_renderer\build_in_element.h"
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
		make_member_adapter<defer_renderer>(this, &new_plugin::init, &new_plugin::tick)
	};
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

void new_plugin::init(defer_renderer& dr)
{
	ts1.poi = float3(0.0, 0.0, 4.0);
	//ts1.sca = float3(1.0, 0.5, 1.0);
	worley = dr.create_tex2_unordered_access(DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT, 256 * 16, 256 * 16);
	dr.make_compute(compute, [](compute_perlin_worley_noise_tex2_3d&) {});
	compute.make_interface([&](property_output_tex2& pot) {
		pot.set_texture(dr, worley, 2.0, { 256, 256, 16, 16 });
	});
	compute.make_interface([&](property_perline_worley_noise_3d_point& pot) {
		pot.set_seed(dr, { 123, 456, 789 });
	});
	dr << compute;

	dr.make_geometry_and_placement(output_volume_cube, [](geometry_cube_static&, placement_view_static&) {});
	dr.make_material(output_volume_cube, [](material_transparent_2d_for_3d_64_without_perlin&) {});
	output_volume_cube.make_interface([&, this](property_render_2d_for_3d& pt) {
		pt.set_texture(dr, worley);
		pt.set_option(dr, float3{ -1.0, -1.0, -1.0 }, float3{ 1.0, 1.0, 1.0 }, float3{ 0.0, -1.0, 0.0 }, max_denstiy);
	});
	output_volume_cube.make_interface([&, this](property_transfer& pt) {
		pt.set_transfer(dr, ts1, ts1.inverse_float4x4());
	});
	ts2.poi = float3(0.0, 0.0, 5.0f);
	dr.make_geometry_and_placement(back_ground, [](geometry_cube_static&, placement_view_static&) {});
	dr.make_material(back_ground, [](material_defer_render_texcoord& mdt) {});
	back_ground.make_interface([&](property_transfer& pt) {
		pt.set_transfer(dr, ts2, ts2.inverse_float4x4());
	});
	back_ground.make_interface([&](property_tex2& pt) {
		pt.set_tex2(dr, worley);
	});

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
	}
}

static int count__ = 0;

void new_plugin::tick(defer_renderer& dr, duration da)
{
	/*
	count__++;
	if (count__ == 6)
	{
		CoInitialize(nullptr);
		DirectX::ScratchImage SI;
		DirectX::CaptureTexture(dr.get_creator().dev, dr.pipeline::ptr, worley.ptr, SI);
		if (!
			SUCCEEDED(DirectX::SaveToWICFile(*SI.GetImages(), DirectX::WIC_FLAGS_NONE, DirectX::GetWICCodec(DirectX::WIC_CODEC_JPEG), L"NEW_IMAGE.JPEG"))
			) __debugbreak();
	}*/


	if (swith_state == 0)
	{
		if (s.apply(da, ts1))
		{
			output_volume_cube.make_interface([&, this](property_transfer& pt) {
				pt.set_transfer(dr, ts1, ts1.inverse_float4x4());
			});
		}
	}
	else if (swith_state == 1)
	{
		if (s.apply(da, ts2))
		{
			back_ground.make_interface([&](property_transfer& pt) {
				pt.set_transfer(dr, ts2, ts2.inverse_float4x4());
			});
		}
	}

	output_volume_cube.make_interface([&, this](property_render_2d_for_3d& pt) {
		pt.set_option(dr, float3{ -1.0, -1.0, -1.0 }, float3{ 1.0, 1.0, 1.0 }, float3{ 0.0, -1.0, 0.0 }, max_denstiy);
	});
	
	dr << back_ground;
	dr << output_volume_cube;
}