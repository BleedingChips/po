#include "test_plugin.h"
#include <vector>
#include <fstream>
#include <random>
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
			PO::binary info{ static_cast<size_t>(end_poi - sta_poi) };
			ib.stream.read(info, end_poi - sta_poi);
			return std::move(info);
		});
	}
} init;

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

test_plugin::test_plugin()
{
	scene.pre_load(
		typeid(PO::binary),
		{
			u"cube_vs.cso",
			u"cube_ps.cso",
			u"vt_creator_cs.cso",
			u"vt_shadow_cs.cso",
			u"noise_creater.cso",
			u"ps.cso"
		}
	);
}


struct cube_ver
{
	float3 poi;
	float3 col;
};

std::vector<cube_ver> poi =
{
	{ { -0.5, -0.5, 0.5 },{ 1.0, 0.0, 0.0 } },
	{ { 0.5, -0.5, 0.5 },{ 1.0, 0.0, 0.0 } },
	{ { 0.5, 0.5, 0.5 },{ 1.0, 0.0, 0.0 } },
	{ { -0.5, 0.5, 0.5 },{ 1.0, 0.0, 0.0 } },

	{ { 0.5, -0.5, 0.5 },{ 0.0, 1.0, 0.0 } },
	{ { 0.5, -0.5, -0.5 },{ 0.0, 1.0, 0.0 } },
	{ { 0.5, 0.5, -0.5 },{ 0.0, 1.0, 0.0 } },
	{ { 0.5, 0.5, 0.5 },{ 0.0, 1.0, 0.0 } },


	{ { 0.5, -0.5, -0.5 },{ 0.0, 0.0, 1.0 } },
	{ { -0.5, -0.5, -0.5 },{ 0.0, 0.0, 1.0 } },
	{ { -0.5, 0.5, -0.5 },{ 0.0, 0.0, 1.0 } },
	{ { 0.5, 0.5, -0.5 },{ 0.0, 0.0, 1.0 } },
	

	{ { -0.5, -0.5, -0.5 },{ 1.0, 0.0, 1.0 } },
	{ { -0.5, -0.5, 0.5 },{ 1.0, 0.0, 1.0 } },
	{ { -0.5, 0.5, 0.5 },{ 1.0, 0.0, 1.0 } },
	{ { -0.5, 0.5, -0.5 },{ 1.0, 0.0, 1.0 } },

	{ { -0.5, 0.5, 0.5 },{ 1.0, 1.0, 0.0 } },
	{ { 0.5, 0.5, 0.5 },{ 1.0, 1.0, 0.0 } },
	{ { 0.5, 0.5, -0.5 },{ 1.0, 1.0, 0.0 } },
	{ { -0.5, 0.5, -0.5 },{ 1.0, 1.0, 0.0 } },

	{ { -0.5, -0.5, -0.5 },{ 0.0, 1.0, 1.0 } },
	{ { 0.5, -0.5, -0.5 },{ 0.0, 1.0, 1.0 } },
	{ { 0.5, -0.5, 0.5 },{ 0.0, 1.0, 1.0 } },

	{ { -0.5, -0.5, 0.5 },{ 0.0, 1.0, 1.0 } }
};



std::vector<uint16_t> ind =
{
	0,1,2,
	2,3,0,

	4,5,6,
	6,7,4,

	8,9,10,
	10,11,8,

	12,13,14,
	14,15,12,

	16,17,18,
	18,19,16,

	20,21,22,
	22,23,20
};

std::vector<uint16_t> ind_frame =
{
	0,1,1,2,2,3,3,0,
	4,5,5,6,6,7,7,4,
	8,9,9,10,10,11,11,8,
	12,13,13,14,14,15,15,12,
	16,17,17,18,18,19,19,16,
	20,21,21,22,22,23,23,20
};

void faile_break(bool i)
{
	if (!i)
		__debugbreak();
}

float3 poi_shift = { 0.0, 0.0, 0.0 };


struct cube_cbuffer
{
	float4x4 view_matrix;
	float4x4 project;
};

struct shader_cbuffer
{
	float4x4 inverse_view;
	float sample_step;
	uint32_t3 tex_size;
	float3 light_direction;
};

struct PS_view
{
	float4x4 matrix;
	float4x4 pro_matrix;
	float nearz;
};

void test_plugin::init(ticker& op)
{

	auto& re = op.tick().res;
	auto& pipe = op.tick().pipe;

	

	std::vector<uint32_t> count = {128, 64, 32, 16, 8};
	std::vector<texture3D_ptr> noise(count.size(), nullptr);
	compute_d noise_cd;
	auto res = scene.find(typeid(PO::binary), u"noise_creater.cso", PO::Tool::any{});
	faile_break(
		res && res->able_cast<PO::binary>() &&
		re.CS.create_shader(noise_cd, res->cast<PO::binary>())
	);
	re.CS.create_cbuffer(noise_cd, 0, (uint32_t*)nullptr);
	for (size_t i = 0; i < count.size(); ++i)
	{
		faile_break(
			re.CS.create_UAT(noise[i], DXGI_FORMAT::DXGI_FORMAT_R16_FLOAT, 256, 256, 256, 1) &&
			re.CS.cast_UAV(noise_cd, 0, noise[i], 0, 0, 256) &&
			pipe.CS.write(noise_cd, 0, [&](void* data, UINT, UINT u) {*(uint32_t*)(data) = count[i]; })
		);
		draw_range_d noise;
		noise.set_dispatch_d(256, 256, 256);
		pipe.CS.bind(noise_cd);
		pipe.DR.draw(noise);
		pipe.CS.unbind();
	}

	res = scene.find(typeid(PO::binary), u"vt_creator_cs.cso", PO::Tool::any{});
	
		faile_break(res && res->able_cast<PO::binary>());
		faile_break(re.CS.create_shader(noise_cd, res->cast<PO::binary>()));
		faile_break(re.CS.create_UAT(vt, DXGI_FORMAT::DXGI_FORMAT_R16_FLOAT, 256, 256, 256, 1));
		faile_break(re.CS.cast_UAV(noise_cd, 0, vt, 0, 0, 256));

	for (size_t i = 0; i < noise.size(); ++i)
		faile_break(re.CS.cast_SRV(noise_cd, i, noise[i], 0, 1));

	{
		draw_range_d noise;
		noise.set_dispatch_d(256, 256, 256);
		pipe.CS.bind(noise_cd);
		pipe.DR.draw(noise);
		pipe.CS.unbind();
		DirectX::ScratchImage SI;
		DirectX::CaptureTexture(re.dp, pipe.ptr, vt, SI);
		DirectX::SaveToDDSFile(SI.GetImages(), SI.GetImageCount(), SI.GetMetadata(), 0, u"valum_texture.dds"_wc);
	}

	float4x4 intee;
	DirectX::XMStoreFloat4x4(&intee, DirectX::XMMatrixInverse(nullptr, inter));

	shader_cbuffer sc
	{
		intee,
		1.0,
		uint32_t3{256,256,256},
		float3{0.0, -1.0, 0.0}
	};

	compute_d vt_shadow_creator;
	res = scene.find(typeid(PO::binary), u"vt_shadow_cs.cso", PO::Tool::any{});
	faile_break(
		res && res->able_cast<PO::binary>() &&
		op.tick().res.CS.create_shader(vt_shadow_creator, res->cast<PO::binary>())
	);

	//shader_d template_sd;
	//uint32_t ran = 

	faile_break(
		re.CS.create_UAT(vt_shadow, DXGI_FORMAT::DXGI_FORMAT_R16_FLOAT, 256, 256, 256, 1) &&
		re.CS.cast_UAV(vt_shadow_creator, 0, vt_shadow, 0, 0, 256) &&
		re.CS.cast_SRV(vt_shadow_creator, 0, vt, 0, 1) &&
		re.CS.create_cbuffer(vt_shadow_creator, 0, &sc)
	);

	{
		pipe.CS.bind(vt_shadow_creator);
		draw_range_d creator;
		creator.set_dispatch_d(256, 256, 256);
		pipe.DR.draw(creator);
		DirectX::ScratchImage SI;
		DirectX::CaptureTexture(re.dp, pipe.ptr, vt_shadow, SI);
		DirectX::SaveToDDSFile(SI.GetImages(), SI.GetImageCount(), SI.GetMetadata(), 0, u"valum_texture_shadow.dds"_wc);
	}
	pipe.CS.unbind();

	res = scene.find(typeid(PO::binary), u"cube_vs.cso", PO::Tool::any{});
	faile_break(
		res && res->able_cast<PO::binary>() && 
		op.tick().res.VS.create_shader(cube_vs_d, res->cast<PO::binary>())
		);

	res = scene.find(typeid(PO::binary), u"cube_ps.cso", PO::Tool::any{});
	faile_break(
		res && res->able_cast<PO::binary>() && 
		op.tick().res.PS.create_shader(cube_ps_d, res->cast<PO::binary>())
	);

	faile_break(
		op.tick().res.IA.create_vertex(cube_ia_d, 0, poi, PO::Dx11::layout_type<PO::Dx11::syntax<position, 0, float3>, PO::Dx11::syntax<diffuse, 0, float3>>{}) &&
		op.tick().res.IA.create_index(cube_ia_d, ind) &&
		op.tick().res.IA.update_layout(cube_ia_d, cube_vs_d)
	);

	texture2D_ptr depth_text;

	faile_break(
		re.OM.create_DST(depth_text, DST_format::F32, op.tick().back_buffer) &&
		re.OM.cast_DSV(cube_m, depth_text, 0) &&
		re.OM.cast_RTV(cube_m, 0, op.tick().back_buffer, 0)
	);

	faile_break(
		re.VS.create_cbuffer(cube_vs_d, 0, (cube_cbuffer*)(nullptr)) &&
		re.PS.create_cbuffer(cube_ps_d, 0, (PS_view*)(nullptr))
	);

	raterizer_s rs;
	//rs.stop_cull();
	rs.view_fill_texture(0, op.tick().back_buffer, 0.0, 1.0);
	blend_s bs;
	depth_stencil_s dss;

	
	bs.set_RT_blend_state([](D3D11_RENDER_TARGET_BLEND_DESC& D) {
		D.BlendEnable = TRUE;
		D.SrcBlend = D3D11_BLEND_ONE;
		D.DestBlend = D3D11_BLEND_SRC_ALPHA;
		D.BlendOp = D3D11_BLEND_OP_ADD;
	});

	faile_break(
	re.RS.create_state(cube_ra_d, rs) &&
	re.PS.cast_SRV(cube_ps_d, 0, vt, 0, 1) &&
	re.PS.cast_SRV(cube_ps_d, 1, vt_shadow, 0, 1) &&
	re.OM.create_state(cube_m, bs) &&
	re.OM.create_state(cube_m, dss)
		);

	res = scene.find(typeid(PO::binary), u"ps.cso");
	faile_break(
		res && res->able_cast<PO::binary>() &&
		re.PS.create_shader(frame_cube_ps_d, res->cast<PO::binary>()) &&
		re.IA.create_vertex(frame_cube_ia_d, 0, poi, PO::Dx11::layout_type<PO::Dx11::syntax<position, 0, float3>, PO::Dx11::syntax<diffuse, 0, float3>>{}) &&
		re.IA.create_index(frame_cube_ia_d, ind_frame) &&
		re.IA.update_layout(frame_cube_ia_d, cube_vs_d)
	);
	frame_cube_ia_d.primitive = D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINELIST;

	



	//DirectX::ScratchImage SI;
	//DirectX::CaptureTexture(re.dp, pipe.ptr, tp, SI);
	//DirectX::SaveToDDSFile(SI.GetImages(), SI.GetImageCount(), SI.GetMetadata(), 0, u"lalalala.dds"_wc);

}

opposite_direct od;
opposite_direct left_right;
opposite_direct up_down;
opposite_direct pp2;

float loca = 4.0;
float lr = 0.0;
float up = 0.0;
float up2 = 0.0;

PO::Respond test_plugin::respond(conveyer& c)
{
	//return vm.capture_event(c.get_event());
	if (c.get_event().is_key())
	{
		auto& u = c.get_event();
		if (u.is_key())
		{
			switch (u.key.get_asc())
			{
			case 'r':
				od.positive(u.key.is_down());
				break;
			case 'f':
				od.negetive(u.key.is_down());
				break;
			case 'a':
				left_right.negetive(u.key.is_down());
				break;
			case 'd':
				left_right.positive(u.key.is_down());
				break;
			case 'w':
				up_down.positive(u.key.is_down());
				break;
			case 's':
				up_down.negetive(u.key.is_down());
				break;
			case 'q':
				pp2.positive(u.key.is_down());
				break;
			case 'e':
				pp2.negetive(u.key.is_down());
				break;
			case 'z':
				if (u.key.is_up())
				{
					lr = 0.0;
					up = 0.0;
					up2 = 0.0;
				}
				break;
			}
		}
	}
	return PO::Respond::Pass;
}



void test_plugin::tick(ticker& op)
{

	if (od.final_direction() != 0)
	{
		loca += od.final_direction() * op.time().count() / 1000.0f * 2.0f;
		if (loca < 1.0) loca = 1.0;
	}

	if (left_right.final_direction() != 0)
		lr += left_right.final_direction() * op.time().count() / 1000.0f * 2.0f;

	if (up_down.final_direction() != 0)
		up += up_down.final_direction() * op.time().count() / 1000.0f * 2.0f;

	if (pp2.final_direction() != 0)
		up2 += pp2.final_direction() * op.time().count() / 1000.0f * 2.0f;

	inter.poi = float3(0.0, 0.0, loca);
	inter.eul = float3(up2, lr, up);
	//cout << inter.poi <<" ,  "<<inter.eul<< endl;
	
	auto& re = op.tick().res;
	auto& pipe = op.tick().pipe;
	//pipe.VS.write()
	draw_range_d drd;
	drd.set_index(UINT(ind.size()),0, 0);

	{
		float4x4 pro;
		DirectX::XMStoreFloat4x4(&pro, DirectX::XMMatrixPerspectiveFovLH(3.1415926f / 4.0f, 1024.0f / 768.0f, 0.01f, 1000.0f));

		pipe.VS.write(cube_vs_d, 0, [&](void* data, UINT c, UINT k) {
			*static_cast<cube_cbuffer*>(data) = cube_cbuffer{ static_cast<float4x4>(inter), pro };
		});
	}

	{
		float4x4 pro;
		DirectX::XMStoreFloat4x4(&pro, DirectX::XMMatrixInverse(nullptr, inter));

		float4x4 pro2;
		DirectX::XMStoreFloat4x4(&pro2, DirectX::XMMatrixInverse(nullptr, DirectX::XMMatrixPerspectiveFovLH(3.1415926f / 4.0f, 1024.0f / 768.0f, 0.01f, 1000.0f)));

		float4x4 pro3;
		DirectX::XMStoreFloat4x4(&pro3, DirectX::XMMatrixPerspectiveFovLH(3.1415926f / 4.0f, 1024.0f / 768.0f, 0.01f, 1000.0f));

		pipe.PS.write(cube_ps_d, 0, [&](void* data, UINT c, UINT k) {
			*static_cast<PS_view*>(data) = PS_view{ pro, pro3, 0.01f };
		});
	}

 	D3D11_VIEWPORT view = { 0.0f, 0.0f, 1024.0f, 768.0f, 0.0f, 1.0f };
	
	pipe.OM.clear_render_target(cube_m, { 0.0f, 0.0f, 0.5f, 1.0f });
	pipe.OM.clear_depth(cube_m, 1.0);
	pipe.RA.bind(cube_ra_d);
	pipe.IA.bind(cube_ia_d);
	pipe.VS.bind(cube_vs_d);
	pipe.PS.bind(cube_ps_d);
	pipe.OM.bind(cube_m);
	pipe.DR.draw(drd);

	drd.set_index(UINT(ind_frame.size()), 0, 0);
	pipe.IA.bind(frame_cube_ia_d);
	pipe.PS.bind(frame_cube_ps_d);
	pipe.DR.draw(drd);

	op.tick().update_screen();
	
	pipe.unbing();






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

