#pragma once
#include "../tool/tool.h"
#include <functional>
#include <thread>
#include <deque>
#include <optional>
#include <future>
namespace PO::Platform
{
	class asynchronous_affairs
	{
		Tool::scope_lock<std::deque<std::function<void(void)>>> affairs_list;
	public:
		std::function<void(void)> pop_front();
		void insert(std::function<void(void)> f);
	};


	class thread_pool
	{
		struct thread_state
		{
			std::mutex function_mutex;
			std::optional<std::function<bool(void)>> main_function;
			bool avalible = true;
		};

		struct thread_swap_block
		{
			bool avalible;
			std::optional<std::function<bool(void)>> main_function;
		};

		std::condition_variable cv;
		std::unordered_map<std::thread::id, std::pair<std::thread, thread_state*>> thread_map;
		void execute(std::promise<thread_state*>& p);
		void clear_implement();
	public:
		void notify_one() { cv.notify_one(); }
		void notity_all() { cv.notify_all(); }
		std::thread::id create_thread(std::function<bool(void)> f);
		bool replace_thread_function(std::thread::id, std::function<bool(void)> f);
		void clear();
		~thread_pool() { clear_implement(); }
	};











	/*
	class asynchronous_affairs
	{	

		struct thread_state
		{
			std::thread current_thread;
			std::function<bool(void)> thread_main_function;
		};

		Tool::scope_lock<std::vector<thread_state>> thread_v;

		Tool::scope_lock<std::vector<>>

		std::atomic_bool avalible;
		std::atomic_uint8_t target_affair_count = 1;

	public:

		~asynchronous_affairs();

		void set_max_asynchronous_affairs(uint8_t i);
		void insert_affairs(std::function<void(void)>);
		void insert_high_level_affairs(std::function<void(void)>);
		
	};
	*/
}