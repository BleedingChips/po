#include <iostream>
using namespace std;

#include <DirectXMath.h>
#include <limits>
#include <random>
#include "../../../po.h"
#include "../../../gui/dx11/dx11_form.h"
#include "form_define.h"
#include "test_plugin.h"
#include <fstream>

using namespace PO;
using namespace PO::Implement;

int main()
{
	//assert((__FILE__, false));
	//cout << __FILE__ << endl;
	//system("pause");
	/*
	PO::context con;
	auto fo = con.create_window<DX11_Test_Form>();
	fo.lock_if(
		[](auto& ui)
	{
		ui.create_plugin(PO::plugin_type<test_plugin>{});
	}
	);
	con.wait_all_form_close();
	*/
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