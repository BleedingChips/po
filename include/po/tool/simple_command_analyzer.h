#pragma once
#include <vector>
namespace PO
{
	std::vector<size_t> simple_command_analyzer(const char** command, size_t command_size, const char** match, size_t match_size);
}
