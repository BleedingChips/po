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
	std::fstream io_method::block::default_filter(const std::u16string& path, const std::u16string& name)
	{
		std::fstream f(utf16_to_asc(path + name).c_str(), std::ios::in | std::ios::binary);
		return std::move(f);
	}

	Tool::optional<Tool::any> io_method::block::operator() (io_method& iom, const std::u16string& path, const std::u16string& name, const Tool::any& a)
	{
		if (method.able_cast<pair_t>())
		{
			auto& pair = method.cast<pair_t>();
			std::fstream f = pair.first(path, name);
			if (!f.good()) return{};
			return pair.second(io_block{ iom, path, name, f, a });
		}
		else if (method.able_cast<analyze_t>())
		{
			std::fstream f = default_filter(path, name);
			if (!f.good()) return{};
			return method.cast<analyze_t>()(io_block{ iom, path, name, f, a });
		}
		else if (method.able_cast<d_filter_t>())
		{
			return method.cast<d_filter_t>()(iom, path, name, a);
		}
		return{};
	}

	void io_method::set_function(std::type_index ti, block k)
	{
		fun_map.lock(
			[&, this](decltype(fun_map)::type& map)
		{
			map[ti] = std::move(k);
		}
		);
	}

	void io_method::add_path(std::u16string u)
	{
		paths.lock([&, this](decltype(paths)::type& a) {
			a.top_path.push_back(std::move(u));
		});
	}

	void io_method::add_path(std::type_index ti, std::u16string u)
	{
		paths.lock([&, this](decltype(paths)::type& a) {
			a.type_path[ti].push_back(std::move(u));
		});
	}

	void io_method::add_path(std::initializer_list<std::u16string> pa)
	{
		paths.lock([&, this](decltype(paths)::type& a) {
			a.top_path.insert(a.top_path.end(), std::make_move_iterator(pa.begin()), std::make_move_iterator(pa.begin()));
		});
	}

	void io_method::add_path(std::initializer_list<std::pair<std::type_index, std::u16string>> pa)
	{
		paths.lock([&, this](decltype(paths)::type& a) {
			for (auto& ptr : pa)
				a.type_path[ptr.first].push_back(std::move(ptr.second));
		});
	}

	void io_method::add_path(std::type_index ti, std::initializer_list<std::u16string> pa)
	{
		paths.lock([&, this](decltype(paths)::type& a) {
			a.type_path[ti].insert(a.top_path.end(), std::make_move_iterator(pa.begin()), std::make_move_iterator(pa.begin()));
		});
	}

	Tool::optional<Tool::any> io_method::calling_specified_path_execute(std::type_index ti, const std::u16string& path, const std::u16string& name, const Tool::any& a)
	{
		return Tool::lock_scope_look(fun_map, paths,[&, this](decltype(fun_map)::type& fun, decltype(paths)::type& pa) mutable -> Tool::optional<Tool::any> {
			auto fun_ptr = fun.find(ti);
			if (fun_ptr == fun.end()) return{};
			return fun_ptr->second(*this, path, name, a);
		});
	}

	Tool::optional<Tool::any> io_method::calling_execute(std::type_index ti, const std::u16string& path, const Tool::any& a)
	{
		return Tool::lock_scope_look(fun_map, paths,
			[&, this](decltype(fun_map)::type& fun, decltype(paths)::type& pa) mutable -> Tool::optional<Tool::any>  {
			auto fun_ptr = fun.find(ti);
			if (fun_ptr == fun.end())
				return Tool::optional<Tool::any>{};
			auto pat_pte = pa.type_path.find(ti);
			if (pat_pte != pa.type_path.end())
			{
				for (auto& ty_pa : pat_pte->second)
				{
					auto re = fun_ptr->second(*this, ty_pa, path, a);
					if (re) return re;
				}
			}
			for (auto& ty_pa : pa.top_path)
			{
				auto re = fun_ptr->second(*this, ty_pa, path, a);
				if (re) return re;
			}
			auto re = fun_ptr->second(*this, u"", path, a);
			if (re) return re;
			return {};
		});
	}

	namespace Implement
	{
		std::future<Tool::optional<Tool::any>> io_task_implement::add_request(std::type_index ti, std::u16string pa, Tool::any a)
		{
			std::shared_ptr<std::promise<Tool::optional<Tool::any>>> pro = std::make_shared<std::promise<Tool::optional<Tool::any>>>();
			auto fu = pro->get_future();
			auto ref = cr;
			auto& meth = method;
			ope.add_task([ref, pro = std::move(pro), &meth, ti, pa=std::move(pa), a = std::move(a)]() mutable{
				ref.lock_if([&]() mutable {
					pro->set_value(meth.calling_execute(ti, pa, a));
				});
				return false;
			});
			return fu;
		}
	}
	

	/*
	std::future<Tool::any> io_method_implement::add_request(std::type_index ti, std::u16string path, Tool::any a)
	{

	std::promise<Tool::any> pro;
	std::future<Tool::any> fur = pro.get_future();
	//std::function<bool()> po2 = [p = std::move(pro)]() mutable {return false; };
	//ope.add_task([po = std::move(pro)]() mutable {return false; });

	ope.add_task(
	[pro = std::move(pro), cr = std::move(cr), path = std::move(path), a = std::move(a), ti, this] () -> bool
	{
	if (!cr.lock_if(
	[&, this]() mutable {
	calling_execute(pro, ti, path, a);
	}))
	{
	pro.set_value(Tool::any{});
	}
	return false;
	}
	);
	return std::move(fur);
	}*/
	
	io_task& io_task_instance()
	{
		static io_task iom;
		return iom;
	}

	Tool::optional<Tool::any> raw_scene::find(std::type_index ti, const std::u16string& path, const Tool::any& a, bool save_raw_data)
	{
		return store_map.lock( [&, this](decltype(store_map)::type& map) -> Tool::optional<Tool::any> {
			auto& type_mapping = map[ti];
			auto ptr = type_mapping.find(path);
			if (ptr == type_mapping.end())
			{
				auto po = io_task_instance().request(ti, path, a);
				if (save_raw_data && po)
					type_mapping.insert({ path, store_type{*po} });
				return std::move(po);
			}
			else {
				if (ptr->second.able_cast<std::future<Tool::optional<Tool::any>>>())
				{
					auto fu = std::move(ptr->second.cast<std::future<Tool::optional<Tool::any>>>());
					if (!fu.valid())
						__debugbreak();
					fu.wait();
					auto result = fu.get();
					if (save_raw_data && result)
						ptr->second = *result;
					else
						type_mapping.erase(ptr);
					return std::move(result);
				}
				else if(ptr->second.able_cast<Tool::any>()) {
					return ptr->second.cast<Tool::any>();
				}
				else
					return{};
			}
		}
		);
	}
	
	void raw_scene::pre_load(std::type_index ti, const std::u16string& path, Tool::any a)
	{
		store_map.lock([&, this](decltype(store_map)::type& map) {
			auto& ty_map_ref = map[ti];
			auto ptr = ty_map_ref.find(path);
			if (ptr == ty_map_ref.end())
			{
				auto fur = io_task_instance().add_request(ti, path, std::move(a));
				ty_map_ref.insert({ path, store_type{std::move(fur)} });
			}
		});
	}

	void raw_scene::pre_load(std::type_index ti, std::initializer_list<std::pair<std::u16string, Tool::any>> path)
	{
		store_map.lock([&, this](decltype(store_map)::type& map) {
			auto& ty_map_ref = map[ti];
			for (auto& pa : path)
			{
				auto ptr = ty_map_ref.find(pa.first);
				if (ptr == ty_map_ref.end())
				{
					auto fur = io_task_instance().add_request(ti, pa.first, pa.second);
					ty_map_ref.insert({ std::move(pa.first), store_type{ std::move(fur) } });
				}
			}
		});
	}

	void raw_scene::pre_load(std::type_index ti, std::initializer_list<std::u16string> path)
	{
		store_map.lock([&, this](decltype(store_map)::type& map) {
			auto& ty_map_ref = map[ti];
			for (auto& pa : path)
			{
				auto ptr = ty_map_ref.find(pa);
				if (ptr == ty_map_ref.end())
				{
					auto fur = io_task_instance().add_request(ti, pa, Tool::any{});
					ty_map_ref.insert({ std::move(pa), store_type{ std::move(fur) } });
				}
			}
		});
	}

	/*

	void raw_scene::pre_load(std::type_index ti, const std::u16string& path)
	{
		store_map.lock([&, this](decltype(store_map)::type& map){
			auto& ty_map_ref = map[ti];
			auto ptr = ty_map_ref.find(path);
			if (ptr == ty_map_ref.end())
			{
				Tool::any ptr = io_method_instance().add_request(ti, path, Tool::any{});
				ty_map_ref.insert({ path, ptr });
			}
		});
	}

	void raw_scene::pre_load(std::type_index ti, std::initializer_list<std::u16string> path)
	{
		store_map.lock([&, this](decltype(store_map)::type& map) {
			auto& ty_map_ref = map[ti];
			for (auto& pa : path)
			{
				auto ptr = ty_map_ref.find(pa);
				if (ptr == ty_map_ref.end())
				{
					Tool::any ptr = io_method_instance().add_request(ti, pa, Tool::any{});
					ty_map_ref.insert({ pa, ptr });
				}
			}
		});
	}

	void raw_scene::pre_load(std::initializer_list<std::pair<std::type_index, std::u16string>> l)
	{
		store_map.lock([&, this](decltype(store_map)::type& map) {
			for (auto& ptr : l)
			{
				auto& ty_map_ref = map[ptr.first];
				auto ite = ty_map_ref.find(ptr.second);
				if (ite == ty_map_ref.end())
				{
					Tool::any ptr2 = io_method_instance().add_request(ptr.first, ptr.second, Tool::any{});
					ty_map_ref.insert({ ptr.second, ptr2 });
				}
			}
		});
	}
	*/
}