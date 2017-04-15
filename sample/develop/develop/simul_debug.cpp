#include "simul_debug.h"
std::vector<uint16_t> data =
{
	0 , 1,2, 2,3,1
};

uint16_t data_t[] =
{
	0 , 1, 2, 2,3,1
};

struct IND
{
	const char* operator() ()const { return "IND"; }
};

PO::Respond simul_debug::respond(conveyer& c)
{
	//return vm.capture_event(c.get_event());
	return PO::Respond::Pass;
}

/*
struct yuan_data
{
	float3 poi;
	float3 
};
*/

std::vector<float3> yuan;

void simul_debug::init(ticker& t)
{
	/*
	pc.bind(t.tick());
	m.bind(t.tick());
	ps.bind(t.tick());
	ms.bind(t.tick());

	if (!b.create(t.tick(), D3D11_USAGE::D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE, D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER, nullptr, sizeof(DirectX::XMFLOAT4X4), 0, 0))
		__debugbreak();
	*/
	/*
	ms.set_blend(D3D11_RENDER_TARGET_BLEND_DESC{ true, D3D11_BLEND_ONE, D3D11_BLEND_SRC1_COLOR, D3D11_BLEND_OP_ADD,  D3D11_BLEND_ONE , D3D11_BLEND_ZERO , D3D11_BLEND_OP_ADD , D3D11_COLOR_WRITE_ENABLE_ALL })
		.set_blend_factor(1.0,1.0,1.0,1.0)
		;*/
	/*
	if (!ms.update()) __debugbreak();

	if (!pc.load_vshader( u"vfinal.cso")) __debugbreak();
	if (!pc.create_vertex(0, data, layout_type<syntax<IND, 0, uint16_t>>{})) __debugbreak();
	pc.set_vertex_range(0, data.size());
	if (!m.load_p(u"pfinal.cso")) __debugbreak();

	if ((depthTexture = cast_resource(t.tick(), load_dds(t.tick(), u"depthTexture.dds"))) == nullptr)
		__debugbreak();
	if ((farImageTexture = cast_resource(t.tick(), load_dds(t.tick(), u"farImageTexture.dds"))) == nullptr)
		__debugbreak();
	if ((nearImageTexture = cast_resource(t.tick(), load_dds(t.tick(), u"nearImageTexture.dds"))) == nullptr)
		__debugbreak();
	if ((nearFarTexture = cast_resource(t.tick(), load_dds(t.tick(), u"nearFarTexture.dds"))) == nullptr)
		__debugbreak();
	if ((lightpassTexture = cast_resource(t.tick(), load_dds(t.tick(), u"lightpassTexture.dds"))) == nullptr)
		__debugbreak();
	if ((loss2dTexture = cast_resource(t.tick(), load_dds(t.tick(), u"loss2dTexture.dds"))) == nullptr)
		__debugbreak();
	if ((inscatterVolumeTexture = cast_resource(t.tick(), load_dds(t.tick(), u"inscatterVolumeTexture.dds"))) == nullptr)
		__debugbreak();
	if ((godraysVolumeTexture = cast_resource(t.tick(), load_dds(t.tick(), u"godraysVolumeTexture.dds"))) == nullptr)
		__debugbreak();
	if ((shadowTexture = cast_resource(t.tick(), load_dds(t.tick(), u"shadowTexture.dds"))) == nullptr)
		__debugbreak();

	D3D11_SAMPLER_DESC cube_sample_state
	{
		D3D11_FILTER_MIN_MAG_MIP_LINEAR,
		D3D11_TEXTURE_ADDRESS_MIRROR,
		D3D11_TEXTURE_ADDRESS_MIRROR,
		D3D11_TEXTURE_ADDRESS_MIRROR,
		0,
		1,
		D3D11_COMPARISON_NEVER,
		{ 1.0,1.0,1.0,1.0 },
		0,
		0 
	};
	PO::Win32::Error::fail_throw(t.tick().dev->CreateSamplerState(&cube_sample_state, &cube_sample));


	D3D11_SAMPLER_DESC wmc_sampler_state
	{
		D3D11_FILTER_MIN_MAG_MIP_LINEAR,
		D3D11_TEXTURE_ADDRESS_WRAP,
		D3D11_TEXTURE_ADDRESS_MIRROR,
		D3D11_TEXTURE_ADDRESS_WRAP,
		0,
		1,
		D3D11_COMPARISON_NEVER,
		{ 1.0,1.0,1.0,1.0 },
		0,
		0
	};

	PO::Win32::Error::fail_throw(t.tick().dev->CreateSamplerState(&wmc_sampler_state, &wmc_sampler));

	D3D11_SAMPLER_DESC wcc_sampler_state
	{
		D3D11_FILTER_MIN_MAG_MIP_LINEAR,
		D3D11_TEXTURE_ADDRESS_WRAP,
		D3D11_TEXTURE_ADDRESS_CLAMP,
		D3D11_TEXTURE_ADDRESS_CLAMP,
		0,
		1,
		D3D11_COMPARISON_NEVER,
		{ 1.0,1.0,1.0,1.0 },
		0,
		0
	};

	PO::Win32::Error::fail_throw(t.tick().dev->CreateSamplerState(&wcc_sampler_state, &wcc_sampler));

	D3D11_SAMPLER_DESC wrap_sampler_state
	{
		D3D11_FILTER_MIN_MAG_MIP_LINEAR,
		D3D11_TEXTURE_ADDRESS_WRAP,
		D3D11_TEXTURE_ADDRESS_WRAP,
		D3D11_TEXTURE_ADDRESS_WRAP,
		0,
		1,
		D3D11_COMPARISON_NEVER,
		{ 1.0,1.0,1.0,1.0 },
		0,
		0
	};
	PO::Win32::Error::fail_throw(t.tick().dev->CreateSamplerState(&wrap_sampler_state, &wrap_sampler));

	D3D11_SAMPLER_DESC cmc_sampler_state
	{
		D3D11_FILTER_MIN_MAG_MIP_LINEAR,
		D3D11_TEXTURE_ADDRESS_CLAMP,
		D3D11_TEXTURE_ADDRESS_WRAP,
		D3D11_TEXTURE_ADDRESS_CLAMP,
		0,
		1,
		D3D11_COMPARISON_NEVER,
		{ 1.0,1.0,1.0,1.0 },
		0,
		0
	};
	PO::Win32::Error::fail_throw(t.tick().dev->CreateSamplerState(&cmc_sampler_state, &cmc_sampler));
	

	rvp = cast_render_view(t.tick(), create_render_target(t.tick(), 1024, 768, DXGI_FORMAT_R8G8B8A8_UNORM));
	*/
}

void simul_debug::tick(ticker& op)
{
	/*
	vm.tick(op.time());
	//std::cout << vm.view(3, 0) << "," << vm.view(3, 1) << "," << vm.view(3, 2) << std::endl;
	std::cout << vm.projection << std::endl;
	
	auto i = vm.get_proj();
	
	if (!b.write(op.tick(),
		[&i](void* d, size_t size)
	{
		std::memcpy(d, &i, size);
	}
	))
		__debugbreak();
		

	float color[4] = { 0.5,0.5,0.5,1.0 };
	ID3D11RenderTargetView* tem_array[] = { op.tick().pView, rvp };
	ID3D11Buffer* buf[] = { b.ptr };
	op.tick().dc->VSSetConstantBuffers(0, 1, buf);
	op.tick().dc->OMSetRenderTargets(2, tem_array, op.tick().pDepthView);
	op.tick().dc->ClearRenderTargetView(op.tick().pView, color);
	op.tick().dc->ClearRenderTargetView(rvp, color);
	op.tick().dc->ClearDepthStencilView(op.tick().pDepthView, D3D11_CLEAR_FLAG::D3D11_CLEAR_DEPTH | D3D11_CLEAR_FLAG::D3D11_CLEAR_STENCIL, 1.0, 0);
	m.apply(op.tick());
	std::vector<ID3D11ShaderResourceView*> tem = { 
		depthTexture,
		farImageTexture,
		nearImageTexture,
		nearFarTexture,
		lightpassTexture,
		loss2dTexture,
		inscatterVolumeTexture,
		godraysVolumeTexture,
		shadowTexture
	};
	op.tick().dc->PSSetShaderResources(0, static_cast<UINT>(tem.size()), tem.data());
	std::vector<ID3D11SamplerState*> temss = { cube_sample, wmc_sampler, wcc_sampler, wrap_sampler, cmc_sampler };
	op.tick().dc->PSSetSamplers(0, static_cast<UINT>(temss.size()), temss.data());
	pc.primitive = D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	*/

	/*
	ID3D11RasterizerState* RS;

	D3D11_RASTERIZER_DESC DRD
	{
		D3D11_FILL_SOLID,
		D3D11_CULL_NONE,
		true,
		0,
		0,
		0,
		false
	};

	re = op.tick().dev->CreateRasterizerState(&DRD, &RS);
	op.tick().dc->RSSetState(RS);
	*/
	/*
	ms.apply(op.tick());
	ps.apply(op.tick());
	pc.apply(op.tick());
	pc.draw(op.tick());
	*/
	//tem_array[0] = op.tick().pView;
	//op.tick().dc->OMSetRenderTargets(1, tem_array, op.tick().pDepthView);


	//op.tick().swap->Present(0, 0);
	//DSS->Release();
	//RS->Release();
}