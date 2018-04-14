#pragma once
#include "../win32/win32_form.h"
#include "dx11_frame.h"
#include "../interface/property.h"
#include "../interface/variable.h"
#include "../dxgi/dxgi_define.h"
#include "dx11_renderer.h"
#include <DirectXMath.h>
#include <DirectXMathVector.inl>
#include <d3d11.h>
#include <memory>
#include <deque>
#include "../../po/tool/tool.h"
namespace PO
{
	namespace Dx11
	{

		using form_property = PO::Win32::form_property;

		class view_interface;

		class form
		{
		public:
			struct control_t
			{
				Tool::atomic_reference_count ref;
				Tool::scope_lock<std::vector<event>> swap_temporary_event;
				Tool::scope_lock<std::vector<std::weak_ptr<view_interface>>> view_temporary_list;
				std::vector<std::weak_ptr<view_interface>> ready_list;
				void add_ref() noexcept { ref.add_ref(); }
				bool sub_ref() noexcept { return ref.sub_ref(); }
				Respond respond(event& e) noexcept;
				void update_list();
				void render(stage_context& sc, tex2& back_buffer);
			};
		private:

			struct thread_control_t
			{
				PO::Win32::form win32_form;
				device_ptr device;
				std::thread execute_thread;
				~thread_control_t();
			};
			
			Tool::intrusive_ptr<control_t> control;
			std::shared_ptr<thread_control_t> thread_control;

			static void execute_function(form_property fp, std::promise<std::tuple<PO::Win32::form, device_ptr>> pro, Tool::intrusive_ptr<control_t> con);

		public:
			void close() { thread_control->win32_form.close_window(); thread_control.reset(); control.reset(); }
			size_t pop_event(event* buffer, size_t size) noexcept;
			bool insert_view(std::weak_ptr<view_interface> sp);
			void create_form(const form_property& = form_property{});
		};



		/*
		class form : public PO::Win32::form
		{
			using win32_form = PO::Win32::form;
			stage_context context;
			swap_chain_ptr swap;
			tex2 back_buffer;
			render_target_view<tex2> back_buffer_view;
			buffer_constant bc;
			output_merge_stage oms;
			buffer_vertex bv;
			
			std::vector<event> thread_event_pool;
			


			std::vector<std::shared_ptr<view_interface>> view_list;
			Tool::scope_lock<std::vector<std::shared_ptr<view_interface>>> view_temporary_list;

			void render_frame();
			Respond respond_event(event& e);

		public:
			Tool::scope_lock<std::vector<event>> event_swap;
			
			void insert_view(std::shared_ptr<view_interface> sp);
			form(const form_property& = form_property{});
			~form();
		};
		*/


		class view_interface
		{
		public:
			virtual Respond respond_event(event& e) { return Respond::Pass; }
			virtual void render(stage_context& sc, tex2& oms) = 0;
		};


		struct view_projection
		{
			viewport view;
			PO::Graphic::Implement::shader_storage<float4, float4x4> buffer;
			view_projection(float2 view_size, float2 view_left_top = float2{ 0.0, 0.0 }, float angle = 3.1415926f / 4.0f, float2 far_near_plane = float2{ 0.1f, 10000.0f }, float2 avalible_depth = float2{ 0.0f, 1.0f });
			view_projection(float4 view_border, float angle = 3.1415926f / 4.0f, float2 far_near_plane = float2{ 0.1f, 10000.0f }, float2 avalible_depth = float2{ 0.0f, 1.0f })
				: view_projection(float2{ view_border.z - view_border.x, view_border.w - view_border.y }, float2{ view_border.x,view_border.y }, angle, far_near_plane, avalible_depth) {}
			view_projection(tex2 tex, float2 view_left_top = float2{ 0.0, 0.0 }, float angle = 3.1415926f / 4.0f, float2 far_near_plane = float2{ 0.1f, 10000.0f }, float2 avalible_depth = float2{ 0.0f, 1.0f }) :
				view_projection(tex.size_f(), view_left_top, angle, far_near_plane, avalible_depth) {}
			view_projection(const view_projection&) = default;
			view_projection& operator=(const view_projection&) = default;
			operator const viewport&() const { return view; }
		};



		/*
		struct form_type : Win32::form_type {};

		class renderer_interface
		{
		public:
			virtual void init(context& c, device_ptr& dp, tex2& back_buffer) = 0;
			virtual void render_frame(duration d, context_ptr& cp, tex2& bb) = 0;
			virtual ~renderer_interface();
		};

		class form_entity_mt : public Win32::form_entity_mt
		{
			Tool::scope_lock<std::unique_ptr<renderer_interface>> renderer_int;
			Tool::scope_lock<std::vector<std::function<void(context_ptr&)>>> function;
			std::vector<std::function<void(context_ptr&)>> buffer;
			swap_chain_ptr swap;
			context_ptr con;
			tex2 back_buffer;
		public:
			device_ptr dev;
			virtual void rander_frame(duration da) override;
			void tick(entity_self& e, context& c, duration a);
			void insert_task(std::function<void(context_ptr&)>);
			template<typename renderer_type, typename ...construction_para> void create_renderer(context& c, construction_para&& ...cp)
			{
				static_assert(std::is_base_of_v<renderer_interface, renderer_type>); 
				renderer_int.lock([&](typename decltype(renderer_int)::type& t)
				{
					t.reset();
					t = std::make_unique<renderer_type>(std::forward<construction_para>(cp)...);
					assert(t);
					t->init(c, dev, back_buffer);
				});
			}
			form_entity_mt(const form_type& ft = form_type{});
			~form_entity_mt() { Win32::form_entity_mt::wait_thread_close(); }
		};
		*/
	}
}
