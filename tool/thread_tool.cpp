#include "thread_tool.h"
#include <random>
namespace PO
{
	namespace Tool
	{
		namespace Assistant
		{

			void completeness_head_data_struct::start_destruct()
			{
				mutex.lock();
				state = completeness_state::Destruction;
				while (read_ref != 0)
				{
					mutex.unlock();
					std::this_thread::yield();
					mutex.lock();
				}
				mutex.unlock();
			}

			bool completeness_head_data_struct::try_add_read_ref()
			{
				std::unique_lock<decltype(mutex)> ul(mutex);
				if (state == completeness_state::Ready)
				{
					++read_ref;
					return true;
				}
				return false;
			}

			bool completeness_head_data_struct::add_read_ref()
			{
				std::unique_lock<decltype(mutex)> ul(mutex);
				if (state == completeness_state::Ready)
				{
					++read_ref;
					return true;
				}
				else if (state != completeness_state::Construction)
				{
					return false;
				}
				else {
					while (state == completeness_state::Construction)
					{
						ul.unlock();
						std::this_thread::yield();
						ul.lock();
					}
					if (state == completeness_state::Ready)
					{
						++read_ref;
						return true;
					}
					else return false;
				}
			}
		}

		void thread_task_operator::main_thread_execute()
		{
			std::chrono::time_point<std::chrono::system_clock> tp = std::chrono::system_clock::now();
			bool empty_call = false;
			while (!exit && !empty_call)
			{
				if (task.lock([&](thread_task_data& ta)
				{
					if (ta.task.empty())
					{
						auto now = std::chrono::system_clock::now();
						if (std::chrono::duration_cast<decltype(ref_duration)>(now - tp) > ref_duration)
						{
							ta.need_joined = true;
							empty_call = true;
							return false;
						}
					}
					std::swap(calling_buffer, ta.task);
					return true;
				}))
				{
					for (auto& f : calling_buffer)
					{
						if (f && f())
							task.lock([&](thread_task_data& ta)
						{
							ta.task.push_back(std::move(f));
						});
					}
					calling_buffer.clear();
					tp = std::chrono::system_clock::now();
				}
				else
					continue;
				std::this_thread::yield();
			}
		}

		void thread_task_operator::add_task(std::function<bool(void)> f)
		{
			if (task.lock([&](thread_task_data& i)
			{
				i.task.push_back(f);
				bool need = i.need_joined;
				i.need_joined = false;
				return need;
			}) && main_thread.joinable())
				main_thread.join();
			if (!main_thread.joinable())
				main_thread = std::thread([this]() {main_thread_execute(); });
		}

	}
}