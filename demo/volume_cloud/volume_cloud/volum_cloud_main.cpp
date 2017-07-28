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
//#include "po_dx11_defer_renderer\element\material.h"
//#include "po_dx11_defer_renderer\element\geometry.h"
//#include "po_dx11_defer_renderer\element\property.h"
#include "po_dx\controller.h"
#include "new_plugin.h"
#include "translate_ue4_custion_node.h"

using namespace std;
using namespace PO;
using namespace PO::Dx;
using namespace PO::Dx11;


struct alignas(128) P
{
	float4x4 data;
};

struct alignas(1) P2
{
	char poi;
	char io;
};

int main()
{

	PO::Dx11::add_shader_path<PO::Dx::shader_binary>(u"shader_lib");
	PO::Dx11::add_shader_path<PO::Dx::shader_binary>(u"..\\..\\..\\..\\project\\vs2017\\po_dx11_defer_renderer\\lib\\Debug\\x64\\shader_lib");
	translate_ue4_node("..\\..\\volume_cloud\\material\\shader\\volume_cloud_material_transparent_2d_for_3d_64_without_perlin_ps.hlsl", "iop");

	system("pause");
	return 0;
	cout << PO::Tool::max_align<float4x4, float4x4, float, float>::value << endl;

	cout << PO::Tool::max_align<float4x4>::value << endl;

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
