#include "entity.h"
#include "component_pool.h"
namespace PO::ECS::Implement
{
	const TypeLayout& EntityImp::layout() const noexcept { return TypeLayout::create<EntityImp>(); }

	bool EntityImp::read_imp(const TypeLayout* id_list, void** output_list, size_t count) const noexcept
	{
		std::shared_lock sl(m_comps_mutex);
		for (size_t i = 0; i < count; ++i)
		{
			auto ite = m_components.find(id_list[i]);
			if (ite == m_components.end())
			{
				output_list[i] = nullptr;
				return false;
			}
			else
				output_list[i] = std::get<0>(ite->second)->components() + std::get<1>(ite->second) * ite->first.size;
		}
		return true;
	}

	bool EntityImp::remove(const TypeLayout& id, ComponentMemoryPageDesc* desc, size_t index) noexcept
	{
		std::unique_lock ul(m_comps_mutex);
		auto ite = m_components.find(id);
		if (ite != m_components.end())
		{
			auto tuple = std::make_tuple(desc, index);
			if (ite->second == tuple)
			{
				m_components.erase(ite);
				return true;
			}
		}
		return false;
	}

	void EntityImp::insert(const TypeLayout& id, ComponentMemoryPageDesc* desc, size_t index, ComponentMemoryPageDesc*& output_desc, size_t& output_index)
	{
		std::unique_lock ul(m_comps_mutex);
		auto result = m_components.insert({ id, {desc, index} });
		if (!result.second)
		{
			auto re = result.first->second;
			result.first->second = { desc, index };
			std::tie(output_desc, output_index) = re;
			return;
		}
		output_desc = nullptr;
		output_index = 0;
		return;
	}

	void EntityImp::read(const TypeLayout& id, ComponentMemoryPageDesc*& desc, size_t& index) const noexcept
	{
		std::shared_lock ul(m_comps_mutex);
		auto ite = m_components.find(id);
		if (ite != m_components.end())
			std::tie(desc, index) = ite->second;
		else {
			desc = nullptr;
			index = 0;
		}
	}

	void EntityImp::add_ref() const noexcept
	{
		m_ref.add_ref();
	}

	void EntityImp::sub_ref() const noexcept
	{
		if (m_ref.sub_ref())
			delete this;
	}

	void EntityImp::remove_all(ComponentPoolInterface& con) noexcept
	{
		std::shared_lock ul(m_comps_mutex);
		for (auto& ite : m_components)
			con.deconstruct_component(this, ite.first);
	}

	Tool::intrusive_ptr<EntityImp> EntityImp::create_one()
	{
		return new EntityImp{};
	}
}