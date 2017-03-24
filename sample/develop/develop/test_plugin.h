#pragma once
#include "form_define.h"
#include "../../../gui/dx11/dx11_define.h"
#include <DirectXTex.h>
using ticker = PO::ticker<PO::Dx11::Dx11_ticker>;
using namespace PO::Dx11;

PO::Tool::variant<Implement::texture1D_ptr, Implement::texture2D_ptr, Implement::texture3D_ptr>
load_dds(Implement::resource_ptr& rs, std::u16string path, PO::Dx11::Purpose::purpose bp = PO::Dx11::Purpose::constant);
Implement::resource_view_ptr cast_resource(Implement::resource_ptr& rp,
	const PO::Tool::variant<Implement::texture1D_ptr, Implement::texture2D_ptr, Implement::texture3D_ptr>& v
);
struct test_plugin
{
	pixel_creater pc;
	material ma;
	Implement::texture2D_ptr tp;
	Implement::resource_view_ptr rvp;
	Implement::sample_state_ptr ss;

	test_plugin()
	{

	}

	void tick(ticker& t);

	void init(ticker& t);
};