#include "component_pool.h"
#include "..//..//tool/tool.h"
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

	bool TypeLayoutArray::hold(const TypeLayout* input, size_t index) const noexcept
	{
		if (count >= index)
		{
			size_t i = 0, k = 0;
			while (i < count && k < index)
			{
				if (layouts[i] == input[k])
					++i, ++k;
				else if (layouts[i] < input[k])
					++i;
				else
					return false;
			}
			return k == index;
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
	size_t TypeLayoutArray::locate(const TypeLayout& input) const noexcept
	{
		for (size_t index = 0; index < count; ++index)
		{
			if (!(layouts[index] < input))
			{
				if (layouts[index] == input)
					return index;
				else
					return count;
			}
		}
		return count;
	}

	bool TypeLayoutArray::locate(const TypeLayout* input, size_t* output, size_t length) const noexcept
	{
		if (count >= length)
		{
			size_t i = 0, k = 0;
			while (i < count && k < length)
			{
				if (layouts[i] == input[k])
				{
					output[k] = i;
					++i, ++k;
				}
				else if (layouts[i] < input[k])
					++i;
				else
					return false;
			}
			return k == length;
		}
		return false;
	}

	StorageBlock* create_storage_block(MemoryPageAllocator& allocator, const TypeGroup* owner)
	{
		auto [buffer, page_size] = allocator.allocate(owner->page_size());
		assert(page_size == owner->page_size());
		size_t element_count = owner->element_count();
		size_t layout_count = owner->layouts().count;
		StorageBlock* result = new (buffer) StorageBlock{};
		result->m_owner = owner;
		buffer += sizeof(StorageBlock);


		Control* control_start = reinterpret_cast<Control*>(buffer);
		buffer += sizeof(Control) * layout_count;
		page_size -= sizeof(StorageBlock) + sizeof(Control) * layout_count;
		for (size_t i = 0; i < layout_count; ++i)
		{
			auto tool = reinterpret_cast<Control::FunctionType*>(buffer);
			page_size -= sizeof(Control::FunctionType) * element_count;
			void* tem_buffer = tool + element_count;
			auto& layout = owner->layouts()[i];
			auto result = std::align(layout.align, layout.size * element_count, tem_buffer, page_size);
			assert(result != nullptr);
			new (control_start + i) Control{ tem_buffer, tool};
			buffer = reinterpret_cast<std::byte*>(tem_buffer);
			buffer += layout.size * element_count;
			page_size -= layout.size * element_count;
		}
		result->entitys = reinterpret_cast<EntityInterface**>(buffer);
		for (size_t i = 0; i < element_count; ++i)
			new (result->entitys + i) EntityInterface** {nullptr};
		return result;
	}

	void StorageBlock::free(StorageBlock* input) noexcept
	{
		assert(input != nullptr);
		input->~StorageBlock();
		MemoryPageAllocator::release(reinterpret_cast<std::byte*>(input));
	}

	StorageBlock::~StorageBlock()
	{
		for (size_t i = 0; i < m_owner->layouts().count; ++i)
		{
			auto& control = controls[i];
			auto& layouts = m_owner->layouts()[i];
			for (size_t k = 0; k < available_count; ++k)
				std::get<0>(*(control.function_start + k))(reinterpret_cast<std::byte*>(control.data_start) + layouts.size * k);
			control.~Control();
		}
		for (size_t k = 0; k < available_count; ++k)
		{
			auto& entity = entitys[k];
			if (entity != nullptr)
			{
				entity->set(nullptr, nullptr, 0);
				entity->sub_ref();
				entity = nullptr;
			}
		}
			
	}

	void TypeGroup::remove_page_from_list(StorageBlock* block)
	{
		auto front = block->front;
		auto next = block->next;
		if (front != nullptr)
			front->next = next;
		else
			m_start_block = next;
		if (next != nullptr)
			next->front = front;
		else
			m_last_block = front;
		block->next = nullptr;
		block->front = nullptr;
	}

	void TypeGroup::insert_page_to_list(StorageBlock* block)
	{
		if (m_start_block == nullptr)
		{
			m_start_block = block;
			m_last_block = block;
		}
		else {
			m_last_block->next = block;
			block->front = m_last_block;
			m_last_block = block;
		}
	}

	void StorageBlock::release_element(size_t index)
	{
		for (size_t i = 0; i < m_owner->layouts().count; ++i)
		{
			auto& control = controls[i];
			auto& layouts = m_owner->layouts()[i];
			std::get<0>(*(control.function_start + index))(reinterpret_cast<std::byte*>(control.data_start) + layouts.size * index);
		}
		auto& entity = entitys[index];
		if (entity != nullptr)
		{
			entity->set(nullptr, nullptr, 0);
			entity->sub_ref();
			entity = nullptr;
		}
	}

	std::tuple<StorageBlock*, size_t> TypeGroup::allocate_group(MemoryPageAllocator& allocator)
	{
		if (!m_deleted_page.empty())
		{
			auto min = m_deleted_page.end();
			for (auto ite = m_deleted_page.begin(); ite != m_deleted_page.end(); ++ite)
			{
				if (ite->second == 1)
				{
					StorageBlock* block = ite->first;
					for (size_t i = 0; i < block->available_count; ++i)
					{
						if (block->entitys[i] == nullptr)
						{
							m_deleted_page.erase(ite);
							return { block, i };
						}
					}
					assert(false);
				}else if (ite != m_deleted_page.end())
				{
					if (min->second > ite->second)
						min = ite;
				}
				else
					min = ite;
			}
			min->second -= 1;
			return *min;
		}
		else {
			if (m_start_block == nullptr || m_last_block->available_count == element_count())
				insert_page_to_list(StorageBlock::create(allocator, this));
			size_t index = m_last_block->available_count;
			++m_last_block->available_count;
			return { m_last_block , index};
		}
		
	}

	void TypeGroup::inside_move(StorageBlock* source, size_t sindex, StorageBlock* target, size_t tindex)
	{
		for (size_t i = 0; i < m_type_layouts.count; ++i)
		{
			auto& s_control = source->controls[i];
			auto& e_control = target->controls[i];
			size_t component_size = m_type_layouts[i].size;
			auto& func = s_control.function_start[sindex];
			func = e_control.function_start[tindex];
			std::get<1>(func)(reinterpret_cast<std::byte*>(s_control.data_start) + component_size * sindex, reinterpret_cast<std::byte*>(e_control.data_start) + component_size * tindex);
		}
		source->entitys[sindex] = target->entitys[tindex];
		target->entitys[tindex] = nullptr;
		source->entitys[sindex]->set(this, source, sindex);
		target->release_element(tindex);
	}

	void TypeGroup::release_group(StorageBlock* block, size_t index)
	{
		assert(index < element_count());
		block->release_element(index);
		if (block == m_last_block && index + 1 == block->available_count)
			--block->available_count;
		else {
			auto ite = m_deleted_page.insert({ block, 0 }).first;
			++ite->second;
			if (block->available_count == ite->second)
			{
				remove_page_from_list(ite->first);
				block->available_count = 0;
				StorageBlock::free(ite->first);
				m_deleted_page.erase(ite);
			}
		}
	}

	void TypeGroup::update()
	{
		// todo list
		if (!m_deleted_page.empty())
		{
			assert(m_last_block != nullptr);
			std::deque<std::pair<StorageBlock*, size_t>> all_block;
			size_t last_page_deleted = 0;
			for (auto& ite : m_deleted_page)
			{
				if (ite.first != m_last_block)
				{
					remove_page_from_list(ite.first);
					all_block.push_back(ite);
				}
				else {
					assert(last_page_deleted == 0);
					last_page_deleted = ite.second;
				}
			}
			m_deleted_page.clear();
			last_page_deleted += element_count() - m_last_block->available_count;
			m_last_block->available_count = element_count();
			
			if (last_page_deleted != 0) {
				all_block.push_back({ m_last_block , last_page_deleted });
				remove_page_from_list(m_last_block);
			}
			std::sort(all_block.begin(), all_block.end(), [](const std::tuple<StorageBlock*, size_t>& in, const std::tuple<StorageBlock*, size_t>& in2) -> bool {
				return std::get<1>(in) < std::get<1>(in2);
			});
			assert(!all_block.empty());
			size_t start_i = 0, end_i = element_count();
			while (all_block.size() > 1)
			{
				auto start = all_block.begin();
				auto end = all_block.end() - 1;
				while (true)
				{
					while (start_i < element_count())
					{
						if (start->first->entitys[start_i] != nullptr)
							++start_i;
						else
							break;
					}
					if (start_i == element_count())
					{
						start_i = 0;
						insert_page_to_list(start->first);
						all_block.pop_front();
						break;
					}
					while (end_i > 0)
					{
						if (end->first->entitys[end_i - 1] == nullptr)
							++end_i;
						else
							break;
					}
					if (end_i == 0)
					{
						end_i = element_count();
						end->first->available_count = 0;
						StorageBlock::free(end->first);
						all_block.pop_back();
						break;
					}
					inside_move(start->first, start_i, end->first, end_i);
				}
			}
			auto cur = all_block.begin();
			while (start_i < end_i)
			{
				while (start_i < end_i)
				{
					if (cur->first->entitys[start_i] != nullptr)
						++start_i;
					else
						break;
				}
				while (end_i > start_i)
				{
					if (cur->first->entitys[end_i - 1] == nullptr)
						++end_i;
					else
						break;
				}
				if (start_i > end_i)
					break;
				else
					inside_move(cur->first, start_i, cur->first, end_i);
			}
			cur->first->available_count = start_i;
			insert_page_to_list(cur->first);
			all_block.clear();
		}
	}

	TypeGroup* TypeGroup::create(TypeLayoutArray array)
	{
		size_t total_size = sizeof(TypeGroup) + array.count * sizeof(TypeLayout);
		std::byte* data = new std::byte[total_size];
		TypeLayout* layout = reinterpret_cast<TypeLayout*>(data + sizeof(TypeGroup));
		for (size_t i = 0; i < array.count; ++i)
			new (layout + i) TypeLayout{array.layouts[i]};
		TypeLayoutArray layouts{ layout , array.count};
		TypeGroup* result = new (data) TypeGroup{layouts};
		return result;
	}

	void TypeGroup::free(TypeGroup* input)
	{
		assert(input != nullptr);
		auto layouts = input->layouts();
		input->~TypeGroup();
		for (size_t i = 0; i < layouts.count; ++i)
			layouts[i].~TypeLayout();
		delete[] reinterpret_cast<std::byte*>(input);
	}

	TypeGroup::~TypeGroup()
	{
		while (m_start_block != nullptr)
		{
			auto tem = m_start_block;
			m_start_block = m_start_block->next;
			StorageBlock::free(tem);
		}
	}

	TypeGroup::TypeGroup(TypeLayoutArray input)
		: m_type_layouts(input)
	{
		size_t all_size = 0;
		size_t all_align = 0;
		for (size_t i = 0; i < m_type_layouts.count; ++i)
		{
			all_size += m_type_layouts.layouts[i].size;
			auto align_size = (m_type_layouts.layouts[i].align > alignof(nullptr_t)) ? m_type_layouts.layouts[i].align - alignof(nullptr_t) : 0;
			all_align += align_size;
		}
		size_t element_size = all_size + sizeof(EntityInterface*) + sizeof(StorageBlockFunctionPair);
		size_t fixed_size = sizeof(StorageBlock) + (sizeof(StorageBlockFunctionPair*) + sizeof(void*)) * m_type_layouts.count + all_align;
		size_t min_size = fixed_size + element_size * min_page_comp_count;
		size_t bound_size = 1024 * 8 - MemoryPageAllocator::reserved_size();
		min_size = (min_size > bound_size) ? min_size : bound_size;
		std::tie(min_size, std::ignore) = MemoryPageAllocator::pre_calculte_size(min_size);
		m_element_count = (min_size - fixed_size) / element_size;
	}

	ComponentPool::InitBlock::~InitBlock()
	{
		assert(start_block != nullptr);
		MemoryPageAllocator::release(start_block);
	}

	ComponentPool::InitHistory::~InitHistory()
	{
		if (is_construction)
		{
			assert(data != nullptr);
			std::get<0>(functions)(data);
		}
	}

	void ComponentPool::construct_component(
		const TypeLayout& layout, void(*constructor)(void*, void*), void* data,
		EntityInterface* entity, void(*deconstructor)(void*) noexcept, void(*mover)(void*, void*) noexcept
	)
	{
		std::lock_guard lg(m_init_lock);
		size_t aligned_size = layout.align > sizeof(nullptr_t) ? layout.align - sizeof(nullptr_t) : 0;
		if (m_init_block.empty() || m_init_block.rbegin()->last_available_count < aligned_size + layout.size)
		{
			size_t allocate_size = 1024 * 16 - MemoryPageAllocator::reserved_size();
			assert(allocate_size > aligned_size + layout.size);
			auto [buffer, size] = m_allocator.allocate(allocate_size);
			m_init_block.push_back({ buffer, buffer, size });
		}

		assert(m_init_block.empty());
		auto& [head, last, size] = *m_init_block.rbegin();
		auto result = std::align(layout.align, layout.size, last, size);
		assert(result != nullptr);
		assert(entity != nullptr);
		constructor(last, data);
		EntityInterfacePtr ptr(entity);
		m_init_history[ptr].emplace_back(true, layout, StorageBlock::Control::FunctionType{deconstructor, mover}, last );
		last = reinterpret_cast<std::byte*>(last) + layout.size;
	}

	bool ComponentPool::deconstruct_component(EntityInterface* entity, const TypeLayout& layout) noexcept
	{
		assert(entity != nullptr);
		std::lock_guard lg(m_init_lock);
		EntityInterfacePtr ptr(entity);
		m_init_history[ptr].emplace_back(false, layout, StorageBlock::Control::FunctionType{nullptr, nullptr}, nullptr);
	}

	ComponentPool::ComponentPool(MemoryPageAllocator& allocator) noexcept : m_allocator(allocator){}

	void ComponentPool::clean()
	{
		std::lock_guard lg(m_init_lock);
		m_init_history.clear();
		m_init_block.clear();
		std::unique_lock ul(m_type_group_mutex);
		for (auto& ite : m_data)
			TypeGroup::free(ite.second);
		m_data.clear();
	}

	void ComponentPool::update()
	{
		std::lock_guard lg(m_init_lock);
		std::unique_lock ul(m_type_group_mutex);
		for (auto& ite : m_init_history)
		{
			Implement::TypeGroup* old_type_group;
			Implement::StorageBlock* old_storage_block;
			size_t old_element_index;
			ite.first->read(old_type_group, old_storage_block, old_element_index);
			m_old_type_template.clear();
			if (old_type_group != nullptr)
			{
				assert(old_storage_block != nullptr);
				assert(old_element_index < old_type_group->element_count());
				for (size_t i = 0; i < old_type_group->layouts().count; ++i)
					m_old_type_template.insert({ old_type_group->layouts()[i], i });
			}
			for (auto& ite2 : ite.second)
			{
				if (ite2.is_construction)
					m_old_type_template[ite2.type] = &ite2;
				else
					m_old_type_template.erase(ite2.type);
			}
			if (!m_old_type_template.empty())
			{
				m_new_type_template.clear();
				m_new_type_state_template.clear();
				for (auto& ite2 : m_old_type_template)
				{
					m_new_type_template.push_back(ite2.first);
					m_new_type_state_template.push_back(ite2.second);
				}
				auto find_result = m_data.find({ m_new_type_template.data(), m_new_type_template.size() });
				if (find_result == m_data.end())
				{
					TypeGroup* ptr = TypeGroup::create({ m_new_type_template.data(), m_new_type_template.size() });
					auto re = m_data.insert({ ptr->layouts(), ptr });
					assert(re.second);
					find_result = re.first;
				}
				if (find_result->second == old_type_group)
				{
					m_state_template.clear();
					m_state_template.resize(find_result->first.count, false);
					for (auto ite2 = ite.second.rbegin(); ite2 != ite.second.rend(); ++ite2)
					{
						if (ite2->is_construction)
						{
							size_t type_index = old_type_group->layouts().locate(ite2->type);
							assert(type_index < m_state_template.size());
							if (!m_state_template[type_index])
							{
								auto control_ptr = old_storage_block->controls + type_index;
								auto& control = control_ptr[old_element_index];
								auto data = reinterpret_cast<std::byte*>(control.data_start) + m_new_type_template[type_index].size * old_element_index;
								std::get<0>(control.function_start[old_element_index])(reinterpret_cast<std::byte*>(data));
								std::get<1>(ite2->functions)(data, ite2->data);
								control.function_start[old_element_index] = ite2->functions;
								m_state_template[type_index] = true;
							}
						}
					}
				}
				else {
					auto [new_block, new_element_index] = find_result->second->allocate_group(m_allocator);
					for (size_t i = 0; i < m_new_type_template.size(); ++i)
					{
						auto control = new_block->controls + i;
						auto& var = m_new_type_state_template[i];
						std::visit(Tool::overloaded{
							[&](size_t target_index) {
							assert(old_type_group != nullptr);
							auto source_control = old_storage_block->controls + target_index;
							control->function_start[new_element_index] = source_control->function_start[old_element_index];
							std::get<1>(control->function_start[new_element_index])(
								reinterpret_cast<std::byte*>(control->data_start) + m_new_type_template[i].size * new_element_index,
								reinterpret_cast<std::byte*>(source_control->data_start) + m_new_type_template[i].size * old_element_index
								);
						},
							[&](InitHistory* source) {
							assert(source->is_construction);
							control->function_start[new_element_index] = source->functions;
							std::get<1>(source->functions)(reinterpret_cast<std::byte*>(control->data_start) + m_new_type_template[i].size * new_element_index, source->data);
						}
							}, var);
					}
					new_block->entitys[new_element_index] = ite.first;
					ite.first->set(find_result->second, new_block, new_element_index);
					if (old_type_group != nullptr)
					{
						old_storage_block->entitys[old_element_index] = nullptr;
						old_type_group->release_group(old_storage_block, old_element_index);
					}

				}
			}
			else {
				if (old_type_group != nullptr)
				{
					old_type_group->release_group(old_storage_block, old_element_index);
				}
			}
		}
		m_init_block.clear();
		m_init_history.clear();
		for (auto& ite : m_data)
			ite.second->update();
	}

	ComponentPool::~ComponentPool()
	{
		clean();
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