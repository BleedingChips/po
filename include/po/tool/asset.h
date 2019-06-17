#pragma once
#include <filesystem>
#include <map>
#include <set>
#include <typeindex>
#include <functional>
#include "../tool/tool.h"
#include "../tool/intrusive_ptr.h"
namespace PO::Asset
{
	namespace fs = std::filesystem;
	using path = fs::path;
	path relative_to(const path& input, const path& target);
	inline uint64_t last_write_time_u64(const path& p) { return fs::last_write_time(p).time_since_epoch().count(); }

	enum class State
	{
		Update,
		New,
		Deleted
	};

	struct info
	{
		path m_absolutely_path;
		path m_relative_path;
		path m_path_name_ite;
		path m_extension;
		path m_filename_without_extension;
		fs::file_time_type m_last_update_time;
	};

	struct monitor
	{
		monitor(path root_path = fs::current_path()) : m_root_path(std::move(root_path)) {}
		void update();
		template<typename CallBackFunc> void update(CallBackFunc&& Cf)
		{
			auto ite = fs::recursive_directory_iterator{ m_root_path };
			while (auto result = update_imp(ite))
				std::forward<CallBackFunc>(Cf)(std::get<0>(*result), static_cast<const info&>(std::get<1>(*result)->second.first));
			auto ite2 = m_asset.begin();
			while (auto result = check_remove_imp(ite2))
				std::forward<CallBackFunc>(Cf)(std::get<0>(*result), static_cast<const info&>(std::get<1>(*result)));
		}

		monitor& register_extension(path);
		monitor& deregister_extension(const path&) noexcept;
		const path& root() const noexcept { return m_root_path; }
	private:

		using map_type = std::map<path, std::pair<info, bool>>;

		auto update_imp(fs::recursive_directory_iterator& cursor) ->std::optional<std::tuple<State, typename map_type::const_iterator>>;
		auto check_remove_imp(typename map_type::iterator& cursor)->std::optional<std::tuple<State, info>>;
		const path m_root_path;
		map_type m_asset;
		std::set<path> m_associate_extension;
	};
	/*
	struct relative_path_map
	{	

		enum State
		{
			Update,
			New,
			Deleted
		};

		struct update_event
		{
			State state;
			Tool::path relative_path;
		};

		struct description
		{
			Tool::path m_absolutely_path;
			Tool::path m_file_name;
			Tool::path m_file_name_without_extension;
			uint64_t last_update_time;
			description(fs::path absolutely_path) : m_absolutely_path(::std::move(absolutely_path))
			{
				m_file_name = m_absolutely_path.filename();
				m_file_name_without_extension = m_file_name;
				while (m_file_name_without_extension.has_extension())
					m_file_name_without_extension.replace_extension();
				last_update_time = fs::last_write_time(m_absolutely_path).time_since_epoch().count();
			}
		};

		struct description_implement : description
		{
			bool reach = false;
		};

		relative_path_map(path root_path = Tool::current_path());
		relative_path_map& add_extension(path extension);
		
		const description* find(const path& relative_path) const noexcept;
		std::vector<update_event> update();

		size_t find_extension(const path& extension, std::function<bool(const description& des)>) const noexcept;
		Tool::path relative(const Tool::path& p) const { return relative_to(p, m_root_path); }
		Tool::path absolutely(const Tool::path& p) const { return Tool::fs::canonical(p, m_root_path); }
		const Tool::path& root() const noexcept { return m_root_path; }

	private:

		Tool::path m_root_path;

		std::map<path, description_implement> m_relative_description_mapping;
		std::multimap<path, decltype(m_relative_description_mapping)::iterator> m_extension_relative_path;

		std::set<path> m_extension;
	};

	struct asset_interface : intrusive_object_base
	{
		asset_interface(std::type_index ti) noexcept : m_id(ti) {}
		std::type_index id() const noexcept { return m_id; }
		template<typename T>
		bool able_cast() const noexcept { return m_id == typeid(std::decay_t<T>); }
		template<typename T> std::decay_t<T>& cast() noexcept { assert(able_cast<T>()); return static_cast<std::decay_t<T>&>(*this); }
		template<typename T> const std::decay_t<T>& cast() const noexcept { assert(able_cast<T>());  return static_cast<const std::decay_t<T>&>(*this); }
	private:
		std::type_index m_id;
	};

	struct asset
	{
		using funtion_t = Tool::intrusive_ptr<asset_interface> (*)(const path& absolutely_path);
		void regedit_importer(path extension, funtion_t fun);
		bool load(const path& relative_path, const relative_path_map& ai);
		Tool::intrusive_ptr<asset_interface> find_resource(const Tool::path& relatived_path) const noexcept;
	private:
		std::map<path, funtion_t> m_regedit_extension_function;
		std::map<path, Tool::intrusive_ptr<asset_interface>> m_resource;
	};
	*/




	/*
	struct asset_preload
	{
		std::vector<asset::id> id_vector;
		void load_all(const asset& a);
		void load_all_extension(const fs::path& p, const asset& a);
		void load(const fs::path& p, const asset& a);
	};

	struct asset_storage
	{
		using funtion_t = asset_interface * (*)(const asset::description&, const asset&);

		void insert_handler(fs::path, funtion_t fun);
		bool load(asset::id, const asset&);
		size_t load(const asset_preload&, const asset&);
		asset_interface* find_resource(asset::id) const noexcept;
		~asset_storage();
	private:
		std::map<fs::path, funtion_t> m_regedit_extension_function;
		std::map<asset::id, asset_interface*> ready_resource;
	};
	*/
}