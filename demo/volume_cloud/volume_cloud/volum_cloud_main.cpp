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


int main()
{

	PO::Dx11::add_shader_path<PO::Dx::shader_binary>(u"shader_lib");
	PO::Dx11::add_shader_path<PO::Dx::shader_binary>(u"..\\..\\..\\..\\project\\vs2017\\po_dx11_defer_renderer\\lib\\Debug\\x64\\shader_lib");

	PO::context con;

	if (true)
	{
		auto fo = con.create(form<Dx11_form>{});
		fo.lock([](decltype(fo)::type& ui) {
			ui.create(renderer<defer_renderer>{});
			ui.create(plugin<new_plugin>{});
		});
	}
	//translate_ue4_node("..\\..\\volume_cloud\\material\\shader\\volume_cloud_material_transparent_2d_for_3d_64_without_perlin_ps.hlsl", "implement");
	con.wait_all_form_close();
	return 0;
}
