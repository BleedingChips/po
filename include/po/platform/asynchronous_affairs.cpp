#include "asynchronous_affairs.h"
#include <iostream>
namespace PO::Platform
{

	std::function<void(void)> asynchronous_affairs::pop_front()
	{
		return affairs_list.lock([](decltype(affairs_list)::type& t) -> std::function<void(void)>{
			auto ite = t.begin();
			if (ite != t.end())
			{
				auto f = std::move(*ite);
				t.pop_front();
				return f;
			}
			return {};
		});
	}

	void asynchronous_affairs::insert(std::function<void(void)> f)
	{
		affairs_list.lock([&](decltype(affairs_list)::type& t){
			t.push_back(std::move(f));
		});
	}

	void thread_pool::execute(std::promise<thread_state*>& p)
	{
		thread_state state;
		bool avalible = true;
		std::function<bool(void)> current_function;
		std::mutex wait_mutex;
		bool sleep = true;
		p.set_value(&state);

		while (avalible)
		{

			{
				std::lock_guard<decltype(state.function_mutex)> ld(state.function_mutex);
				avalible = state.avalible;
				if (state.main_function.has_value())
				{
					current_function = std::move(state.main_function.value());
					state.main_function = std::nullopt;
				}
			}
			
			if (current_function)
				sleep = current_function();

			if (sleep && avalible)
			{
				std::unique_lock<decltype(wait_mutex)> up(wait_mutex);
				cv.wait(up);
			}
			
		}
	}

	std::thread::id thread_pool::create_thread(std::function<bool(void)> f)
	{
		std::promise<thread_state*> promise;
		auto fur = promise.get_future();
		std::thread temporary([&, this]() {
			execute(promise);
		});
		fur.wait();
		auto ptr = fur.get();
		auto id = temporary.get_id();
		thread_map.insert(decltype(thread_map)::value_type{ id, std::pair<std::thread, thread_state*>{std::move(temporary), ptr} });
		std::lock_guard<decltype(ptr->function_mutex)> ld(ptr->function_mutex);
		ptr->main_function = std::move(f);
		return id;
	}

	bool thread_pool::replace_thread_function(std::thread::id id, std::function<bool(void)> f)
	{
		auto ite = thread_map.find(id);
		if (ite != thread_map.end())
		{
			auto ptr = ite->second.second;
			std::lock_guard<decltype(ptr->function_mutex)> ld(ptr->function_mutex);
			ptr->main_function = std::move(f);
			return true;
		}
		return false;
	}

	void thread_pool::clear_implement()
	{
		for (auto& ite : thread_map)
		{
			auto ptr = ite.second.second;
			std::lock_guard<decltype(ptr->function_mutex)> ld(ptr->function_mutex);
			ptr->avalible = false;
		}
		cv.notify_all();
		for (auto& ite : thread_map)
		{
			assert(ite.second.first.joinable());
			ite.second.first.join();
		}
	}

	void thread_pool::clear()
	{
		clear_implement();
		thread_map.clear();
	}



	/*
	void asynchronous_affairs::asynchronous_affairs_execution(uint8_t current_index)
	{
		while (current_index < target_affair_count)
		{
			std::function<void(void)> current_affairs;
			job_tank.lock([&](decltype(job_tank)::type& t) {
				if (!t.first.empty())
				{
					current_affairs = std::move(*t.first.begin());
					t.first.pop_front();
				}
				else if (!t.second.empty())
				{
					current_affairs = std::move(*t.second.begin());
					t.second.pop_front();
				}
			});
			if (current_affairs)
				std::cout << "thread:" << int(current_index) << std::endl, current_affairs();
			else
				std::this_thread::sleep_for(std::chrono::milliseconds{ 100 });
		}
	}

	void asynchronous_affairs::set_max_asynchronous_affairs(uint8_t i)
	{
		assert(i >= 1);
		{
			std::lock_guard<decltype(affairs_mutex)> lg(affairs_mutex);
			assert(i >= target_affair_count);
			target_affair_count = i;
			uint8_t current_size = static_cast<uint8_t>(thread_v.size());
			while (current_size < target_affair_count)
				thread_v.push_back(std::thread{ &asynchronous_affairs::asynchronous_affairs_execution, this, current_size++ });
		}
	}

	void asynchronous_affairs::insert_affairs(std::function<void(void)> f)
	{
		job_tank.lock([&](decltype(job_tank)::type& t) {
			t.second.push_back(std::move(f));
		});
	}

	void asynchronous_affairs::insert_high_level_affairs(std::function<void(void)> f)
	{
		job_tank.lock([&](decltype(job_tank)::type& t) {
			t.first.push_back(std::move(f));
		});
	}

	asynchronous_affairs::asynchronous_affairs()
	{
		thread_v.push_back(std::thread{ &asynchronous_affairs::asynchronous_affairs_execution, this, 0 });
	}

	asynchronous_affairs::~asynchronous_affairs()
	{
		{
			std::lock_guard<decltype(affairs_mutex)> lg(affairs_mutex);
			target_affair_count = 0;
		}

		for (auto& ite : thread_v)
			if (ite.joinable())
				ite.join();
	}
	*/
}


