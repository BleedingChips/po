#pragma once

#include <variant>
#include <string>
#include "../po/tool/intrusive_ptr.h"

namespace PO::Graphic
{

	enum StorageType
	{
		Null,
		Bool,
		Int,
		Float,
		String,
		List,
		Shader,
		Pass,
		SubMaterial,
	};

	struct storage_ref
	{
		StorageType type;
		uint64_t index;
	};

	struct export_data
	{
		std::vector<std::string> int_list;
		virtual storage_ref get_export(const char*, size_t count) = 0;
	};

	struct shader : Tool::intrusive_object_base, export_data
	{
		virtual storage_ref get_export(const char*, size_t count);
	};

	struct pass : Tool::intrusive_object_base, export_data
	{

	};

	struct sub_material : Tool::intrusive_object_base
	{

	};

	struct material : Tool::intrusive_object_base
	{

	};
}
