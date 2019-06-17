#pragma once
#include <filesystem>
#include <map>
#include "character_encoding.h"
#include <exception>
namespace PO::Path
{
	namespace Error
	{
		struct path_exception : std::exception {};

		/*
		struct bad_path : path_exception
		{
			bad_path(std::wstring index);
			std::wstring m_path;
			const char* what() const noexcept override;
		};
		*/
	}

	struct path;

	struct root_mapping
	{
		static root_mapping& gobal();
	private:
		std::map<std::wstring, path> m_mapping;
	};

	struct path
	{
		path(std::wstring path);
		path(path&&) = default;
		path(const path&) = default;
		path& operator=(const path&) = default;
		path& operator=(path&&) = default;
		path operator/(const path&) const;
		path& operator/=(const path&);
	private:
		std::wstring m_path;
		std::wstring::iterator m_root_end;
		std::wstring::iterator m_filename_start;
		std::wstring::iterator m_expension_start;
	};

	struct simplify
	{

	private:
		std::wstring m_path;
	};
}