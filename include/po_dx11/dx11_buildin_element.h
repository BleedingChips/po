#pragma once
#include "dx11_element.h"
#include <string>
namespace PO
{
	namespace Dx11
	{
		using namespace PO::Dx;
		
		class property_local_transfer : public property_resource
		{
			float4x4 local_to_world;
			float4x4 world_to_local;
			bool need_update = false;
			constant_buffer transfer;
		public:
			struct renderer_data
			{
				constant_buffer transfer;
			};
			void set_local_to_world(const float4x4& l_w, const float4x4& w_l) { local_to_world = l_w; world_to_local = w_l; need_update = true; }
			void update(renderer_data& rd, stage_context& sc);
			void push(property_local_transfer& pt, creator& c);
		};

		class property_viewport_transfer
		{
			float4x4 world_to_eye;
			float4x4 eye_to_world;
			float4x4 world_to_camera;
			float4x4 camera_to_world;
			float near_surface, far_surface, view_near_surface, view_far_surface;
			float time = 0.0;
		public:
			struct renderer_data
			{
				constant_buffer viewport;
			};
			void set_world_eye(const float4x4& mat, const float4x4& ins_mat) { world_to_eye = mat; eye_to_world = ins_mat; }
			void set_world_camera(const float4x4& mat, const float4x4& ins_mat) { world_to_camera = mat; camera_to_world = ins_mat; }
			void set_surface(float ns, float fs, float vns, float vfs) { near_surface = ns; far_surface = fs; view_near_surface = vns; view_far_surface = vfs; }
			void set_time(float f) { time = f; }
			void update(renderer_data& rd, stage_context& sc);
			void push(property_viewport_transfer& pt, creator& c);
		};

		class geometry_screen
		{
			primitive_topology ele;
			index_buffer index;
			vertex_buffer vertex;
			layout_view lv;
			raterizer_state rs;
		public:
			geometry_screen(creator& c);
			void geometry_apply(stage_context& sc);
			bool geometry_update(stage_context& sc, property_interface& pi);
			const std::set<std::type_index>& geometry_requirement() const;
			const layout_view& geometry_layout_view() { return lv; }
		};

		class placement_direct
		{
		public:
			const std::u16string& placement_shader_patch_vs();
			placement_direct(creator& c) {}
			void placement_apply(stage_context&);
			bool placement_update(stage_context& sc, property_interface& pi);
			const std::set<std::type_index>& placement_requirement() const;
		};

		class material_testing: public material_default
		{
		public:
			const std::u16string& material_shader_patch_ps();
			material_testing(creator& c) {}
			void material_apply(stage_context&);
			bool material_update(stage_context& sc, property_interface& pi);
			const std::set<std::type_index>& material_requirement() const;
		};

		class geometry_cube
		{
			primitive_topology ele;
			index_buffer index;
			vertex_buffer vertex;
			layout_view lv;
			raterizer_state rs;
		public:
			geometry_cube(creator& c);
			void geometry_apply(stage_context& sc);
			bool geometry_update(stage_context& sc, property_interface& pi);
			const std::set<std::type_index>& geometry_requirement() const;
			const layout_view& geometry_layout_view() { return lv; }
		};

		class placement_static_viewport_static
		{
		public:
			placement_static_viewport_static(creator& c) {}

			const std::u16string& placement_shader_patch_vs();
			void placement_apply(stage_context&);
			bool placement_update(stage_context& sc, property_interface& pi);
			const std::set<std::type_index>& placement_requirement() const;
		};

		/*
		class property_transfer : public property_interface
		{
			constant_buffer m_cb;
		public:
			const constant_buffer& cb() const { return m_cb; }
			void set_transfer(creator& c, const float4x4& local, const float4x4& world);
			property_transfer();
		};

		class property_transfer_instance : public property_interface
		{
			structured_buffer transfer_sb;
			shader_resource_view transfer_srv;
			unordered_access_view transfer_uav;
			size_t buffer_size = 0;
			UINT count = 0;
		public:
			UINT instance_count() const { return count; }
			void set_transfer(creator& c, const std::vector<std::pair<float4x4, float4x4>>& v);
			const shader_resource_view& srv() const { return transfer_srv; }
			const unordered_access_view& uav() const { return transfer_uav; }
			property_transfer_instance();
		};

		class property_screen_static : public property_interface
		{
			constant_buffer screen_cb;
		public:
			const constant_buffer& cb() const { return screen_cb; }
			void set(creator& c, const float4x4& projection, float near_plane, float far_plane, float xy_rate, float2 viewport_left_top, float2 viewport_right_button, float2 viewport_near_far);
			property_screen_static();
		};

		class property_screen : public property_interface
		{
			constant_buffer screen_cb;
		public:
			void set(creator& c, const float4x4& view, const float4x4& world_to_screen, const PO::Dx::float4x4& screen_to_world, float time);
			const constant_buffer& cb() const { return screen_cb; }
			property_screen();
		};

		class property_tex2 : public property_interface
		{
			shader_resource_view m_srv;
			sample_state m_ss;
		public:
			void set_tex2(creator& c, const tex2& target, sample_state::description des = sample_state::default_description);
			const shader_resource_view& srv() const { return m_srv; }
			const sample_state& ss() const { return m_ss; }
			property_tex2();
			property_tex2(std::type_index ti);
		};

		template<size_t i> class property_tex2_index : public property_tex2
		{
		public:
			property_tex2_index() : property_tex2(typeid(tex2_index_property)) {}
		};

		class placement_direct: public placement_interface
		{
		public:
			placement_direct();
			virtual void init(creator&) override;
		};

		class placement_screen_static : public placement_interface
		{
		public:
			placement_screen_static();
			virtual bool update(property_interface&, pipeline&) override;
			virtual auto acceptance() const -> const acceptance_t& override;
			virtual void init(creator&) override;
		};

		class placement_view_static : public placement_interface
		{
		public:
			placement_view_static();

			virtual bool update(property_interface&, pipeline&) override;
			virtual auto acceptance() const -> const acceptance_t& override;
			virtual void init(creator&) override;
		};

		class geometry_square_static : public geometry_interface
		{
		public:
			geometry_square_static();
			geometry_square_static(std::type_index);
			virtual void init(creator&) override;
			virtual void draw(pipeline&) override;
		};

		class geometry_cube_static: public geometry_interface
		{
		public:
			geometry_cube_static();
			virtual void init(creator&) override;
			virtual void draw(pipeline&) override;
		};

		class property_gbuffer : public property_interface
		{
			shader_resource_view color_srv;
			shader_resource_view linearization_z_srv;
			sample_state m_ss;
		public:
			void set_gbuffer(creator& c, const tex2& color, const tex2& liner_z, const sample_state::description& des = sample_state::default_description);
			const shader_resource_view& color() const { return color_srv; }
			const shader_resource_view& linearization_z() const { return linearization_z_srv; }
			const sample_state& ss() const { return m_ss; }
			property_gbuffer();
		};

		class material_merge_gbuffer : public defer_material_interface
		{
		public:
			material_merge_gbuffer();
			virtual bool update(property_interface&, pipeline&) override;
			virtual auto acceptance() const -> const acceptance_t& override;
			virtual void init(creator&) override;
		};

		class material_defer_render_texcoord : public defer_material_interface
		{
		public:
			material_defer_render_texcoord();
			virtual void init(creator&) override;
		};

		class material_defer_tex2 : public defer_material_interface
		{
		public:
			material_defer_tex2();
			virtual bool update(property_interface&, pipeline&) override;
			virtual auto acceptance() const -> const acceptance_t& override;
			virtual void init(creator&) override;
		};

		class property_linearize_z : public property_interface
		{
			shader_resource_view m_srv;
			unordered_access_view m_uav;
			uint32_t2 m_size;
		public:
			void set_depth_stencil_texture(creator& c, const tex2& t, const tex2& t2) { m_srv = c.cast_shader_resource_view(t); m_size = t.size(); m_uav = c.cast_unordered_access_view(t2); }
			const shader_resource_view& srv() const { return m_srv; }
			const unordered_access_view& uav() const { return m_uav; }
			const uint32_t2& size() const { return m_size; }
			property_linearize_z();
		};

		class compute_linearize_z : public compute_interface
		{
			uint32_t2 size;
		public:
			compute_linearize_z();
			virtual bool update(property_interface& pi, pipeline&) override;
			virtual void dispath(pipeline& p) override ;
			virtual auto acceptance() const -> const acceptance_t& override;
			virtual void init(creator&) override;
		};
		*/

	}
}