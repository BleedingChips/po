#include "new_creater.h"
#include "po_dx11/dx11_vertex.h"
#include <random>
new_creator::new_creator() {
	rs.pre_load(
		typeid(binary),
		{
			u"new_creator_cs.cso",
			u"new_creator_ps.cso",
			u"new_creator_vs.cso"
		}
	);
	/*
	p.auto_bind_init(&new_creator::init, this);
	p.auto_bind_tick(&new_creator::tick, this);
	p.auto_bind_respond(&new_creator::respond, this);
	*/
}

adapter_map new_creator::mapping(self& s) {
	s.auto_bind_respond(&new_creator::respond, this);
	return {
		make_member_adapter<simple_renderer>(this, &new_creator::init, &new_creator::tick)
	};
}

Respond new_creator::respond(event& e) {
	if (e.is_key() && e.key.is_up())
	{
		switch (e.key.get_asc())
		{
		case 'r':
			if (con.filter.x > 0.5)
				con.filter.x = 0.0;
			else
				con.filter.x = 1.0;
			break;
		case 'g':
			if (con.filter.y > 0.5)
				con.filter.y = 0.0;
			else
				con.filter.y = 1.0;
			break;
		case 'b':
			if (con.filter.z > 0.5)
				con.filter.z = 0.0;
			else
				con.filter.z = 1.0;
			break;
		
		default:
			break;
		}
	}
	else if (e.is_key() && e.key.is_down())
	{
		switch (e.key.get_asc())
		{
		case 'q':
			con.layout = con.layout - 1.0 / max_layer;
			if (con.layout < 0.0)
				con.layout = 0.0;
			std::cout << "layer: " << con.layout << std::endl;
			break;
		case 'e':
			con.layout = con.layout + 1.0 / max_layer;
			if (con.layout > 1.0)
				con.layout = 1.0;
			std::cout << "layer: " << con.layout << std::endl;
			break;
		default:
			break;
		}
	}
	return Respond::Pass;
}

struct position
{
	const char* operator()() { return "POSITION"; }
};

struct texcoord
{
	const char* operator()() { return "TEXCOORD"; }
};

struct diffuse
{
	const char* operator()() { return "DIFFUSE"; }
};

struct cube_ver
{
	float3 poi;
	float3 col;
	using type = PO::Dx11::layout_type<PO::Dx11::syntax<position, 0, float3>, PO::Dx11::syntax<diffuse, 0, float3>>;
};


void new_creator::init(simple_renderer& s) {
	using namespace DirectX;
	try {
		creator& res = s;
		pipeline& pipe = s;
		
		volume = res.create_tex3_unordered_access(DXGI_FORMAT::DXGI_FORMAT_R16_FLOAT, 256, 256, 256);
		//structed_buffer sb = res.create_struct_buffer_unorder_access(sizeof(float2), 120 * 64);
		
		//generator 3d texture
		{
			compute_stage cs;
			auto sha = rs.find(typeid(binary), u"new_creator_cs.cso");
			if (!sha || !sha->able_cast<binary>()) throw 1; 
			cs.set(res.create_compute_shader(sha->cast<binary>()));
			cs.set(res.cast_unordered_access_view(volume), 0);
			uint32_t3 da = { 256, 256, 256 };
			cs << res.create_constant_buffer(&da)[0];

			std::vector<float3> IC;
			IC.resize(200);

			std::mt19937 r_mt(233);
			std::normal_distribution<float> nd(0.5f, 0.16f);

			for (size_t i = 0; i < 100; ++i)
			{
				IC[i] = float3(nd(r_mt), nd(r_mt), nd(r_mt));// +IC[i - 1];
				std::cout << IC[i] << std::endl;
			}

			auto  p = res.create_struct_buffer(IC);

			cs.set(res.cast_shader_resource_view(p), 0);
			pipe << cs;
			pipe.dispatch(256 / 32, 256 / 32, 256);
			pipe.unbind();

			/*
			DirectX::ScratchImage SI;
			DirectX::CaptureTexture(res.dev, pipe.ptr, tex.ptr, SI);
			DirectX::SaveToDDSFile(SI.GetImages(), SI.GetImageCount(), SI.GetMetadata(), 0, u"new_out.dds"_wc);
			*/

		}

		//show texture
		{

			struct vertex_type
			{
				float4 poi;
				float2 coo;
				using type = layout_type<syntax<position, 0, float4>, syntax<texcoord, 0, float2>>;
			};

			std::vector<vertex_type> po = {
				{ {-1.0, 1.0, 0.9f, 1.0}, {0.0, 0.0} },
				{ { -1.0, -1.0, 0.9f, 1.0 },{ 0.0, 1.0 } },
				{ { 1.0, -1.0, 0.9f, 1.0 },{ 1.0, 1.0 } },
				{ { 1.0, 1.0, 0.9f, 1.0 },{ 1.0, 0.0 } }
			};

			for (size_t i = 0; i < 4; ++i)
			{
				po[i].poi.x *= 768.0 / 1024.0;
			}

			std::vector<uint16_t> draw_index = {
				0, 3, 2, 2, 1, 0
			};

			ias.set(res.create_vertex(po, typename decltype(po)::value_type::type{}), 0)
				.set(res.create_index(draw_index));
			//ias.primitive = decltype(ias.primitive)::D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
			auto sha = rs.find(typeid(binary), u"new_creater_vs.cso");
			if (!sha || !sha->able_cast<binary>()) throw 1;
			vs.set(res.create_vertex_shader(sha->cast<binary>()));
			sha = rs.find(typeid(binary), u"new_creater_ps.cso");
			if (!sha || !sha->able_cast<binary>()) throw 1;
			ps.set(res.create_pixel_shader(sha->cast<binary>()))
				.set(res.create_sample_state(), 0)
				.set(res.cast_shader_resource_view(volume), 0)
				.set(res.create_constant_buffer(&con,true), 0);
			res.update_layout(ias, vs);
			
			auto scr = depth_stencil_state::default_dscription;
			scr.DepthEnable = false;
			scr.DepthFunc = decltype(scr.DepthFunc)::D3D11_COMPARISON_ALWAYS;
			dss = res.create_depth_stencil_state(scr);
			

			oms.set(res.cast_render_target_view(s.back_buffer), 0);
		}

		{

		}

		
	}
	catch (HRESULT r) {
		HRESULT re = r;
		__debugbreak();
	}
}

void new_creator::tick(simple_renderer& s) {
	creator& res = s;
	pipeline& pipe = s;

	//ps<<res.cast_shader_resource_view(texture, )

	pipe.write_constant_buffer(ps, 0, [this](void* data) {
		control_input* po = static_cast<control_input*>(data);
		*po = con;
	});
	pipe << ias << vs << ps <<dss;
	pipe.draw_index(6, 0, 0);
	pipe.unbind();

}