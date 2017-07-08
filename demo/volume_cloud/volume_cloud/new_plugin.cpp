#include "new_plugin.h"
#include "po_dx11_defer_renderer\element\geometry.h"
#include "po_dx11_defer_renderer\element\property.h"
#include "po_dx11_defer_renderer\element\material.h"
#include "po_dx11_defer_renderer\element\placement.h"

#include "compute\generator_3d_volume_texture.h"
#include "material\texture_3d_viewer.h"

#include "../DirectXTex/DirectXTex.h"

using namespace std;

adapter_map new_plugin::mapping(self& sel)
{
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
			case PO::KeyValue::K_O:
				layer -= 1.0 / 256;
				if (layer < 0.0)
					layer = 0.0;
				ele2.find([this](texture_3d_viewer::input_property& ip) {
					ip.set_layer(layer);
				});
				std::cout << layer << std::endl;
				return Respond::Truncation;
			case PO::KeyValue::K_P:
				layer += 1.0 / 256;
				if (layer > 1.0)
					layer = 1.0;
				ele2.find([this](texture_3d_viewer::input_property& ip) {
					ip.set_layer(layer);
				});
				std::cout << layer << std::endl;
				return Respond::Truncation;
			}
		}
	}
	return Respond::Pass;
}

void new_plugin::init(defer_renderer& dr)
{
	tex3 tex = dr.create_tex3_unordered_access(DXGI_FORMAT::DXGI_FORMAT_R16_FLOAT, 256, 256, 256);
	element compu;
	compu = dr.find(compute<generator_3d_volume_texture>{});

	{
		auto& ref = compu.create<typename generator_3d_volume_texture::input_property>();
		ref.set_texture(tex);
		ref.set_random(123, 456, 789);
	}

	ele2 = dr.find(geometry<Geometry::square_static_2d>{});
	ele2 = dr.find(material<texture_3d_viewer>{});

	{
		auto& ref = ele2.create<Property::transfer_2d_static>();
		//ref.set_shift({ 0.4f, 0.4f });
		ref.set_scale({ 0.5f, 0.5f });
		ref.set_adapt_screen(true);
	}

	{
		auto& ref = ele2.create<texture_3d_viewer::input_property>();
		ref.set_texture(tex);
		ref.set_layer(0.5);
	}

	
	dr.push_element(compu);



	/*
	void* data = SI.GetPixels();
	UINT count = static_cast<UINT>(TM.width) * 4;
	tex2  tex = dr.create_tex2(TM.format, static_cast<UINT>(TM.width), static_cast<UINT>(TM.height), static_cast<UINT>(TM.mipLevels), static_cast<UINT>(TM.arraySize), D3D11_USAGE::D3D11_USAGE_DEFAULT, &data, &count);
	ts.poi = float3(0.0, 0.0, 10.0);
	ts.sca.x = 2.0;
	ele = dr.find(material<Material::test_texcoord>{});
	ele = dr.find(geometry<Geometry::cube_static_3d>{});
	Property::transfer_3d_static& o = ele.create<Property::transfer_3d_static>();
	Material::test_texcoord::texture& tex_ref = ele.create<Material::test_texcoord::texture>();
	tex_ref.srv = dr.cast_shader_resource_view(tex);
	tex_ref.ss = dr.create_sample_state();
	o.local_to_world = ts;
	ele.push(dr);
	*/
}

void new_plugin::tick(defer_renderer& dr, duration da)
{
	dr.push_element(ele2);
	/*
	s.apply(da, ts);
	if (!ele.find([&, this](Property::transfer_3d_static& t) {
		t.local_to_world = ts;
	})) __debugbreak();
	dr.push_element(ele);
	*/
}