#include <iostream>
using namespace std;
#include <limits>
#include <random>
#include "po/po.h"
#include "po_dx11/dx11_form.h"
#include "test_plugin/test_plugin.h"
#include <fstream>
#include <algorithm>
#include "ue4_testing/ue4_testing.h"
#include "DirectXTex.h"
#include <sstream>
#include "new_creator/new_creater.h"
#include "po_dx11/dx11_renderer.h"
#include "po_dx11_defer_renderer\defer_renderer.h"
#include "po_dx11_defer_renderer\element\material.h"
#include "po_dx11_defer_renderer\element\geometry.h"
#include "po_dx11_defer_renderer\element\property.h"
#include "po_dx\controller.h"
using namespace PO;
using namespace PO::Dx11;

struct material_testing
{
	tex2 testing;
	element ele;
	transfer3D ts;
	showcase s;
	adapter_map mapping(self& sel)
	{
		s.binding(KeyValue::K_D, showcase::State::Y_CW);
		s.binding(KeyValue::K_A, showcase::State::Y_ACW);
		s.binding(KeyValue::K_W, showcase::State::X_CW);
		s.binding(KeyValue::K_S, showcase::State::X_ACW);
		s.binding(KeyValue::K_Q, showcase::State::Z_CW);
		s.binding(KeyValue::K_E, showcase::State::Z_ACW);
		s.binding(KeyValue::K_R, showcase::State::T_FR);
		s.binding(KeyValue::K_F, showcase::State::T_BA);
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
		ele.init(dr);
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

	PO::context con;
	
	if(false)
	{
		auto fo = con.create(form<Dx11_form>{});
		fo.lock([](decltype(fo)::type& ui) {
			ui.create(renderer<defer_renderer>{});
			ui.create(plugin<UE4_testing>{});
		}); 
	}

	if(false)
	{
		auto fo = con.create(form<Dx11_form>{});
		fo.lock([](decltype(fo)::type& ui) {
			ui.create(renderer<defer_renderer>{});
			ui.create(plugin<new_creator>{});
		});
	}

	if(false)
	{
		auto fo = con.create(form<Dx11_form>{});
		fo.lock([](decltype(fo)::type& ui) {
			ui.create(renderer<defer_renderer>{});
			ui.create(plugin<test_plugin>{});
		});
	}

	if (true)
	{
		auto fo = con.create(form<Dx11_form>{});
		fo.lock([](decltype(fo)::type& ui) {
			ui.create(renderer<defer_renderer>{});
			ui.create(plugin<material_testing>{});
		});
	}

	con.wait_all_form_close();
	return 0;
}
