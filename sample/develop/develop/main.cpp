#include <iostream>
using namespace std;

#include <DirectXMath.h>
#include <limits>
#include <random>
#include "../../../po.h"
#include "../../../gui/dx11/dx11_form.h"
#include "form_define.h"
#include "test_plugin.h"
#include "simul_debug.h"
#include <fstream>
#include <algorithm>

std::vector<int> fa = { 0,1,2,34,5,50,0,0,2,3 };

struct io
{
	int o;
	void operator()() {}
};

/*T single;
template<typename T> T& get()
{
	static T single;
	return single;
}
*/

using ver = std::array<double, 4>;
using mat = std::array<ver, 4>;


double operator*(const ver& v1, const ver& v2)
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2] + v1[3] * v2[3];
}

ver operator*(const mat& v1, const ver& m)
{
	return{v1[0] * m, v1[1] * m, v1[2] * m, v1[3] * m};
}

ver normalizer(const ver& v)
{
	double da = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]) /*+ v[3] * v[3]*/;
	return{v[0] / da, v[1] / da, v[2] / da, v[3]};
}

mat tran(const mat& m)
{
	return{
		ver{ m[0][0], m[1][0], m[2][0], m[3][0] },
		ver{ m[0][1], m[1][1], m[2][1], m[3][1] },
		ver{ m[0][2], m[1][2], m[2][2], m[3][2] },
		ver{ m[0][3], m[1][3], m[2][3], m[3][3] },
	};
}

float2 ui = float2(2.0f, 0.2f);
float2 op = float2(2.78f, 7.9f);


int call6(int o) { cout << 6 << "," << o << endl; return 6; }
int call7(int o) { cout << 7 << "," << o << endl; return 7; }


class Data
{
	int a;
	int b;
public:
	Data(int c, int d) {}
	//Data(int ) {}
};

struct Data2
{
	Data op = { 1,2 };
};



int main()
{

	PO::Tool::mail<int(int)> mal;
	auto ty = mal.bind(call6);
	mal([](int) { return true; }, 122);
	cout << "===" << endl;
	auto ty2 = mal.bind(call7);
	mal([](int) { return true; }, 122);
	cout << "===" << endl;
	cout << alignof(uint16_t) << endl;
	mat ma = 
	{
		ver{-0.0254603, -0.0468672, 0, 0.996687},
		ver{-0.999675, 0.0011937, 0, -0.0253842},
		ver{9.3534e-08, 0.605025, 0, 0.0772568},
		ver{ 0, 0, 0.999997, 3.33333e-06}
	};
	ver op = {-1.0, -1.0, 1.0, 1.0};
	//auto  re = op * ma;
	ver tem = normalizer(ma * op);
	for(size_t i = 0; i<4;++i)
		cout << tem[i]<<",";
	cout << endl;
	//DirectX::XMLoadFloat4(&op) * DirectX::XMLoadFloat4x4(&ma);
	PO::context con;
	auto fo = con.create_window<DX11_Test_Form>();
	fo.lock_if(
		[](auto& ui)
	{
		ui.create_plugin(PO::plugin_type<test_plugin>{});
	}
	);
	con.wait_all_form_close();
	
	system("pause");
}

















/*
#include "../po/po.h"
#include "../po_expand/dx11/dx11_interface.h"

struct Text
{
	using form = PO::mod_pair<PO::DX11::dx11_form, PO::DX11::dx11_interface>;
	using renderer = PO::mod_pair<PO::DX11::dx11_renderer>;
};

struct plugin_text
{
	plugin_text() { cout << "plugin_text::construct" << endl; }
	void init(PO::self_control& ps)
	{
		cout << "plugin_text::init" << endl;
	}
	void tick(PO::ticker_self ps)
	{
		//cout << "plugin_text::tick" << endl;
	}
	bool event_respond(PO::event& ev)
	{
		if (ev.is_click() && ev.click.is_down() && ev.click.is_left())
			cout << "is_click down" << endl;
		else if (ev.is_key() && ev.key.is_down() && ev.key.get_asc() != 0)
			cout << ev.key.get_asc();
		else if (ev.is_key() && ev.key.is_down() && ev.key.get_value() == PO::KeyValue::K_ENTER)
			cout << endl;
		return false;
	}
	~plugin_text() 
	{
		cout << "plugin_text::desconstruct" << endl;
	}
};

struct A
{
	A(int) { std::cout << "A" << endl; }
};

struct B {};
struct C {};
struct D {};

using data34 = PO::Tool::index_swap<0, 2, PO::Tool::type_container>;
using Text22 = PO::Tool::call<data34>::template in_t<A, B, C>;
void ui(double) {}

int main()
{
	std::cout << typeid(Text22).name() << endl;
	//PO::Tool::variant<A> da(1);
	//PO::context cont;
	//auto form = cont.create_window<Text>();
	//form.create_plugin<plugin_text>();
	//cont.wait_all_form_close();
}*/