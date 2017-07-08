#include <iostream>
using namespace std;
#include <limits>
#include <random>
#include "po/po.h"
#include "po_dx11/dx11_form.h"
#include <fstream>
#include <algorithm>
#include "DirectXTex.h"
#include <sstream>
#include "po_dx11/dx11_renderer.h"
#include "po_dx11_defer_renderer\defer_renderer.h"
#include "po_dx11_defer_renderer\element\material.h"
#include "po_dx11_defer_renderer\element\geometry.h"
#include "po_dx11_defer_renderer\element\property.h"
#include "po_dx\controller.h"
#include "new_plugin.h"
using namespace PO;
using namespace PO::Dx11;

struct material_testing
{
	tex2 testing;
	element ele;
	element ele2;
	element compu;
	transfer3D ts;
	showcase s;
	adapter_map mapping(self& sel)
	{
		s.binding({
			{KeyValue::K_D, showcase::State::Y_CW},
			{KeyValue::K_A, showcase::State::Y_ACW},
			{KeyValue::K_S, showcase::State::X_CW},
			{KeyValue::K_W, showcase::State::X_ACW},
			{KeyValue::K_E, showcase::State::Z_CW},
			{KeyValue::K_Q, showcase::State::Z_ACW},
			{KeyValue::K_R, showcase::State::T_FR},
			{KeyValue::K_F, showcase::State::T_BA}
		});
		sel.auto_bind_respond(&material_testing::respond, this);
		return {
			make_member_adapter<defer_renderer>(this, &material_testing::init, &material_testing::tick)
		};
	}
	Respond respond(event& e) { return s.respond(e); }
	void init(defer_renderer& dr)
	{
		CoInitialize(NULL);
		using namespace DirectX;
		TexMetadata TM;
		ScratchImage SI;
		HRESULT re = DirectX::LoadFromWICFile(u"text_tex.jpg"_wc, WIC_FLAGS_NONE, &TM, SI);
		
		if (!SUCCEEDED(re))
			__debugbreak();
		re = DirectX::SaveToDDSFile(*SI.GetImages(), 0, u"WTF.DDS"_wc);
		if (!SUCCEEDED(re))
			__debugbreak();
		cout << TM.width << " *** " << TM.height << endl;
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
	}
	void tick(defer_renderer& dr, duration da)
	{
		s.apply(da, ts);
		if(!ele.find([&, this](Property::transfer_3d_static& t) {
			t.local_to_world = ts;
		})) __debugbreak();
		dr.push_element(ele);
		
	}
};

int main()
{
	PO::Dx11::add_shader_path<PO::Dx::shader_binary>(u"..\\..\\..\\..\\project\\vs2017\\po_dx11_defer_renderer\\lib\\Debug\\x64\\shader_lib");
	//PO::Dx11::add_shader_path<PO::Dx::shader_binary>(u"shader_lib");


	volatile size_t i = PO::Dx::Implement::calculate_start_size < sizeof(uint32_t3), sizeof(PO::Dx::aligned_array<float3, 100>)> ::size;

	volatile size_t i2 = PO::Dx::Implement::aligned_storage_get<1, 0, uint32_t3, PO::Dx::aligned_array<float3, 100>>::size;

	PO::context con;

	if (true)
	{
		auto fo = con.create(form<Dx11_form>{});
		fo.lock([](decltype(fo)::type& ui) {
			ui.create(renderer<defer_renderer>{});
			ui.create(plugin<new_plugin>{});
		});
	}

	con.wait_all_form_close();
	return 0;
}
