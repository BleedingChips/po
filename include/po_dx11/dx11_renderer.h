#pragma once
#include "../po/renderer.h"
#include "dx11_frame.h"
#include "../po_dx/dx_type.h"
#include "dx11_element.h"
#include "dx11_buildin_element.h"
namespace PO
{
	namespace Dx11
	{
		struct sub_viewport_perspective
		{
			float4 projection_property;
			float4x4 projection;
			float4x4 eye;
			viewport view;

			sub_viewport_perspective(float2 view_size, float2 view_left_top = float2{ 0.0, 0.0 }, float angle = 3.1415926f / 4.0f, float2 far_near_plane = float2{ 0.0001f, 10000.0f }, float2 avalible_depth = float2{ 0.0f, 1.0f });
			sub_viewport_perspective(float4 view_border, float angle = 3.1415926f / 4.0f, float2 far_near_plane = float2{ 0.0001f, 10000.0f }, float2 avalible_depth = float2{ 0.0f, 1.0f })
				: sub_viewport_perspective(float2{ view_border.z - view_border.x, view_border.w - view_border.y }, float2{ view_border.x,view_border.y }, angle, far_near_plane, avalible_depth) {}
			sub_viewport_perspective(tex2 tex, float2 view_left_top = float2{ 0.0, 0.0 }, float angle = 3.1415926f / 4.0f, float2 far_near_plane = float2{ 0.0001f, 10000.0f }, float2 avalible_depth = float2{ 0.0f, 1.0f }) :
				sub_viewport_perspective(tex.size_f(), view_left_top, angle, far_near_plane, avalible_depth) {}
			sub_viewport_perspective(const sub_viewport_perspective&) = default;
			sub_viewport_perspective& operator=(const sub_viewport_perspective&) = default;
			operator const viewport&() const { return view; }
		};
	}
}

PO::Dx11::property_proxy_map& operator>>(PO::Dx11::property_proxy_map& ppm, const PO::Dx11::sub_viewport_perspective& svp);

namespace PO
{
	namespace Dx11
	{

		struct sub_viewport_parallel
		{
			float4 projection_property;
			float4x4 projection;
			viewport view;

			sub_viewport_parallel(float2 d) {}
		};

		struct pipeline_compute_default : public pipeline_interface
		{
			void execute_implement(stage_context& sc, element_renderer_storage& storage, Tool::stack_list<Implement::property_map>* ptr) override;
			pipeline_compute_default();
		};

		using namespace PO::Dx;
		struct renderer_default : creator
		{
			tex2 back_buffer;
			output_merge_stage om;
			stage_context context;
			sub_viewport_parallel main_view;
			viewport view;

			element_logic_storage els;
			element_swap_block esb;
			element_renderer_storage ers;
			stage_instance ins;

			operator stage_context& () { return context; }

			//element_instance instance;

			template<typename T, typename ...AT> decltype(auto) create_if_no_exist(AT&& ...at) { return context.create_if_no_exist<T>(std::forward<AT>(at)...); }

			void clear_back_buffer(const std::array<float, 4>& bc) { context.clear_render_target(om, bc); }
			renderer_default(value_table& vt);

			void pre_tick(duration da);
			void pos_tick(duration da);

			renderer_default& operator << (const element& el) { els << el; return *this; }

		};

		
		class property_gbuffer_default
		{
			shader_resource_view srv;
			shader_resource_view linear_z;
			sample_state ss;
		public:
			struct renderer_data
			{
				shader_resource_view srv;
				shader_resource_view linear_z;
				sample_state ss;
			};
			void push(property_gbuffer_default& pgb, creator& sc) { pgb.srv = srv;  pgb.ss = ss;  pgb.linear_z = linear_z; }
			void update(renderer_data& rd, creator& c) { rd.srv = srv; rd.ss = ss; rd.linear_z = linear_z; }
			void set_gbuffer(creator& c, const tex2& t, const tex2& linear) { srv = c.cast_shader_resource_view(t); ss = c.create_sample_state(); linear_z = c.cast_shader_resource_view(linear); }
		};


		class material_merga_gbuffer_default : public material_default
		{
		public:
			static const char16_t* material_shader_patch_ps();
			material_merga_gbuffer_default(creator& c) {}
			void material_apply(stage_context&);
			bool material_update(stage_context& sc, property_interface& pi);
			const std::set<std::type_index>& material_requirement() const;
		};


		struct pipeline_opaque_default : public pipeline_interface
		{
			tex2 g_buffer;
			render_target_view rtv;
			tex2 depth;
			depth_stencil_view dsv;
			depth_stencil_state dss;
			blend_state bs;
			output_merge_stage om;
			void execute_implement(stage_context& sc, element_renderer_storage& storage, Tool::stack_list<Implement::property_map>* ptr) override;
			void set(creator& c, uint32_t2 size);
			pipeline_opaque_default();
		};

		class material_opaque_testing : public material_default
		{
		public:
			static const char16_t* material_shader_patch_ps();
			static std::type_index pipeline_id();
			material_opaque_testing(creator&) {}
		};


		class material_qpaque_texture_coord : public material_default
		{
		public:
			static const char16_t* material_shader_patch_ps() { return u"build_in_material_defer_render_texcoord_ps.cso"; }
			static std::type_index pipeline_id() { return typeid(pipeline_opaque_default); }
			material_qpaque_texture_coord(creator&) {}
		};

		class property_linearize_z
		{
			shader_resource_view input_depth;
			unordered_access_view output_depth;
			uint32_t2 size;
		public:
			struct renderer_data
			{
				shader_resource_view input_depth;
				unordered_access_view output_depth;
				uint32_t2 size;
			};
			void set_taregt(creator& c, const tex2& input, const tex2& output) {
				input_depth = c.cast_shader_resource_view(input);
				output_depth = c.cast_unordered_access_view(output);
				size = output.size();
			}
			void push(property_linearize_z& plz, creator& c) { plz = *this; }
			void update(renderer_data& rd, stage_context& sc)
			{
				rd.input_depth = input_depth;
				rd.output_depth = output_depth;
				rd.size = size;
			}
		};

		class compute_linearize_z
		{
		public:
			compute_linearize_z(creator&) {}
			static const char16_t* compute_shader_patch_cs() { return u"build_in_compute_linearize_z_cs.cso"; }
			static const std::set<std::type_index>& compute_requirement() { return make_property_info_set<property_linearize_z, property_viewport_transfer>{}; }
			static void compute_apply(stage_context& sc) {};
			static bool compute_update(stage_context& sc, property_interface& pi)
			{
				return pi.cast([&](property_linearize_z::renderer_data& plz) {
					sc.CS() << plz.input_depth[0] << plz.output_depth[0];
					sc << dispatch_call{ plz.size.x, plz.size.y , 1};
				}) || pi.cast([&](property_viewport_transfer::renderer_data& pvt) {
					sc.CS() << pvt.viewport[0];
				});
			}
		};

		class pipeline_transparent_default : public pipeline_interface
		{
			blend_state bs;
			depth_stencil_state dss;
		public:
			void execute_implement(stage_context& sc, element_renderer_storage& storage, Tool::stack_list<Implement::property_map>* ptr) override;
			void set(creator& c);
			pipeline_transparent_default();
		};

		class material_transparent_testing : public material_default
		{
		public:
			static const char16_t* material_shader_patch_ps();
			static std::type_index pipeline_id();
			material_transparent_testing(creator&) {}
		};

		struct defer_renderer_default : creator
		{

			sub_viewport_perspective view;

			tex2 back_buffer;

			tex2 linear_z_buffer;
			output_merge_stage om;
			stage_context context;

			element_logic_storage els;
			element_swap_block esb;
			element_renderer_storage ers;
			stage_instance ins;
			depth_stencil_state dss;


			property_proxy_map mapping;
			property_proxy_map post_mapping;
			element merga;
			element linear_z;

			pipeline_compute_default compute_pipeline;
			pipeline_opaque_default opaque_pipeline;
			pipeline_transparent_default transparent_pipeline;
			operator stage_context& () { return context; }

			//element_instance instance;

			template<typename T, typename ...AT> decltype(auto) create_if_no_exist(AT&& ...at) { return context.create_if_no_exist<T>(std::forward<AT>(at)...); }

			void clear_back_buffer(const std::array<float, 4>& bc) { context.clear_render_target(om, bc); }
			defer_renderer_default(value_table& vt);

			void pre_tick(duration da);
			void pos_tick(duration da);

			defer_renderer_default& operator << (const element& el) { els << el; return *this; }
		};




		/*
		struct compute_pipeline_default : Implement::pipeline_interface
		{
			std::vector <std::shared_ptr<Implement::element_compute_implement>> element_vector;
			template<typename ...PropertyMapping> void dispatch(stage_context& sc, PropertyMapping& ...property_mapping)
			{
				for (auto& ite : element_vector)
					if (ite)
						ite->dispatch(sc, property_mapping...);
			}
			void clear() { element_vector.clear(); }
			void insert(std::shared_ptr<Implement::element_compute_implement> sp) { element_vector.push_back(sp); }
		};
		

		struct defer_render_pipeline_default : Implement::pipeline_interface
		{
			tex2 G_buffer;
			tex2 depth_stencial;

			output_merge_stage oms;

			std::vector<std::shared_ptr<Implement::element_implement>> element_vector;

			void set_back_buffer_size(creator& p, tex2);

			template<typename ...PropertyMapping> void dispatch(stage_context& sc, PropertyMapping& ...property_mapping)
			{
				for (auto& ite : element_vector)
					if (ite)
						ite->draw(sc, property_mapping...);
			}

			defer_render_pipeline_default();
			defer_render_pipeline_default(const std::type_index& ti);

			void clear() { element_vector.clear(); }
		};

		struct transparent_pipeline_default : Implement::pipeline_interface
		{

		};
		*/

		/*
		class renderer_defer_default_3d : creator
		{
			renderer_default renderer;
			//Implement::element_instance_store store;

			//defer_render_pipeline_default 5

		public:
			renderer_defer_default_3d(value_table& vt);
		};
		*/

	}
}
