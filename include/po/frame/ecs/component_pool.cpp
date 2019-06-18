#include "component_pool.h"
namespace PO::ECS::Implement
{
	static constexpr size_t min_page_comp_count = 32;

	bool TypeLayoutArray::operator<(const TypeLayoutArray& input) const noexcept
	{
		if (count < input.count) return true;
		else if (count == input.count)
		{
			for (size_t i = 0; i < count; ++i)
			{
				if (layouts[i] < input[i])
					return true;
				else if (!(layouts[i] == input[i]))
					break;
			}
		}
		return false;
	}

	bool TypeLayoutArray::operator==(const TypeLayoutArray& input) const noexcept
	{
		if (count == input.count)
		{
			for (size_t i = 0; i < count; ++i)
			{
				if (!(layouts[i] == input.layouts[i]))
					return false;
			}
			return true;
		}
		return false;
	}

	StorageBlock* StorageBlock::create(const TypeGroud* owner)
	{
		std::byte* buffer = reinterpret_cast<std::byte*>(owner->allocator().allocate(owner->space()));
		size_t layout_count = owner->layouts().count;
		StorageBlock* result = new (buffer) StorageBlock{};
		result->m_owner = owner;
		buffer += sizeof(StorageBlock);
		Control* control_start = reinterpret_cast<Control*>(buffer);
		buffer += sizeof(Control) * layout_count;
		auto tool = reinterpret_cast<std::tuple<void (*)(void*) noexcept, void (*)(void*, void*)>*>(data);
		std::byte* data = reinterpret_cast<std::byte*>(tool + owner->max_count() * layout_count);
		for (size_t i = 0; i < layout_count; ++i)
		{
			
		}
	}

	void StorageBlock::free(StorageBlock* owner) noexcept;

	TypeGroud* TypeGroud::create(MemoryPageAllocator& allocator, TypeLayoutArray array)
	{
		size_t total_size = sizeof(TypeGroud) + array.count * sizeof(TypeLayout);
		std::byte* data = new std::byte[total_size];
		TypeLayout* layout = reinterpret_cast<TypeLayout*>(data + sizeof(TypeGroud));
		for (size_t i = 0; i < array.count; ++i)
			new (layout + i) TypeLayout{array.layouts[i]};
		TypeLayoutArray layouts{ layout , array.count};
		TypeGroud* result = new (data) TypeGroud{allocator, layouts};
		return result;
	}

	void TypeGroud::free(TypeGroud* input)
	{
		size_t count = input->m_type_layouts.count;
		for (size_t i = 0; i < count; ++i)
			input->m_type_layouts.layouts[i].~TypeLayout();
		input->~TypeGroud();
		delete[] reinterpret_cast<std::byte*>(input);
	}

	TypeGroud::TypeGroud(MemoryPageAllocator& allocator, TypeLayoutArray input)
		: m_allocator(allocator), m_type_layouts(input)
	{
		size_t all_size = 0;
		size_t all_align = 0;
		for (size_t i = 0; i < m_type_layouts.count; ++i)
		{
			all_size += m_type_layouts.layouts[i].size;
			auto align_size = (m_type_layouts.layouts[i].align > alignof(nullptr_t)) ? m_type_layouts.layouts[i].align : alignof(nullptr_t);
			all_align += align_size;
		}
		size_t element_size = all_size + sizeof(EntityInterface*) + sizeof(nullptr_t) * 2;
		size_t min_size = sizeof(StorageBlock) + sizeof(StorageBlock::Control) * m_type_layouts.count + 
			all_align + element_size * min_page_comp_count;
		m_space = m_allocator.recalculate_space(min_size);
		m_max_count = (m_space.space - (sizeof(StorageBlock) + sizeof(StorageBlock::Control) * m_type_layouts.count +
			all_align)) / element_size;
	}


	/*
	ComponentMemoryPageDesc::ComponentMemoryPageDesc(const SimilerComponentPool* pool, size_t component_count) noexcept
		: m_owner(pool), m_layout(pool->layout()), m_component_count(component_count){}

	ComponentMemoryPageDesc* ComponentMemoryPageDesc::allocate(MemoryPageAllocator& allocator, const MemoryPageAllocator::SpaceResult& result, size_t component_count, const SimilerComponentPool* pool)
	{
		std::byte* data = reinterpret_cast<std::byte*>(allocator.allocate(result));
		ComponentMemoryPageDesc* desc = new (data) ComponentMemoryPageDesc{ pool, component_count };
		desc->m_available_count = component_count;
		desc->m_control = reinterpret_cast<ComponentMemoryPageDesc::Control*>(desc + 1);
		for (size_t i = 0; i < component_count; ++i)
			new (desc->m_control + i) ComponentMemoryPageDesc::Control{};
		desc->m_entitys = reinterpret_cast<EntityInterface**>(desc->m_control + component_count);
		for (size_t i = 0; i < component_count; ++i)
			desc->m_entitys[i] = nullptr;
		size_t space = result.space - sizeof(ComponentMemoryPageDesc) -
			(sizeof(ComponentMemoryPageDesc::Control) + sizeof(EntityInterface**)) * component_count;
		void* components = desc->m_entitys + component_count;
		void* re = std::align(desc->m_layout.align, desc->m_layout.size * component_count, components, space);
		assert(re != nullptr);
		desc->m_components = reinterpret_cast<std::byte*>(components);
		return desc;
	}

	std::tuple<ComponentMemoryPageDesc*, ComponentMemoryPageDesc*> ComponentMemoryPageDesc::free_page(MemoryPageAllocator& allocator, const MemoryPageAllocator::SpaceResult& result, ComponentMemoryPageDesc* desc) noexcept
	{
		assert(desc != nullptr);
		auto p = std::make_tuple(desc->m_front_page, desc->m_next_page);
		for (size_t i = 0; i < desc->m_component_count; ++i)
		{
			desc->try_release_component(i);
			desc->m_control[i].~Control();
		}
		desc->~ComponentMemoryPageDesc();
		allocator.release(desc, result);
		return p;
	}

	void ComponentMemoryPageDesc::release_component(size_t index)
	{
		bool re = try_release_component(index);
		assert(re);
	}

	bool ComponentMemoryPageDesc::try_release_component(size_t index)
	{
		assert(index < m_component_count);
		auto& control = m_control[index];
		if (control.owner)
		{
			control.deconstructor(m_components + m_layout.size * index);
			control.deconstructor = nullptr;
			auto& entity = m_entitys[index];
			if (entity != nullptr)
			{
				entity->sub_ref();
				entity = nullptr;
				assert(m_poll_count >= 1);
				--m_poll_count;
			}
			control.owner->remove(m_layout, this, index);
			control.owner.reset();
			assert(m_available_count < m_component_count);
			++m_available_count;
			return true;
		}
		return false;
	}

	size_t ComponentMemoryPageDesc::construct_component(void(*constructor)(void*, void*), void* parameter, EntityInterface* entity, void(*deconstructor)(void*) noexcept)
	{
		assert(m_available_count >= 1);
		for (size_t i = 0; i < m_component_count; ++i)
		{
			auto& control = m_control[i];
			if (!control.owner)
			{
				constructor(m_components + m_layout.size * i, parameter);
				control.deconstructor = deconstructor;
				control.owner = entity;
				--m_available_count;
				return i;
			}
		}
		assert(false);
		return m_component_count;
	}

	std::tuple<ComponentMemoryPageDesc*, size_t> ComponentMemoryPageDesc::update(size_t index)
	{
		assert(index < m_component_count);
		assert(m_control[index].owner);
		assert(m_entitys[index] == nullptr);
		auto& entity = m_entitys[index];
		entity = m_control[index].owner;
		entity->add_ref();
		++m_poll_count;
		ComponentMemoryPageDesc* desc;
		size_t desc_index;
		entity->insert(m_layout, this, index, desc, desc_index);
		return { desc, desc_index };
	}

	std::tuple<MemoryPageAllocator::SpaceResult, size_t> SimilerComponentPool::calculate_space(size_t align, size_t size) noexcept
	{
		size_t aligned_space = (align > sizeof(nullptr) ? align - sizeof(nullptr) : 0);
		size_t require_space = sizeof(ComponentMemoryPageDesc) + aligned_space + (sizeof(ComponentMemoryPageDesc::Control) + sizeof(EntityInterface*) + size) * min_page_comp_count;
		auto result = MemoryPageAllocator::recalculate_space(require_space);
		size_t comp_count = result.space - sizeof(ComponentMemoryPageDesc) - aligned_space;
		comp_count = comp_count / (sizeof(ComponentMemoryPageDesc::Control) + sizeof(EntityInterface*) + size);
		assert(comp_count >= min_page_comp_count);
		return { result, comp_count };
	}

	SimilerComponentPool::SimilerComponentPool(MemoryPageAllocator& allocator, const TypeLayout& layout)
		:m_allocator(allocator),  m_layout(layout), m_component_count(0)
	{
		std::tie(m_page_space, m_component_count) = calculate_space(layout.align, layout.size);
	}

	SimilerComponentPool::~SimilerComponentPool()
	{
		std::lock_guard lg(m_record_mutex);
		std::unique_lock ul(m_poll_mutex);
		while (m_top_page != nullptr)
		{
			auto re = ComponentMemoryPageDesc::free_page(m_allocator, m_page_space, m_top_page);
			m_top_page = std::get<1>(re);
		}
		m_constructed_comps.clear();
		m_need_deleted_comps.clear();
		m_poll_count = 0;
	}

	void SimilerComponentPool::construction_component(EntityInterface* owner, void(*constructor)(void*, void*), void* parameter, void (*deconstructor)(void*) noexcept)
	{
		std::lock_guard lg(m_record_mutex);
		if (m_top_page == nullptr)
			m_top_page = ComponentMemoryPageDesc::allocate(m_allocator, m_page_space, m_component_count, this);
		ComponentMemoryPageDesc* ite = m_top_page;
		while (true)
		{
			if (ite->available_count() != 0)
			{
				size_t index = ite->construct_component(constructor, parameter, owner, deconstructor);
				m_constructed_comps.push_back({ ite, index });
				break;
			}
			else {
				auto last = ite->next_page();
				if (last == nullptr)
				{
					last = ComponentMemoryPageDesc::allocate(m_allocator, m_page_space, m_component_count, this);
					ite->next_page() = last;
					last->front_page() = ite;
				}
				ite = last;
			}
		}
	}

	void SimilerComponentPool::deconstruction(ComponentMemoryPageDesc * desc, size_t index)
	{
		std::lock_guard lg(m_record_mutex);
		m_need_deleted_comps.push_back({ desc, index });
	}

	void SimilerComponentPool::update()
	{
		std::lock_guard lg(m_record_mutex);
		std::unique_lock ul(m_poll_mutex);
		if (!(m_constructed_comps.empty() && m_need_deleted_comps.empty()))
		{
			++m_version;
			for (auto& ite : m_constructed_comps)
			{
				auto [desc, index] = ite;
				auto [remove, index2] = desc->update(index);
				if (remove != nullptr)
					m_need_deleted_comps.push_back({ remove , index2 });
				++m_poll_count;
			}
			m_constructed_comps.clear();
			for (auto& ite : m_need_deleted_comps)
			{
				auto [desc, index] = ite;
				if (desc != nullptr)
				{
					if (desc->try_release_component(index))
					{
						--m_poll_count;
						if (desc->available_count() == m_component_count)
						{
							auto [front, last] = ComponentMemoryPageDesc::free_page(m_allocator, m_page_space, desc);
							if (front != nullptr)
							{
								front->next_page() = last;
								if (last != nullptr)
									last->front_page() = front;
							}
							else {
								assert(m_top_page == desc);
								m_top_page = last;
							}
							for (auto& ite : m_need_deleted_comps)
							{
								auto& [desc2, index2] = ite;
								if (desc2 == desc)
									desc2 = nullptr;
							}
						}
					}
				}
			}
			m_need_deleted_comps.clear();
		}
	}

	std::tuple<size_t, ComponentMemoryPageDesc*, uint64_t> SimilerComponentPool::read_lock(std::shared_lock<std::shared_mutex>* lock) noexcept
	{
		new (lock) std::shared_lock<std::shared_mutex>{m_poll_mutex};
		return { m_poll_count, m_top_page, m_version };
	}

	void SimilerComponentPool::next_desc(ComponentPoolReadWrapper& corw)
	{
		if (corw.page != nullptr)
		{
			corw.page = corw.page->next_page();
			if (corw.page != nullptr)
			{
				corw.available_count = corw.page->poll_count();
				corw.entitys = corw.page->entitys();
				corw.components = corw.page->components();
				return;
			}
		}
		corw.available_count = 0;
		corw.entitys = nullptr;
		corw.components = nullptr;
	}

	bool ComponentPool::lock(ComponentPoolReadWrapper& wrapper, size_t count, const TypeLayout* layout, uint64_t* version, size_t mutex_size, void* mutex)
	{
		assert(sizeof(std::shared_lock<std::shared_mutex>) <= mutex_size);
		std::shared_lock<std::shared_mutex>* ssm = static_cast<std::shared_lock<std::shared_mutex>*>(mutex);
		ComponentMemoryPageDesc* min_desc = nullptr;
		size_t min_count = 0;
		size_t component_size = 0;
		bool need_update = false;
		for (size_t i = 0; i < count; ++i)
		{
			auto pool = find_pool(layout[i]);
			auto [total_count, desc, cur_version] = pool->read_lock(ssm + i);
			if (min_desc == nullptr || min_count > total_count)
			{
				min_desc = desc;
				min_count = total_count;
				component_size = layout[i].size;
			}
			if (cur_version != version[i])
			{
				need_update = true;
				version[i] = cur_version;
			}
		}
		if (min_desc != nullptr)
			wrapper = ComponentPoolReadWrapper{ min_desc, component_size, min_desc->poll_count(), min_desc->entitys(),  min_desc->components(), min_count };
		else
			wrapper = ComponentPoolReadWrapper{ nullptr, component_size, 0, nullptr, nullptr, 0 };
		return need_update;
	}

	void ComponentPool::next(ComponentPoolReadWrapper& wrapper)
	{
		SimilerComponentPool::next_desc(wrapper);
	}

	void ComponentPool::unlock(size_t count, size_t mutex_size, void* mutex) noexcept
	{
		assert(sizeof(std::shared_lock<std::shared_mutex>) <= mutex_size);
		std::shared_lock<std::shared_mutex>* ssm = static_cast<std::shared_lock<std::shared_mutex>*>(mutex);
		for (size_t i = 0; i < count; ++i)
			(ssm + i)->~shared_lock();
	}
	*/

	/*
	ComponentReadWrapperInterface* ComponentPool::lock(const TypeLayout* layout, size_t count) noexcept
	{
		std::shared_lock sl(m_components_mutex);
		size_t align = ((sizeof(nullptr) == sizeof(uint64_t)) ? 0 : sizeof(uint64_t) - sizeof(nullptr));
		std::byte* buffer = new std::byte[sizeof(ComponentReadWrapper) + sizeof(std::shared_lock<std::shared_mutex>) * count + align + sizeof(uint64_t) * count];
		ComponentReadWrapper* wrapper = new (buffer) ComponentReadWrapper{};
		std::byte* last = (buffer + sizeof(ComponentReadWrapper));
		wrapper->m_lock_start = reinterpret_cast<std::shared_lock<std::shared_mutex>*>(last);
		size_t space;
		void* tem = last + sizeof(std::shared_lock<std::shared_mutex>*) * count;
		auto re = std::align(alignof(uint64_t), sizeof(uint64_t) * count + align, tem, space);
		assert(re != nullptr);
		wrapper->m_version_start = reinterpret_cast<uint64_t*>(tem);
		size_t min_count = std::numeric_limits<size_t>::max();
		for (size_t index = 0; index < count; ++index)
		{
			auto ite = m_components.find(layout[index]);
			if (ite != m_components.end())
			{
				auto [count, pool_ite] = ite->second.read_lock(wrapper->m_version_start + index, wrapper->m_lock_start + index);
				if (count < min_count)
				{
					min_count = count;
					wrapper->m_ite = pool_ite;
					wrapper->m_poll_component_count = count;
				}
				++wrapper->m_info_count;
			}
			else {
				for (size_t index = wrapper->m_info_count; index > 0; --index)
					wrapper->m_lock_start[index - 1].~shared_lock();
				wrapper->~ComponentReadWrapper();
				delete[] reinterpret_cast<std::byte*>(wrapper);
				return nullptr;
			}
		}
		return wrapper;
	}

	void ComponentPool::unlock(ComponentReadWrapperInterface* in) noexcept
	{
		std::shared_lock sl(m_components_mutex);
		ComponentReadWrapper* wrapper = reinterpret_cast<ComponentReadWrapper*>(in);
		for (size_t index = wrapper->m_info_count; index > 0; --index)
			wrapper->m_lock_start[index - 1].~shared_lock();
		wrapper->~ComponentReadWrapper();
		delete[] reinterpret_cast<std::byte*>(wrapper);
	}
	*/
	
/*
	void ComponentPool::construct_component(const TypeLayout& layout, void(*constructor)(void*, void*), void* data, EntityInterface* in, void(*deconstructor)(void*) noexcept)
	{
		auto* pool = find_pool(layout);
		pool->construction_component(static_cast<EntityImp*>(in), constructor, data, deconstructor);
	}

	ComponentPool::ComponentPool(MemoryPageAllocator& allocator) noexcept
		: m_allocator(allocator) {}

	ComponentPool::~ComponentPool()
	{
		std::unique_lock ul(m_components_mutex);
		m_components.clear();
	}

	void ComponentPool::clean_all()
	{
		std::unique_lock sl(m_components_mutex);
		m_components.clear();
	}

	void ComponentPool::update()
	{
		std::unique_lock sl(m_components_mutex);
		for (auto& ite : m_components)
			ite.second.update();
	}

	SimilerComponentPool* ComponentPool::find_pool(const TypeLayout & layout)
	{
		std::shared_lock sl(m_components_mutex);
		auto ite = m_components.find(layout);
		if (ite == m_components.end())
		{
			sl.unlock();
			{
				std::unique_lock ul(m_components_mutex);
				auto re = m_components.emplace(std::piecewise_construct, std::forward_as_tuple(layout), std::forward_as_tuple(m_allocator, layout));
				ite = re.first;
			}
			sl.lock();
		}
		return &ite->second;
	}

	bool ComponentPool::deconstruct_component(EntityInterface* in, const TypeLayout& layout) noexcept
	{
		assert(in != nullptr);
		auto owner = static_cast<EntityImp*>(in);
		ComponentMemoryPageDesc* desc;
		size_t index;
		owner->read(layout, desc, index);
		if (desc != nullptr)
		{
			auto ite = find_pool(layout);
			ite->deconstruction(desc, index);
			return true;
		}
		else {
			return false;
		}
	}
	*/
}