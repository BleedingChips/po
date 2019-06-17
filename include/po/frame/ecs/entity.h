#pragma once
#include "interface.h"
#include <shared_mutex>
#include <map>
namespace PO::ECS::Implement
{
	struct ComponentMemoryPageDesc;

	struct EntityImp : EntityInterface
	{

		virtual const TypeLayout& layout() const noexcept override;
		virtual bool read_imp(const TypeLayout* id_list, void** output_list, size_t count) const noexcept override;
		virtual bool remove(const TypeLayout& id, ComponentMemoryPageDesc* desc, size_t index) noexcept override;
		virtual void insert(const TypeLayout& id, ComponentMemoryPageDesc* desc, size_t index, ComponentMemoryPageDesc*& output_desc, size_t& output_index) override;
		virtual void read(const TypeLayout& id, ComponentMemoryPageDesc*& desc, size_t& index) const noexcept override;
		virtual void remove_all(ComponentPoolInterface&) noexcept override;
		virtual void add_ref() const noexcept override;
		virtual void sub_ref() const noexcept override;

		static Tool::intrusive_ptr<EntityImp> create_one();

	private:

		EntityImp() = default;

		mutable std::shared_mutex m_comps_mutex;
		std::map<TypeLayout, std::tuple<ComponentMemoryPageDesc*, size_t>> m_components;
		mutable Tool::atomic_reference_count m_ref;
	};

	using EntityImpPtr = Tool::intrusive_ptr<EntityImp>;
}