#include "path_system.h"
namespace PO::Tool
{
	void path_system::add_path(path p) { search_path.push_back(p); }


	std::ifstream path_system::load(const path& p, std::ios_base::openmode mode)
	{
		std::ifstream file;
		for (auto& ite : search_path)
		{
			file.open(ite / p, mode);
			if (file.good())
				return std::move(file);
			else
				file.close();
		}
		file.open(p, mode);
		return std::move(file);
	}
}