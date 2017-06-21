#include "../../../po.h"
#include <iostream>
using namespace PO;
using namespace std;


struct renderer_a
{
	proxy mapping(std::type_index ti, adapter_interface& i) {
		if (ti == typeid(renderer_a)) {
			return make_proxy<renderer_a>(i, *this);
		}
		return {};
	}
	renderer_a() {}
	renderer_a(const renderer_a&) = delete;
	void ask() { cout << "fuck!!!  " << i << endl; }
	int i = 10086;
	void init(value_table& om) {
		auto po = om.get<int>();
		if (po)
		{
			i = *po;
			cout << "find " << i << endl;
		}
	}
};

struct renderer_b 
{
	renderer_a a;

	int i;

	renderer_b() {}
	renderer_b(const renderer_b&) = delete;

	proxy mapping(std::type_index ti, adapter_interface& i) {
		if (ti == typeid(renderer_a)) {
			return make_proxy<renderer_a>(i, this->a);
		}
		return {};
	}
	void pre_tick(duration da) {
		//cout << "pre_tick" << endl;
	}
	void pos_tick(duration da) {
		//cout << "pos_tick" << endl;
	}
	void init(value_table& om) {
		a.init(om);
	}
	
};


struct add {
	adapter_map mapping(self&) {
		return {
			make_member_adapter<renderer_a>(this, &add::init, &add::tick)
		};
	}
	void init(renderer_a& ra) { ra.ask(); }
	void tick() { /*cout << "tick" << endl;*/ }
};

struct ff {
	bool available() const { return true; }
	value_table mapping() { return {}; }
};


int main()
{
	context co;
	auto ref = co.create(form<ff>{});
	ref.lock([](decltype(ref)::type& i) {
		i.create(renderer<renderer_b>{});
		i.create(plugin<add>{});
	});
	system("pause");
}