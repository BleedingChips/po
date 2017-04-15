#include "test_plugin.h"
#include <vector>
#include <fstream>
using namespace std;
using namespace PO::Dx;
struct position
{
	const char* operator()() { return "POSITION"; }
};

struct diffuse
{
	const char* operator()() { return "DIFFUSE"; }
};

struct normal
{
	const char* operator()() { return "NORMAL"; }
};

struct shifting
{
	const char* operator()() { return "SHIFTING"; }
};

struct texcoord
{
	const char* operator()() { return "TEXCOORD"; }
};

/*
PO::Tool::variant<Implement::texture1D_ptr, Implement::texture2D_ptr, Implement::texture3D_ptr> 
load_dds(Implement::resource_ptr& rs, std::u16string path, PO::Dx11::Purpose::purpose bp)
{
	DirectX::TexMetadata TM;
	DirectX::ScratchImage si;
	if (DirectX::LoadFromDDSFile(reinterpret_cast<const wchar_t*>(path.c_str()), 0, &TM, si) != S_OK)
		return{};
	static std::vector<D3D11_SUBRESOURCE_DATA> buffer;
	static std::mutex buffer_mutex;
	std::lock_guard<std::mutex> lg(buffer_mutex);
	buffer.clear();
	buffer.resize(TM.arraySize);
	for (size_t i = 0; i < TM.arraySize; ++i)
	{
		buffer[i].pSysMem = (si.GetImages() + i)->pixels;
		buffer[i].SysMemPitch = static_cast<UINT>((si.GetImages() + i)->rowPitch);
		buffer[i].SysMemSlicePitch = static_cast<UINT>((si.GetImages() + i)->slicePitch);
	}

	if (TM.height == 1 && TM.depth == 1)
	{
		D3D11_TEXTURE1D_DESC DTD
		{
			static_cast<UINT>(TM.width),
			static_cast<UINT>(TM.mipLevels),
			static_cast<UINT>(TM.arraySize),
			adjust_texture_format(TM.format),
			bp.usage,
			bp.additional_bind | D3D11_BIND_SHADER_RESOURCE,
			bp.cpu_flag,
			static_cast<UINT>(TM.miscFlags)
		};
		Implement::texture1D_ptr ptr;
		if (rs->CreateTexture1D(&DTD, buffer.data(), &ptr) == S_OK)
			return{ ptr };
		return{};
	}
	else if (TM.depth == 1)
	{
		D3D11_TEXTURE2D_DESC DTD
		{
			static_cast<UINT>(TM.width),
			static_cast<UINT>(TM.height),
			static_cast<UINT>(TM.mipLevels),
			static_cast<UINT>(TM.arraySize),
			adjust_texture_format(TM.format),
			DXGI_SAMPLE_DESC{ 1, 0 },
			bp.usage,
			bp.additional_bind | D3D11_BIND_SHADER_RESOURCE,
			bp.cpu_flag,
			static_cast<UINT>(TM.miscFlags)
		};
		Implement::texture2D_ptr ptr;
		if (rs->CreateTexture2D(&DTD, buffer.data(), &ptr) == S_OK)
			return{ ptr };
		return{};
	}
	else {
		D3D11_TEXTURE3D_DESC DTD
		{
			static_cast<UINT>(TM.width),
			static_cast<UINT>(TM.height),
			static_cast<UINT>(TM.depth),
			static_cast<UINT>(TM.mipLevels),
			adjust_texture_format(TM.format),
			bp.usage,
			bp.additional_bind | D3D11_BIND_SHADER_RESOURCE,
			bp.cpu_flag,
			static_cast<UINT>(TM.miscFlags)
		};
		Implement::texture3D_ptr ptr;
		if (rs->CreateTexture3D(&DTD, buffer.data(), &ptr) == S_OK)
			return{ ptr };
		return{};
	}
	return{};
}
*/
Implement::resource_view_ptr cast_resource(Implement::resource_ptr& rp, 
	const PO::Tool::variant<Implement::texture1D_ptr, Implement::texture2D_ptr, Implement::texture3D_ptr>& v
)
{
	if (v.able_cast<Implement::texture2D_ptr>())
		return PO::Dx11::cast_resource(rp, v.cast<Implement::texture2D_ptr>());
	else if (v.able_cast<Implement::texture3D_ptr>())
		return PO::Dx11::cast_resource(rp, v.cast<Implement::texture3D_ptr>());
	else if (v.able_cast<Implement::texture1D_ptr>())
		return PO::Dx11::cast_resource(rp, v.cast<Implement::texture1D_ptr>());
	return Implement::resource_view_ptr{};
}

struct start_init
{
	start_init()
	{
		PO::io_task_instance().set_function(typeid(PO::binary), [](PO::io_block ib) -> PO::Tool::any {
			if (!ib.stream.good())
				__debugbreak();
			ib.stream.seekg(0, std::ios::end);
			auto end_poi = ib.stream.tellg();
			ib.stream.seekg(0, std::ios::beg);
			auto sta_poi = ib.stream.tellg();
			PO::binary info {static_cast<size_t>(end_poi - sta_poi) };
			ib.stream.read(info, end_poi - sta_poi);
			return std::move(info);
		});
	}
} init;

struct int_detect
{
	uint32_t3 GroupID;
	uint32_t3 GroupThreadID;
	uint32_t GroupIndex;
	uint32_t3 DispatchGrounpIndex;
};

std::ostream& operator<<(std::ostream& o, uint32_t3 u)
{
	o << "(" << u.x << "," << u.y << "," << u.z << ")";
	return o;
}

std::ostream& operator<<(std::ostream& o, int_detect& u)
{
	o << "<GroupID>"<<u.GroupID << ",<GroupThreadID>" << u.GroupThreadID << ",<GroupIndex>" << u.GroupIndex << ",<DispatchGrounpIndex>" << u.DispatchGrounpIndex;
	return o;
}

float3 poi[] = 
{
	{-1.0, 1.0, 0.0},
	{-1.0, -1.0, 0.0},
	{1.0, -1.0, 0.0},
	{1.0, 1.0, 0.0}
};

uint16_t ind[] = 
{
	0, 1, 2, 2,3,0
};

void faile_break(bool i)
{
	if (!i)
		__debugbreak();
}

void test_plugin::init(ticker& op)
{
	auto res = rs.find(typeid(PO::binary), u"base_vshader.cso", PO::Tool::any{});

	faile_break(res && res->able_cast<PO::binary>());
	faile_break(res && op.tick().res.SR.create_shader(vs, res->cast<PO::binary>()));

	res = rs.find(typeid(PO::binary), u"base_pshader.cso", PO::Tool::any{});
	faile_break(res && res->able_cast<PO::binary>());
	faile_break(op.tick().res.SR.create_shader(ps, res->cast<PO::binary>()));

	faile_break(op.tick().res.IA.create_vertex(iad, 0, poi, 4, PO::Dx11::layout_type<PO::Dx11::syntax<position, 0, float3>>{}));
	faile_break(op.tick().res.IA.create_index(iad, ind, 6));
	faile_break(op.tick().res.IA.update_layout(iad, vs));


	/*
	pc.create_vertex(0, poi, 4, PO::Dx11::layout_type<PO::Dx11::syntax<position, 0, float3>>{});
	pc.create_index(ind, 6);
	pc.set_index_range(0, 6);
	pc.set_vertex_range(0, 4);
	pc.load_vshader(u"vs.cso");
	ma.load_p(u"ps.cso");
	*/
}



void test_plugin::tick(ticker& op)
{
	/*
	pc.apply(op.tick());
	ma.apply(op.tick());
	pc.draw(op.tick());
	*/









	/*
	float color[4] = { 0.5,0.5,0.5,1.0 };
	op.tick().dc->ClearRenderTargetView(op.tick().pView, color);
	op.tick().dc->ClearDepthStencilView(op.tick().pDepthView, D3D11_CLEAR_FLAG::D3D11_CLEAR_DEPTH | D3D11_CLEAR_FLAG::D3D11_CLEAR_STENCIL, 1.0, 0);
	


	ma.apply(op.tick());
	ID3D11ShaderResourceView* tem[] = { rvp };
	op.tick().dc->PSSetShaderResources(0, 1, tem);
	ID3D11SamplerState* temss[] = { ss };
	op.tick().dc->PSSetSamplers(0, 1, temss);
	pc.primitive = D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	ID3D11DepthStencilState* DSS;
	D3D11_DEPTH_STENCIL_DESC tds
	{
		true,
		D3D11_DEPTH_WRITE_MASK_ALL,
		D3D11_COMPARISON_LESS,
		false,
		0,
		0,
		D3D11_DEPTH_STENCILOP_DESC{ D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS }
	};
	
	HRESULT re = op.tick().dev->CreateDepthStencilState(&tds, &DSS);
	op.tick().dc->OMSetDepthStencilState(DSS, 0);
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


	pc.draw(op.tick());

	//op.tick().swap->Present(0, 0);
	DSS->Release();
	RS->Release();
	*/
	/*
	float color[4] = { 0.5,0.5,0.5,1.0 };
	op.tick().dc->ClearRenderTargetView(op.tick().pView, color);
	op.tick().dc->ClearDepthStencilView(op.tick().pDepthView, D3D11_CLEAR_FLAG::D3D11_CLEAR_DEPTH | D3D11_CLEAR_FLAG::D3D11_CLEAR_STENCIL, 1.0, 0);
	ID3D11ShaderResourceView* t[] = { sv };
	op.tick().dc->PSSetShaderResources(0, 1, t);
	D3D11_BUFFER_DESC BD
	{
		sizeof(ConstBuffer),
		D3D11_USAGE_DYNAMIC,
		D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER,
		D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE,
		0,// D3D11_RESOURCE_MISC_BUFFER_STRUCTURED,
		0//sizeof(ConstBuffer)
	};
	ID3D11Buffer* IB;
	ConstBuffer opc{ {1.0,0.0,1.0,1.0},{0.0,1.0,1.0,1.0} };
	D3D11_SUBRESOURCE_DATA SD{static_cast<void*>(&opc), 0 ,0};
	try {
		PO::Win32::Error::fail_throw(op.tick().dev->CreateBuffer(&BD, &SD, &IB));
		op.tick().dc->PSSetConstantBuffers(1, 1, &IB);
	}
	catch (...)
	{
		__debugbreak();
	}
	
		pl.draw(op.tick().dc, vp, 6);
	op.tick().swap->Present(0, 0);
	//op.tick().dc->CSSetConstantBuffers()

	//op.form().close();
	*/
}

