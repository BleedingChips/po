#pragma once
#include "form_define.h"
#include "../../../gui/dx11/dx11_define.h"
using ticker = PO::ticker<PO::Dx11::Dx11_ticker>;
using namespace PO::Dx11;
struct test_plugin
{
	vertex_const ver;
	test_plugin()
	{

	}

	template<typename ...AT> void plug_tick(AT&& ...at)
	{
		PO::Tool::auto_adapter_unorder(&test_plugin::tick, this, std::forward<AT>(at)...);
	}

	template<typename ...AT> void plug_init(AT&& ...at)
	{
		PO::Tool::auto_adapter_unorder(&test_plugin::init, this, at...);
	}

	void tick(ticker& op);

	void init(PO::form_self& fs, ticker& op);
};