#include "context_dx11.h"
#include "windows.h"
#include "../DirectXTex/DirectXTex/DirectXTex.h"
#include <thread>
#include <future>
#include <deque>

namespace PO::Dx11
{
	//********************************************* vertex_buffer_dx11 *******************************************************
	size_t vertex_dx11::add_vertex(size_t count, std::initializer_list<std::tuple<FormatPixel, const char*>> semantic, const std::byte* buffer)
	{
		if (m_vertex_count != 0)
			m_vertex_count = m_vertex_count < count ? m_vertex_count : static_cast<uint32_t>(count);
		UINT solt = static_cast<uint32_t>(m_vertex_buffer.size());
		UINT space = 0;
		std::vector<D3D11_INPUT_ELEMENT_DESC> temporary;
		for (auto& ite2 : semantic)
		{
			temporary.push_back(
				D3D11_INPUT_ELEMENT_DESC{
					std::get<1>(ite2), 0, DXGI::translate(std::get<0>(ite2)),
					solt,
					space,
					D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA,
					0
				}
			);
			space += Graphic::calculate_pixel_size(std::get<0>(ite2));
		}
		CComPtr<Buffer> result;
		HRESULT re = create_vertex(&result, *m_device, buffer, m_vertex_count * space);
		if (SUCCEEDED(re))
		{
			m_semantic.insert(m_semantic.end(), temporary.begin(), temporary.end());
			m_vertex_buffer.push_back({ result, space });
			return m_vertex_count;
		}
		return 0;
	}

	bool vertex_dx11::set_index(size_t count, FormatPixel format, const std::byte* buffer)
	{
		m_index_buffer = nullptr;
		DXGI::Format dxgi_format = DXGI::translate(format);
		if (dxgi_format == DXGI_FORMAT_R16_UINT || dxgi_format == DXGI_FORMAT_R32_UINT)
		{
			m_index_format = dxgi_format;
			m_index_count = static_cast<UINT>(count);
			HRESULT re = create_index(&m_index_buffer, *m_device, buffer, m_vertex_count * DXGI::calculate_pixel_size(dxgi_format));
			if (SUCCEEDED(re))
				return true;
		}
		return false;
	}

	bool vertex_dx11::apply_and_call(DeviceContext& con) const
	{
		UINT offset = 0;
		for (UINT i = 0; i < m_vertex_buffer.size(); ++i)
		{
			auto& ref = m_vertex_buffer[i];
			ID3D11Buffer * ptr = std::get<0>(ref);
			con.IASetVertexBuffers(i, 1, &ptr, &std::get<1>(ref), &offset);
		}
		con.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		if (m_index_count != 0)
		{
			con.IASetIndexBuffer(m_index_buffer, m_index_format, 0);
			con.DrawIndexed(m_index_count, 0, 0);
		}
		else
			con.Draw(m_vertex_count, 0);
		return true;
	}

	vertex_resource_ptr vertex_dx11::clone()
	{
		return new vertex_dx11{ m_device };
	}

	//********************************************* readable_resource_dx11 *******************************************************
	/*
	readable_resource_dx11_ptr readable_resource_dx11::create(Graphic::ResourceType type, DXGI::Format format, Graphic::uint3 size, size_t length)
	{
		return new (Tool::aligna_buffer<alignof(readable_resource_dx11)>::allocate(sizeof(readable_resource_dx11) + length))
			readable_resource_dx11{ type , format, size, length };
	}

	void readable_resource_dx11::release() noexcept
	{
		this->~readable_resource_dx11();
		Tool::aligna_buffer<alignof(readable_resource_dx11)>::release(this);
	}

	readable_resource_dx11::readable_resource_dx11(Graphic::ResourceType type, DXGI::Format format, Graphic::uint3 size, size_t length) noexcept
		: Graphic::readable_resource(typeid(readable_resource_dx11), type, Graphic::Usage::Output, size), m_format(format), m_length(length)
	{}
	*/

	namespace Implement
	{

		//********************************************* context_control_block *******************************************************
		bool context_control_block::execute_front(DeviceContext& DC)
		{
			function_type output;
			{
				std::lock_guard<std::mutex> lg(m_list_mutex);
				if (!m_list.empty())
				{
					output = std::move(*m_list.begin());
					m_list.pop_front();
				}
			}
			if (output)
			{
				output(DC);
				return true;
			}
			return false;
		}

		void context_control_block::close() { m_available = false; }

		bool context_control_block::is_available() noexcept { return m_available; }

		void context_control_block::push(function_type&& t)
		{
			std::lock_guard <std::mutex> lg(m_list_mutex);
			m_list.push_back(std::move(t));
		}
	}

	//********************************************* renderer_dx11 *******************************************************
	CComPtr<CommandList> renderer_dx11::create_command() noexcept
	{
		CComPtr<CommandList> result;
		HRESULT re = m_device->FinishCommandList(false, &result);
		assert(SUCCEEDED(re));
		return result;
	}

	renderer_dx11::renderer_dx11(Device& dev)
		: Graphic::renderer(typeid(renderer_dx11))
	{
		HRESULT re = dev.CreateDeferredContext(0, &m_device);
		assert(SUCCEEDED(re));
	}

	renderer_dx11::renderer_dx11(CComPtr<DeviceContext> con) : Graphic::renderer(typeid(renderer_dx11)), m_device(std::move(con))
	{
		assert(m_device);
	}

	//********************************************* renderer_dx11 *******************************************************
	bool renderer_dx11::clear_render_target(float4 color, const view_resource& vr)
	{
		if (vr.is<render_target_dx11>())
		{
			auto& ref = vr.cast_static<render_target_dx11>();
			m_device->ClearRenderTargetView(ref.m_rtv, &color.x);
			return true;
		}
		return false;
	}

	namespace Implement
	{

		//********************************************* pass_dx11 *******************************************************

		template<typename input_t, typename output_t1, typename output_t2>
		bool equation(input_t& input, output_t1& output1, output_t2& output2, uint64_t index) {
			auto ite = input.find(index);
			if (ite != input.end())
			{
				std::tie(output1, output2) = ite->second;
				return true;
			}
			else
				return false;
		};

		/*
		pass_dx11_ptr pass_dx11::create(Device& gc,
			const std::map<uint64_t, std::tuple<CComPtr<ShaderVS>, CComPtr<ShaderReflection>>>& vsm,
			const std::map<uint64_t, std::tuple<CComPtr<ShaderHS>, CComPtr<ShaderReflection>>>& hsm,
			const std::map<uint64_t, std::tuple<CComPtr<ShaderDS>, CComPtr<ShaderReflection>>>& dsm,
			const std::map<uint64_t, std::tuple<CComPtr<ShaderGS>, CComPtr<ShaderReflection>>>& gsm,
			const std::map<uint64_t, std::tuple<CComPtr<ShaderPS>, CComPtr<ShaderReflection>>>& psm,
			const std::vector<Dx::HLSL::shader_code_ptr>& all_code,
			const Dx::HLSL::mscfbdx_pass& pass)
		{
			pass_dx11_ptr tem = new pass_dx11{};
			auto& ref = *tem;
			ref.m_ia = pass.ia;
			ref.m_render_target = pass.render_target;
			ref.m_depth_stencil = pass.depth_stencil;
			ref.m_stream_out = pass.stream_out;

			{
				bool result = equation(vsm, ref.m_vs, ref.m_vs_reflection, pass.shader_index[0]);
				assert(result);
				ref.m_vs_code = all_code[pass.shader_index[0]];
			}
			equation(hsm, ref.m_hs, ref.m_hs_reflection, pass.shader_index[1]);
			equation(dsm, ref.m_ds, ref.m_ds_reflection, pass.shader_index[2]);
			equation(psm, ref.m_ps, ref.m_ps_reflection, pass.shader_index[4]);
			uint64_t index = pass.shader_index[3];
			if (ref.m_stream_out.empty())
			{
				equation(vsm, ref.m_gs, ref.m_gs_reflection, pass.shader_index[3]);
			}
			else {
				Dx::HLSL::shader_code_ptr code;
				if (index < all_code.size())
					code = all_code[index];
				else
				{
					assert(pass.shader_index[0] < all_code.size());
					code = all_code[pass.shader_index[0]];
				}
				CComPtr<ShaderReflection> reflect;
				HRESULT re = create_shader_reflection(&reflect, code->code(), code->length());
				assert(SUCCEEDED(re));
				re = create_gs_shader_with_stream_out(&ref.m_gs, gc, code->code(), code->length(), *reflect, ref.m_gs, 0);
				assert(SUCCEEDED(re));
				if (index < all_code.size())
					ref.m_gs_reflection = std::move(reflect);
			}
			return tem;
		}

		void pass_dx11::release() noexcept
		{
			delete this;
		}

		const vertex_resource* find_vertex(const char* name, const Graphic::resource_map& self, const Graphic::resource_map& gobal)
		{
			auto first = self.find_vertex(name);
			if (first != nullptr)
				return first;
			else
				return gobal.find_vertex(name);
		}

		const view_resource* find_view(const char* name, ResourceType type, Usage usage, const Graphic::resource_map& self, const Graphic::resource_map& gobal)
		{
			auto first = self.find_view(name, type, usage);
			if (first != nullptr)
				return first;
			else
				return gobal.find_view(name, type, usage);
		}

		bool pass_dx11::apply(uint32_t index, const pass_cache& pc, graphic_context& gc, renderer& rc, const resource_map& self, const resource_map& gobal) const
		{
			if (gc.is<graphic_context_dx11>() && rc.is<renderer_dx11>())
			{
				Device& device_ref = gc.cast_static<graphic_context_dx11>();
				auto vertex = find_vertex(m_ia.c_str(), self, gobal);
				if (vertex != nullptr)
				{
					if (vertex->is<vertex_dx11>())
					{
						auto& vertex_ref = vertex->cast_static<vertex_dx11>();
						CComPtr<InputLayout> layout;
						HRESULT re = device_ref.CreateInputLayout(vertex_ref.semantic(), vertex_ref.semantic_length(), m_vs_code->code(), m_vs_code->length(), &layout);
						if (SUCCEEDED(re))
						{
							auto& con = rc.cast_static<renderer_dx11>();
							con->IASetInputLayout(layout);
							con->VSSetShader(m_vs, nullptr, 0);
							if (m_ps) con->PSSetShader(m_ps, nullptr, 0);
							std::array<ID3D11RenderTargetView*, 8> art;
							UINT count = (m_render_target.size() < 8) ? static_cast<UINT>(m_render_target.size()) : 8;

							for (UINT i = 0; i < count; ++i)
							{
								auto& name = m_render_target[i];
								auto rt = find_view(name.c_str(), Graphic::ResourceType::Tex2, Graphic::Usage::RenderTarget, self, gobal);
								if (rt && rt->is<render_target_dx11>())
									art[i] = rt->cast_static<render_target_dx11>().m_rtv;
								else
									return false;
							}
							con->OMSetRenderTargets(count, art.data(), nullptr);
							return vertex_ref.apply_and_call(con);
						}
					}
				}
			}
			return false;
		}

		//********************************************* compute_dx11 *******************************************************
		void compute_dx11::release() noexcept
		{
			delete this;
		}

		compute_dx11_ptr compute_dx11::create(Device& gc, const std::map<uint64_t, std::tuple<CComPtr<ShaderCS>, CComPtr<ShaderReflection>>>& gsm, const Dx::HLSL::mscfbdx_compute& compute)
		{
			compute_dx11_ptr tem = new compute_dx11{};
			auto& ref = *tem;
			ref.di = compute.di;
			bool result = equation(gsm, ref.m_cs, ref.m_cs_reflect, compute.shader_cs);
			assert(result);
			return tem;
		}
		*/

		//********************************************* render_command *******************************************************

		void execute_command(DeviceContext& DC, CommandList* CL, SwapChain* SC)
		{
			assert(CL != nullptr);
			DC.ExecuteCommandList(CL, false);
			if (SC)
			{
				DXGI_PRESENT_PARAMETERS tem{ 0, nullptr, nullptr, nullptr };
				SC->Present1(0, 0, &tem);
			}
		}

		void render_command::operator()(DeviceContext& DC)
		{
			std::lock_guard<std::mutex> lg(m_mutex);
			assert(m_state == RenderCommandState::Waiting);
			assert(m_command);
			execute_command(DC, m_command, m_swap);
			m_state = RenderCommandState::Done;
		}


		//********************************************* renderer_control *******************************************************

		renderer_control::renderer_control(Implement::context_control_block_ptr p) : m_control(p),
			m_current_state(std::make_shared<render_command>()),
			m_last_state(std::make_shared<render_command>())
		{}

		bool renderer_control::can_request_new_draw() noexcept
		{
			assert(m_current_state);
			auto& ref = *m_current_state;
			if (ref.m_mutex.try_lock())
			{
				std::lock_guard<std::mutex> lg(ref.m_mutex, std::adopt_lock);
				if (ref.m_state == RenderCommandState::Done)
					return true;
			}
			return false;
		}

		void renderer_control::push_draw_syn(CComPtr<CommandList> cl, CComPtr<SwapChain> sc) noexcept
		{
			assert(cl);
			std::promise<void> pro;
			auto fu = pro.get_future();
			m_control->push([&](DeviceContext& DC) {
				execute_command(DC, cl, sc);
				pro.set_value();
			});
			fu.wait();
			fu.get();
		}
		void renderer_control::push_draw_asyn(CComPtr<CommandList> cl, CComPtr<SwapChain> sc) noexcept
		{
			assert(cl);
			assert(m_last_state);
			auto& last_ref = *m_last_state;
			if (last_ref.m_mutex.try_lock())
			{
				std::lock_guard<std::mutex> lg(last_ref.m_mutex, std::adopt_lock);
				if (last_ref.m_state == RenderCommandState::Waiting)
				{
					last_ref.m_command = std::move(cl);
					last_ref.m_swap = std::move(sc);
					return;
				}
			}
			auto& ref = *m_current_state;
			{
				std::lock_guard<std::mutex> lg(ref.m_mutex);
				ref.m_state = RenderCommandState::Waiting;
				ref.m_command = std::move(cl);
				ref.m_swap = std::move(sc);
				m_control->push([m = m_current_state](DeviceContext& DC) {
					(*m)(DC); 
				});
			}
			std::swap(m_last_state, m_current_state);
		}
	}

	//********************************************* renderer_context_dx11 *******************************************************
	renderer_context_dx11_ptr renderer_context_dx11::create(Device& con, Implement::context_control_block_ptr bp)
	{
		return new renderer_context_dx11{ con, std::move(bp) };
	}

	void renderer_context_dx11::release() noexcept { delete this; }

	bool renderer_context_dx11::can_request_new_draw() noexcept {
		return Implement::renderer_control::can_request_new_draw();
	}

	void renderer_context_dx11::push_draw_syn()
	{
		Implement::renderer_control::push_draw_syn(renderer_dx11::create_command(), {});
	}
	void renderer_context_dx11::push_draw_asyn()
	{
		Implement::renderer_control::push_draw_asyn(renderer_dx11::create_command(), {});
	}

	renderer_context_dx11::renderer_context_dx11(Device& con, Implement::context_control_block_ptr bp) :
		Graphic::renderer_context(typeid(renderer_context)), Implement::renderer_control(std::move(bp)), renderer_dx11(con)
	{}

	//********************************************* form_context_dx11 *******************************************************
	form_context_dx11_ptr form_context_dx11::create(HWND hwnd, Graphic::FormatRT format, Graphic::uint2 size, Device& dev, Implement::context_control_block_ptr b)
	{
		CComPtr<SwapChain> swap;
		CComPtr<RenderTargetView> view;
		HRESULT re = create_swap_chain(&swap, &view, dev, DXGI::translate(format), hwnd, size.x, size.y);
		if (SUCCEEDED(re))
		{
			auto view_ptr = new render_target_dx11{ ResourceType::Tex2, {}, std::move(view) };
				
				//render_target_dx11::create(Graphic::ResourceType::Tex2, { size.x, size.y, 0 }, {}, std::move(view));
			assert(view_ptr);
			return new form_context_dx11{ hwnd, format, size, dev, std::move(b), std::move(swap), std::move(view_ptr) };
		}
		else
			return {};
	}

	void form_context_dx11::release() noexcept { delete this; }

	bool form_context_dx11::can_request_new_draw() noexcept
	{
		return Implement::renderer_control::can_request_new_draw();
	}

	void form_context_dx11::push_draw_syn()
	{
		Implement::renderer_control::push_draw_syn(renderer_dx11::create_command(), m_swapchain);
	}
	void form_context_dx11::push_draw_asyn()
	{
		Implement::renderer_control::push_draw_asyn(renderer_dx11::create_command(), m_swapchain);
	}

	form_context_dx11::form_context_dx11(
		HWND hwnd, Graphic::FormatRT format, Graphic::uint2 size, Device& dev, Implement::context_control_block_ptr bp, 
		CComPtr<SwapChain> sc, render_target_dx11_ptr view
	)
		: Graphic::form_context(typeid(form_context_dx11)), Implement::renderer_control(std::move(bp)), renderer_dx11(dev), m_swapchain(std::move(sc)), m_back_buffer(std::move(view))
	{
	}

	/*
	const Graphic::renderer_resource* form_context_dx11::find_resource(const char* name, Graphic::ResourceType type, Graphic::Usage usg) const noexcept
	{
		if (std::strcmp(name, "__form_output") == 0 && type == Graphic::ResourceType::Tex2 && usg == Graphic::Usage::RenderTarget)
			return m_back_buffer;
		else
			return {};
	}
	*/

	namespace Implement
	{
		//********************************************* graphic_context_dx11_execute *******************************************************
		int graphic_context_dx11_execute(Implement::context_control_block_ptr control, std::promise<CComPtr<Device>> pro)
		{
			assert(control);
			CComPtr<Device> device;
			CComPtr<DeviceContext> context;
			D3D_FEATURE_LEVEL final_level;
			HRESULT re = create_device(&device, &context, final_level);
			assert(SUCCEEDED(re));
			pro.set_value(std::move(device));
			while (control->is_available())
			{
				bool loop = false;
				while(true)
				{
					if (!control->execute_front(*context))
						break;
				}
				std::this_thread::sleep_for(std::chrono::milliseconds{1});
			};
			return 0;
		}
	}

	//********************************************* graphic_context_dx11 *******************************************************
	UINT usage_to_cpu_access(Graphic::Usage usage)
	{
		switch (usage)
		{
		case Graphic::Usage::Readable:
			return D3D11_CPU_ACCESS_READ;
		default:
			return 0;
		}
	}

	UINT usage_to_texture_bind_flag(Graphic::Usage usage)
	{
		switch (usage)
		{
		case Graphic::Usage::RenderTarget:
			return D3D11_BIND_FLAG::D3D11_BIND_RENDER_TARGET | D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
		case Graphic::Usage::Const:
			return D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
		case Graphic::Usage::DepthStencil:
			return D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_FLAG::D3D11_BIND_DEPTH_STENCIL;
		case Graphic::Usage::Output:
			return D3D11_BIND_FLAG::D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
		default:
			return 0;
		}
	}

	D3D11_USAGE usage_to_usage(Graphic::Usage usage)
	{
		switch (usage)
		{
		case Graphic::Usage::Const:
			return D3D11_USAGE::D3D11_USAGE_IMMUTABLE;
		case Graphic::Usage::Readable:
			return D3D11_USAGE::D3D11_USAGE_STAGING;
		default:
			return D3D11_USAGE::D3D11_USAGE_DEFAULT;
		}
	}

	bool graphic_context_dx11::init()
	{
		bool last_state = Graphic::graphic_context::ins_ptr();
		graphic_context_dx11_ptr ptr = new graphic_context_dx11{};
		Graphic::graphic_context::rebind_context(std::move(ptr));
		return last_state;
	}

	graphic_context_dx11_ptr graphic_context_dx11::ins_ptr()
	{
		auto ptr = Graphic::graphic_context::ins_ptr();
		if (ptr && ptr->is<graphic_context_dx11>())
			return ptr.cast_static<graphic_context_dx11>();
		return {};
	}

	void graphic_context_dx11::release() noexcept
	{
		delete this;
	}

	graphic_context_dx11::graphic_context_dx11() : Graphic::graphic_context(typeid(graphic_context_dx11)){
		m_control = new  Implement::context_control_block{};
		std::promise<CComPtr<Device>> dev_ptr;
		auto fur = dev_ptr.get_future();
		execute_thread = std::thread(
			Implement::graphic_context_dx11_execute,
			m_control,
			std::move(dev_ptr)
		);
		fur.wait();
		m_device = fur.get();
	}

	graphic_context_dx11::~graphic_context_dx11()
	{
		m_control->close();
		m_control.reset();
		m_device = nullptr;
		execute_thread.join();
	}

	Graphic::renderer_context_ptr graphic_context_dx11::create_renderer() {
		return renderer_context_dx11::create(*m_device, m_control);
	}

	Tool::intrusive_ptr<Graphic::form_context> graphic_context_dx11::create_form_context(HWND hwnd, Graphic::uint2 size, Graphic::FormatRT format)
	{
		return form_context_dx11::create(hwnd, format, size, *m_device, m_control);
	}

	Graphic::vertex_resource_ptr graphic_context_dx11::create_vertex()
	{
		return new vertex_dx11{ m_device };
	}

	void graphic_context_dx11::draw_syn(void(* function)(void*, Graphic::renderer& r), void* data) noexcept
	{
		std::promise<void> m_promise;
		auto furture = m_promise.get_future();
		m_control->push([&](DeviceContext& rd) {
			CComPtr<DeviceContext> ptr{ &rd };
			renderer_dx11 rd2{std::move(ptr)};
			(*function)(data, rd2);
			m_promise.set_value();
		});
		furture.wait();
		furture.get();
	}


	void Wang()
	//Graphic::material graphic_context_dx11::create_material(const Tool::path& p)
	{
		/*
		auto target_path = p;
		target_path += PO::Dx::HLSL::mscfb_extension;
		Dx::HLSL::mscfbdx_tech techdx;
		if (Dx::HLSL::load_mscfbdx_tech(techdx, target_path))
		{
			std::map<uint64_t, std::tuple<CComPtr<ShaderVS>, CComPtr<ShaderReflection>>> vsm;
			std::map<uint64_t, std::tuple<CComPtr<ShaderHS>, CComPtr<ShaderReflection>>> hsm;
			std::map<uint64_t, std::tuple<CComPtr<ShaderDS>, CComPtr<ShaderReflection>>> dsm;
			std::map<uint64_t, std::tuple<CComPtr<ShaderGS>, CComPtr<ShaderReflection>>> gsm;
			std::map<uint64_t, std::tuple<CComPtr<ShaderPS>, CComPtr<ShaderReflection>>> psm;
			std::map<uint64_t, std::tuple<CComPtr<ShaderCS>, CComPtr<ShaderReflection>>> csm;
			for (size_t i = 0; i < techdx.all_code.size(); ++i)
			{
				auto code = techdx.all_code[i];
				CComPtr<ShaderReflection> reflection;
				HRESULT re = create_shader_reflection(&reflection, code->code(), code->length());
				assert(SUCCEEDED(re));
				D3D11_SHADER_DESC dec;
				reflection->GetDesc(&dec);
				D3D11_SHADER_VERSION_TYPE type = static_cast<D3D11_SHADER_VERSION_TYPE>(D3D11_SHVER_GET_TYPE(dec.Version));
				switch (type)
				{
				case D3D11_SHVER_VERTEX_SHADER:
				{
					CComPtr<ShaderVS> tem;
					HRESULT re = create_shader(&tem, *m_device, code->code(), code->length());
					assert(SUCCEEDED(re));
					vsm.insert({ i, {std::move(tem), std::move(reflection)} });
					break;
				}
				case D3D11_SHVER_HULL_SHADER:
				{
					CComPtr<ShaderHS> tem;
					HRESULT re = create_shader(&tem, *m_device, code->code(), code->length());
					assert(SUCCEEDED(re));
					hsm.insert({ i,{ std::move(tem), std::move(reflection) } });
					break;
				}
				case D3D11_SHVER_DOMAIN_SHADER:
				{
					CComPtr<ShaderDS> tem;
					HRESULT re = create_shader(&tem, *m_device, code->code(), code->length());
					assert(SUCCEEDED(re));
					dsm.insert({ i,{ std::move(tem), std::move(reflection) } });
					break;
				}
				case D3D11_SHVER_GEOMETRY_SHADER:
				{
					CComPtr<ShaderGS> tem;
					HRESULT re = create_shader(&tem, *m_device, code->code(), code->length());
					assert(SUCCEEDED(re));
					gsm.insert({ i,{ std::move(tem), std::move(reflection) } });
					break;
				}
				case D3D11_SHVER_PIXEL_SHADER:
				{
					CComPtr<ShaderPS> tem;
					HRESULT re = create_shader(&tem, *m_device, code->code(), code->length());
					assert(SUCCEEDED(re));
					psm.insert({ i,{ std::move(tem), std::move(reflection) } });
					break;
				}
				case D3D11_SHVER_COMPUTE_SHADER:
				{
					CComPtr<ShaderCS> tem;
					HRESULT re = create_shader(&tem, *m_device, code->code(), code->length());
					assert(SUCCEEDED(re));
					csm.insert({ i,{ std::move(tem), std::move(reflection) } });
					break;
				}
				default:
					assert(false);
				}
				//switch(dec.)
			}
			Graphic::material tartget{ };
			for (auto& ite : techdx.techs)
			{
				auto tech = tartget.locate_tech(ite.first);
				for (auto& ite2 : ite.second)
				{
					if (std::holds_alternative<Dx::HLSL::mscfbdx_pass>(ite2))
					{
						auto& ref = std::get<Dx::HLSL::mscfbdx_pass>(ite2);
						tech->insert_pass(Implement::pass_dx11::create(*this, vsm, hsm, dsm, gsm, psm, techdx.all_code, ref));
					}
					else if (std::holds_alternative<Dx::HLSL::mscfbdx_compute>(ite2))
					{
						auto& ref = std::get<Dx::HLSL::mscfbdx_compute>(ite2);
						tech->insert_pass(Implement::compute_dx11::create(*this, csm, ref));
					}
				}
			}
			return tartget;
		}
		*/
		return;
	}

	/*
	Graphic::renderer_texture_resource_ptr graphic_context_dx11::create_texture(Graphic::FormatPixel format, Graphic::uint3 size, const std::byte* buffer, Graphic::uint2 surface_count, bool generate_mipmap = false)
	{

	}
	*/

	/*
	Graphic::renderer_resource_ptr graphic_context_dx11::create_texture(
		Graphic::Usage usage, Graphic::FormatPixel view_format, Graphic::uint3 size, 
		const std::byte* buffer, Graphic::uint2 surface_count, bool used_mipmap, 
		Graphic::FormatPixel storage_fomat)
	{
		if (storage_fomat == Graphic::FormatPixel::UNKNOW)
			storage_fomat = view_format;
		uint8_t dim = Graphic::tex_dimension(size);
		switch (dim)
		{
		case 2:
		{
			Graphic::ResourceType type = Graphic::ResourceType::Tex2;
			CComPtr<Texture2D> tex;
			HRESULT re = create_texture2D(
				&tex, *m_device, usage_to_usage(usage), usage_to_texture_bind_flag(usage), usage_to_cpu_access(usage),
				DXGI::translate(storage_fomat), size.x, size.y, used_mipmap ? 0 : 1, &buffer, &surface_count.x);
			if (SUCCEEDED(re))
			{
				switch (usage)
				{
				case Graphic::Usage::Const:
				{
					CComPtr<ShaderResourceView> view;
					Graphic::uint2 mipmap = used_mipmap ? Graphic::uint2{ 0, static_cast<uint32_t>(-1) } : Graphic::uint2{ 0, 1 };
					re = create_texture2D_srv(&view, *m_device, *tex, DXGI::translate(view_format), mipmap.x, mipmap.y);
					if (SUCCEEDED(re))
						return shader_resource_dx11::create(type, Graphic::Usage::Const, size, std::move(view));
					break;
				}
				}
			}
			break;
		}
		}
		return {};
	}
	*/


	/*
	Graphic::renderer_resource_ptr graphic_context_dx11::create_texture(
		Graphic::FormatPixel view_format,
		Graphic::uint3 size,
		const std::byte* buffer,
		Graphic::uint2 surface_count,
		bool used_mipmap,
		Graphic::FormatPixel storage_fomat
		)
	{
		if (storage_fomat == Graphic::FormatPixel::UNKNOW)
			storage_fomat = view_format;
		uint8_t dim = Graphic::tex_dimension(size);
		CComPtr<ShaderResourceView> view;
		Graphic::ResourceType type;
		switch (dim)
		{
		case 2:
		{
			type = Graphic::ResourceType::Tex2;
			CComPtr<Texture2D> tex;
			HRESULT re = create_texture2D(&tex, *m_device, DXGI::translate(storage_fomat), size.x, size.y, used_mipmap ? 0 : 1, &buffer, &surface_count.x);
			if (SUCCEEDED(re))
			{
				Graphic::uint2 mipmap = used_mipmap ? Graphic::uint2{0, static_cast<uint32_t>(-1)} : Graphic::uint2{0, 1};
				re = create_texture2D_srv(&view, *m_device, *tex, DXGI::translate(view_format), mipmap.x, mipmap.y);
				if (SUCCEEDED(re));
				else
					return {};
			}
		}
		default:
			return {};
		}
		return shader_resource_dx11::create(type, Graphic::Usage::Const, size, std::move(view));
	}
	*/

	/*
	Graphic::readable_resource_ptr graphic_context_dx11::capture_resource(const Graphic::renderer_resource& rr)
	{
		CComPtr<Resource> resource;
		CComPtr<Resource> output_resource;
		size_t length;
		DXGI::Format format;
		if (rr.is<render_target_dx11>())
		{
			auto& res = rr.cast_static<render_target_dx11>();
			res.m_view->GetResource(&resource);
		}
		else if (rr.is<shader_resource_dx11>())
		{
			auto& res = rr.cast_static<shader_resource_dx11>();
			res.m_view->GetResource(&resource);
		}
		else {
			return {};
		}
		D3D11_RESOURCE_DIMENSION type;
		resource->GetType(&type);
		switch (type)
		{
		case D3D11_RESOURCE_DIMENSION::D3D11_RESOURCE_DIMENSION_TEXTURE2D:
		{
			CComPtr<Texture2D> result;
			HRESULT re = resource->QueryInterface(&result);
			assert(SUCCEEDED(re));
			D3D11_TEXTURE2D_DESC desc;
			result->GetDesc(&desc);
			format = desc.Format;
			CComPtr<Texture2D> output;
			re = create_texture2D(&output, *m_device, 
				usage_to_usage(Graphic::Usage::Readable), 
				usage_to_texture_bind_flag(Graphic::Usage::Readable), 
				usage_to_cpu_access(Graphic::Usage::Readable), 
				desc.Format, desc.Width, desc.Height, 1, nullptr, nullptr);
			assert(SUCCEEDED(re));
			length = DXGI::calculate_pixel_size(desc.Format) * desc.Width * desc.Height;
			re = output->QueryInterface(&output_resource);
			assert(SUCCEEDED(re));
			break;
		}
		}
		auto ptr = readable_resource_dx11::create(rr.type(), format, rr.size(), length);
		std::promise<void> m_promise;
		auto fur = m_promise.get_future();
		m_control->push(Implement::context_capture_event{ std::move(resource), std::move(output_resource), ptr, std::move(m_promise) });
		fur.wait();
		fur.get();
		return ptr;
	}
	*/
	
	/*
	bool graphic_context_dx11::save(const Graphic::readable_resource& rr, Tool::path p)
	{
		if (rr.is<readable_resource_dx11>())
		{
			auto& ref = rr.cast_static<readable_resource_dx11>();
			switch (ref.type())
			{
			case Graphic::ResourceType::Tex1:
			case Graphic::ResourceType::Tex2:
			{
				DirectX::Image img{ 
					ref.size().x, 
					(ref.size().y == 0 ? 1 : ref.size().y), 
					ref.format(), 
					ref.size().x * DXGI::calculate_pixel_size(ref.format()), 
					ref.size().x * (ref.size().y == 0 ? 1 : ref.size().y) * DXGI::calculate_pixel_size(ref.format()),
					const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(ref.buffer())) };
				HRESULT re = DirectX::SaveToDDSFile(img, 0, p.wstring().c_str());
				if (SUCCEEDED(re))
					return true;
			}
				break;
			}
		}
		return false;
	}

	Graphic::renderer_resource_ptr graphic_context_dx11::create_texture(Graphic::Usage usage, const Tool::path& p, bool generate_mipmap, Graphic::FormatPixel view_format)
	{
		DirectX::TexMetadata md;
		DirectX::ScratchImage image;
		auto exp = p.extension();
		if (exp == ".dds" || exp == ".DDS")
		{
			HRESULT re = DirectX::LoadFromDDSFile(p.wstring().c_str(), 0, &md, image);
			if (!SUCCEEDED(re)) return {};
		}
		else {
			return {};
		}

		Graphic::uint3 size;

		switch (md.dimension)
		{
		case DirectX::TEX_DIMENSION::TEX_DIMENSION_TEXTURE2D:
			size = Graphic::uint3{ static_cast<uint32_t>(md.width), static_cast<uint32_t>(md.height) };
			break;
		case DirectX::TEX_DIMENSION::TEX_DIMENSION_TEXTURE1D:
			size = Graphic::uint3{ static_cast<uint32_t>(md.width) };
			break;
		case DirectX::TEX_DIMENSION::TEX_DIMENSION_TEXTURE3D:
			size = Graphic::uint3{ static_cast<uint32_t>(md.width), static_cast<uint32_t>(md.height), static_cast<uint32_t>(md.depth) };
			break;
		default:
			assert(false);
			return {};
		}

		return graphic_context_dx11::create_texture(usage, DXGI::inversen_translate(md.format), size, 
			reinterpret_cast<const std::byte*>(image.GetImages()->pixels), 
			{ static_cast<uint32_t>(image.GetImages()->rowPitch), static_cast<uint32_t>(image.GetImages()->slicePitch) }, 
			generate_mipmap, view_format);
	}
	*/
}