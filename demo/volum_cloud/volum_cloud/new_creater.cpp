#include "new_creater.h"
#include "../../po/gui/dx11/dx11_vertex.h"
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
			if (filter.x > 0.5)
				filter.x = 0.0;
			else
				filter.x = 1.0;
			break;
		case 'g':
			if (filter.y > 0.5)
				filter.y = 0.0;
			else
				filter.y = 1.0;
			break;
		case 'b':
			if (filter.z > 0.5)
				filter.z = 0.0;
			else
				filter.z = 1.0;
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

void new_creator::init(simple_renderer& s) {
	try {
		creator& res = s;
		pipeline& pipe = s;
		
		tex2 tex = res.create_tex2_unordered_access(DXGI_FORMAT::DXGI_FORMAT_R16G16_FLOAT, 1024, 1024);
		//structed_buffer sb = res.create_struct_buffer_unorder_access(sizeof(float2), 120 * 64);
		
		{
			compute_stage cs;
			auto sha = rs.find(typeid(binary), u"new_creator_cs.cso");
			if (!sha || !sha->able_cast<binary>()) throw 1;
			cs.set(res.create_compute_shader(sha->cast<binary>()));
			cs.set(res.cast_unordered_access_view(tex), 0);

			std::vector<float> IC;
			IC.resize(92);
			std::mt19937 r_mt(233);
			std::uniform_real_distribution<float> nd(-0.3f, 0.3f);

			IC[1] = float(nd(r_mt)) / 2.0;

			for (size_t i = 1; i < 91; ++i)
			{
				IC[i] = IC[i-1] + float(nd(r_mt)) / 2.0;
				std::cout << IC[i] << std::endl;
			}
				

			IC[0] = IC[90];
			IC[91] = IC[1];

			auto  p = res.create_struct_buffer(IC);

			cs.set(res.cast_shader_resource_view(p), 0);
			pipe << cs;
			pipe.dispatch(1024, 1024, 1);
			pipe.unbind();

			/*
			DirectX::ScratchImage SI;
			DirectX::CaptureTexture(res.dev, pipe.ptr, tex.ptr, SI);
			DirectX::SaveToDDSFile(SI.GetImages(), SI.GetImageCount(), SI.GetMetadata(), 0, u"new_out.dds"_wc);
			*/

		}

		{
			/*
			std::vector<float4> vertex;
			vertex.resize(91);

			std::mt19937 r_mt(233);
			std::uniform_real_distribution<float> nd(-0.3f, 0.3f);

			for (size_t i = 0; i < 90; ++i)
			{
				float r = nd(r_mt) / 2.0 + 0.5;
				vertex[i] = float4(
					sin(3.141592653 / 45.0 * i) * abs(r),
					cos(3.141592653 / 45.0 * i) * abs(r),
					0.9,
					1.0
				);
			}*/

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
			float4 filter{1.0, 1.0, 1.0, 1.0};
			ps.set(res.create_pixel_shader(sha->cast<binary>()))
				.set(res.create_sample_state(), 0)
				.set(res.cast_shader_resource_view(tex), 0)
				.set(res.create_constant_buffer(&filter,true), 0);
			res.update_layout(ias, vs);
			
			auto scr = depth_stencil_state::default_scription;
			scr.DepthEnable = false;
			scr.DepthFunc = decltype(scr.DepthFunc)::D3D11_COMPARISON_ALWAYS;
			dss = res.create_depth_stencil_state(scr);
			

			oms.set(res.cast_render_target_view(s.back_buffer), 0);
		}

		
	}
	catch (...) {
		__debugbreak();
	}
}

void new_creator::tick(simple_renderer& s) {
	creator& res = s;
	pipeline& pipe = s;

	pipe.write_constant_buffer(ps, 0, [this](void* data) {
		float4* po = static_cast<float4*>(data);
		*po = filter;
	});
	pipe << ias << vs << ps <<dss << oms;
	pipe.draw_index(6, 0, 0);
	pipe.unbind();

}