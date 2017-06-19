#include "test_plugin.h"
#include <vector>
#include <fstream>
#include <random>
#include "DirectXTex.h"
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


opposite_direct od;
opposite_direct left_right;
opposite_direct up_down;
opposite_direct pp2;

PO::Respond test_plugin::respond(event& c)
{
	//return vm.capture_event(c.get_event());
	if (c.is_key())
	{
		auto& u = c;
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
				mfo.qua = quaternions{};
				break;
			case 'p':
				if (u.key.is_up())
				{
					float4x4 p = mfo;
					cout << p << endl;
					cout << mfo.poi << endl;
					cout << mfo.sca << endl;
					cout << mfo.qua << endl;

				}
				break;
			}
		}
	}
	return PO::Respond::Pass;
}

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

test_plugin::test_plugin(construction<simple_renderer> p)
{
	std::cout << "create test_plugin" << endl;
	/*
	scene.pre_load(
		typeid(PO::binary),
		{
			u"cube_vs.cso",
			u"cube_ps.cso",
			u"volum_cs.cso",
			u"screen_vs.cso",
			u"screen_ps.cso"
		}
	);
	*/
	p.auto_bind_init(&test_plugin::init, this);
	p.auto_bind_tick(&test_plugin::tick, this);
	p.auto_bind_respond(&test_plugin::respond, this);
}

struct cube_ver
{
	float3 poi;
	float3 col;
	using type = PO::Dx11::layout_type<PO::Dx11::syntax<position, 0, float3>, PO::Dx11::syntax<diffuse, 0, float3>>;
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

struct screen_vertex_t
{
	float2 poi;
	float2 tex;
	using type = PO::Dx11::layout_type<PO::Dx11::syntax<position, 0, float2>, PO::Dx11::syntax<texcoord, 0, float2>>;
};

std::vector<screen_vertex_t> screen_vertex =
{
	{ { -1.0, -1.0 },{ 0.0, 1.0 } },
	{ { -1.0, 1.0 },{ 0.0, 0.0 } },
	{ { 1.0, 1.0 },{ 1.0, 0.0 } },
	{ { 1.0, -1.0 },{ 1.0, 1.0 } }
};

std::vector<uint16_t> screen_index = {
	0 , 1, 2,
	2, 3, 0
};

void faile_break(bool i)
{
	if (!i)
		__debugbreak();
}




void test_plugin::init(simple_renderer& op)
{
	
	auto& re = op;
	auto& pipe = op;
	pipe << op.om;

	try {
		

		back = re.create_tex2_depth_stencil(PO::Dx11::DST_format::D24_UI8, op.back_buffer);
		g_poi = re.create_tex2_render_target(DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT, back);
		g_col = re.create_tex2_render_target(DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT, back);

		{
			/*
			std::mt19937 r_mt(233);
			std::array<float4, 4> ran;
#undef max
			for (size_t i = 0; i < 4; ++i)
			{
				ran[i] = float4{
					r_mt() / static_cast<float>(decltype(r_mt)::max()),
					r_mt() / static_cast<float>(decltype(r_mt)::max()),
					r_mt() / static_cast<float>(decltype(r_mt)::max()),
					r_mt() / static_cast<float>(decltype(r_mt)::max())
				};
				cout << ran[i] << endl;
			}
				
			compute_stage cs;
			auto res = scene.find(typeid(PO::binary), u"volum_noise_cs.cso", PO::Tool::any{});
			faile_break(res && res->able_cast<PO::binary>());
			da = re.create_tex3_unordered_access(DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT, 256, 256, 256);
			cs.set(re.create_compute_shader(res->cast<PO::binary>()))
				.set(re.cast_unordered_access_view(da), 0)
				.set(re.create_constant_buffer(sizeof(float4) * 8, ran.data()), 0);
			pipe << cs;
			pipe.dispatch(256, 256, 256);
			pipe.unbind();
			*/

			/*
			DirectX::ScratchImage SI;
			DirectX::CaptureTexture(re.dev, pipe.ptr, da.ptr, SI);
			DirectX::SaveToDDSFile(SI.GetImages(), SI.GetImageCount(), SI.GetMetadata(), 0, u"noise.dds"_wc);
			*/
			
		}

		/*
		{
			std::mt19937 r_mt(189);
			std::uniform_real_distribution<float> nd(-0.3f, 0.3f);
			compute_stage new_vt;
			auto res = scene.find(typeid(PO::binary), u"volum_cs.cso", PO::Tool::any{});
			faile_break(res && res->able_cast<PO::binary>());
			volum = re.create_tex3_unordered_access(DXGI_FORMAT::DXGI_FORMAT_R16_FLOAT, 256, 256, 256);
			std::vector<float4> point;
			for (size_t i = 0; i < 30; ++i)
				point.push_back(
					float4{
				nd(r_mt),
				nd(r_mt),
				nd(r_mt),
				r_mt() / static_cast<float>(decltype(r_mt)::max())
			}

				);
			structed_buffer sb = re.create_struct_buffer(point);
			new_vt.set(re.create_compute_shader(res->cast<PO::binary>()))
				.set(re.cast_shader_resource_view(da), 0)

				.set(re.cast_shader_resource_view(sb), 1)
				.set(re.cast_unordered_access_view(volum), 0);
			pipe << new_vt;
			pipe.dispatch(8, 8, 256);
			pipe.unbind();
		}
		*/

		tex3 tem = re.create_tex3_unordered_access(DXGI_FORMAT::DXGI_FORMAT_R16_FLOAT, 256, 256, 256);
		volum = re.create_tex3_unordered_access(DXGI_FORMAT::DXGI_FORMAT_R16G16_FLOAT, 256, 256, 256);

		{
			compute_stage cs;
			auto shader = scene.find(typeid(binary), u"volum_cs.cso");
			if (!shader || !shader->able_cast<binary>()) throw 1;
			cs.set(re.create_compute_shader(shader->cast<binary>()));
			cs.set(re.cast_unordered_access_view(volum), 0);
			struct buffer {
				float4 Center[20];
				float4 Perlin[4];
			} da;
			std::mt19937 r_mt(233);
#undef max
			for (size_t i = 0; i < 4; ++i)
				da.Perlin[i] = float4{ static_cast<float>(decltype(r_mt)::max()) , static_cast<float>(decltype(r_mt)::max()) , static_cast<float>(decltype(r_mt)::max()) , static_cast<float>(decltype(r_mt)::max()) };

			std::uniform_real_distribution<float> nd(-0.3f, 0.3f);

			for (size_t i = 0; i < 20; ++i)
				da.Center[i] = float4{ nd(r_mt),
				nd(r_mt),
				nd(r_mt),
				r_mt() / static_cast<float>(decltype(r_mt)::max())
			};

			cs << re.create_constant_buffer(&da)[0];
			pipe << cs;
			pipe.dispatch(256, 256, 256);
			pipe.unbind();

			DirectX::ScratchImage SI;
			DirectX::CaptureTexture(re.dev, pipe.ptr, tem.ptr, SI);
			DirectX::SaveToDDSFile(SI.GetImages(), SI.GetImageCount(), SI.GetMetadata(), 0, u"out_volume.dds"_wc);

		}

		

		{
			/*
			const uint32_t center = 20;
			std::mt19937 r_mt(200);
			std::uniform_real_distribution<float> nd(-0.3f, 0.3f);
			std::vector<float3> poi23333;
			poi23333.reserve(center);
			for (uint32_t i = 0; i < center; ++i)
			{
				float3 cur = float3{ nd(r_mt), nd(r_mt), nd(r_mt) };
				poi23333.push_back(cur);
			}
			compute_stage new_vt;
			auto res = scene.find(typeid(PO::binary), u"volum_cs.cso", PO::Tool::any{});
			faile_break(res && res->able_cast<PO::binary>());
			new_vt.set(re.create_compute_shader(res->cast<PO::binary>()));
			structed_buffer sb = re.create_struct_buffer(poi23333);
			new_vt.set(re.cast_shader_resource_view(sb), 0);
			volum = re.create_tex3_unordered_access(DXGI_FORMAT::DXGI_FORMAT_R16_FLOAT, 256, 256, 256);
			new_vt.set(re.cast_unordered_access_view(volum), 0);
			pipe.bind(new_vt);
			//pipe.dispatch(256, 256, 256);
			pipe.unbind();
			//DirectX::ScratchImage SI;
			//DirectX::CaptureTexture(re.dev, pipe.ptr, volum.ptr, SI);
			//DirectX::SaveToDDSFile(SI.GetImages(), SI.GetImageCount(), SI.GetMetadata(), 0, u"new_valum_texture.dds"_wc);
			*/
		}

		/*
		{
			//fractal
			compute_stage fractal;
			auto res = scene.find(typeid(PO::binary), u"fractal.cso", PO::Tool::any{});
			faile_break(res && res->able_cast<PO::binary>());
			fractal.set(re.create_compute_shader(res->cast<PO::binary>()));
			const uint32_t count = 256 * 256 * 256;
			std::mt19937 r_mt(200);
			std::tuple<float4, float4> random_data = {
				{ ((r_mt() % 256) / 256.0f), ((r_mt() % 256) / 256.0f), ((r_mt() % 256) / 256.0f), ((r_mt() % 256) / 256.0f) },
				{ ((r_mt() % 256) / 256.0f) ,((r_mt() % 256) / 256.0f) ,((r_mt() % 256) / 256.0f) ,((r_mt() % 256) / 256.0f) }
			};
			tex3 output = re.create_tex3_unordered_access(DXGI_FORMAT::DXGI_FORMAT_R16_FLOAT, 256, 256, 256);
			fractal.set(re.create_constant_buffer(&random_data), 0);
			for (size_t i = 0; i < 2; ++i)
			{
				fractal.set(re.cast_shader_resource_view(((i % 2) == 0) ? volum : output), 0)
					.set(re.cast_unordered_access_view(((i % 2) == 0) ? output : volum), 0);
				pipe << fractal;
				pipe.dispatch(256, 256, 256);
				pipe.unbind();
			}
			
			DirectX::ScratchImage SI;
			DirectX::CaptureTexture(re.dev, pipe.ptr, output.ptr, SI);
			DirectX::SaveToDDSFile(SI.GetImages(), SI.GetImageCount(), SI.GetMetadata(), 0, u"fractal.dds"_wc);
		}
		*/

		

		{
			compute_stage shadow_shader;
			auto res = scene.find(typeid(PO::binary), u"volum_shadow_cs.cso", PO::Tool::any{});
			faile_break(res && res->able_cast<PO::binary>());
			shadow_shader.set(re.create_compute_shader(res->cast<PO::binary>()))
				//.set(re.cast_shader_resource_view(tem), 0)
				.set(re.cast_unordered_access_view(volum), 0);
			pipe.bind(shadow_shader);
			pipe.dispatch(256, 256, 256);
			pipe.unbind();
			//DirectX::ScratchImage SI;
			//DirectX::CaptureTexture(re.dev, pipe.ptr, volum_shadow.ptr, SI);
			//DirectX::SaveToDDSFile(SI.GetImages(), SI.GetImageCount(), SI.GetMetadata(), 0, u"new_valum_texture_shadow.dds"_wc);
			DirectX::ScratchImage SI;
			DirectX::CaptureTexture(re.dev, pipe.ptr, volum.ptr, SI);
			DirectX::SaveToDDSFile(SI.GetImages(), SI.GetImageCount(), SI.GetMetadata(), 0, u"out_volume2.dds"_wc);
		}
		
		
		{
			cube_ia.set(re.create_vertex(poi, decltype(poi)::value_type::type{}), 0);
			cube_ia.set(re.create_index(ind));
			auto res = scene.find(typeid(PO::binary), u"cube_vs.cso", PO::Tool::any{});
			faile_break(res && res->able_cast<PO::binary>());
			cube_vs.set(re.create_vertex_shader(res->cast<PO::binary>()));
			cube_vs.set(re.create_constant_buffer(sizeof(float4)), 0);
			re.update_layout(cube_ia, cube_vs);
			cube_ra = re.create_raterizer_state();
		}
		
		{
			auto res = scene.find(typeid(PO::binary), u"cube_ps.cso", PO::Tool::any{});
			faile_break(res && res->able_cast<PO::binary>());
			deffer_ps.set(re.create_pixel_shader(res->cast<PO::binary>()));
			deffer_om.set(re.cast_render_target_view(g_col), 0)
				.set(re.cast_render_target_view(g_poi), 1)
				.set(re.cast_depth_setncil_view(back));
		}

		{
			auto res = scene.find(typeid(PO::binary), u"screen_vs.cso", PO::Tool::any{});
			faile_break(res && res->able_cast<PO::binary>());
			screen_vs.set(re.create_vertex_shader(res->cast<PO::binary>()));
			screen_ia.set(re.create_vertex(screen_vertex, decltype(screen_vertex)::value_type::type{}), 0)
				.set(re.create_index(screen_index));
			re.update_layout(screen_ia, screen_vs);
			screen_ra = re.create_raterizer_state();
		}
		
		{
			auto res = scene.find(typeid(PO::binary), u"screen_ps.cso", PO::Tool::any{});
			faile_break(res && res->able_cast<PO::binary>());
			screen_ps.set(re.create_pixel_shader(res->cast<PO::binary>()))
				.set(re.cast_shader_resource_view(g_col), 0)
				.set(re.cast_shader_resource_view(g_poi), 1)
				.set(re.create_sample_state(), 0);
			depth_stencil_state::scription scr = depth_stencil_state::default_scription;
			scr.DepthEnable = FALSE;
			scr.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_ALWAYS;
			scr.StencilEnable = FALSE;
			blend_state::scription scr_ble = blend_state::default_scription;
			scr_ble.RenderTarget[0].BlendEnable = FALSE;
			screen_om.set(re.cast_render_target_view(op.back_buffer), 0);
			screen_bs = re.create_blend_state(scr_ble);
			screen_dss = re.create_depth_stencil_state(scr);
		}
		
		{
			frame_ia.set(re.create_vertex(poi, decltype(poi)::value_type::type{}), 0);
			frame_ia.primitive = D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_LINELIST;
			frame_ia.set(re.create_index(ind_frame));
			re.update_layout(frame_ia, cube_vs);
			auto res = scene.find(typeid(PO::binary), u"frame_ps.cso", PO::Tool::any{});
			faile_break(res && res->able_cast<PO::binary>());
			frame_ps.set(re.create_pixel_shader(res->cast<PO::binary>()));
			frame_dss = re.create_depth_stencil_state();
			frame_bs = re.create_blend_state();
		}

		{
			auto res = scene.find(typeid(PO::binary), u"volum_ps.cso", PO::Tool::any{});
			faile_break(res && res->able_cast<PO::binary>());
			sample_state::scription scr_sample = sample_state::default_scription;
			scr_sample.Filter = D3D11_FILTER::D3D11_FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR;
			volum_ps.set(re.create_pixel_shader(res->cast<PO::binary>()))
				.set(re.cast_shader_resource_view(volum), 0)
				.set(re.create_sample_state(scr_sample), 0)
				.set(re.create_constant_buffer(sizeof(float4x4) + sizeof(float)), 0);
			blend_state::scription scr = blend_state::default_scription;
			scr.RenderTarget[0].BlendEnable = TRUE;
			scr.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
			scr.RenderTarget[0].DestBlend = D3D11_BLEND_SRC_ALPHA;
			scr.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			depth_stencil_state::scription scr22 = depth_stencil_state::default_scription;
			scr22.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			volum_bs = re.create_blend_state(scr);
			volum_dss = re.create_depth_stencil_state(scr22);
			
		}

	}
	catch (HRESULT re)
	{
		HRESULT res = re;
		__debugbreak();
	}

	

	
	/*faile_break(
	);*/

	

	//shader_d template_sd;
	//uint32_t ran = 

	/*
	{
		compute_d vt_shadow_creator;
		auto res = scene.find(typeid(PO::binary), u"vt_shadow_cs.cso", PO::Tool::any{});
		faile_break(
			res && res->able_cast<PO::binary>() &&
			op.rt.res.CS.create_shader(vt_shadow_creator, res->cast<PO::binary>())
		);
		float4x4 intee;
		DirectX::XMStoreFloat4x4(&intee, DirectX::XMMatrixInverse(nullptr, inter));
		shader_cbuffer sc
		{
			intee,
			1.0,
			uint32_t3{ 256,256,256 },
			float3{ 0.0, -1.0, 0.0 }
		};
		faile_break(
			re.CS.create_UAT(vt_shadow, DXGI_FORMAT::DXGI_FORMAT_R16_FLOAT, 256, 256, 256, 1) &&
			re.CS.cast_UAV(vt_shadow_creator, 0, vt_shadow, 0, 0, 256) &&
			re.CS.cast_SRV(vt_shadow_creator, 0, t, 0, 1) &&
			re.CS.create_cbuffer(vt_shadow_creator, 0, &sc)
		);
		pipe.bind(vt_shadow_creator);
		draw_range_d creator;
		creator.set_dispatch_d(256, 256, 256);
		pipe.draw(creator);
		DirectX::ScratchImage SI;
		DirectX::CaptureTexture(re.dp, pipe.ptr, vt_shadow, SI);
		DirectX::SaveToDDSFile(SI.GetImages(), SI.GetImageCount(), SI.GetMetadata(), 0, u"valum_texture_shadow.dds"_wc);
		pipe.unbind();
	}
	
	{
		auto res = scene.find(typeid(PO::binary), u"cube_vs.cso", PO::Tool::any{});
		faile_break(
			res && res->able_cast<PO::binary>() &&
			op.rt.res.VS.create_shader(cube_vs_d, res->cast<PO::binary>())
		);

		res = scene.find(typeid(PO::binary), u"cube_ps.cso", PO::Tool::any{});
		faile_break(
			res && res->able_cast<PO::binary>() &&
			op.rt.res.PS.create_shader(cube_ps_d, res->cast<PO::binary>())
		);

		faile_break(
			op.rt.res.IA.create_vertex(cube_ia_d, 0, poi, PO::Dx11::layout_type<PO::Dx11::syntax<position, 0, float3>, PO::Dx11::syntax<diffuse, 0, float3>>{}) &&
			op.rt.res.IA.create_index(cube_ia_d, ind) &&
			op.rt.res.IA.update_layout(cube_ia_d, cube_vs_d)
		);

		texture2D_ptr depth_text;

		faile_break(
			re.OM.create_DST(depth_text, DST_format::F32, op.rt.back_buffer) &&
			re.OM.cast_DSV(cube_m, depth_text, 0) &&
			re.OM.cast_RTV(cube_m, 0, op.rt.back_buffer, 0)
		);

		faile_break(
			re.VS.create_cbuffer(cube_vs_d, 0, (cube_cbuffer*)(nullptr)) &&
			re.PS.create_cbuffer(cube_ps_d, 0, (PS_view*)(nullptr))
		);

		raterizer_s rs;
		//rs.stop_cull();
		rs.view_fill_texture(0, op.rt.back_buffer, 0.0, 1.0);
		blend_s bs;
		depth_stencil_s dss;


		bs.set_RT_blend_state([](D3D11_RENDER_TARGET_BLEND_DESC& D) {
			D.BlendEnable = FALSE;
			D.SrcBlend = D3D11_BLEND_ONE;
			D.DestBlend = D3D11_BLEND_SRC_ALPHA;
			D.BlendOp = D3D11_BLEND_OP_ADD;
		});

		faile_break(
			re.RS.create_state(cube_ra_d, rs) &&
			re.PS.cast_SRV(cube_ps_d, 0, t, 0, 1) &&
			re.PS.cast_SRV(cube_ps_d, 1, vt_shadow, 0, 1) &&
			re.OM.create_state(cube_m, bs) &&
			re.OM.create_state(cube_m, dss)
		);
	}

	{
		auto res = scene.find(typeid(PO::binary), u"ps.cso");
		faile_break(
			res && res->able_cast<PO::binary>() &&
			re.PS.create_shader(frame_cube_ps_d, res->cast<PO::binary>()) &&
			re.IA.create_vertex(frame_cube_ia_d, 0, poi, PO::Dx11::layout_type<PO::Dx11::syntax<position, 0, float3>, PO::Dx11::syntax<diffuse, 0, float3>>{}) &&
			re.IA.create_index(frame_cube_ia_d, ind_frame) &&
			re.IA.update_layout(frame_cube_ia_d, cube_vs_d)
		);
		frame_cube_ia_d.primitive = D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
	}
	*/

}

float loc = 5.0f;

void test_plugin::tick(simple_renderer& t, duration da)
{

	PO::Dx::quaternions_template qt = mfo.qua;

	float angle_speed = da.count() / 1000.0f / 3.141592653f * 180.0f * 1.0f;

	if (left_right.final_direction() != 0)
		qt = qt * PO::Dx::rotation_axis{ left_right.final_direction() * angle_speed , {0.0, 1.0, 0.0} };

	if (up_down.final_direction() != 0)
		qt = qt * PO::Dx::rotation_axis{ up_down.final_direction() * angle_speed ,{ -1.0, 0.0, 0.0 } };

	if (pp2.final_direction() != 0)
		qt = qt * PO::Dx::rotation_axis{ pp2.final_direction() * angle_speed , { 0.0, 0.0, -1.0 } };

	loc += od.final_direction() * da.count() / 1000.0f * 5.0f;

	mfo.qua = qt;

	mfo.poi = float3(0.0, 0.0, loc);


	auto& re = t;
	auto& pipe = t;

	pipe.clear_render_target(deffer_om, { 0.0, 0.0, 0.8f, 1.0 });
	pipe.clear_depth(deffer_om, 1.0f);

	float4x4 pro;
	DirectX::XMStoreFloat4x4(&pro, DirectX::XMMatrixMultiply(mfo, DirectX::XMMatrixPerspectiveFovLH(3.1415926f / 4.0f, 1024.0f / 768.0f, 0.01f, 1000.0f)));

	
	if(!pipe.write_constant_buffer(cube_vs, 0, [&](void* data, UINT c, UINT k) {
		*static_cast<float4x4*>(data) = pro;
	}))__debugbreak();

	DirectX::XMStoreFloat4x4(&pro, DirectX::XMMatrixInverse(nullptr, DirectX::XMMatrixMultiply(mfo, DirectX::XMMatrixPerspectiveFovLH(3.1415926f / 4.0f, 1024.0f / 768.0f, 0.01f, 1000.0f))));

	if (!pipe.write_constant_buffer(volum_ps, 0, [&](void* data, UINT c, UINT k) {
		if (c <= sizeof(float4x4) + sizeof(float)) __debugbreak();
		*static_cast<float4x4*>(data) = pro;
		*reinterpret_cast<float*>(static_cast<float4x4*>(data) + 1) = 0.01f;
	}))__debugbreak();

	pipe.clear_render_target(screen_om, { 0.0, 0.0, 0.0, 1.0 });
	pipe.clear_depth(screen_om, 1.0f);


	pipe << cube_ia << cube_vs << cube_ra << deffer_om << pipe.vp;
	pipe << volum_ps << volum_bs << volum_dss;
	pipe.draw_index(static_cast<UINT>(ind.size()), 0, 0);

	pipe << frame_ia << frame_bs << frame_dss << frame_ps;
	pipe.draw_index(static_cast<UINT>(ind_frame.size()), 0, 0);
	pipe.unbind();
	
	pipe << screen_om << screen_vs << screen_ra << screen_ps << screen_ia << screen_bs << screen_dss;
	pipe.draw_index(static_cast<UINT>(screen_index.size()), 0, 0);
	pipe.unbind();

	


	







	/*
	if (od.final_direction() != 0)
	{
		loca += od.final_direction() * da.count() / 1000.0f * 2.0f;
		if (loca < 1.0) loca = 1.0;
	}

	if (left_right.final_direction() != 0)
		lr += left_right.final_direction() * da.count() / 1000.0f * 2.0f;

	if (up_down.final_direction() != 0)
		up += up_down.final_direction() * da.count() / 1000.0f * 2.0f;

	if (pp2.final_direction() != 0)
		up2 += pp2.final_direction() * da.count() / 1000.0f * 2.0f;

	inter.poi = float3(0.0, 0.0, loca);
	inter.eul = float3(up2, lr, up);
	//cout << inter.poi <<" ,  "<<inter.eul<< endl;
	
	auto& re = t.rt.res;
	auto& pipe = t.rt.pipe;
	//pipe.VS.write()
	draw_range_d drd;
	drd.set_index(UINT(ind.size()),0, 0);

	{
		float4x4 pro;
		DirectX::XMStoreFloat4x4(&pro, DirectX::XMMatrixPerspectiveFovLH(3.1415926f / 4.0f, 1024.0f / 768.0f, 0.01f, 1000.0f));

		pipe.write_cbuffer(cube_vs_d, 0, [&](void* data, UINT c, UINT k) {
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

		pipe.write_cbuffer(cube_ps_d, 0, [&](void* data, UINT c, UINT k) {
			*static_cast<PS_view*>(data) = PS_view{ pro, pro3, 0.01f };
		});
	}

 	D3D11_VIEWPORT view = { 0.0f, 0.0f, 1024.0f, 768.0f, 0.0f, 1.0f };
	
	pipe.clear_render_target(cube_m, { 0.0f, 0.0f, 0.5f, 1.0f });
	pipe.clear_depth(cube_m, 1.0);

	pipe.bind(cube_ra_d);
	pipe.bind(cube_ia_d);
	pipe.bind(cube_vs_d);
	pipe.bind(cube_ps_d);
	pipe.bind(cube_m);
	pipe.draw(drd);

	drd.set_index(UINT(ind_frame.size()), 0, 0);
	pipe.bind(frame_cube_ia_d);
	pipe.bind(frame_cube_ps_d);
	pipe.draw(drd);
	
	pipe.unbind();
	*/
}

