#include "dx11_form.h"
#include <fstream>
#include <filesystem>
#include <array>
#include <iostream>
#include "../win32/win32_log.h"
namespace
{

	const char16_t po_dx11_default_form_style[] = u"po_dx11_defult_form_style";


	LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		std::cout << PO::Platform::Win32::Log::translate_event_type_to_string(msg) << std::endl;
		switch (msg)
		{
		case WM_DESTROY:
		{
			LONG_PTR data = GetWindowLongPtr(hWnd, GWLP_USERDATA);
			PO::Dx11::form::control_t* ptr = reinterpret_cast<PO::Dx11::form::control_t*>(data);
			if (ptr != nullptr)
			{
				if (ptr->sub_ref())
					delete ptr;
			}
			SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
			PostQuitMessage(0);
			return DefWindowProcW(hWnd, msg, wParam, lParam);
		}
		default:
		{
			MSG message{ hWnd, msg, wParam, lParam };
			auto ev = PO::Win32::translate_message_to_event(message);
			if (ev.has_value())
			{
				LONG_PTR data = GetWindowLongPtr(hWnd, GWLP_USERDATA);
				PO::Dx11::form::control_t* ptr = reinterpret_cast<PO::Dx11::form::control_t*>(data);
				if (ptr != nullptr)
					if (ptr->respond(ev.value()) == PO::Respond::Truncation)
						return 0;
			}
			return DefWindowProcW(hWnd, msg, wParam, lParam);
		}
		}
	}


	const WNDCLASSEXW static_class = { sizeof(WNDCLASSEXW), CS_HREDRAW | CS_VREDRAW , WndProc, 0, 0, GetModuleHandle(0), NULL,NULL, 0, NULL, (const wchar_t*)po_dx11_default_form_style, NULL };

	const struct static_class_init_struct
	{
		static_class_init_struct()
		{
			HRESULT res = RegisterClassExW(&static_class);
			assert(SUCCEEDED(res));
		}

		~static_class_init_struct()
		{
			UnregisterClassW((const wchar_t*)po_dx11_default_form_style, GetModuleHandleW(0));
		}
	}init;
}


namespace PO
{
	namespace Dx11
	{
		//std::arrary<float3, 4> = { {0.0, 0.0}, {1.0, 0.0}, {1.0, 1.0} , {0.0, 1.0} };

		view_projection::view_projection(float2 view_size, float2 view_left_top, float angle, float2 far_near_plane, float2 avalible_depth)
			:buffer{ float4{ view_size.x / view_size.y, angle, far_near_plane.x, far_near_plane.y } },
			view{ view_size, view_left_top, avalible_depth }
		{
			matrix ma = DirectX::XMMatrixPerspectiveFovLH(angle, buffer.get<0>().x, far_near_plane.x, far_near_plane.y);
			DirectX::XMStoreFloat4x4(&buffer.get<1>(), ma);
			//DirectX::XMStoreFloat4x4(&eye, DirectX::XMMatrixIdentity());
		}


		Respond form::control_t::respond(event& e) noexcept
		{
			Respond res = Respond::Pass;
			for (auto& ite : ready_list)
			{
				auto ptr = ite.lock();
				if (ptr)
				{
					res = ptr->respond_event(e);
					if (res == PO::Respond::Truncation)
						break;
				}
			}
			if (res == Respond::Pass)
			{
				swap_temporary_event.lock([&](decltype(swap_temporary_event)::type& t) {t.push_back(e); });
				return Respond::Truncation;
			}
			return res;
		}

		void form::control_t::update_list()
		{
			ready_list.erase(
				std::remove_if(ready_list.begin(), ready_list.end(), [](std::weak_ptr<view_interface>& ite) {return ite.expired(); }),
				ready_list.end()
			);

			view_temporary_list.lock([&](std::vector<std::weak_ptr<view_interface>>& ite) {
				using ite_t = std::vector<std::weak_ptr<view_interface>>::iterator;
				if (!ite.empty())
				{
					ready_list.insert(ready_list.end(), std::move_iterator<ite_t>(ite.begin()), std::move_iterator<ite_t>(ite.end()));
					ite.clear();
				}
			});
		}
		void form::control_t::render(stage_context& sc, tex2& back_buffer)
		{
			for (auto& ite : ready_list)
			{
				auto view = ite.lock();
				if (view)
					view->render(sc, back_buffer);
			}
		}





		form::thread_control_t::~thread_control_t()
		{
			device = device_ptr{};
			win32_form.close_window();
			if (execute_thread.joinable())
				execute_thread.join();
		}

		size_t form::pop_event(event* buffer, size_t size) noexcept
		{
			if (control)
			{
				auto& ref = control->swap_temporary_event;
				return ref.lock([&](std::vector<event>& i) -> size_t {
					if (i.empty())
						return 0;
					size_t pool_szie = i.size();
					if (size >= pool_szie)
					{
						size = pool_szie;
						for (auto& ite : i)
							*(buffer++) = std::move(ite);
						i.clear();
						return size;
					}else
					{
						auto ite = i.begin();
						for (size_t index = 0; index < size; ++index)
							*(buffer++) = std::move(*ite ++);
						i.erase(i.begin(), ite);
						return size;
					}
				});
			}
			return 0;
		}

		bool form::insert_view(std::weak_ptr<view_interface> sp)
		{
			if (!sp.expired() && control)
			{
				control->view_temporary_list.lock([&](std::vector<std::weak_ptr<view_interface>>& i) {
					i.push_back(std::move(sp));
				});
				return true;
			}	
			return false;
		}

		void form::execute_function(form_property fp, std::promise<std::tuple<PO::Win32::form, device_ptr>> pro, Tool::intrusive_ptr<control_t> con)
		{
			control_t * da = con;
			da->add_ref();
			PO::Win32::form win32_form = PO::Win32::create_form(fp, po_dx11_default_form_style, da);
			D3D_FEATURE_LEVEL lel[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_11_1 };
			D3D_FEATURE_LEVEL final_level;
			PO::DXGI::swap_chain_desc swc(win32_form.raw_handle());
			swap_chain_ptr swap;
			stage_context context;
			tex2 back_buffer;
			HRESULT re = D3D11CreateDeviceAndSwapChain(
				nullptr,
				D3D_DRIVER_TYPE_HARDWARE,
				nullptr,
				D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DISABLE_GPU_TIMEOUT | D3D11_CREATE_DEVICE_DEBUG,
				lel,
				1,
				D3D11_SDK_VERSION,
				&swc,
				swap.adress(),
				context.dev.adress(),
				&final_level,
				context.ptr.adress()
			);
			assert(SUCCEEDED(re));
			re = (swap)->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&back_buffer.ptr);
			assert(SUCCEEDED(re));
			pro.set_value({ win32_form , context.dev });

			std::vector<event> event_buffer;
			std::vector<std::weak_ptr<view_interface>> interface_view;
			bool avalible = true;
			while (PO::Win32::respond_event())
			{
				con->update_list();
				con->render(context, back_buffer);
				swap->Present(0 , 0);
			}
		}



		void form::create_form(const form_property& fp)
		{
			control = new control_t{};
			thread_control = std::make_shared<thread_control_t>();
			std::promise<std::tuple<PO::Win32::form, device_ptr>> pro;
			auto fur = pro.get_future();
			thread_control->execute_thread = std::thread(&form::execute_function, fp, std::move(pro), control);
			fur.wait();
			std::tie(thread_control->win32_form, thread_control->device) = fur.get();
		}







		/*
		form::form(const form_property& ft) : win32_form(ft)
		{
			D3D_FEATURE_LEVEL lel[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_11_1 };
			D3D_FEATURE_LEVEL final_level;
			PO::DXGI::swap_chain_desc swc(raw_handle);
			HRESULT re = D3D11CreateDeviceAndSwapChain(
				nullptr,
				D3D_DRIVER_TYPE_HARDWARE,
				nullptr,
				D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DISABLE_GPU_TIMEOUT | D3D11_CREATE_DEVICE_DEBUG,
				lel,
				1,
				D3D11_SDK_VERSION,
				&swc,
				swap.adress(),
				context.dev.adress(),
				&final_level,
				context.ptr.adress()
			);
			assert(SUCCEEDED(re));
			re = (swap)->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&back_buffer.ptr);
			assert(SUCCEEDED(re));
			win32_form::set_function([this]() {render_frame(); }, [this](event& e) { return respond_event(e);  });
			win32_form::show_window();
			view_projection view{ {1024, 768} };

			context << view.view;
			back_buffer_view = back_buffer.cast_render_target_view(context.dev);
			bc.create_pod(context.dev, view.buffer);
			oms.set(back_buffer_view);
			//bv.create_vertex(context.dev,);
			input_layout lay;
			//lay.create(context.dev, Dx11::layout_type<PO::Dx11::syntax<float4, 0, float4>>{}, PO::Dx11::vshader_ptr{});

		}

		void form::insert_view(std::shared_ptr<view_interface> sp)
		{
			if (sp)
			{
				view_temporary_list.lock([&](decltype(view_temporary_list)::type& t) {
					t.push_back(std::move(sp));
				});
			}
		}

		Respond form::respond_event(event& e)
		{
			thread_event_pool.push_back(e);
			return Respond::Pass;
		}

		void form::render_frame()
		{
			if (!thread_event_pool.empty())
			{
				event_swap.lock([this](decltype(event_swap)::type& t) {
					t.insert(t.end(), thread_event_pool.begin(), thread_event_pool.end());
					thread_event_pool.clear();
				});
			}

			view_temporary_list.lock([this](decltype(view_temporary_list)::type& t) {
				if (!t.empty())
				{
					view_list.insert(view_list.end(), std::move_iterator<decltype(t.begin())>(t.begin()), std::move_iterator<decltype(t.end())>(t.end()));
					t.clear();
				}
			});
			context.clear_render_target(oms, { 0.0, 0.0, 0.0, 0.0 });

			for (auto& ite : view_list)
				ite->render(context, back_buffer);

			swap->Present(0, 0);
		}

		form::~form()
		{
			win32_form::set_function_wait({}, {});
		}
		*/

	}
}





























/*
namespace PO
{
	namespace Dx11
	{
		
		renderer_interface::~renderer_interface() {}

		void form_entity_mt::tick(entity_self& e, context& c, duration a)
		{
			Win32::form_entity_mt::tick(e, c, a);
			renderer_int.lock([&](decltype(renderer_int)::type& t) {
				if (t)
					t->render_frame(a, con, back_buffer);
			});
			function.lock([&](decltype(function)::type& t) {
				std::swap(buffer, t);
			});
			for (auto& ite : buffer)
				if (ite) ite(con);
			buffer.clear();
			swap->Present(0, 0);
		}

		form_entity_mt::form_entity_mt(const form_type& ft) : Win32::form_entity_mt(ft) {
			D3D_FEATURE_LEVEL lel[] = { D3D_FEATURE_LEVEL_11_1 };
			D3D_FEATURE_LEVEL lel2 = D3D_FEATURE_LEVEL_11_1;
			PO::DXGI::swap_chain_desc swc(raw_handle);
			HRESULT re = D3D11CreateDeviceAndSwapChain(
				nullptr,
				D3D_DRIVER_TYPE_HARDWARE,
				nullptr,
				D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DISABLE_GPU_TIMEOUT | D3D11_CREATE_DEVICE_DEBUG,
				lel,
				1,
				D3D11_SDK_VERSION,
				&swc,
				swap.adress(),
				dev.adress(),
				&lel2,
				con.adress()
			);
			assert(SUCCEEDED(re));
			re = (swap)->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&back_buffer.ptr);
			assert(SUCCEEDED(re));
		}
		
		void form_entity_mt::rander_frame(duration da)
		{
			
		}

		void form_entity_mt::insert_task(std::function<void(context_ptr&)> f)
		{
			function.lock([&](decltype(function)::type& t) {
				t.push_back(std::move(f));
			});
		}

	}
}







/*
namespace PO
{
	namespace Interface
	{
		Tool::optional<std::string> init_dx11_initial_form_win32_initial(dx11_initial_implement*& dii, win32_initial_implement* wii) noexcept
		{
			try {
				dii = new dx11_initial_implement;
				return{};
			}
			catch (const std::exception& ex)
			{
				return{ std::string(__FUNCDNAME__) + " : " + ex.what() };
			}
			catch (...)
			{
				return{ std::string(__FUNCDNAME__) + " : Unknow exeception"};
			}
		}

		void dest_dx11_initial(dx11_initial_implement*& dii) noexcept
		{
			delete dii;
			dii = nullptr;
		}

		Tool::optional<std::string> init_dx11_form_form_win32_form(dx11_form_implement*& dfi, dx11_initial_implement* dii, win32_form_implement* wfi) noexcept
		{
			try {
				dfi = new dx11_form_implement();
				D3D_FEATURE_LEVEL lel[] = { D3D_FEATURE_LEVEL_11_0 };
				D3D_FEATURE_LEVEL lel2 = D3D_FEATURE_LEVEL_11_0;
				PO::Platform::Dxgi::swap_chain_desc swc(wfi->raw_handle);
				HRESULT re = D3D11CreateDeviceAndSwapChain(
					nullptr,
					D3D_DRIVER_TYPE_HARDWARE,
					nullptr,
					D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DISABLE_GPU_TIMEOUT,
					lel,
					1,
					D3D11_SDK_VERSION,
					&swc,
					&dfi->swap,
					&dfi->dev,
					&lel2,
					&dfi->dc
				);
				Error::fail_throw(re, [&]() {delete dfi; });

				Error::fail_throw(dfi->swap->GetBuffer(0,
					__uuidof(ID3D11Texture2D),
					(void **)&dfi->main_buffer));

				D3D11_TEXTURE2D_DESC desc;
				desc.Width = 1024;
				desc.Height = 768;
				desc.MipLevels = 1;
				desc.ArraySize = 1;
				desc.Format = DXGI_FORMAT_D32_FLOAT;
				desc.SampleDesc.Count = 1;
				desc.SampleDesc.Quality = 0;
				desc.Usage = D3D11_USAGE_DEFAULT;
				desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
				desc.CPUAccessFlags = 0;
				desc.MiscFlags = 0;
				HRESULT hr = dfi->dev->CreateTexture2D(&desc,
					0, &dfi->pDepthStencilBuffer);
				hr = dfi->dev->CreateDepthStencilView(dfi->pDepthStencilBuffer,
					0, &dfi->pDepthView);
				dfi->dev->CreateRenderTargetView(dfi->main_buffer, 0, &dfi->pView);

				ID3D11RenderTargetView* RenderTargetViews[1] = { dfi->pView };
				ID3D11DepthStencilView* DepthTargetView = dfi->pDepthView;

				dfi->dc->OMSetRenderTargets(1, RenderTargetViews, dfi->pDepthView);
				return{};
			}
			catch (std::exception& ex)
			{
				return{ std::string(__FUNCDNAME__) + " : " + ex.what() };
			}
			catch (...)
			{
				return{ std::string(__FUNCDNAME__) + " Unknow " };
			}
		}
		
		void dest_dx11_form(dx11_form_implement*& dfi) noexcept
		{
			dfi->dc->ClearState();
			delete dfi;
			dfi = nullptr;
		}

		void clean_dx11_form(dx11_form_implement* dfi, float R,float G,float B,float A) noexcept
		{
			float color[4] = { R,G,B,A };
			dfi->dc->ClearRenderTargetView(dfi->pView, color);
		}
		void swap_dx11_form(dx11_form_implement* dfi) noexcept
		{
			dfi->swap->Present(0, 0);
		}

		Tool::optional<std::string> init_dx11_renderer_from_from(dx11_renderer_implement*& dri, dx11_form_implement* dfi) noexcept
		{
			try
			{
				dri = new dx11_renderer_implement();
				dri->dc = dfi->dc;
				dri->dev = dfi->dev;
				dri->swap = dfi->swap;
				dri->pView = dfi->pView;
				return{};
			}
			catch (std::exception& ex)
			{
				return{ std::string(__FUNCDNAME__) + " : " + ex.what() };
			}
			catch (...)
			{
				return{ std::string(__FUNCDNAME__) + " Unknow " };
			}
		}

		void dest_dx11_renderer(dx11_renderer_implement*& dri) noexcept
		{
			delete dri;
			dri = nullptr;
		}

		void tick_dx11_renderer(dx11_renderer_implement* dri, duration da) noexcept
		{
			static float all_time = 0.0f;
			all_time += da.count();
			float a = abs(sin(all_time * 0.001f));
			float a2 = abs(sin(all_time * 0.001f * 2));
			float a3 = abs(sin(all_time * 0.001f * 3));
			float color[4] = { a,a2,a3,1.0f };
			dri->dc->ClearRenderTargetView(dri->pView, color);
			dri->swap->Present(0, 0);
		}
	}
}
*/