#include <iostream>
using namespace std;

#include <limits>
#include <random>
#include "../../../po.h"
#include "../../../gui/dx11/dx11_form.h"
#include "form_define.h"
#include "test_plugin.h"
#include <fstream>
#include <algorithm>
#include "ue4_testing.h"
#include "DirectXTex.h"
#include <sstream>
#include "new_creater.h"
#include "../../po/gui/dx11/simple_renderer.h"

int main()
{


	PO::context con;
	auto fo = con.create_form(Tmp::itself<Dx11_form>{});
	fo.lock([](auto& ui){
		auto fo2 = ui.create_renderer(Tmp::itself<simple_renderer>{});
		fo2.lock([](auto& ui2) {
			ui2.create(Tmp::itself<test_plugin>{});
		});
		//ui.depute_create_plugin(PO::plugin<test_plugin>{});
		//ui.depute_create_plugin(PO::plugin<new_creator>{});
	});
	con.wait_all_form_close();
}
