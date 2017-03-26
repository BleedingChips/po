#pragma once
#include "test_plugin.h"

struct simul_debug
{
	pixel_creater pc;
	material m;
	pixel_state ps;
	material_state ms;
	Implement::render_view_ptr rvp;
	Implement::resource_view_ptr depthTexture;
	Implement::resource_view_ptr farImageTexture;
	Implement::resource_view_ptr nearImageTexture;
	Implement::resource_view_ptr nearFarTexture;
	Implement::resource_view_ptr lightpassTexture;
	Implement::resource_view_ptr loss2dTexture;
	Implement::resource_view_ptr inscatterVolumeTexture;
	Implement::resource_view_ptr godraysVolumeTexture;
	Implement::resource_view_ptr shadowTexture;
	Implement::sample_state_ptr cube_sample;
	Implement::sample_state_ptr wmc_sampler;
	Implement::sample_state_ptr wcc_sampler;
	Implement::sample_state_ptr wrap_sampler;
	Implement::sample_state_ptr cmc_sampler;
	simul_debug() {}
	void init(ticker& t);
	void tick(ticker& t);
	PO::Respond respond(conveyer& c);
};