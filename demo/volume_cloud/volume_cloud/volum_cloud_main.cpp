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

int main()
{
	PO::Dx11::add_shader_path<PO::Dx::shader_binary>(u"..\\..\\..\\..\\project\\vs2017\\po_dx11_defer_renderer\\lib\\Debug\\x64\\shader_lib");
	PO::Dx11::add_shader_path<PO::Dx::shader_binary>(u"shader_lib");
	//PO::Dx11::add_shader_path<PO::Dx::shader_binary>(u"shader_lib");


	volatile size_t i = PO::Dx::Implement::calculate_start_size < sizeof(uint32_t3), sizeof(PO::Dx::aligned_array<float3, 100>)> ::size;

	volatile size_t i2 = PO::Dx::Implement::aligned_storage_get<1, 0, uint32_t3, PO::Dx::aligned_array<float3, 100>>::size;

	PO::context con;

	if (true)
	{
		auto fo = con.create(form<Dx11_form>{});
		fo.lock([](decltype(fo)::type& ui) {
			ui.create(renderer<defer_renderer>{});
			//ui.create(plugin<new_plugin>{});
		});
	}

	con.wait_all_form_close();
	return 0;
}
