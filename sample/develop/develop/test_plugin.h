#pragma once
#include "form_define.h"
#include "../../../gui/dx11/dx11_define.h"
#include <DirectXTex.h>
using ticker = PO::ticker<PO::Dx11::Dx11_ticker>;
using namespace PO::Dx11;
struct test_plugin
{
	//vertex_const ver;
	shader_loader sl;
	//shader_v vs;
	//shader_p ps;
	//vertex_layout vl;
	//pipe_line pl;
	DirectX::ScratchImage si;
	Implement::texture2D_ptr t;
	Implement::sample_state_ptr ss;
	Implement::resource_view_ptr sv;
	pipe_line pl;
	vertex_pool vp;
	compute cp;
	ID3D11Buffer* buf;
	ID3D11Buffer* buf2;
	test_plugin()
	{

	}

	void tick(ticker& t);

	void init(ticker& t);
};