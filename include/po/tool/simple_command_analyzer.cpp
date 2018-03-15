#include "simple_command_analyzer.h"
namespace PO
{
	std::vector<size_t> simple_command_analyzer(const char** command, size_t command_size, const char** match, size_t match_size)
	{
		std::vector<size_t> temporary;
		temporary.reserve(command_size);
		size_t parameter_count = 0;
		for (size_t i = 0; i < command_size; ++i)
		{
			const char* com = command[i];
			if (*com == '-')
			{
				parameter_count = 0;
				++com;
				size_t mi = 0;
				for (;mi < match_size; ++mi)
				{
					auto com_detect = com, mat = match[mi];
					for (;*com_detect != '\0' && *mat != '\0'; ++com_detect, ++mat)
					{
						if (*com_detect != *mat)
							break;
					}
					if (*com_detect == '\0' && *mat == '\0')
					{
						temporary.push_back(mi);
						break;
					}
				}
				if (mi == match_size)
					temporary.push_back(match_size);
			}
			else {
				temporary.push_back(match_size + 1 + parameter_count);
				parameter_count++;
			}
		}
		return std::move(temporary);
	}
}