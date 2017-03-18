#pragma once
#include "test_plugin.h"

struct simul_debug
{
	shader_loader sl;
	//compute com;
	simul_debug() {}
	void init(ticker& t);
	void tick(ticker& t);
};