#include <iostream>
using namespace std;

#include <DirectXMath.h>
#include <limits>
#include <random>
#include "../../../po.h"
#include "../../../gui/dx11/dx11_form.h"


using namespace PO::Tool;
using namespace PO::Tmp;
using namespace PO::TmpCall;

struct Test_Frame
{
	using form = PO::Dx11::Dx11_form;
	using ticker = PO::Dx11::Dx11_ticker;
};

struct Test_plugin
{
	Test_plugin()
	{

	}

	template<typename ...AT> void plug_init(AT&& ...at)
	{
		PO::Tool::auto_adapter<PO::Tool::unorder_adapt>(&Test_plugin::init, this, at...);
	}

	void tick(PO::ticker<PO::Dx11::Dx11_ticker>& op)
	{
	}
	void init(PO::form_self& fs,PO::ticker<PO::Dx11::Dx11_ticker>& op)
	{
	}
};

struct da
{
	bool op;
	void uio() const volatile
	{

	}
	void operator()() {}
};

template<typename T> struct dad { void operator()() { cout << "null" << endl; } };
template<typename T, typename U> struct dad<T U::*> { void operator()() { cout << typeid(T).name() << endl; } };
template<typename T, typename ...U> struct dad<T(U...) const > { void operator()() { cout << typeid(T).name() << endl; } };

void yuio(bool) noexcept {}

template<typename Text> void uiii(Text&& t) { cout << typeid(PO::Tmp::extract_func<Text>::type).name() << endl; }


struct IOP
{
	IOP() {}
	//IOP(const IOP& i) { cout << "const iop&" << endl; }
	template<typename T> IOP(T&& t) 
	{ 
		static_assert(!std::is_same < std::remove_reference_t<std::remove_const_t<T>>, IOP >::value , "");
	};
};

template<typename T> void Data222(T t)
{
	
}

struct UIO 
{ 
	UIO() {}; 
	//explicit UIO(const UIO&) {} 
	template<typename T> UIO(T&& t)
	{
		static_assert(std::is_same <std::remove_reference_t<std::remove_const_t<T>>, IOP >::value, "");
	};
};
struct UIO2 :UIO {};



int main()
{


	//UIO da(UIO2{});



	//dad<decltype(&da::uio)>{}();
	//IOP a{};
	//Data222(a);
	//cout << "========" << endl;
	//auto po = []() {};
	//uiii([]() {});
	//cout << typeid(PO::Tmp::extract_func<decltype(po)>::type).name() << endl;
	//cout << typeid(PO::Tmp::extract_func<decltype(yuio)>::type).name() << endl;
	//cout << std::is_same<PO::Tmp::extract_func<decltype(po)>::type, PO::Tmp::extract_func<decltype(yuio)>::type>::value << endl;
	//cout << typeid(std::remove_reference_t<PO::Tmp::extract_func<decltype(&da::uio)>::type>).name() << endl;

	
	PO::context con;
	auto fo = con.create_window<Test_Frame>();
	fo.lock_if(
		[](auto& ui)
	{
		ui.create_plugin<Test_plugin>();
	}
	);
	con.wait_all_form_close();
	
	//system("pause");
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