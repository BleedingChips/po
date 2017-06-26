#include "ue4_testing.h"
#include <random>
#include "DirectXTex.h"
#include "dx11/dx11_vertex.h"

adapter_map UE4_testing::mapping(self& s) {
	s.auto_bind_respond(&UE4_testing::respond, this);
	return {
		make_member_adapter<simple_renderer>(this, &UE4_testing::init, &UE4_testing::tick)
	};
}


UE4_testing::UE4_testing() {

	scene.pre_load(
		typeid(binary), 
		{
			u"test_cs.cso",
			u"test_cs2.cso",
			u"test_vs.cso",
			u"test_ps.cso"
		}
	);

	mfo.poi.z = 5.0;
}

struct position
{
	const char* operator()() { return "POSITION"; }
};

struct Cube_poi {
	float3 point;
	using type = PO::Dx11::layout_type<PO::Dx11::syntax<position, 0, float3>>;
};

std::vector<Cube_poi> poi =
{
	{ {-0.5, -0.5, 0.5 } },
	{ { 0.5, -0.5, 0.5 } },
	{ { 0.5, 0.5, 0.5 } },
	{ { -0.5, 0.5, 0.5 } },

	{ { 0.5, -0.5, 0.5 } },
	{ { 0.5, -0.5, -0.5 } },
	{ { 0.5, 0.5, -0.5 } },
	{ { 0.5, 0.5, 0.5 } },


	{ { 0.5, -0.5, -0.5 } },
	{ { -0.5, -0.5, -0.5 } },
	{ { -0.5, 0.5, -0.5 } },
	{ { 0.5, 0.5, -0.5 } },


	{ { -0.5, -0.5, -0.5 } },
	{ { -0.5, -0.5, 0.5 } },
	{ { -0.5, 0.5, 0.5 } },
	{ { -0.5, 0.5, -0.5 } },

	{ { -0.5, 0.5, 0.5 } },
	{ { 0.5, 0.5, 0.5 } },
	{ { 0.5, 0.5, -0.5 } },
	{ { -0.5, 0.5, -0.5 } },

	{ { -0.5, -0.5, -0.5 } },
	{ { 0.5, -0.5, -0.5 } },
	{ { 0.5, -0.5, 0.5 } },

	{ { -0.5, -0.5, 0.5 } }
};

static std::vector<uint16_t> ind =
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

void UE4_testing::init(simple_renderer& p)
{

	CoInitialize(NULL);

	/*
	// The factory pointer
	IWICImagingFactory *pFactory = NULL;

	// Create the COM imaging factory
	HRESULT hr = CoCreateInstance(
	CLSID_WICImagingFactory,
	NULL,
	CLSCTX_INPROC_SERVER,
	IID_PPV_ARGS(&pFactory)
	);
	*/


	try
	{
		creator& res = p;
		pipeline& pipe = p;
		unsigned int random_seed[20];
		std::mt19937 r_mt(233);
		for (size_t i = 0; i < 20; ++i)
		{
			random_seed[i] = r_mt();
		}

		//volume_texture = res.create_tex2_unordered_access(DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM, 4096, 4096);

		{
			compute_stage cs;
			auto shader = scene.find(typeid(binary), u"test_cs.cso");
			if (!shader || !shader->able_cast<binary>()) throw 1;
			cs.set(res.create_compute_shader(shader->cast<binary>()));
			for (size_t i = 0; i < 1; ++i)
			{
				volume_texture[i] = res.create_tex2_unordered_access(DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM, 4096, 4096);
				cs.set(res.cast_unordered_access_view(volume_texture[i]), 0);
				struct buffer {
					float4 Center[300];
					float4 Perlin[4];
				} da;
				std::mt19937 r_mt(random_seed[i]);
#undef max
				for (size_t i = 0; i < 4; ++i)
					da.Perlin[i] = float4{ static_cast<float>(decltype(r_mt)::max()) , static_cast<float>(decltype(r_mt)::max()) , static_cast<float>(decltype(r_mt)::max()) , static_cast<float>(decltype(r_mt)::max()) };

				std::uniform_real_distribution<float> nd(-0.3f, 0.3f);

				for (size_t i = 0; i < 300; ++i)
					da.Center[i] = float4{
					nd(r_mt),
					nd(r_mt),
					nd(r_mt),
					//r_mt() / static_cast<float>(decltype(r_mt)::max()) - 0.5f,
					//r_mt() / static_cast<float>(decltype(r_mt)::max()) - 0.5f,
					//r_mt() / static_cast<float>(decltype(r_mt)::max()) - 0.5f,
					r_mt() / static_cast<float>(decltype(r_mt)::max())
				};

				cs.set(res.create_constant_buffer(&da), 0);
				pipe << cs;
				pipe.dispatch(128, 128, 1);
				pipe.unbind();
				std::cout << "Create "<<i << std::endl;
			}
		}

		{

			compute_stage cs;
			auto shader = scene.find(typeid(binary), u"test_cs2.cso");
			if (!shader || !shader->able_cast<binary>()) throw 1;
			cs.set(res.create_compute_shader(shader->cast<binary>()));
			for (size_t i = 0; i < 1; ++i)
			{
				cs << res.cast_unordered_access_view(volume_texture[i])[0];
				float3 Dir = float3{ 0.0, -1.0, 0.0 };
				cs.set(res.create_constant_buffer(&Dir), 0);
				pipe << cs;
				pipe.dispatch(128, 128, 1);
				pipe.unbind();
				/*
				DirectX::ScratchImage SI;
				DirectX::CaptureTexture(res.dev, pipe.ptr, volume_texture[i].ptr, SI);
				//if (!SUCCEEDED(DirectX::SaveToDDSFile(SI.GetImages(), SI.GetImageCount(), SI.GetMetadata(), 0, u"out_volume233.dds"_wc))) throw 1;


				HRESULT t = DirectX::SaveToWICFile(
					*SI.GetImages(),
					static_cast<DWORD>(DirectX::WIC_FLAGS_NONE),
					DirectX::GetWICCodec(DirectX::WICCodecs::WIC_CODEC_PNG),
					reinterpret_cast<const wchar_t*>((std::u16string(u"VolumeTexture_") + static_cast<char16_t>(u'a' + i) + u"_.png").c_str())
				);
				if (!SUCCEEDED(t)) throw t;
				*/
				std::cout << "Shadow " << i << std::endl;
			}




			//DirectX::SaveToTGAFile(*SI.GetImages(), u"out_volume2.tga"_wc);
			//DirectX::SaveToHDRFile(*SI.GetImages(), u"out_volume2.hdr"_wc);

		}

		{
			ia << res.create_vertex(poi, decltype(poi)::value_type::type{})[0] 
				<< res.create_index(ind);
			vs.set(res.create_constant_buffer_with_size(sizeof(float4x4) * 2, true), 0);

			auto shader = scene.find(typeid(binary), u"test_vs.cso");
			if (!shader || !shader->able_cast<binary>()) throw 1;
			vs.set(res.create_vertex_shader(shader->cast<binary>()));
			res.update_layout(ia, vs);

			shader = scene.find(typeid(binary), u"test_ps.cso");
			if (!shader || !shader->able_cast<binary>()) throw 1;
			ps.set(res.create_pixel_shader(shader->cast<binary>()));
			ps.set(res.create_constant_buffer_with_size(sizeof(float4x4), true), 0);

			om.set(res.cast_render_target_view(p.back_buffer), 0);

			blend_state::scription scr = blend_state::default_scription;
			scr.RenderTarget[0].BlendEnable = TRUE;
			scr.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
			scr.RenderTarget[0].DestBlend = D3D11_BLEND_SRC_ALPHA;
			scr.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;

			bs = res.create_blend_state(scr);

			depth_stencil_state::scription scr22 = depth_stencil_state::default_scription;
			scr22.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			scr22.DepthEnable = false;
			scr22.DepthFunc = D3D11_COMPARISON_ALWAYS;

			dss = res.create_depth_stencil_state(scr22);

		}

	}
	catch (HRESULT HE)
	{
		HRESULT i = HE;
		__debugbreak();
	}

}

void UE4_testing::tick(simple_renderer& p, duration da)
{

	PO::Dx::quaternions_template qt = mfo.qua;

	float angle_speed = da.count() / 1000.0f / 3.141592653f * 180.0f * 1.0f;

	if (left_right.final_direction() != 0)
		qt = qt * PO::Dx::rotation_axis{ left_right.final_direction() * angle_speed ,{ 0.0, 1.0, 0.0 } };

	if (up_down.final_direction() != 0)
		qt = qt * PO::Dx::rotation_axis{ up_down.final_direction() * angle_speed ,{ -1.0, 0.0, 0.0 } };

	if (pp2.final_direction() != 0)
		qt = qt * PO::Dx::rotation_axis{ pp2.final_direction() * angle_speed ,{ 0.0, 0.0, -1.0 } };

	mfo.qua = qt;

	mfo.poi.z += od.final_direction() * da.count() / 1000.0f * 5.0f;


	creator& res = p;
	pipeline& pipe = p;

	
	pipe.clear_render_target(om, { 0.0, 0.0, 0.8f, 1.0 });

	float4x4 pro;
	DirectX::XMStoreFloat4x4(&pro, DirectX::XMMatrixPerspectiveFovLH(3.1415926f / 4.0f, 1024.0f / 768.0f, 0.01f, 1000.0f));

	try {
		pipe.write_constant_buffer(vs, 0, [&, this](void* data, UINT c, UINT i) {
			float4x4* da = static_cast<float4x4*>(data);
			da[0] = mfo;
			da[1] = pro;
		});

		DirectX::XMStoreFloat4x4(&pro, DirectX::XMMatrixInverse(nullptr, mfo.qua));

		pipe.write_constant_buffer(ps, 0, [&, this](void* data, UINT c, UINT ) {
			float4x4* da = static_cast<float4x4*>(data);
			da[0] = pro;
		});
	}
	catch (...) {
		__debugbreak();
	}
	ps.set(res.cast_shader_resource_view(volume_texture[current_view]), 0);
	pipe << ia << vs << ps << om << bs << dss;
	pipe.draw_index(static_cast<UINT>(ind.size()), 0, 0);
	pipe.unbind();
}


Respond UE4_testing::respond(event& c) {
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
			case '+':
			case '=':
				/*
				if (c.key.is_up())
				{
					current_view += 1;
					if (current_view > 19)
						current_view = 0;
					std::cout << current_view << std::endl;
				}
				*/
				break;
			case '-':
				/*
				if (c.key.is_up())
				{
					if (current_view == 0)
						current_view = 19;
					else
						--current_view;
					std::cout << current_view << std::endl;
				}
				*/
				break;
			}
		}
	}
	return PO::Respond::Pass;
}