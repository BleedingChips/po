#pragma once
#include <string>
#include <filesystem>
#include <map>
#include <fstream>
namespace PO::Tool
{
	/*
	class path
	{
		std::experimental::filesystem::path path_name;
		size_t hash_code;
	public:
		path(std::experimental::filesystem::path p) : path_name(std::move(p)), hash_code(std::hash_value(path_name)) {
			path_name.
		}
		bool operator==(const path& p) const noexcept { return hash_code == p.hash_code; }
		bool operator<(const path& p) const noexcept { return hash_code < p.hash_code; }
	};
	*/
	using path = std::experimental::filesystem::path;

	class path_system
	{
		std::vector<path> search_path;
	public:
		void add_path(path p);
		std::ifstream load(const path& p, std::ios_base::openmode mode = std::ios_base::in);
		std::ifstream load(const path& p, path& result_path, std::ios_base::openmode mode = std::ios_base::in);
	};

}