#include <map>
#include "po/po.h"
#include "po_dx11/dx11_form.h"
#include "po_dx11/dx11_renderer.h"
#include "po_dx11/dx11_buildin_element.h"
#include "po_dx\controller.h"
struct A {};


class test_plugin
{
	PO::Dx11::element ele;
	PO::Dx11::element ele2;
	PO::Dx::transfer3D ts;
	PO::Dx::showcase show;
public:
	void tick(PO::Dx11::defer_renderer_default& ef, PO::duration da)
	{
		show.apply(da, ts);
		ele2 << [&](PO::Dx11::property_local_transfer& plt) {
			plt.set_local_to_world(ts, ts.inverse_float4x4());
		};
		ef << ele;
		ef << ele2;
	}

	PO::adapter_map mapping(PO::self& sel)
	{
		show.binding({
			{ PO::KeyValue::K_D, PO::Dx::showcase::State::Y_CW },
			{ PO::KeyValue::K_A, PO::Dx::showcase::State::Y_ACW },
			{ PO::KeyValue::K_S, PO::Dx::showcase::State::X_CW },
			{ PO::KeyValue::K_W, PO::Dx::showcase::State::X_ACW },
			{ PO::KeyValue::K_E, PO::Dx::showcase::State::Z_CW },
			{ PO::KeyValue::K_Q, PO::Dx::showcase::State::Z_ACW },
			{ PO::KeyValue::K_R, PO::Dx::showcase::State::T_FR },
			{ PO::KeyValue::K_F, PO::Dx::showcase::State::T_BA }
		});

		sel.auto_bind_respond(&test_plugin::respond, this);
		return {
			PO::make_member_adapter<PO::Dx11::defer_renderer_default>(this, &test_plugin::init, &test_plugin::tick)
		};
	}

	PO::Respond respond(PO::event& e)
	{
		return show.respond(e);
	}

	void init(PO::Dx11::defer_renderer_default& ef) {
		PO::Dx::float3 poi = PO::Dx::float3(0.0, 0.0, 6.0);
		ts.poi = poi;
		ele << ef.ins.create_geometry<PO::Dx11::geometry_cube>()
			<< ef.ins.create_material<PO::Dx11::material_qpaque_texture_coord>()
			<< ef.ins.create_placement<PO::Dx11::placement_static_viewport_static>();
		ele << [&](PO::Dx11::property_local_transfer& plt)
		{
			plt.set_local_to_world(ts, ts.inverse_float4x4());
		};
		ts.poi = PO::Dx::float3(0.0, 0.0, 4.0);
		ele2 << ef.ins.create_geometry<PO::Dx11::geometry_cube>()
			<< ef.ins.create_material<PO::Dx11::material_transparent_testing>()
			<< ef.ins.create_placement<PO::Dx11::placement_static_viewport_static>()
			<< [&](PO::Dx11::property_local_transfer& plt)
		{
			plt.set_local_to_world(ts, ts.inverse_float4x4());
		};

	}
};


int main()
{
#ifdef DEBUG
	PO::Dx11::add_shader_path<PO::Dx::shader_binary>(u"..\\..\\..\\..\\project\\vs2017\\po_dx11\\lib\\shader\\Debug");
	PO::Dx11::add_shader_path<PO::Dx::shader_binary>(u"..\\..\\..\\..\\..\\project\\vs2017\\po_dx11\\lib\\shader\\Debug");
	PO::Dx11::add_shader_path<PO::Dx::shader_binary>(u"..\\..\\..\\project\\vs2017\\po_dx11\\lib\\shader\\Debug");
#else
	PO::Dx11::add_shader_path<PO::Dx::shader_binary>(u"..\\..\\..\\..\\project\\vs2017\\po_dx11\\lib\\shader\\Release");
	PO::Dx11::add_shader_path<PO::Dx::shader_binary>(u"..\\..\\..\\..\\..\\project\\vs2017\\po_dx11\\lib\\shader\\Release");
	PO::Dx11::add_shader_path<PO::Dx::shader_binary>(u"..\\..\\..\\project\\vs2017\\po_dx11\\lib\\shader\\Release");
#endif // DEBUG

	
	PO::context con;
	auto p = con.create(PO::form<PO::Dx11::form_default>{});
	p.lock([](decltype(p)::type& t) {
		t.create(PO::renderer<PO::Dx11::defer_renderer_default>{});
		t.create(PO::plugin<test_plugin>{});
	});
	con.wait_all_form_close();
	return 0;
}