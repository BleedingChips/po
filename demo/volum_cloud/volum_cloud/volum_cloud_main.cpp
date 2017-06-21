#include <iostream>
using namespace std;
#include <limits>
#include <random>
#include "../../../po.h"
#include "../../../gui/dx11/dx11_form.h"
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
	
	//{\
		auto fo = con.create(form<Dx11_form>{}); \
		fo.lock([](decltype(fo)::type& ui) { \
			ui.create(renderer<simple_renderer>{}); \
			ui.create(plugin<UE4_testing>{}); \
		}); \
	}


	
	{ \
		auto fo = con.create(form<Dx11_form>{});\
		fo.lock([](decltype(fo)::type& ui) {\
			ui.create(renderer<simple_renderer>{});\
			ui.create(plugin<new_creator>{});\
		});\
	}

	
	//{\
		auto fo = con.create(form<Dx11_form>{});\
		fo.lock([](decltype(fo)::type& ui) {\
			ui.create(renderer<simple_renderer>{});\
			ui.create(plugin<test_plugin>{});\
		});\
	}\
	
	con.wait_all_form_close();
}
