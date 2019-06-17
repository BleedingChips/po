#include "tool.h"
#include <iostream>

namespace PO::Tool
{
	bool atomic_reference_count::try_add_ref() noexcept
	{
		auto oldValue = ref.load(std::memory_order_relaxed);
		assert(static_cast<std::ptrdiff_t>(oldValue) >= 0);
		do
		{
			if (oldValue == 0)
			{
				return false;
			}
		} while (!ref.compare_exchange_strong(oldValue, oldValue + 1, std::memory_order_relaxed, std::memory_order_relaxed));
		return true;
	}

	void atomic_reference_count::wait_touch(size_t targe_value) const noexcept
	{
		while (auto oldValue = ref.load(std::memory_order_relaxed))
		{
			assert(static_cast<std::ptrdiff_t>(oldValue) >= 0);
			if (oldValue == targe_value)
				break;
		}
	}

	namespace Implement
	{
		/*
		void stack_ref_control_block::start_destruction() noexcept
		{
			state.store(State::Destruction, std::memory_order::memory_order_release);
			using_count.wait_touch(0);
		}

		void stack_ref_control_block::try_add_using_ref(bool& avalible, bool& need_wait) noexcept
		{
			using_count.add_ref();
			State current_state = state.load(std::memory_order::memory_order_acquire);
			if (current_state == State::Ready)
			{
				avalible = true;
				need_wait = false;
			}
			else
			{
				avalible = false;
				need_wait = (current_state == State::Construction);
				using_count.sub_ref();
			}
		}

		void stack_ref_control_block::sub_using_ref() noexcept
		{
			using_count.sub_ref();
		}

		stack_ref_head::stack_ref_head() : block(new stack_ref_control_block{})
		{
			assert(block);
			block->start_construction();
		}

		stack_ref_head::~stack_ref_head()
		{
			block->finish_destruction();
		}
		*/

	}

}