#include "test_plugin.h"
#include <vector>
#include <fstream>
using namespace std;

struct position
{
	const char* operator()() { return "POSITION"; }
};

struct diffuse
{
	const char* operator()() { return "DIFFUSE"; }
};

struct shifting
{
	const char* operator()() { return "SHIFTING"; }
};

struct texcoord
{
	const char* operator()() { return "TEXCOORD"; }
};

void test_plugin::tick(ticker& op)
{
	
	float color[4] = { 0.5,0.5,0.5,1.0 };
	op.tick().dc->ClearRenderTargetView(op.tick().pView, color);
	op.tick().dc->ClearDepthStencilView(op.tick().pDepthView, D3D11_CLEAR_FLAG::D3D11_CLEAR_DEPTH | D3D11_CLEAR_FLAG::D3D11_CLEAR_STENCIL, 1.0, 0);
	ID3D11ShaderResourceView* t[] = { sv };
	op.tick().dc->PSSetShaderResources(0, 1, t);
	pl.draw(op.tick().dc, vp, 6);
	op.tick().swap->Present(0, 0);
	//op.tick().dc->CSSetConstantBuffers()
	
	//op.form().close();
}

struct vert
{
	float2 ver;
	float2 tex;
	using type = layout_type<syntax<position, 0, float2>, syntax<texcoord, 0, float2>>;
};

struct inst
{
	float2 shift;
	using type = layout_type<syntax<shifting, 0, float2, 1>>;
};

struct ind
{
	uint16_t index;
	static constexpr DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_R16_UINT;
};

struct uint3 
{
	uint32_t x;
	uint32_t y;
	uint32_t z;
};

struct int_detect
{
	uint3 GroupID;
	uint3 GroupThreadID;
	uint32_t GroupIndex;
	uint3 DispatchGrounpIndex;
};

std::ostream& operator<<(std::ostream& o, uint3 u)
{
	o << "(" << u.x << "," << u.y << "," << u.z << ")";
	return o;
}

std::ostream& operator<<(std::ostream& o, int_detect& u)
{
	o << u.GroupID << "," << u.GroupThreadID << "," << u.GroupIndex << "," << u.DispatchGrounpIndex;
	return o;
}

void test_plugin::init(ticker& op)
{
	pl.set_resource(op.tick().dev);
	vp.set_resource(op.tick().dev);
	cp.set_resource(op.tick().dev);
	vert data[] =
	{
		{ {-0.2f, 0.2f}, {0.0, 0.0} },
		{ {-0.2f, -0.2f}, {0.0, 1.0} },
		{ {0.2f, -0.2f}, {1.0, 1.0} },
		{ {0.2f, 0.2f},{1.0,0.0} }
	};
	
	inst data2[] =
	{
		{{-0.3f,0.3f}},
		{{0.3f,-0.3f}},
		{ { 0.3f,0.3f } },
		{ { -0.3f,-0.3f } },
	};

	ind yu[] = { {0} , {1}, {2}, {2}, {3}, {0} };
	ind yu2[] = { { 0 } ,{ 1 },{ 2 },{ 3 } };
	

	//ver.create(op.tick(), data, 6);
	try
	{
		int** io233 = new int*[12];
		PO::Win32::Error::fail_throw(vp.create_vertex(0, data, 4, typename vert::type{}));
		vp.set_vertex(0, 4);
		PO::Win32::Error::fail_throw(vp.create_vertex(1, data2, 4, typename inst::type{}));
		vp.set_instance(0, 4);
		PO::Win32::Error::fail_throw(vp.create_index(yu2, sizeof(ind) * 4, ind::format));
		vp.set_index(0, 4);
		vp.set_primitive(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
		sl.load(op.form(), u"base_vshader.cso");
		std::vector<int> io;
		//io.
		auto fin = sl.wait_get();
		if (fin)
			PO::Win32::Error::fail_throw(pl.load_shader_v(std::move(*fin)));
		sl.load(op.form(), u"base_pshader.cso");
		fin = sl.wait_get();
		if (fin)
			PO::Win32::Error::fail_throw(pl.load_shader_p(std::move(*fin)));
		sl.load(op.form(), u"base_gshader.cso");
		fin = sl.wait_get();
		if (fin)
			PO::Win32::Error::fail_throw(pl.load_shader_g(std::move(*fin)));

		DirectX::TexMetadata TM;
		PO::Win32::Error::fail_throw(DirectX::LoadFromTGAFile(u"ty.tga"_wc, &TM, si));
		D3D11_TEXTURE2D_DESC desc;
		desc.Width = static_cast<UINT>(TM.width);
		desc.Height = static_cast<UINT>(TM.height);
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
			static_cast<UINT>(si.GetImages()->rowPitch),
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
			{ 0.0,0.0,0.0,0.0 },
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
		t.Release();
		//int_detect data[8 * 8 * 8 * 8];
		D3D11_BUFFER_DESC des
		{
			sizeof(int_detect) * 8 * 8 * 8 * 8,
			D3D11_USAGE_DEFAULT,
			D3D11_BIND_UNORDERED_ACCESS,
			0,
			D3D11_RESOURCE_MISC_BUFFER_STRUCTURED,
			sizeof(int_detect)
		};
		PO::Win32::Error::fail_throw(op.tick().dev->CreateBuffer(&des, nullptr, &buf));
		D3D11_BUFFER_DESC des2
		{
			sizeof(int_detect) * 8 * 8 * 8 * 8,
			D3D11_USAGE_STAGING,
			0,
			D3D11_CPU_ACCESS_READ,
			D3D11_RESOURCE_MISC_BUFFER_STRUCTURED,
			sizeof(int_detect)
		};
		PO::Win32::Error::fail_throw(op.tick().dev->CreateBuffer(&des2, nullptr, &buf2));


		sl.load(op.form(), u"base_cshader.cso");
		fin = sl.wait_get();
		if (fin)
			PO::Win32::Error::fail_throw(cp.load_shader_c(std::move(*fin)));
		D3D11_UNORDERED_ACCESS_VIEW_DESC des3
		{ 
			DXGI_FORMAT_UNKNOWN,
			D3D11_UAV_DIMENSION_BUFFER
		};
		des3.Buffer.FirstElement = 0;
		des3.Format = DXGI_FORMAT_UNKNOWN;
		des3.Buffer.NumElements = 8 * 8 * 8 * 8;
		ID3D11UnorderedAccessView* uav;

		PO::Win32::Error::fail_throw(op.tick().dev->CreateUnorderedAccessView(buf, &des3, &uav));
		
		op.tick().dc->CSSetShader(cp.shader_c, nullptr, 0);
		ID3D11UnorderedAccessView* te[] = { uav };
		op.tick().dc->CSSetUnorderedAccessViews(0, 1, te, nullptr);
		op.tick().dc->Dispatch(8, 8, 1);
		op.tick().dc->CopyResource(buf2, buf);
		D3D11_MAPPED_SUBRESOURCE tt;
		PO::Win32::Error::fail_throw(op.tick().dc->Map(buf2, 0, D3D11_MAP::D3D11_MAP_READ, 0, &tt));
		cout << tt.pData << endl;
		int_detect* yu = static_cast<int_detect*>(tt.pData);
		std::fstream iop("wtf.txt", std::ios::out);
		for (size_t x1 = 0; x1 < 8; ++x1)
		{
			for (size_t y1 = 0; y1 < 8; ++y1)
			{
				for (size_t x2 = 0; x2 < 8; ++x2)
				{
					for (size_t y2 = 0; y2 < 8; ++y2)
					{
						auto& ui = yu[x1 * 8 * 8 * 8 + y1 * 8 * 8 + x2 * 8 + y2];
						//cout << "asd" << endl;
						iop << "[" << x1 << "," << y1 << "," << x2 << "," << y2 << "]:[" << ui << "]" << endl;
					}
				}
			}
		}
		op.tick().dc->Unmap(buf2, 0);


		/*
		PO::Win32::Error::fail_throw(vl.create(vert::type{}, op.tick(), vs));
		

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