#include "frame.h"
namespace PO
{
	namespace Implement
	{
		void thread_task_runer::thread_funtion()
		{
			while (!exit)
			{
				{
					std::lock_guard<std::mutex> lg(input_mutex);
					std::swap(input, calling);
				}
				for (auto& task : calling)
				{
					auto task_ptr = task.lock();
					task_ptr && task_ptr->ref.lock_if
					(
						[this, &task_ptr, &task]()
					{
						std::lock_guard<std::mutex> lg(task_ptr->task_mutex);
						if ((*task_ptr)())
						{
							std::lock_guard<std::mutex> lg(input_mutex);
							input.push_back(task);
						}
						else {
							task_ptr->cv.notify_one();
							task_ptr->task_state = thread_task::State::FINISH;
						}	
					}
					);
				}
				calling.clear();
				std::this_thread::yield();
			}
		}

		thread_task_runer::thread_task_runer() : exit(false)
		{
			main = std::thread(&thread_task_runer::thread_funtion, this);
		}

		thread_task_runer::~thread_task_runer()
		{
			exit = true;
			main.join();
		}

		bool thread_task_runer::push_task(std::weak_ptr<thread_task> task)
		{
			auto ta = task.lock();
			if (ta && ta->task_state == thread_task::State::READY)
			{
				ta->task_state = thread_task::State::WAITING;
				std::lock_guard<std::mutex> lg(input_mutex);
				input.push_back(std::move(task));
				return true;
			}
			return false;
		}
	}
}