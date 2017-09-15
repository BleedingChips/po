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

		
		class property_gbuffer_default : public property_resource
		{
			shader_resource_view<tex2> srv;
			shader_resource_view<tex2> linear_z;
			sample_state ss;
		public:
			struct renderer_data
			{
				shader_resource_view<tex2> srv;
				shader_resource_view<tex2> linear_z;
				sample_state ss;
			};
			void update(creator& c, renderer_data& rd) { rd.srv = srv; rd.ss = ss; rd.linear_z = linear_z; }
			void set_gbuffer(creator& c, const tex2& t, const tex2& linear) { srv = t.cast_shader_resource_view(c); ss.create(c); linear_z = linear.cast_shader_resource_view(c);  need_update();}
		};


		class material_merga_gbuffer_default : public material_resource
		{
		public:
			material_merga_gbuffer_default(creator& c);
			const element_requirement& requirement() const;
		};


		struct pipeline_opaque_default : public pipeline_interface
		{
			tex2 g_buffer;
			render_target_view<tex2> rtv;
			tex2 depth;
			depth_stencil_view<tex2> dsv;
			depth_stencil_state dss;
			blend_state bs;
			output_merge_stage om;
			void execute_implement(stage_context& sc, element_renderer_storage& storage, Tool::stack_list<Implement::property_map>* ptr) override;
			void set(creator& c, uint32_t2 size);
			pipeline_opaque_default();
		};

		struct material_opaque_resource : public material_resource
		{
			material_opaque_resource(creator& c, std::u16string patch) : material_resource(c, std::move(patch), {}, typeid(pipeline_opaque_default)) {}
		};

		class material_opaque_testing : public material_opaque_resource
		{
		public:
			material_opaque_testing(creator&);
		};


		class material_qpaque_texture_coord : public material_opaque_resource
		{
		public:
			material_qpaque_texture_coord(creator&);
		};

		class property_linearize_z : public property_resource
		{
			shader_resource_view<tex2> input_depth;
			unordered_access_view<tex2> output_depth;
			uint32_t2 size;
		public:
			struct renderer_data
			{
				shader_resource_view<tex2> input_depth;
				unordered_access_view<tex2> output_depth;
				uint32_t2 size;
			};
			void set_taregt_f(shader_resource_view<tex2> input, unordered_access_view<tex2> output_f, uint32_t2 output_size);
			void update(creator& c, renderer_data& rd)
			{
				rd.input_depth = input_depth;
				rd.output_depth = output_depth;
				rd.size = size;
			}
		};

		class compute_linearize_z : public compute_resource
		{
		public:
			compute_linearize_z(creator&);
			const element_requirement& requirement() const;
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

		struct material_transparent_resource : public material_resource
		{
			material_transparent_resource(creator& c, std::u16string patch, std::optional<blend_state::description> des = {}) :
				material_resource(c, std::move(patch), des, typeid(pipeline_transparent_default)) {}
		};

		class material_transparent_testing : public material_transparent_resource
		{
		public:
			material_transparent_testing(creator&);
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
			duration total_time;


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

		struct property_tex2 : public property_resource
		{
			shader_resource_view<tex2> srv;
			sample_state ss;
			struct renderer_data
			{
				shader_resource_view<tex2> srv;
				sample_state ss;
			};
			void update(creator& c, renderer_data& rd)
			{
				rd.srv = srv;
				rd.ss = ss;
			}
			void set_texture(creator& c, const shader_resource_view<tex2>& t, const sample_state::description& des = sample_state::default_description) {
				srv = t; 
				ss.create(c, des);
				need_update();
			}
		};

		class material_opaque_tex2_viewer : public material_opaque_resource
		{
		public:
			const element_requirement& requirement() const;
			material_opaque_tex2_viewer(creator& v);
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
