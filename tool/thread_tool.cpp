#include "thread_tool.hpp"
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
				else return false;
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
	}
}