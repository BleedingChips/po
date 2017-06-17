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

struct A { float e[100]; };
struct B : virtual public A { };

struct C : public B, virtual public A {};

int main()
{
	
	int a = 3;
	int &b = a;
	b = a++;
	a = b++;
	std::cout << a << endl;

	system("pause");
	return 0;

	PO::context con;
	auto fo = con.create_frame(PO::frame<DX11_Test_Form>{});
	fo.lock([](auto& ui){
		ui.depute_create_plugin(PO::plugin<UE4_testing>{});
		//ui.depute_create_plugin(PO::plugin<test_plugin>{});
		//ui.depute_create_plugin(PO::plugin<new_creator>{});
	});
	con.wait_all_form_close();
	
	system("pause");
	
}
