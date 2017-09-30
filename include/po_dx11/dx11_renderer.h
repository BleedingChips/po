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

		using namespace PO::Dx;
		class renderer_default : creator
		{
			tex2 back_buffer;
			output_merge_stage om;
			stage_context m_context;
			sub_viewport_parallel main_view;
			viewport view;

			element_compute_storage compute_storage;
			element_draw_storage draw_storage;
			swap_chain_ptr swap;

			renderer_default(Dx11_frame_initializer& DFI);

		public:

			stage_context& context() { return m_context; }

			template<typename T, typename ...AT> decltype(auto) create_if_no_exist(AT&& ...at) { return context.create_if_no_exist<T>(std::forward<AT>(at)...); }

			void clear_back_buffer(const std::array<float, 4>& bc) { m_context.clear_render_target(om, bc); }
			renderer_default(value_table& vt) : renderer_default(vt.get<Dx11_frame_initializer>()) {}

			void pre_tick(duration da);
			void pos_tick(duration da);

			renderer_default& operator << (const element_compute& el) { compute_storage.logic << el; return *this; }
			renderer_default& operator << (const element_draw& el) { draw_storage.logic << el; return *this; }
		};


		struct defer_renderer_default : creator
		{
			defer_renderer_default(value_table& vt) : defer_renderer_default(vt.get<Dx11_frame_initializer>()) {}
			defer_renderer_default(Dx11_frame_initializer& DFi);

			void pre_tick(duration da);
			void pos_tick(duration da);

			defer_renderer_default& operator << (const element_compute& el) { compute.logic << el; return *this; }

			class property_gbuffer : public property_resource
			{
				shader_resource_view<tex2> srv;
				sample_state ss;
			public:
				struct renderer_data
				{
					shader_resource_view<tex2> srv;
					sample_state ss;
				};
				void update(creator& c, renderer_data& rd) { rd.srv = srv; rd.ss = ss; }
				void set_gbuffer(shader_resource_view<tex2> color, sample_state ss) 
				{ 
					srv = std::move(color); 
					this->ss = std::move(ss);
					need_update(); 
				}
			};

			class material_merga_gbuffer : public material_resource
			{
			public:
				material_merga_gbuffer(creator& c);
				const element_requirement& requirement() const;
			};

			class compute_linearize_z : public compute_resource
			{
			public:
				compute_linearize_z(creator&);
				const element_requirement& requirement() const;
			};

			class property_linearize_z_output : public property_resource
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

			class property_linear_z : public property_resource
			{
				shader_resource_view<tex2> z_buffer;
				sample_state ss;
			public:
				struct renderer_data
				{
					shader_resource_view<tex2> z_buffer;
					sample_state ss;
				};

				void update(creator&, renderer_data& rd)
				{
					rd.z_buffer = z_buffer;
					rd.ss = ss;
				}

				void set_linear_z(shader_resource_view<tex2> z, sample_state ss)
				{
					z_buffer = std::move(z);
					this->ss = std::move(ss);
					need_update();
				}
			};

			stage_context& get_context() { return context; }
			

		private:

			operator stage_context& () { return context; }

			sub_viewport_perspective view;

			stage_context context;

			element_compute_storage compute;

			element_draw_storage opaque;
			property_proxy_map opaque_mapping;
			depth_stencil_state opaque_depth_stencil_state;
			blend_state qpaque_blend;
			output_merge_stage opaque_output_merga;

			tex2 linear_z_buffer;
			Implement::element_dispatch_request element_linear_z;
			property_proxy_map linear_z_maping;

			element_draw_storage transparent;
			property_proxy_map transparent_mapping;
			depth_stencil_state transparent_depth_stencil_state;

			Implement::element_draw_request element_merga;
			property_proxy_map merga_map;
			tex2 final_back_buffer;
			output_merge_stage final_output;

			stage_instance ins;

			duration total_time;
			swap_chain_ptr swap;
		public:
			decltype(opaque.logic)& pipeline_opaque() { return opaque.logic; }
			decltype(transparent.logic)& pipeline_transparent() { return transparent.logic; }
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
