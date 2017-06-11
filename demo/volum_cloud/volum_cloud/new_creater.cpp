#include "new_creater.h"
#include <random>
new_creator::new_creator(peek<Dx11_ticker> p) {
	rs.pre_load(
		typeid(binary),
		{
			u"new_creator_cs.cso"
		}
	);
	p.self.auto_bind_init(&new_creator::init, this);
	p.self.auto_bind_tick(&new_creator::tick, this);
}

struct position
{
	const char* operator()() { return "POSITION"; }
};

void new_creator::init(self_depute<Dx11_ticker> s) {
	try {
		auto& res = s.rt.res;
		auto& pipe = s.rt.pipe;
		
		tex2 tex = res.create_tex2_unordered_access(DXGI_FORMAT::DXGI_FORMAT_R16G16_FLOAT, 1024, 1024);
		//structed_buffer sb = res.create_struct_buffer_unorder_access(sizeof(float2), 120 * 64);
		/*
		{
			compute_stage cs;
			auto sha = rs.find(typeid(binary), u"new_creator_cs.cso");
			if (!sha || !sha->able_cast<binary>()) throw 1;
			cs.set(res.create_compute_shader(sha->cast<binary>()));
			cs.set(res.cast_unordered_access_view(tex), 0);

			std::array<float, 92> IC;

			std::mt19937 r_mt(233);
			std::uniform_real_distribution<float> nd(-0.3f, 0.3f);

			for (size_t i = 1; i < 91; ++i)
				IC[i] = float(nd(r_mt));
			IC[0] = IC[90];
			IC[91] = IC[1];

			cs.set(res.create_constant_buffer(&IC), 0);
			pipe << cs;
			pipe.dispatch(1024, 1024, 1);
			pipe.unbind();

			DirectX::ScratchImage SI;
			DirectX::CaptureTexture(res.dev, pipe.ptr, tex.ptr, SI);
			DirectX::SaveToDDSFile(SI.GetImages(), SI.GetImageCount(), SI.GetMetadata(), 0, u"new_out.dds"_wc);

		}
		*/

		{

			std::vector<float4> vertex;
			vertex.resize(90);

			std::mt19937 r_mt(233);
			std::uniform_real_distribution<float> nd(-0.3f, 0.3f);

			for (size_t i = 0; i < 90; ++i)
			{
				float r = nd(r_mt) / 4.0 + 0.5;
				vertex[i] = float4(
					sin(3.141592653 / 45.0 * i) * abs(r),
					cos(3.141592653 / 45.0 * i) * abs(r),
					0.9,
					1.0
				);
			}

			ias.set(res.create_vertex(vertex, PO::Dx11::layout_type<PO::Dx11::syntax<position, 0, float4>>{}), 0);
			ias.primitive = decltype(ias.primitive)::D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
			auto sha = rs.find(typeid(binary), u"default_vs.cso");
			if (!sha || !sha->able_cast<binary>()) throw 1;
			vs.set(res.create_vertex_shader(sha->cast<binary>()));
			sha = rs.find(typeid(binary), u"default_ps.cso");
			if (!sha || !sha->able_cast<binary>()) throw 1;
			ps.set(res.create_pixel_shader(sha->cast<binary>()));
			res.update_layout(ias, vs);

			auto scr = depth_stencil_state::default_scription;
			scr.DepthEnable = false;
			scr.DepthFunc = decltype(scr.DepthFunc)::D3D11_COMPARISON_ALWAYS;
			dss = res.create_depth_stencil_state(scr);
		}

		
	}
	catch (...) {
		__debugbreak();
	}
}

void new_creator::tick(self_depute<Dx11_ticker> s) {
	auto& res = s.rt.res;
	auto& pipe = s.rt.pipe;


	pipe << ias << vs << ps <<dss;
	pipe.draw_vertex(90, 0);
	pipe.unbind();

}