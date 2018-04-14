#pragma once
#include "dx11_frame.h"
#include "../dx/dx_type.h"
#include "dx11_element.h"
#include "dx11_buildin_element.h"
#include "dx11_form.h"
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
			void set(float2 view_size, float2 view_left_top = float2{ 0.0, 0.0 }, float angle = 3.1415926f / 4.0f, float2 far_near_plane = float2{ 0.1f, 10000.0f }, float2 avalible_depth = float2{ 0.0f, 1.0f }) 
			{
				*this = sub_viewport_perspective{ view_size , view_left_top , angle , far_near_plane , avalible_depth };
			}
			sub_viewport_perspective() {}

			sub_viewport_perspective(float2 view_size, float2 view_left_top = float2{ 0.0, 0.0 }, float angle = 3.1415926f / 4.0f, float2 far_near_plane = float2{ 0.1f, 10000.0f }, float2 avalible_depth = float2{ 0.0f, 1.0f });
			sub_viewport_perspective(float4 view_border, float angle = 3.1415926f / 4.0f, float2 far_near_plane = float2{ 0.1f, 10000.0f }, float2 avalible_depth = float2{ 0.0f, 1.0f })
				: sub_viewport_perspective(float2{ view_border.z - view_border.x, view_border.w - view_border.y }, float2{ view_border.x,view_border.y }, angle, far_near_plane, avalible_depth) {}
			sub_viewport_perspective(tex2 tex, float2 view_left_top = float2{ 0.0, 0.0 }, float angle = 3.1415926f / 4.0f, float2 far_near_plane = float2{ 0.1f, 10000.0f }, float2 avalible_depth = float2{ 0.0f, 1.0f }) :
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

		/*
		using namespace PO::Dx;
		class renderer_default : device_ptr
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
		*/

		/*
		namespace Implement
		{
			struct defer_renderer_default_holder
			{
				element_compute_storage compute;
				element_draw_storage opaque;
				element_draw_storage transparent;
			};
		}

		struct defer_renderer_static_mesh_component : public component_res
		{
			element_draw ele;
			bool is_opaque = false;
		};

		struct defer_renderer_compute_component : public component_res
		{
			element_compute ele;
		};

		stage_instance& stage_instance_instance() noexcept;

		struct defer_renderer_default : public renderer_interface
		{

			struct property_gbuffer
			{
				shader_resource_view<tex2> srv;
				sample_state::description ss_des = sample_state::description{
					D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP, 0.0f, 1,
					D3D11_COMPARISON_NEVER,{ 1.0f,1.0f,1.0f,1.0f }, -FLT_MAX, FLT_MAX
				};
				struct renderer_data_append
				{
					sample_state ss;
				};
				void update(device_ptr& c, renderer_data_append& rd) { 
					rd.ss.create(c, ss_des);
				}
			};

			class material_merga_gbuffer : public material_resource
			{
			public:
				material_merga_gbuffer(device_ptr& c);
				const element_requirement& requirement() const;
			};

			class compute_linearize_z : public compute_resource
			{
			public:
				compute_linearize_z(device_ptr&);
				const element_requirement& requirement() const;
			};

			struct property_linearize_z_output 
			{
				shader_resource_view<tex2> input_depth;
				unordered_access_view<tex2> output_depth;
				uint32_t2 size;
				void set_taregt_f(shader_resource_view<tex2> input, unordered_access_view<tex2> output_f, uint32_t2 output_size);
			};

			struct property_linear_z
			{
				shader_resource_view<tex2> z_buffer;
				sample_state::description ss_des = sample_state::default_description;
				struct renderer_data_append
				{
					sample_state ss;
				};

				void update(device_ptr& c, renderer_data_append& rd)
				{
					rd.ss.create(c, ss_des);
				}
			};

			void init(context& c, device_ptr& dp, tex2& back_buffer) override;
			void render_frame(duration d, context_ptr& cp, tex2& bb) override;

		private:

			stage_context stage_con;

			std::shared_ptr<Implement::defer_renderer_default_holder> sp;

			device_ptr dev;

			sub_viewport_perspective view;
			
			property_proxy_map opaque_mapping;
			depth_stencil_state opaque_depth_stencil_state;
			blend_state qpaque_blend;
			output_merge_stage opaque_output_merga;

			tex2 linear_z_buffer;
			Implement::element_dispatch_request element_linear_z;
			property_proxy_map linear_z_maping;

			property_proxy_map transparent_mapping;
			depth_stencil_state transparent_depth_stencil_state;

			Implement::element_draw_request element_merga;
			property_proxy_map merga_map;
			output_merge_stage final_output;
			duration total_count;
		};

	
		struct capture_system : public system_res
		{
			std::shared_ptr<Implement::defer_renderer_default_holder> sp;
			device_ptr dev;
			capture_system(std::shared_ptr<Implement::defer_renderer_default_holder> s, device_ptr d) : sp(std::move(s)), dev(std::move(d)) {  }
			using tick_capture = PO::capture<defer_renderer_compute_component>;

			void operator() (defer_renderer_compute_component& ders, context& c, duration da)
			{
				assert(sp);
				sp->compute.logic << ders.ele;
				ders.destory();
			}

			void end_tick(context& c)
			{
				assert(sp);
				sp->compute.logic_to_swap(dev);
			}
		};

		struct capture2_system : public system_res
		{
			std::shared_ptr<Implement::defer_renderer_default_holder> sp;
			device_ptr dev;
			capture2_system(std::shared_ptr<Implement::defer_renderer_default_holder> s, device_ptr d) : sp(std::move(s)), dev(std::move(d)) {  }
			using tick_capture = PO::capture<defer_renderer_static_mesh_component>;
			void start_tick(context& c)
			{
				assert(sp);
				sp->opaque.logic.element_draw_store.clear();
				sp->transparent.logic.element_draw_store.clear();
			}

			void operator() (defer_renderer_static_mesh_component& ders, context& c, duration da)
			{
				assert(sp);
				if (ders.is_opaque)
				{
					sp->opaque.logic << ders.ele;
				}
				else
					sp->transparent.logic << ders.ele;
			}

			void end_tick(context& c)
			{
				assert(sp);
				sp->opaque.logic_to_swap(dev);
				sp->transparent.logic_to_swap(dev);
			}
		};
		*/

		/*
		struct capture_system2 : public system_res
		{
			std::shared_ptr<Implement::defer_renderer_default_holder> sp;
			device_ptr dev;
			capture_system2(std::shared_ptr<Implement::defer_renderer_default_holder> s, device_ptr d) : sp(std::move(s)), dev(std::move(d)) {}
			using tick_caputre = PO::capture<defer_renderer_static_mesh_component>;
			void start_tick_component(context& c)
			{
				assert(sp);
				sp->opaque.logic.element_draw_store.clear();
				sp->transparent.logic.element_draw_store.clear();
			}

			void operator() (defer_renderer_static_mesh_component& ders)
			{
				assert(sp);
				if (ders.is_opaque)
				{
					sp->opaque.logic << ders.ele;
				}
				else
					sp->transparent.logic << ders.ele;
			}

			void end_tick_component(context& c)
			{
				assert(sp);
				sp->opaque.logic_to_swap(dev);
				sp->transparent.logic_to_swap(dev);
			}
		};*/
		

		

		



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

			void set_back_buffer_size(device_ptr& p, tex2);

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
		class renderer_defer_default_3d : device_ptr
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
