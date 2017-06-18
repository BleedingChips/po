#include "../../../po.h"
#include "../../../gui/dx11/dx11_form.h"


struct AA
{
	AA(PO::construction<PO::Dx11::Dx11_ticker> c) {
		std::cout << "AA" << std::endl;
		c.self_ref.auto_bind_tick(&AA::tick, this);
	}
	~AA()
	{
		std::cout << "~AA" << std::endl;
	}

	void tick() {
		std::cout << "tick" << std::endl;
	}

};


int main()
{
	PO::context con;
	auto p = con.create_form(PO::Tmp::itself<PO::Dx11::Dx11_form>{});
	p.lock([](decltype(p)::type& i) {
		auto k = i.create_renderer(PO::Tmp::itself<PO::Dx11::Dx11_ticker>{});
		k.lock([](decltype(k)::type& pc) {
			pc.create(PO::Tmp::itself<AA>{});
		});
	});
	con.wait_all_form_close();
	system("pause");
}