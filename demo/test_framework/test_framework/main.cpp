#include <map>
#include "po/po.h"
#include "po_dx11/dx11_form.h"
#include "po_dx11/dx11_renderer.h"
#include "po_dx11/dx11_buildin_element.h"
struct A {};


class test_plugin
{
	PO::Dx11::element ele;
public:
	void tick(PO::Dx11::renderer_default& ef)
	{
		ef << ele;
	}

	PO::adapter_map mapping(PO::self& sel)
	{
		return {
			PO::make_member_adapter<PO::Dx11::renderer_default>(this, &test_plugin::init, &test_plugin::tick)
		};
	}


	void init(PO::Dx11::renderer_default& ef) {
		ele << ef.ins.create_geometry<PO::Dx11::geometry_screen>()
			<< ef.ins.create_material<PO::Dx11::material_testing>()
			<< ef.ins.create_placement<PO::Dx11::placement_direct>();
	}
};







int main()
{
	PO::Dx11::add_shader_path<PO::Dx::shader_binary>(u"..\\..\\..\\..\\project\\vs2017\\po_dx11\\lib\\Debug\\x64\\shader_lib");
	PO::Dx11::add_shader_path<PO::Dx::shader_binary>(u"..\\..\\..\\..\\..\\project\\vs2017\\po_dx11\\lib\\Debug\\x64\\shader_lib");
	PO::Dx11::add_shader_path<PO::Dx::shader_binary>(u"..\\..\\..\\project\\vs2017\\po_dx11\\lib\\Debug\\x64\\shader_lib");
	PO::context con;
	auto p = con.create(PO::form<PO::Dx11::form_default>{});
	p.lock([](decltype(p)::type& t) {
		t.create(PO::renderer<PO::Dx11::renderer_default>{});
		t.create(PO::plugin<test_plugin>{});
	});
	con.wait_all_form_close();
	return 0;
}