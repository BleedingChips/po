#include "scene.h"
namespace PO
{
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
		return Tool::lock_scope_look(fun_map, paths, [&, this](decltype(fun_map)::type& fun, decltype(paths)::type& pa) mutable -> Tool::optional<Tool::any> {
			auto fun_ptr = fun.find(ti);
			if (fun_ptr == fun.end()) return{};
			return fun_ptr->second(*this, path, name, a);
		});
	}

	Tool::optional<Tool::any> io_method::calling_execute(std::type_index ti, const std::u16string& path, const Tool::any& a)
	{
		return Tool::lock_scope_look(fun_map, paths,
			[&, this](decltype(fun_map)::type& fun, decltype(paths)::type& pa) mutable -> Tool::optional<Tool::any> {
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
			ope.add_task([ref, pro = std::move(pro), &meth, ti, pa = std::move(pa), a = std::move(a)]() mutable{
				ref.lock_if([&]() mutable {
					pro->set_value(meth.calling_execute(ti, pa, a));
				});
				return false;
			});
			return fu;
		}
	}
	io_task& io_task_instance()
	{
		static io_task iom;
		return iom;
	}

	Tool::optional<Tool::any> raw_scene::find(std::type_index ti, const std::u16string& path, const Tool::any& a, bool save_raw_data)
	{
		return store_map.lock([&, this](decltype(store_map)::type& map) -> Tool::optional<Tool::any> {
			auto& type_mapping = map[ti];
			auto ptr = type_mapping.find(path);
			if (ptr == type_mapping.end())
			{
				auto po = io_task_instance().request(ti, path, a);
				if (save_raw_data && po)
					type_mapping.insert({ path, store_type{ *po } });
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
				else if (ptr->second.able_cast<Tool::any>()) {
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
				ty_map_ref.insert({ path, store_type{ std::move(fur) } });
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

	std::shared_ptr<void> scene::load(std::type_index ti, const std::u16string& p)
	{
		auto ite = store.find(ti);
		if (ite != store.end())
		{
			auto ite2 = ite->second.find(p);
			if (ite2 != ite->second.end())
				return ite2->second;
		}
		return {};
	}

	void scene::push(std::type_index ti, const std::u16string& path, std::shared_ptr<void> p)
	{
		store[ti][path] = std::move(p);
	}

	std::fstream scene::load_file(std::type_index ti, const std::u16string& path, bool is_binary)
	{
		auto flag = is_binary ? std::ios::in | std::ios::binary : std::ios::in;
		auto map_ite = special_path.find(ti);
		if (map_ite != special_path.end())
		{
			for (auto& p : map_ite->second)
			{
				auto f = p + path;
				std::fstream fem(reinterpret_cast<const wchar_t*>(f.c_str()), flag);
				if (fem.is_open())
					return std::move(fem);
			}
		}
		std::fstream fem(reinterpret_cast<const wchar_t*>(path.c_str()), flag);
		return std::move(fem);
	}

	bool scene::add_path(std::type_index ti, const std::u16string& p)
	{
		std::u16string tem;
		tem.reserve(p.size() + 2);
		tem = p;
		if (*tem.rbegin() != char16_t{ '\\' })
			tem.push_back('\\');

		auto& vec = special_path[ti];
		for (auto& ite : vec)
			if (ite == tem) return false;

		vec.push_back(std::move(tem));
		return true;
	}
}