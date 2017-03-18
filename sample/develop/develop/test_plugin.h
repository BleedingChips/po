#pragma once
#include "form_define.h"
#include "../../../gui/dx11/dx11_define.h"
#include <DirectXTex.h>
using ticker = PO::ticker<PO::Dx11::Dx11_ticker>;
using namespace PO::Dx11;
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