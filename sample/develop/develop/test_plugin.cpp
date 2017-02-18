#include "test_plugin.h"
using namespace std;
void test_plugin::tick(ticker& op)
{
	float color[4] = { 0.5,0.5,0.5,1.0 };
	op.tick().dc->ClearRenderTargetView(op.tick().pView, color);
	op.tick().dc->ClearDepthStencilView(op.tick().pDepthView, D3D11_CLEAR_FLAG::D3D11_CLEAR_DEPTH | D3D11_CLEAR_FLAG::D3D11_CLEAR_STENCIL, 1.0, 0);
	Implement::raterizer_state st;
	D3D11_RASTERIZER_DESC tem
	{
		D3D11_FILL_MODE::D3D11_FILL_SOLID,
		D3D11_CULL_MODE::D3D11_CULL_NONE,
		true,
		0,
		1.0,
		1.0,
		false
	};
	op.tick().dev->CreateRasterizerState(&tem, &st);
	op.tick().dc->RSSetState(st);
	ID3D11ShaderResourceView* t[] = { sv };
	op.tick().dc->PSSetShaderResources(0, 1, t);
	//pl.draw(op.tick().dc, op.tick().pView, op.tick().pDepthView, vl, ver);
	//pl.draw(op.tick().dc, op.tick().pView, op.tick().pDepthView, vl, ver);
	op.tick().swap->Present(0, 0);

}

struct vert
{
	float2 ver;
	float2 tex;
	//using type = layout_type<position<0, float2>, texcoord<0, float2>>;
};

void test_plugin::init(ticker& op)
{
	vert data[] =
	{
		{ {-0.5, 0.5}, {0.0, 0.0} },
		{ {-0.5, -0.5}, {0.0, 1.0} },
		{ {0.5, -0.5}, {1.0, 1.0} },
		{ {0.5, -0.5}, {1.0,1.0} },
		{ {0.5, 0.5},{1.0,0.0} },
		{ {-0.5, 0.5},{0.0,0.0} }
	};
	//ver.create(op.tick(), data, 6);
	try
	{
		/*
		sl.load(op.form(), u"base_vshader.cso");
		auto fin = sl.wait_get();
		if (fin)
			PO::Win32::Error::fail_throw(vs.load_binary(op.tick(), std::move(*fin)));
		sl.load(op.form(), u"base_pshader.cso");
		fin = sl.wait_get();
		if (fin)
			PO::Win32::Error::fail_throw(ps.load_binary(op.tick(), std::move(*fin)));
		PO::Win32::Error::fail_throw(vl.create(vert::type{}, op.tick(), vs));
		DirectX::TexMetadata TM;
		PO::Win32::Error::fail_throw(DirectX::LoadFromTGAFile(u"ty.tga"_wc, &TM, si));

		D3D11_TEXTURE2D_DESC desc;
		desc.Width = TM.width;
		desc.Height = TM.height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = TM.format;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		D3D11_SUBRESOURCE_DATA SD
		{
			static_cast<void*>(si.GetImages()->pixels),
			si.GetImages()->rowPitch,
			0
		};
		PO::Win32::Error::fail_throw(op.tick().dev->CreateTexture2D(&desc, &SD, &t));
		float baseColor[4] = { 0.0,0.0,0.0,0.0 };
		D3D11_SAMPLER_DESC tem
		{
			D3D11_FILTER_MIN_MAG_MIP_POINT,
			D3D11_TEXTURE_ADDRESS_WRAP,
			D3D11_TEXTURE_ADDRESS_WRAP,
			D3D11_TEXTURE_ADDRESS_WRAP,
			0,
			6,
			D3D11_COMPARISON_ALWAYS,
			{0.0,0.0,0.0,0.0},
			0,
		0 };
		PO::Win32::Error::fail_throw(op.tick().dev->CreateSamplerState(&tem, &ss));
		D3D11_SHADER_RESOURCE_VIEW_DESC ssk
		{
			TM.format,
			D3D11_SRV_DIMENSION_TEXTURE2D
		};
		ssk.Texture2D.MipLevels = 1;
		ssk.Texture2D.MostDetailedMip = 0;
		HRESULT re = op.tick().dev->CreateShaderResourceView(t, &ssk, &sv);
		PO::Win32::Error::fail_throw(re);
		
		//op.tick().dc->
		*/
	}
	catch (PO::Error::po_error& poe)
	{
		std::cout << poe << std::endl;
		__debugbreak();
	}


}