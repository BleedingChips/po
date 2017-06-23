#include <iostream>
using namespace std;
#include <limits>
#include <random>
#include "po/po.h"
#include "dx11/dx11_form.h"
#include "test_plugin/test_plugin.h"
#include <fstream>
#include <algorithm>
#include "ue4_testing/ue4_testing.h"
#include "DirectXTex.h"
#include <sstream>
#include "new_creator/new_creater.h"
#include "dx11/dx11_renderer.h"

int main()
{
	PO::context con;
	
	if(false)
	{
		auto fo = con.create(form<Dx11_form>{});
		fo.lock([](decltype(fo)::type& ui) {
			ui.create(renderer<simple_renderer>{});
			ui.create(plugin<UE4_testing>{});
		}); 
	}

	if(true)
	{
		auto fo = con.create(form<Dx11_form>{});
		fo.lock([](decltype(fo)::type& ui) {
			ui.create(renderer<simple_renderer>{});
			ui.create(plugin<new_creator>{});
		});
	}

	if(false)
	{
		auto fo = con.create(form<Dx11_form>{});
		fo.lock([](decltype(fo)::type& ui) {
			ui.create(renderer<simple_renderer>{});
			ui.create(plugin<test_plugin>{});
		});
	}

	con.wait_all_form_close();
}
