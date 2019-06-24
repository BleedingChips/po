#include "context.h"
namespace PO::Graphic
{
	Tool::intrusive_ptr<graphic_context> static_context_ins;

	graphic_context& graphic_context::ins()
	{
		return *static_context_ins;
	}

	Tool::intrusive_ptr<graphic_context>& graphic_context::ins_ptr() noexcept
	{
		return static_context_ins;
	}

	bool graphic_context::rebind_context(Tool::intrusive_ptr<graphic_context> ptr) noexcept
	{
		bool exist = ptr;
		static_context_ins = std::move(ptr);
		return exist;
	}

	bool tech_wrapper::apply(graphic_context& gc, renderer& rc, const resource_map& gobal)
	{
		resource_map nullptr_map(typeid(void));
		assert(m_tech);
		/*
		if (m_resource)
			return m_tech->apply(gc, rc, *m_resource, gobal);
		else
			return m_tech->apply(gc, rc, nullptr_map, gobal);
			*/
		return true;
	}

	/*
	Tool::intrusive_ptr<tech> tech::create(std::vector<Tool::intrusive_ptr<pass>> p)
	{
		return new tech{std::move(p)};
	}

	void tech::release() noexcept 
	{ 
		delete this; 
	}

	bool tech::apply(graphic_context& gc, renderer_context& rc, resource_binding& self, resource_binding& gobal)
	{
		for (auto& ite : m_pass_map)
		{
			if (!ite->apply(gc, rc, self, gobal))
				return false;
		}
		return true;
	}
	

	bool tech_wrapper::apply(graphic_context& gc, renderer_context& rc, resource_binding& rb)
	{
		if (m_tech)
		{
			assert(m_resource);
			return m_tech->apply(gc, rc, *m_resource, rb);
		}
		return false;
	}
	*/

	tech_ptr material::locate_tech(const std::string& s)
	{
		auto ite = m_techs_map.find(s);
		if (ite != m_techs_map.end())
			return ite->second;
		else {
			return m_techs_map.insert({ s, tech::create() }).first->second;
		}
	}

	tech_wrapper material::find_tech(const std::string& name) const noexcept
	{
		auto ite = m_techs_map.find(name);
		if (ite != m_techs_map.end())
			return { ite->second/*, m_self_resource*/ };
		return {};
	}

	/*
	void vertexs_description::add_vertex(uint32_t count, std::vector<std::tuple<FormatPixel, const char*>> semantic, const std::byte* buffer)
	{
		if (m_vertex_count == 0)
			m_vertex_count = count;
		else
			m_vertex_count = (m_vertex_count < count ? m_vertex_count : count);
		m_buffer.push_back({ buffer, std::move(semantic) });
	}

	void vertexs_description::set_index(uint32_t count, FormatPixel index_format, const std::byte* buffer)
	{
		m_index_count = count;
		m_index_format = index_format;
		m_index = buffer;
	}
	*/

	/*
	void instances_description::add_instance(uint32_t count, std::vector<std::tuple<FormatPixel, const char*>> semantic, const std::byte* buffer, uint32_t instance_effect)
	{
		uint32_t final_count = count * instance_effect;
		if (m_instance_count == 0)
			m_instance_count = final_count;
		else
			m_instance_count = (m_instance_count < count ? m_instance_count : count);
		m_buffer.push_back({ buffer, instance_effect, std::move(semantic) });
	}
	*/
	
}
