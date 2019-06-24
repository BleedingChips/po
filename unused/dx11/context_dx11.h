#pragma once
#include "define_dx11.h"
//#include "../interface/material.h"
#include "../po/tool/asset.h"
#include "../dx/hlsl.h"
#include "../graphic/context.h"
//#include "implement_dx11.h"
#include <shared_mutex>
#include <map>
#include <deque>
#include <future>
namespace PO::Dx11
{
	using namespace PO::Graphic;
	//********************************************* shader_resource_dx11 *******************************************************
	struct shader_resource_view_dx11;
	using shader_resource_view_dx11_ptr = Tool::intrusive_ptr<shader_resource_view_dx11>;

	struct shader_resource_view_dx11 : view_resource
	{
		CComPtr<ShaderResourceView> m_srv;
		renderer_resource_ptr resource() const noexcept override { return {}; }
		shader_resource_view_dx11(ResourceType resource_type, Usage usage, CComPtr<ShaderResourceView> view) :
			view_resource(typeid(shader_resource_view_dx11), resource_type, usage), m_srv(std::move(view)) {}
	};

	//********************************************* render_target_dx11 *******************************************************
	struct render_target_dx11;
	using render_target_dx11_ptr = Tool::intrusive_ptr<render_target_dx11>;

	struct render_target_dx11 : view_resource
	{
		CComPtr<ShaderResourceView> m_srv;
		CComPtr<RenderTargetView> m_rtv;
		renderer_resource_ptr resource() const noexcept override { return {}; }
		render_target_dx11(ResourceType resource_type, CComPtr<ShaderResourceView> srv, CComPtr<RenderTargetView> rtv) :
			view_resource(typeid(render_target_dx11), resource_type, Graphic::Usage::RenderTarget), m_srv(std::move(srv)), m_rtv(std::move(rtv)) {}
	};

	//********************************************* vertex_buffer_dx11 *******************************************************
	struct vertex_dx11;
	using vertex_dx11_ptr = Tool::intrusive_ptr<vertex_dx11>;

	struct vertex_dx11 : vertex_resource
	{
		vertex_dx11(CComPtr<Device> device) :vertex_resource(typeid(vertex_dx11)), m_device(std::move(device)) {}
		vertex_dx11(const vertex_dx11& vd) = default;
		vertex_dx11(vertex_dx11&&) = default;
		virtual size_t add_vertex(size_t count, std::initializer_list<std::tuple<FormatPixel, const char*>> semantic, const std::byte* buffer) override;
		virtual vertex_resource_ptr clone() override;
		bool apply_and_call(DeviceContext& con) const;
		virtual bool set_index(size_t count, FormatPixel format, const std::byte* buffer) override;
		const D3D11_INPUT_ELEMENT_DESC* semantic() const noexcept { return m_semantic.data(); }
		UINT semantic_length() const noexcept { return static_cast<UINT>(m_semantic.size()); }
		vertex_dx11() : vertex_resource(typeid(vertex_dx11)) {}
	private:
		CComPtr<Device> m_device;
		std::vector<std::tuple<CComPtr<Buffer>, UINT>> m_vertex_buffer;
		uint32_t m_vertex_count;
		std::vector<D3D11_INPUT_ELEMENT_DESC> m_semantic;
		CComPtr<Buffer> m_index_buffer;
		DXGI::Format m_index_format = DXGI::Format::DXGI_FORMAT_UNKNOWN;
		uint32_t m_index_count = 0;
	};

	//********************************************* readable_resource_dx11 *******************************************************
	//struct readable_resource_dx11;
	//using readable_resource_dx11_ptr = Tool::intrusive_ptr<readable_resource_dx11>;

	/*
	struct readable_resource_dx11 : Graphic::readable_resource
	{
		static readable_resource_dx11_ptr create(Graphic::ResourceType type, DXGI::Format format, Graphic::uint3 size, size_t length);
		void release() noexcept override;
		virtual size_t pixel_space() const noexcept override { return DXGI::calculate_pixel_size(m_format); }
		virtual const std::byte* buffer() const noexcept { return reinterpret_cast<const std::byte*>(this + 1); }
		std::byte* buffer() noexcept { return reinterpret_cast<std::byte*>(this + 1); }
		size_t buffer_length() const noexcept override { return m_length; }
		DXGI::Format format() const noexcept { return m_format; }
	private:
		readable_resource_dx11(Graphic::ResourceType type, DXGI::Format format, Graphic::uint3 size, size_t length) noexcept;
		DXGI::Format m_format;
		size_t m_length;
	};
	*/

	namespace Implement
	{

		//********************************************* context_control_block *******************************************************
		struct context_control_block : Tool::intrusive_object<context_control_block>
		{

			using function_type = std::function<void(DeviceContext&)>;

			//void push_back(renderer_delegate_block_ptr p);
			bool execute_front(DeviceContext&);
			void close();
			bool is_available() noexcept;
			context_control_block() : m_available(true) {}
			void push(function_type&& t);

		private:

			std::atomic_bool m_available;
			std::mutex m_list_mutex;
			std::deque<function_type> m_list;
		};

		using context_control_block_ptr = Tool::intrusive_ptr<context_control_block>;

	}

	//********************************************* renderer_dx11 *******************************************************
	struct renderer_dx11 : Graphic::renderer
	{
		renderer_dx11(Device& dev);
		renderer_dx11(CComPtr<DeviceContext> con);
		operator DeviceContext& () { return *m_device; }
		DeviceContext* operator->() { return m_device; }
		virtual bool clear_render_target(float4, const view_resource& vr) override;
	protected:
		CComPtr<CommandList> create_command() noexcept;
		//renderer_dx11(CComPtr<DeviceContext> C) : m_deffer_device(std::move(C)) {}
	private:
		CComPtr<DeviceContext> m_device;
	};

	namespace Implement
	{
		//********************************************* pass_dx11 *******************************************************
		/*
		struct pass_dx11;
		using pass_dx11_ptr = Tool::intrusive_ptr<pass_dx11>;

		struct pass_dx11 : Graphic::pass
		{
			static pass_dx11_ptr create(Device&,
				const std::map<uint64_t, std::tuple<CComPtr<ShaderVS>, CComPtr<ShaderReflection>>>&,
				const std::map<uint64_t, std::tuple<CComPtr<ShaderHS>, CComPtr<ShaderReflection>>>&,
				const std::map<uint64_t, std::tuple<CComPtr<ShaderDS>, CComPtr<ShaderReflection>>>&,
				const std::map<uint64_t, std::tuple<CComPtr<ShaderGS>, CComPtr<ShaderReflection>>>&,
				const std::map<uint64_t, std::tuple<CComPtr<ShaderPS>, CComPtr<ShaderReflection>>>&,
				const std::vector<Dx::HLSL::shader_code_ptr>& all_code,
				const Dx::HLSL::mscfbdx_pass&);
			void release() noexcept override;

			bool apply(uint32_t index, const pass_cache& pc, graphic_context&, renderer& rc, const resource_map& self, const resource_map& gobal) const override;

		private:
			pass_dx11() = default;
			std::string m_ia;
			CComPtr<ShaderVS> m_vs;
			CComPtr<ShaderReflection> m_vs_reflection;
			Dx::HLSL::shader_code_ptr m_vs_code;
			CComPtr<ShaderHS> m_hs;
			CComPtr<ShaderReflection> m_hs_reflection;
			CComPtr<ShaderDS> m_ds;
			CComPtr<ShaderReflection> m_ds_reflection;
			CComPtr<ShaderGS> m_gs;
			CComPtr<ShaderReflection> m_gs_reflection;
			CComPtr<ShaderPS> m_ps;
			CComPtr<ShaderReflection> m_ps_reflection;
			std::vector<std::string> m_render_target;
			std::string m_depth_stencil;
			std::string m_stream_out;
		};

		//********************************************* compute_dx11 *******************************************************
		struct compute_dx11;
		using compute_dx11_ptr = Tool::intrusive_ptr<compute_dx11>;

		struct compute_dx11 : Graphic::pass
		{
			static compute_dx11_ptr create(Device&, const std::map<uint64_t, std::tuple<CComPtr<ShaderCS>, CComPtr<ShaderReflection>>>&, const Dx::HLSL::mscfbdx_compute&);
			void release() noexcept override;
			bool apply(uint32_t index, const pass_cache& pc, graphic_context&, renderer& rc, const resource_map& self, const resource_map& gobal) const override
			{
				return false;
			}
		private:
			compute_dx11() = default;
			std::string di;
			CComPtr<ShaderCS> m_cs;
			CComPtr<ShaderReflection> m_cs_reflect;
		};
		*/




		//********************************************* render_command *******************************************************
		enum class RenderCommandState
		{
			Done,
			Waiting
		};
		
		struct render_command
		{
			std::mutex m_mutex;
			RenderCommandState m_state = RenderCommandState::Done;
			CComPtr<CommandList> m_command;
			CComPtr<SwapChain> m_swap;
			void operator()(DeviceContext& DC);
		};


		//********************************************* renderer_control *******************************************************
		struct renderer_control
		{
			renderer_control(Implement::context_control_block_ptr p);
			bool can_request_new_draw() noexcept;
			void push_draw_syn(CComPtr<CommandList>, CComPtr<SwapChain>) noexcept;
			void push_draw_asyn(CComPtr<CommandList>, CComPtr<SwapChain>) noexcept;
		private:
			Implement::context_control_block_ptr m_control;
			std::shared_ptr<render_command> m_current_state;
			std::shared_ptr<render_command> m_last_state;
		};
	}

	//********************************************* renderer_context_dx11 *******************************************************
	struct renderer_context_dx11;
	using renderer_context_dx11_ptr = Tool::intrusive_ptr<renderer_context_dx11>;

	struct renderer_context_dx11 : Graphic::renderer_context, Implement::renderer_control, renderer_dx11
	{
		static renderer_context_dx11_ptr create(Device& con, Implement::context_control_block_ptr bp);
	protected:
		virtual bool can_request_new_draw() noexcept override;
		virtual void push_draw_syn() override;
		virtual void push_draw_asyn() override;
		void release() noexcept;
	private:
		Graphic::renderer& as_renderer() noexcept override { return *this; }
		renderer_context_dx11(Device& con, Implement::context_control_block_ptr bp);
	};


	//********************************************* form_context_dx11 *******************************************************
	struct form_context_dx11;
	using form_context_dx11_ptr = Tool::intrusive_ptr<form_context_dx11>;

	struct form_context_dx11 : Graphic::form_context, Implement::renderer_control, renderer_dx11
	{
		static form_context_dx11_ptr create(HWND hwnd, Graphic::FormatRT format, Graphic::uint2 size, Device& dev, Implement::context_control_block_ptr bp);
		
		virtual bool can_request_new_draw() noexcept override;
		virtual void push_draw_syn() override;
		virtual void push_draw_asyn() override;
		virtual const view_resource* back_buffer() const noexcept override { return m_back_buffer; }
		//const Graphic::renderer_resource* find_resource(const char* name, Graphic::ResourceType type, Graphic::Usage usg) const noexcept override;
		Graphic::renderer& as_renderer()noexcept override { return *this; }
	protected:
		void release() noexcept override;
	private:
		CComPtr<SwapChain> m_swapchain;
		form_context_dx11(HWND hwnd, Graphic::FormatRT format, Graphic::uint2 size, Device& dev, Implement::context_control_block_ptr bp, CComPtr<SwapChain> sc, render_target_dx11_ptr view);
		render_target_dx11_ptr m_back_buffer;
	};

	//********************************************* graphic_context_dx11 *******************************************************
	struct graphic_context_dx11;
	using graphic_context_dx11_ptr = Tool::intrusive_ptr<graphic_context_dx11>;

	struct graphic_context_dx11 : Graphic::graphic_context
	{
		static bool init();
		static graphic_context_dx11_ptr ins_ptr();
		void release() noexcept override;
		operator Device& () noexcept { return *m_device; }
	private:
		CComPtr<Device> m_device;
		graphic_context_dx11();
		~graphic_context_dx11();
		std::thread execute_thread;
		Implement::context_control_block_ptr m_control;
		
		Graphic::renderer_context_ptr create_renderer() override;
		Tool::intrusive_ptr<Graphic::form_context> create_form_context(HWND hwnd, Graphic::uint2 size, Graphic::FormatRT format) override;

		vertex_resource_ptr create_vertex() override;
		//Graphic::renderer_texture_resource_ptr create_texture(Graphic::FormatPixel format, Graphic::uint3 size, const std::byte* buffer, Graphic::uint2 surface_count, bool generate_mipmap = false);
		//Graphic::renderer_resource_ptr create_texture(Graphic::Usage usage, Graphic::FormatPixel view_format, Graphic::uint3 size, const std::byte* buffer = nullptr, Graphic::uint2 surface_count = { 0, 0 }, bool used_mipmap = true, Graphic::FormatPixel storage_fomat = Graphic::FormatPixel::UNKNOW) override;
		//virtual Graphic::renderer_resource_ptr create_texture(Graphic::FormatPixel view_format, Graphic::uint3 size, const std::byte* buffer, Graphic::uint2 surface_count, bool used_mipmap, Graphic::FormatPixel storage_fomat) override;
		//virtual material create_material(const Tool::path& p) override;
		//virtual Graphic::readable_resource_ptr capture_resource(const Graphic::renderer_resource&) override;
		
		//virtual Graphic::renderer_resource_ptr create_texture(Graphic::Usage, const Tool::path&, bool used_mipmap, Graphic::FormatPixel view_format = Graphic::FormatPixel::UNKNOW, Graphic::FormatPixel overwrite_format = Graphic::FormatPixel::UNKNOW) override;
		//virtual bool save(const Graphic::readable_resource&, Tool::path) override;
		virtual void draw_syn(void (*function)(void*, renderer&), void*) noexcept override;
	};

	//********************************************* init_graphic_context_dx11 *******************************************************
	void init_graphic_context_dx11();

	inline Graphic::uint adject_compute_count(Graphic::uint input, Graphic::uint target)
	{
		return input / target + ((input % target == 0) ? 0 : 1);
	}

	inline Graphic::uint3 adject_compute_count(Graphic::uint3 input, Graphic::uint3 target)
	{
		return Graphic::uint3{
		adject_compute_count(input.x, target.x),
		adject_compute_count(input.y, target.y),
		adject_compute_count(input.z, target.z)
		};
	}

}