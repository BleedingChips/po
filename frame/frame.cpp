#include "frame.h"
namespace PO
{
	namespace Implement
	{










		/*
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
						task_ptr->task_state = thread_task::State::RUNNING;
						if ((*task_ptr)())
						{
							task_ptr->task_state = thread_task::State::WAITING;
							std::lock_guard<std::mutex> lg(input_mutex);
							input.push_back(task);
						}
						else {
							task_ptr->task_state = thread_task::State::FINISH;
							task_ptr->cv.notify_one();
						}	
					}
					);
				}
				calling.clear();
				std::this_thread::yield();
			}
			
		}*/
	/*
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

		file_io_task_runner io_runner;

		void raw_scene_data::load(std::initializer_list<request_t> il)
		{
			auto re = Tool::lock_scope_look(
				request_list, store_mapping,
				[&, this](decltype(request_list)::type& rl, decltype(store_mapping)::type& sm)
			{
				size_t all_size = il.size();
				rl.reserve(rl.size() + all_size);
				for (auto& lptr : il)
				{
					auto ptr = sm.find(lptr.type);
					if (ptr != sm.end())
					{
						auto ptr2 = ptr->second.find(lptr.path);
						if (ptr2 == ptr->second.end())
							rl.push_back(lptr);
					}
				}
				return il.size() != 0;
			}
			);
			if (re)
				start_task();
		}

		void raw_scene_data::load(request_t type)
		{
			auto re = Tool::lock_scope_look(
				request_list, store_mapping,
				[&, this](decltype(request_list)::type& rl, decltype(store_mapping)::type& sm)
			{
					auto ptr = sm.find(type.type);
					if (ptr == sm.end())
					{
						ptr = sm.insert({ type.type, std::unordered_map<std::u16string, store_t> {} }).first;
						ptr->second.insert({ type.path, store_t{Tool::any{}, type.path} });
						rl.push_back(type);
						return true;
					}
					if (ptr != sm.end())
					{
						auto ptr2 = ptr->second.find(type.path);
						if (ptr2 == ptr->second.end())
						{
							rl.push_back(type);
							return true;
						}
					}
					return false;
			}
			);
			if (re)
				start_task();
		}

		void raw_scene_data::start_task()
		{
			if (!task_ptr) task_ptr = std::make_shared<task_operator>(ref, this);
			if (task_ptr->set_ready())
				io_runner.push_task(task_ptr);
		}

		bool raw_scene_data::task_operator::operator()()
		{
			rsd->request_list.lock(
				[this](auto& i)
			{
				std::swap(request_list, i);
			}
			);

			if (request_list.empty()) return false;
			for (auto& wo : request_list)
			{
				if (
					rsd->store_mapping.lock(
						[&, this](store_map_t& map)
				{
					auto ptr = map.find(wo.first);
					return ptr == map.end();
				}
					)
					)
				{

				}
			}
			request_list.clear();
			return true;
		}*/
	}

	std::shared_ptr<Tool::any> raw_scene::find(std::type_index ti, std::u16string path)
	{
		store_map.lock(
			[&, this](decltype(store_map)::type& map)
		{
			auto& var = map[ti][path];
			if (var)
			{
				//todo
			}
			else if (var.able_cast<std::future<Tool::any>>())
			{
				auto ptr = std::make_shared<Tool::any>(var.cast<std::future<Tool::any>>().get());
				var = ptr;
			}
		}
		);
	}
}