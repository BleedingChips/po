#pragma once
#include "../po/tool/intrusive_ptr.h"
#include "./event.h"
#include "./format.h"
#include "../po/tool/asset.h"
#include <vector>
#include <map>
#include <typeindex>
#include <Windows.h>
#include <filesystem>
namespace PO::Graphic
{
	enum class PrimitiveType
	{
		Point,
		Line,
		Triangle,
	};

	enum class ResourceType
	{
		Tex1,
		Tex2,
		Tex2MS,
		Tex3,
		Cube,
		Buffer,
		Vertex,
		StreamOut
	};

	enum class Usage
	{
		RenderTarget,
		DepthStencil,
		Output,
		Const,
		Readable
	};

	namespace Implement
	{
		struct raw_resource : Tool::intrusive_object_base
		{
			std::type_index id() const noexcept { return m_id; }
			template<typename T> bool is() const noexcept { return id() == typeid(T); }
			template<typename T> T& cast_static() noexcept { assert(is<T>());  return static_cast<T&>(*this); }
			template<typename T> const T& cast_static() const noexcept { assert(is<T>());  return static_cast<const T&>(*this); }
		protected:
			raw_resource(std::type_index id) : m_id(id) {}
			virtual void release() noexcept { delete this; }
			virtual ~raw_resource() = default;
		private:
			std::type_index m_id;
		};
	}

	struct renderer_resource : Implement::raw_resource
	{
		ResourceType type() const noexcept { return m_type; }
		Usage usage() const noexcept { return m_usage; }
	protected:
		renderer_resource(std::type_index id, ResourceType type, Usage usage) :Implement::raw_resource(id), m_type(type), m_usage(usage) {}
	private:
		ResourceType m_type;
		Usage m_usage;
	};

	using renderer_resource_ptr = Tool::intrusive_ptr<renderer_resource>;

	struct texture_resource;
	using texture_resource_ptr = Tool::intrusive_ptr<texture_resource>;

	struct view_resource : renderer_resource
	{
		virtual renderer_resource_ptr resource() const noexcept = 0;
	protected:
		view_resource(std::type_index id, ResourceType type, Usage usage) : renderer_resource(id, type, usage) {  }
	};

	using view_resource_ptr = Tool::intrusive_ptr<view_resource>;

	struct texture_resource : Implement::raw_resource
	{
		uint3 size() const noexcept { return m_size; }
		FormatPixel format() const noexcept { return m_format; }
		uint mimap_count() const noexcept { return m_mipmap; }
		Usage usage() const noexcept { return m_usage; }
		virtual view_resource_ptr as_view() const = 0;
	protected:
		texture_resource(std::type_index id, Usage usage, FormatPixel format, uint3 size) :
			Implement::raw_resource{ id }, m_usage(usage), m_format(format), m_size(size){}
	private:
		Usage m_usage;
		FormatPixel m_format;
		uint3 m_size;
		uint m_mipmap;
	};
	
	struct vertex_resource;
	using vertex_resource_ptr = Tool::intrusive_ptr<vertex_resource>;

	struct vertex_resource : Implement::raw_resource
	{
		virtual vertex_resource_ptr clone() = 0;
		virtual size_t add_vertex(size_t count, std::initializer_list<std::tuple<FormatPixel, const char*>> semantic, const std::byte* buffer) = 0;
		virtual bool set_index(size_t count, FormatPixel format, const std::byte* buffer) = 0;
	protected:
		using Implement::raw_resource::raw_resource;
	private:
	};

	//struct renderer_context;

	struct resource_map
	{
		//static resource_binding_ptr create() { return new resource_binding{typeid(void)}; }
		virtual const typename vertex_resource* find_vertex(const char*) const noexcept { return nullptr; }
		virtual const typename view_resource* find_view(const char*, ResourceType type, Usage usage) const noexcept { return nullptr; }
		virtual uint3 find_compute_count(const char* s) const { return { 0, 0, 0 }; };
		//virtual ~resource_binding() = default;
		std::type_index id() const noexcept { return m_id; }
		template<typename T> bool is() const noexcept { return m_id == typeid(T); }
		template<typename T> T& cast() noexcept { assert(is<T>()); return static_cast<T&>(*this); }
		template<typename T> const T& cast() const noexcept { assert(is<T>()); return static_cast<const T&>(*this); }
		resource_map(std::type_index id = typeid(void)) : m_id(id) {}
	private:
		std::type_index m_id;
	};

	/* material_resource_map */
	struct material_resource_map;
	using material_resource_map_ptr = Tool::intrusive_ptr<material_resource_map>;

	struct material_resource_map : resource_map, Implement::raw_resource
	{
		virtual material_resource_map_ptr clone() const = 0;
	protected:
		using Implement::raw_resource::raw_resource;
	private:
	};

	struct renderer
	{
		renderer(std::type_index id) : m_id(id) {}
		virtual bool clear_render_target(float4, const view_resource& vr) = 0;
		//virtual void capture_resource(readable_resource&, renderer_resource&) = 0;
		std::type_index id() const noexcept { return m_id; }
		template<typename T> bool is() const noexcept { return m_id == typeid(T); }
		template<typename T> T& cast_static() noexcept { assert(is<T>()); return static_cast<T&>(*this); }
		template<typename T> const T& cast_static() const noexcept { assert(is<T>()); return static_cast<const T&>(*this); }
	private:
		std::type_index m_id;
	};

	struct graphic_context;

	struct pass_cache;
	using pass_cache_ptr = Tool::intrusive_ptr<pass_cache>;

	struct pass_cache : Implement::raw_resource
	{
		using Implement::raw_resource::raw_resource;
		virtual pass_cache_ptr clone() const = 0;
	};

	struct pass : Tool::intrusive_object_base
	{
		virtual bool apply(uint32_t index, const pass_cache& pc, graphic_context&, renderer& rc, const resource_map& self, const resource_map& gobal) const = 0;
	};

	using pass_ptr = Tool::intrusive_ptr<pass>;

	/*  tech  */
	struct tech;
	using tech_ptr = Tool::intrusive_ptr<tech>;

	struct tech final : Tool::intrusive_object<tech>
	{
		static tech_ptr create() { return new tech{}; }
		bool apply(const pass_cache& pass_cache, graphic_context& gc, renderer& rc, const resource_map& self, const resource_map& gobal) const
		{
			
			for (size_t index = 0; index < m_pass_map.size(); ++index)
			{
				if (!m_pass_map[index]->apply(static_cast<uint32_t>(index), pass_cache, gc, rc, self, gobal))
					return false;
			}
			return true;
		}
		void insert_pass(pass_ptr p) { m_pass_map.push_back(std::move(p)); }
		//void release() noexcept;
	private:
		tech() = default;
		std::vector<pass_ptr> m_pass_map;
		//tech(std::vector<Tool::intrusive_ptr<pass>> passs) : m_pass_map(std::move(passs)) {}
	};

	using tech_ptr = Tool::intrusive_ptr<tech>;

	struct tech_wrapper
	{
		Tool::as_const<tech_ptr> m_tech;
		//Tool::as_const<pass_cache_ptr> m_resource;
		operator bool() const noexcept { return m_tech; }
		bool apply(graphic_context& gc, renderer& rc, const resource_map& gobal);
	};

	struct material
	{
		material() = default;
		material(const material& m) : m_techs_map(m.m_techs_map), m_self_resource(m.m_self_resource->clone()) {}
		material(material&&) = default;
		//material(property_map_ptr p) : m_self_resource(std::move(p)) { assert(m_self_resource); }

		operator bool() const noexcept { return m_self_resource; }
		tech_ptr locate_tech(const std::string& s);
		//void insert_pass(pass_ptr p);
		tech_wrapper find_tech(const std::string&) const noexcept;
		void replace_resource(material_resource_map_ptr p) { m_self_resource = std::move(p); };
		template<typename call_function> bool operator<<(call_function&& func)
		{
			using type = Tmp::pick_func<Tmp::degenerate_func_t<Tmp::extract_func_t<call_function>>>;
			static_assert(type::size == 1);
			using result = typename type::template out<Tmp::itself>::type;
			if (m_self_resource && m_self_resource->is<result>())
			{
				std::forward<call_function>(func)(m_self_resource->cast<result>());
				return true;
			}
			return false;
		}
	private:
		std::map<std::string, Tool::intrusive_ptr<tech>> m_techs_map;
		material_resource_map_ptr m_self_resource;
		//bool apply_tech(const std::string&, renderer_context& rc, const resource_binding& self, const resource_binding& gobal);
	};

	//using material_ptr = Tool::intrusive_ptr<material>;

	struct renderer_context : Implement::raw_resource
	{
		// void(renderer&)
		template<typename command_function> bool draw_asyn(command_function&& function)
		{
			if (can_request_new_draw())
			{
				std::forward<command_function>(function)(as_renderer());
				push_draw_asyn();
				return true;
			}
			return false;
		}

		template<typename command_function> void draw_syn(command_function&& function)
		{
			std::forward<command_function>(function)(as_renderer());
			push_draw_syn();
		}
	protected:
		using Implement::raw_resource::raw_resource;
	private:
		virtual renderer& as_renderer() noexcept = 0;
		virtual bool can_request_new_draw() noexcept = 0;
		virtual void push_draw_asyn() = 0;
		virtual void push_draw_syn() = 0;
	};

	using renderer_context_ptr = Tool::intrusive_ptr<renderer_context>;

	struct form_context : renderer_context
	{
		virtual const view_resource* back_buffer() const noexcept = 0;
	protected:
		using Graphic::renderer_context::renderer_context;
	};

	using form_context_ptr = Tool::intrusive_ptr<form_context>;

	/*
	struct vertexs_description
	{
		std::vector<std::tuple<const std::byte*, std::vector<std::tuple<FormatPixel, const char*>>>> m_buffer;
		uint32_t m_vertex_count = 0;
		const std::byte* m_index = nullptr;
		FormatPixel m_index_format = FormatPixel::UNKNOW;
		uint32_t m_index_count = 0;
		void add_vertex(uint32_t count, std::vector<std::tuple<FormatPixel, const char*>> semantic, const std::byte* buffer);
		void set_index(uint32_t count, FormatPixel index_format, const std::byte* buffer);
	};
	*/

	/*
	struct instances_description
	{
		// buffer£¬ buffer_count, instance_effect_count, {format, semciel}
		std::vector<std::tuple<const std::byte*, uint32_t, uint32_t, std::vector<std::tuple<FormatPixel, const char*>>>> m_buffer;
		uint32_t m_instance_count = 0;
		void add_instance(uint32_t count, std::vector<std::tuple<FormatPixel, const char*>> semantic, const std::byte* buffer, uint32_t instance_effext = 1);
	};
	*/

	struct graphic_context : Implement::raw_resource
	{
		static graphic_context& ins();
		static Tool::intrusive_ptr<graphic_context>& ins_ptr() noexcept;
		virtual renderer_context_ptr create_renderer() = 0;
#ifdef _WIN32
		virtual form_context_ptr create_form_context(HWND, uint2 size, FormatRT format) = 0;
#endif
		template<typename RenderFunction> void draw_syn(RenderFunction RF)
		{
			void* data = &RF;
			draw_syn([](void* data, renderer& r) {
				(*reinterpret_cast<RenderFunction*>(data))(r);
			}, data);
		}
		//virtual material create_material(const Tool::path& p) = 0;

		//virtual bool add_vertex(vertex_resource_ptr& rr, std::vector<std::tuple<const char*, FormatPixel>>, const std::byte* buffer, size_t count) = 0;
		//virtual bool set_index(vertex_resource_ptr& rr, FormatPixel format, const std::byte* buffer, size_t count) = 0;
		virtual vertex_resource_ptr create_vertex() = 0;
		//virtual void frame_immediately_and_wait

		/*
		virtual renderer_resource_ptr create_vertex(const vertexs_description&) = 0;
		virtual readable_resource_ptr load_texture(Tool::)
		*/
		//virtual renderer_texture_resource_ptr create_texture(FormatPixel format, Graphic::uint3 size, const std::byte* buffer, uint2 surface_count, bool generate_mipmap = false) = 0;
		//virtual renderer_resource_ptr create_texture(FormatPixel format, Graphic::uint3 size, const std::byte* buffer, uint2 surface_count, bool used_mipmap = true) = 0;
		//virtual renderer_resource_ptr load_texture(const Tool::path& p, bool used_mipmap = true, FormatPixel overwrite_format = FormatPixel::UNKNOW) = 0;
		//virtual renderer_resource_ptr create_texture(const Tool::path&, bool used_mipmap, FormatPixel view_format = FormatPixel::UNKNOW) = 0;
		//virtual readable_resource_ptr capture_resource(const renderer_resource&) = 0;
		//virtual bool save(const readable_resource&, Tool::path) = 0;
		//virtual renderer_resource_ptr create_texture(const Tool::path&, bool used_mipmap = true, FormatPixel overwrite_format = FormatPixel::UNKNOW) = 0;
		//virtual renderer_resource_ptr create_render_target_texture(FormatPixel fomat, uint3 size, const std::byte* buffer);
	protected:
		static bool rebind_context(Tool::intrusive_ptr<graphic_context> ptr) noexcept;
		using Implement::raw_resource::raw_resource;
		virtual void draw_syn(void(*)(void*, renderer& r), void*) noexcept = 0;
	private:
	};

}