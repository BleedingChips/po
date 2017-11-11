#include "dx11_buildin_element.h"
#include "dx11_vertex.h"

using namespace PO::Dx;
using namespace PO::Dx11;

namespace {
	struct position
	{
		const char* operator()() const { return "POSITION"; }
	};
	struct texcoord
	{
		const char* operator()() const { return "TEXCOORD"; }
	};

	struct cube_static_3d_t
	{
		float3 position;
		float2 texturecoord;
	};

	std::array<cube_static_3d_t, 24> cube_static_3d =
	{
		cube_static_3d_t
	{ { -1.0f, -1.0f, 1.0f },{ 1.0f, 1.0f } },
	{ { 1.0, -1.0, 1.0 },{ 0.0, 1.0 } },
	{ { 1.0, 1.0, 1.0 },{ 0.0, 0.0 } },
	{ { -1.0, 1.0, 1.0 },{ 1.0, 0.0 } },

	{ { 1.0, -1.0, 1.0 },{ 1.0, 1.0 } },
	{ { 1.0, -1.0, -1.0 },{ 0.0, 1.0 } },
	{ { 1.0, 1.0, -1.0 },{ 0.0, 0.0 } },
	{ { 1.0, 1.0, 1.0 },{ 1.0, 0.0 } },


	{ { 1.0, -1.0, -1.0 },{ 1.0, 1.0 } },
	{ { -1.0, -1.0, -1.0 },{ 0.0, 1.0 } },
	{ { -1.0, 1.0, -1.0 },{ 0.0, 0.0 } },
	{ { 1.0, 1.0, -1.0 },{ 1.0, 0.0 } },


	{ { -1.0, -1.0, -1.0 },{ 1.0, 1.0 } },
	{ { -1.0, -1.0, 1.0 },{ 0.0, 1.0 } },
	{ { -1.0, 1.0, 1.0 },{ 0.0, 0.0 } },
	{ { -1.0, 1.0, -1.0 },{ 1.0, 0.0 } },

	{ { 1.0, 1.0, -1.0 },{ 1.0, 1.0 } },
	{ { -1.0, 1.0, -1.0 },{ 0.0, 1.0 } },
	{ { -1.0, 1.0, 1.0 },{ 0.0, 0.0 } },
	{ { 1.0, 1.0, 1.0 },{ 1.0, 0.0 } },

	{ { -1.0, -1.0, -1.0 },{ 1.0, 1.0 } },
	{ { 1.0, -1.0, -1.0 },{ 0.0, 1.0 } },
	{ { 1.0, -1.0, 1.0 },{ 0.0, 0.0 } },
	{ { -1.0, -1.0, 1.0 },{ 1.0, 0.0 } }
	};

	std::array<uint16_t, 36> cube_static_3d_index =
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

	struct square_2d_static_t
	{
		float2 position;
		float2 textoord;
	};

	std::array<square_2d_static_t, 4> square_2d_static =
	{
		square_2d_static_t
	{ { -1.0f, -1.0f },{ 0.0f, 1.0f } },
	{ { 1.0, -1.0 },{ 1.0, 1.0 } },
	{ { 1.0, 1.0 },{ 1.0, 0.0 } },
	{ { -1.0, 1.0 },{ 0.0, 0.0 } },
	};

	std::array<uint16_t, 6> square_2d_static_index =
	{
		2,1,0,
		0,3,2
	};
}


namespace PO
{
	namespace Dx11
	{

		void property_local_transfer::update(creator& c, renderer_data& rd)
		{
			shader_storage<float4x4, float4x4> ss{ LocalToWorld, WorldToLocal };
			rd.transfer.create_pod(c, ss);
		}

		void property_viewport_transfer::update(creator& c, renderer_data& rd)
		{
			shader_storage<float4x4, float4x4, float4x4, float4x4, float, float, float, float, float>
				ss{ world_to_eye, eye_to_world, world_to_camera, camera_to_world, near_surface, far_surface,
				view_near_surface, view_far_surface, time
			};
			rd.viewport.create_pod(c, ss);
		}

		geometry_screen::geometry_screen(creator& c) : geometry_resource(c, 
			layout_type<buffer_layout<syntax<position, 0, float2>, syntax<texcoord, 0, float2>>>{}
			)
		{
			index.create_index(c, square_2d_static_index);
			vertex.create_vertex(c, square_2d_static);
		}
		void geometry_screen::apply(stage_context& sc)
		{
			geometry_resource::apply(sc);
			sc << index_call{ static_cast<uint32_t>(square_2d_static_index.size()), 0, 0 };
			sc << index << vertex[0];
		}

		placement_direct::placement_direct(creator& c) : placement_resource(
			c,
			u"build_in_placement_direct_vs.cso"
		) {}



		geometry_cube::geometry_cube(creator& c) : geometry_resource(
			c,
			layout_type<buffer_layout<syntax<position, 0, float3>, syntax<texcoord, 0, float2>>>{}
		)
		{
			index.create_index(c, cube_static_3d_index);
			vertex.create_vertex(c, cube_static_3d);
		}

		void geometry_cube::apply(stage_context& sc) {
			geometry_resource::apply(sc);
			sc << index << vertex[0] << index_call{static_cast<UINT>(cube_static_3d_index.size()), 0, 0}; 
		}

		placement_static_viewport_static::placement_static_viewport_static(creator& c) : placement_resource(c, u"build_in_placement_view_static_vs.cso") {}

		const element_requirement& placement_static_viewport_static::requirement() const
		{
			return make_element_requirement(
				[](stage_context& sc, property_wrapper_t<property_local_transfer>& rd) {
				sc.VS() << rd.transfer[1];
			},
				[](stage_context& sc, property_wrapper_t<property_viewport_transfer>& rd) {
				sc.VS() << rd.viewport[0];
			}
			);
		}

		material_tex2_viewer::material_tex2_viewer(creator& c) :
			material_resource(c, u"build_in_material_tex2_view.cso")
		{}

		const element_requirement& material_tex2_viewer::requirement() const
		{
			return make_element_requirement(
				[](stage_context& sc, property_wrapper_t<property_tex2>& rd) {
				sc.PS() << rd.srv[0] << rd.ss[0];
			}
			);
		}

		material_testing::material_testing(creator& c) : material_resource(c, u"buildin_material_testing_ps.cso")
		{

		}

		/*

		property_transfer::property_transfer() : property_interface(typeid(property_transfer)) {}
		void property_transfer::set_transfer(creator& c, const float4x4& local, const float4x4& world)
		{
			shader_storage<float4x4, float4x4> as{ local, world };
			if (!m_cb)
				m_cb = c.create_buffer_constant(&as, true);
			else {
				update_function = [as, this](pipeline& p) {
					p.write_buffer_constant(m_cb, [&, this](void* data) {
						*static_cast<shader_storage<float4x4, float4x4>*>(data) = as;
					});
				};
			}
		}

		property_transfer_instance::property_transfer_instance() :property_interface(typeid(property_transfer_instance)) {}
		void property_transfer_instance::set_transfer(creator& c, const std::vector<std::pair<PO::Dx::float4x4, PO::Dx::float4x4>>& v)
		{
			if (!transfer_srv || v.size() > buffer_size)
			{
				transfer_sb = c.create_buffer_structured_unorder_access(v);
				transfer_srv = c.cast_shader_resource_view(transfer_sb);
				transfer_uav = c.cast_unordered_access_view(transfer_sb);
				count = static_cast<
				>(v.size());
				buffer_size = count;
			}
			else {
				update_function = [v, this](pipeline& p) {
					if (!p.write_buffer_structured(transfer_sb, [&, this](void* da) {
						auto p = static_cast<std::pair<PO::Dx::float4x4, PO::Dx::float4x4>*>(da);
						for (auto& io : v)
							*(p++) = io;
					})) throw 1;
				};
			}
		}

		property_screen_static::property_screen_static() : property_interface(typeid(property_screen_static)) {}
		void property_screen_static::set(creator& c, const float4x4& projection, float near_plane, float far_plane, float xy_rate, float2 viewport_left_top, float2 viewport_right_button, float2 viewport_near_far)
		{
			using type = shader_storage<float4x4, float, float, float, float2, float2, float2>;
			type temporary{ projection, near_plane, far_plane, xy_rate,viewport_left_top,viewport_right_button, viewport_near_far };
			screen_cb = c.create_buffer_constant(&temporary);
		}

		property_screen::property_screen() : property_interface(typeid(property_screen)) {}
		void property_screen::set(creator& c, const float4x4& view, const float4x4& world_to_screen, const PO::Dx::float4x4& screen_to_world, float time)
		{
			using type = shader_storage<float4x4, float4x4, float4x4, float3, float>;
			type temporary{ view, world_to_screen, screen_to_world, float3{ view._41, view._42, view._43 }, time };
			if (!screen_cb)
				screen_cb = c.create_buffer_constant(&temporary, true);
			else {
				update_function = [temporary, this](pipeline& p) {
					if (!p.write_buffer_constant(screen_cb, [&, this](void* data) {
						*static_cast<type*>(data) = temporary;
					})) throw 1;
				};
			}
		}

		property_tex2::property_tex2() : property_interface(typeid(property_tex2)) {}
		property_tex2::property_tex2(std::type_index ti) : property_interface(ti) {}
		void property_tex2::set_tex2(creator& c, const tex2& target, sample_state::description des)
		{
			m_srv = c.cast_shader_resource_view(target);
			m_ss = c.create_sample_state(des);
		}

		placement_direct::placement_direct() : placement_interface(typeid(placement_direct)) {}
		void placement_direct::init(creator& c)
		{
			if (!load_vs(u"build_in_placement_direct_vs.cso", c)) throw 1;
		}

		placement_screen_static::placement_screen_static() : placement_interface(typeid(placement_screen_static)) {}
		auto placement_screen_static::acceptance() const -> const acceptance_t&
		{
			return make_acceptance<property_screen_static, property_transfer>{};
		}
			void placement_screen_static::init(creator& c)
		{
			load_vs(u"build_in_placement_screen_static_vs.cso", c);
		}
		bool placement_screen_static::update(property_interface& pi, pipeline& p)
		{
			return pi.cast([this](property_screen_static& sp) {
				stage_vs << sp.cb()[0];
			}) || pi.cast([this](property_transfer& sp) {
				stage_vs << sp.cb()[1];
			});
		}

		placement_view_static::placement_view_static() : placement_interface(typeid(placement_view_static)) {}
		auto placement_view_static::acceptance() const -> const acceptance_t&
		{
			return make_acceptance<property_screen, property_transfer>{};
		}
			void placement_view_static::init(creator& c)
		{
			load_vs(u"build_in_placement_view_static_vs.cso", c);
		}
		bool placement_view_static::update(property_interface& pi, pipeline& p)
		{
			return pi.cast([this](property_screen& sp) {
				stage_vs << sp.cb()[0];
			}) || pi.cast([this](property_transfer& sp) {
				stage_vs << sp.cb()[1];
			});
		}

		geometry_square_static::geometry_square_static() : geometry_interface(typeid(geometry_square_static)) {}
		void geometry_square_static::init(creator& c)
		{
			stage_ia << c.create_vertex(square_2d_static, layout_type<syntax<::position, 0, float2>, syntax<::texcoord, 0, float2>>{})[0]
				<< c.create_index(square_2d_static_index);
			decltype(stage_rs)::description des = decltype(stage_rs)::default_description;
			des.FrontCounterClockwise = TRUE;
			stage_rs = c.create_raterizer_state(des);
		}
		void geometry_square_static::draw(pipeline& p)
		{
			p.draw_index(6, 0, 0);
		}
		geometry_square_static::geometry_square_static(std::type_index ti) : geometry_interface(ti) {}

		geometry_cube_static::geometry_cube_static() : geometry_interface(typeid(geometry_cube_static)) {}
		void geometry_cube_static::init(creator& c)
		{
			stage_ia << c.create_vertex(cube_static_3d, layout_type<syntax<::position, 0, float3>, syntax<::texcoord, 0, float2>>{})[0]
				<< c.create_index(cube_static_3d_index);
		}
		void geometry_cube_static::draw(pipeline& p)
		{
			p.draw_index(36, 0, 0);
		}

		property_gbuffer::property_gbuffer() :property_interface(typeid(property_gbuffer)) {}
		void property_gbuffer::set_gbuffer(creator& c, const tex2& color, const tex2& liner_z, const sample_state::description& des)
		{
			color_srv = c.cast_shader_resource_view(color);
			linearization_z_srv = c.cast_shader_resource_view(liner_z);
			m_ss = c.create_sample_state(des);
		}

		material_merge_gbuffer::material_merge_gbuffer() : defer_material_interface(typeid(material_merge_gbuffer), render_order::Post) {}
		bool material_merge_gbuffer::update(property_interface& pi, pipeline& p)
		{
			return pi.cast([this](property_gbuffer& gb) {
				stage_ps << gb.color()[0] << gb.ss()[0];
			});
		}
		auto material_merge_gbuffer::acceptance() const -> const acceptance_t&
		{
			return make_acceptance<property_gbuffer>{};
		}
			void material_merge_gbuffer::init(creator& c)
		{
			if (!load_ps(u"build_in_material_merge_gbuffer_ps.cso", c)) throw 1;
		}

		material_defer_render_texcoord::material_defer_render_texcoord() : defer_material_interface(typeid(material_defer_render_texcoord), render_order::Defer) {}
		void material_defer_render_texcoord::init(creator& c)
		{
			if (!load_ps(u"build_in_material_defer_render_texcoord_ps.cso", c)) throw 1;
		}

		material_defer_tex2::material_defer_tex2() : defer_material_interface(typeid(material_defer_tex2), render_order::Defer) {}
		auto material_defer_tex2::acceptance() const -> const acceptance_t&
		{
			return make_acceptance<property_tex2>{};
		}
			bool material_defer_tex2::update(property_interface& pi, pipeline& p)
		{
			return pi.cast([this](property_tex2& sop) {
				stage_ps << sop.srv()[0] << sop.ss()[0];
			});
		}
		void material_defer_tex2::init(creator& c)
		{
			if (!load_ps(u"build_in_material_defer_tex2_ps.cso", c)) throw 1;
		}

		property_linearize_z::property_linearize_z() : property_interface(typeid(property_linearize_z)) {}
		compute_linearize_z::compute_linearize_z() : compute_interface(typeid(compute_linearize_z)) {}

		bool compute_linearize_z::update(property_interface& pi, pipeline& p)
		{
			return pi.cast([this](property_screen_static& pss) {
				stage_cs << pss.cb()[0];
			}) || pi.cast([this](property_linearize_z& plz) {
				stage_cs << plz.srv()[0];
				stage_cs << plz.uav()[0];
				size = plz.size();
			});
		}
		void compute_linearize_z::dispath(pipeline& p)
		{
			p.dispatch(size.x, size.y, 1);
		}
		auto compute_linearize_z::acceptance() const -> const acceptance_t&
		{
			return make_acceptance<property_screen_static, property_linearize_z>{};
		}
			void compute_linearize_z::init(creator& c)
		{
			if (!load_cs(u"build_in_compute_linearize_z_cs.cso", c)) throw 1;
		}
		*/
	}
}