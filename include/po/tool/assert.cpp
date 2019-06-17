#include "asset.h"
#include <iostream>

namespace PO::Asset
{
	void monitor::update()
	{
		auto ite = fs::recursive_directory_iterator{ m_root_path };
		while (auto res = update_imp(ite));
		auto ite2 = m_asset.begin();
		while (auto res = check_remove_imp(ite2));
	}

	monitor& monitor::register_extension(path extension)
	{
		m_associate_extension.insert(std::move(extension));
		return *this;
	}

	monitor& monitor::deregister_extension(const path& extension) noexcept
	{
		auto ite = m_associate_extension.find(extension);
		if (ite != m_associate_extension.end())
		{
			m_associate_extension.erase(ite);
			for (auto ite2 = m_asset.begin(); ite2 != m_asset.end();)
			{
				if (ite2->second.first.m_extension == extension)
					m_asset.erase(ite2++);
				else
					++ite2;
			}
		}
		return *this;
	}

	auto monitor::update_imp(fs::recursive_directory_iterator& cursor) ->std::optional<std::tuple<State, typename map_type::const_iterator>>
	{
		while (cursor != fs::recursive_directory_iterator{})
		{
			if (fs::is_regular_file(*cursor))
			{
				path current_path = *cursor;
				auto extension = current_path.extension();
				if (m_associate_extension.find(extension) != m_associate_extension.end())
				{
					auto ite = m_asset.find(current_path);
					if (ite != m_asset.end())
					{
						ite->second.second = true;
						if (fs::last_write_time(current_path) != ite->second.first.m_last_update_time)
						{
							++cursor;
							return std::tuple<State, typename map_type::const_iterator>{ State::Update, ite };
						}
					}
					else {
						auto name = current_path.filename();
						auto exten = name;
						exten.replace_extension();
						info temporary_info
						{
							current_path,
							fs::relative(current_path, m_root_path),
							std::move(name),
							current_path.extension(),
							std::move(exten),
							fs::last_write_time(current_path),
						};

						ite = m_asset.insert({ current_path, {std::move(temporary_info), true} }).first;
						++cursor;
						return std::tuple<State, typename map_type::const_iterator>{ State::New, ite };
					}
				}
			}
			++cursor;
		}
		return {};
	}

	auto monitor::check_remove_imp(typename map_type::iterator& cursor)->std::optional<std::tuple<State, info>>
	{
		while (cursor != m_asset.end())
		{
			if (!cursor->second.second)
			{
				info temporary = std::move(cursor->second.first);
				m_asset.erase(cursor++);
				return std::tuple<State, info>{State::Deleted, std::move(temporary)};
			}
			else
				cursor->second.second = false;
		}
		return {};
	}
}

namespace PO::Tool
{
	/*
	path relative_to(const path& input, const path& target)
	{
		auto ite = input.begin();
		auto ite2 = target.begin();
		Tool::fs::path output;
		while (ite2 != target.end())
		{
			if (ite == input.end() || *ite != *(ite2))
			{ 
				output /= u"..\\";
			}
			else {
				++ite;
			}
			++ite2;
		}
		while (ite != input.end())
			output /= *(ite++);
		return output;
	}

	relative_path_map::relative_path_map(fs::path root_path) : m_root_path(std::move(root_path)) {}
	relative_path_map& relative_path_map::add_extension(fs::path p)
	{
		m_extension.insert(std::move(p));
		return *this;
	}

	const relative_path_map::description* relative_path_map::find(const Tool::path& relative_path) const noexcept
	{
		auto ite = m_relative_description_mapping.find(relative_path);
		if (ite != m_relative_description_mapping.end())
			return &ite->second;
		else
			return nullptr;
	}

	std::vector<relative_path_map::update_event> relative_path_map::update()
	{
		std::vector<relative_path_map::update_event> event;
		for (const auto& ite : fs::recursive_directory_iterator{ m_root_path })
		{
			if (fs::is_regular_file(ite))
			{
				path p = ite;
				path exten = p.extension();
				if (m_extension.find(exten) != m_extension.end())
				{
					auto r_path = relative_to(ite.path(), m_root_path);
					auto ite2 = m_relative_description_mapping.find(r_path);
					if (ite2 != m_relative_description_mapping.end())
					{
						ite2->second.reach = true;
						if (ite2->second.last_update_time != fs::last_write_time(ite).time_since_epoch().count())
						{
							ite2->second.last_update_time = fs::last_write_time(ite).time_since_epoch().count();
							event.push_back({ State ::Update, std::move(r_path)});
						}
					}
					else {
						description_implement tem{ ite };
						tem.reach = true;
						auto result = m_relative_description_mapping.insert({ r_path , std::move(tem) });
						m_extension_relative_path.insert({ std::move(exten), result.first });
						event.push_back({ State::New, std::move(r_path) });
					}
				}
			}
		}

		for (auto ite2 = m_extension_relative_path.begin(); ite2 != m_extension_relative_path.end();)
		{
			if (!ite2->second->second.reach)
			{
				event.push_back({ State::Deleted, ite2->second->first });
				m_relative_description_mapping.erase(ite2->second);
				m_extension_relative_path.erase(ite2++);
			}
			else
			{
				ite2->second->second.reach = false;
				++ite2;
			}
		}
		return event;
	}

	size_t relative_path_map::find_extension(const Tool::path& extension, std::function<bool(const description& des)> f) const noexcept
	{
		size_t count = 0;
		auto ite = m_extension_relative_path.equal_range(extension);
		for (auto ite2 = ite.first; ite2 != ite.second; ++ite2)
		{
			++count;
			if (!f(ite2->second->second))
				break;
		}
		return count;
	}

	void asset::regedit_importer(fs::path extension, funtion_t fun)
	{
		m_regedit_extension_function.insert({ std::move(extension), fun });
	}

	bool asset::load(const path& relative_path, const relative_path_map& ai)
	{
		auto extension = relative_path.extension();
		auto ite = m_regedit_extension_function.find(extension);
		if (ite != m_regedit_extension_function.end())
		{
			auto ite2 = m_resource.find(relative_path);
			if (ite2 == m_resource.end())
			{
				auto ptr = ai.find(relative_path);
				if (ptr != nullptr)
				{
					auto res = ite->second(ptr->m_absolutely_path);
					m_resource.insert({ relative_path, std::move(res) });
					return true;
				}
			}
			else
				return true;
		}
		return false;
	}

	Tool::intrusive_ptr<asset_interface> asset::find_resource(const Tool::path& path) const noexcept
	{
		auto ite = m_resource.find(path);
		if (ite != m_resource.end())
			return ite->second;
		return {};
	}
	*/

	/*
	void asset_preload::load_all(const asset& a)
	{
		a.for_each([this](const asset::description& d) {
			id_vector.push_back(d.id);
		});
	}

	void asset_preload::load_all_extension(const fs::path& p, const asset& a)
	{
		a.for_each_with_extension(p, [this](const asset::description& id) {
			id_vector.push_back(id.id);
		});
	}

	asset_interface::asset_interface(std::type_index id,  void(*deleter)(asset_interface*) noexcept) :  m_id(id), m_deleter(deleter) { }

	void asset_storage::insert_handler(fs::path p, funtion_t fun)
	{
		m_regedit_extension_function.insert({ std::move(p), fun });
	}

	void asset_interface::release() noexcept
	{
		auto m_dele = m_deleter;
		(*m_dele)(this);
	}
	*/

	/*
	void asset_storage::for_each(std::function<void(asset::id, const asset_interface&)> f) const noexcept
	{
		for (auto& ite : ready_resource)
		{
			f(ite.first, *ite.second);
		}
	}
	*/

	/*
	bool asset_storage::load(asset::id id, const asset& a)
	{
		if (ready_resource.find(id) != ready_resource.end()) return false;
		const asset::description* ref = a.id_description(id);
		if (ref != nullptr)
		{
			auto ite = m_regedit_extension_function.find(ref->path.extension());
			if (ite != m_regedit_extension_function.end())
			{
				auto ptr = (*ite->second)(*ref, a);
				if(ptr != nullptr)
					return (ready_resource.insert({ id, ptr }), true);
				return false;
			}
		}
		return false;
	}

	size_t asset_storage::load(const asset_preload& ap, const asset& t)
	{
		size_t load_count = 0;
		for (auto& ite : ap.id_vector)
			if (load(ite, t))
				++load_count;
		return load_count;
	}

	asset_interface* asset_storage::find_resource(asset::id id) const noexcept
	{
		auto ite = ready_resource.find(id);
		if (ite != ready_resource.end())
			return ite->second;
		else
			return nullptr;
	}

	asset_storage::~asset_storage()
	{
		for (auto& ite : ready_resource)
		{
			assert(ite.second != nullptr);
			ite.second->release();
		}
	}
	*/

	/*
	namespace Implement
	{
		void asset_handler::insert(fs::path p, funtion_t fun)
		{
			m_regedit_extension_function.insert({ std::move(p), fun });
		}

		asset_interface* asset_handler::handle(const fs::path& p) const noexcept
		{
			auto extension = p.extension();
			auto ite = m_regedit_extension_function.find(extension);
			if (ite != m_regedit_extension_function.end())
			{
				return (*ite->second)(p);
			}
			else
				return nullptr;
		}

		void asset_readyed_resource::insert(asset::id id, asset_interface* ai)
		{
			ready_resource
		}
	}
	*/

}