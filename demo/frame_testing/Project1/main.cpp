#include "../../../po.h"
#include "../../../gui/dx11/dx11_form.h"
#include "../../../gui/dx11/simple_renderer.h"

struct AA
{
	AA(PO::construction<PO::Dx11::simple_renderer> c) {
		std::cout << "AA" << std::endl;
		c.auto_bind_init(&AA::init, this);
		c.auto_bind_respond(&AA::respond, this);
		//c.auto_bind_tick(&AA::tick, this);
		//c.self_ref.auto_bind_tick(&AA::tick, this);
		//c.plugins_ref.bind_init()
	}
	~AA()
	{
		std::cout << "~AA" << std::endl;
	}

	void tick() {
		std::cout << "tick" << std::endl;
	}

	void init(PO::Dx11::simple_renderer& ) {
		std::cout << "init" << std::endl;
	}

	PO::Respond respond(PO::event& e) {
		if (e.is_key() && e.key.is_up())
			std::cout << "Yes" << std::endl;
		return PO::Respond::Pass;
	}

};


struct PP {
	int* i = nullptr;
	PP(int p){
		i = new int(p);
		std::cout << "init PP" << std::endl; 
	}
	PP(const PP&) { std::cout << "const PP" << std::endl; }
	void text() { std::cout << "666" << *i << std::endl; }
	~PP() { std::cout << "de" << std::endl; delete i; }
};
struct DD {
	PP& p;
	DD(PP& i) : p(i) {}
	std::function<void()> get() {
		return [&ui = p]() mutable {
			ui.text();
		};
	}
};


int main()
{
	PP ptt{ 1 };
	std::function<void()> tem;
	{
		
		DD d{ ptt };
		tem = d.get();
	}
	tem();

	auto p234567 = &PO::Dx11::Dx11_form::virtual_ready;

	std::cout << typeid(&PO::Dx11::Dx11_form::virtual_ready).name() << std::endl;

	PO::context con;
	auto p = con.create_form(PO::Tmp::itself<PO::Dx11::Dx11_form>{});
	p.lock([](decltype(p)::type& i) {
		auto k = i.create_renderer(PO::Tmp::itself<PO::Dx11::simple_renderer>{});
		k.lock([](decltype(k)::type& pc) {
			pc.create(PO::Tmp::itself<AA>{});
		});
	});
	con.wait_all_form_close();
	system("pause");
}