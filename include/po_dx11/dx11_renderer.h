#pragma once
#include "../po/renderer.h"
#include "dx11_frame.h"
#include "../po_dx/dx_type.h"
#include "dx11_element.h"
namespace PO
{
	namespace Dx11
	{

		struct property_proxy_viewport 
		{
			float xy_rage, far_plane, near_plane;
			float4x4 projection;
			float4 view;
			bool static_data_update = false;

			float4x4 eye;
			float time;
			bool dynamic_data_update = false;

			struct renderer_data
			{
				constant_buffer buffer_static;
				constant_buffer buffer_dynamic;
			};
		};



		struct sub_viewport_perspective
		{
			float4 projection_property;
			float4x4 projection;
			viewport view;

			sub_viewport_perspective(float2 view_size, float2 view_left_top = float2{0.0, 0.0}, float angle = 3.1415926f / 4.0f, float2 far_near_plane = float2{ 0.0001f, 10000.0f }, float2 avalible_depth = float2{ 0.0f, 1.0f });
			sub_viewport_perspective(float4 view_border, float angle = 3.1415926f / 4.0f, float2 far_near_plane = float2{ 0.0001f, 10000.0f }, float2 avalible_depth = float2{ 0.0f, 1.0f })
				: sub_viewport_perspective(float2{ view_border.z - view_border.x, view_border.w - view_border.y }, float2{ view_border.x,view_border.y }, angle, far_near_plane, avalible_depth) {}
			sub_viewport_perspective(tex2 tex, float2 view_left_top = float2{0.0, 0.0}, float angle = 3.1415926f / 4.0f, float2 far_near_plane = float2{ 0.0001f, 10000.0f }, float2 avalible_depth = float2{ 0.0f, 1.0f }) :
				sub_viewport_perspective(tex.size_f(), view_left_top, angle, far_near_plane, avalible_depth) {}
			sub_viewport_perspective(const sub_viewport_perspective&) = default;
			sub_viewport_perspective& operator=(const sub_viewport_perspective&) = default;
			operator const viewport&() const { return view; }
		};

		struct sub_viewport_parallel
		{
			float4 projection_property;
			float4x4 projection;
			viewport view;

			sub_viewport_parallel(float2 d) {}
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
