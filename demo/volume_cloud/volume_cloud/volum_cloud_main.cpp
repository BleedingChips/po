#include <iostream>
#include <limits>
#include <random>
#include "po/po.h"
#include "po_dx11/dx11_form.h"
#include <fstream>
#include <algorithm>
#include "DirectXTex.h"
#include <sstream>
#include "po_dx11/dx11_renderer.h"
//#include "po_dx11_defer_renderer\element\material.h"
//#include "po_dx11_defer_renderer\element\geometry.h"
//#include "po_dx11_defer_renderer\element\property.h"
#include "po_dx\controller.h"
#include "new_plugin.h"
#include "translate_ue4_custion_node.h"

using namespace PO;


std::tuple<int, double> get() { return { 1, 2.0f }; }

int main()
{

	auto[a, b] = get();
	std::cout << a << b << std::endl;

#ifdef DEBUG
	PO::Dx11::add_shader_path<PO::Dx::shader_binary>(u"..\\..\\..\\..\\project\\vs2017\\po_dx11\\lib\\shader\\Debug");
	PO::Dx11::add_shader_path<PO::Dx::shader_binary>(u"..\\..\\..\\..\\..\\project\\vs2017\\po_dx11\\lib\\shader\\Debug");
	PO::Dx11::add_shader_path<PO::Dx::shader_binary>(u"..\\..\\..\\project\\vs2017\\po_dx11\\lib\\shader\\Debug");
#else
	PO::Dx11::add_shader_path<PO::Dx::shader_binary>(u"..\\..\\..\\..\\project\\vs2017\\po_dx11\\lib\\shader\\Release");
	PO::Dx11::add_shader_path<PO::Dx::shader_binary>(u"..\\..\\..\\..\\..\\project\\vs2017\\po_dx11\\lib\\shader\\Release");
	PO::Dx11::add_shader_path<PO::Dx::shader_binary>(u"..\\..\\..\\project\\vs2017\\po_dx11\\lib\\shader\\Release");
#endif // DEBUG
	PO::context con;

	if (true)
	{
		auto fo = con.create(form<form_default>{});
		fo.lock([](decltype(fo)::type& ui) {
			ui.create(renderer<defer_renderer_default>{});
			ui.create(plugin<new_plugin>{});
		});
	}
	
	con.wait_all_form_close();
	//translate_ue4_node("..\\..\\volume_cloud\\material\\shader\\volume_cloud_material_transparent_2d_for_3d_64_without_perlin_ps.hlsl", "implement");
	return 0;
}
