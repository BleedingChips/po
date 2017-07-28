#include "translate_ue4_custion_node.h"
using namespace std;

auto read(ifstream& i, char* buffer, size_t buffer_size)
{
	auto ite = i.tellg();
	i.read(buffer, buffer_size);
	auto now = i.tellg();
	return now - ite;
}

bool translate_ue4_node(const std::string& file_name, const std::string& main_function, const std::string& output)
{
	ifstream io(file_name);
	ofstream io2(output);
	if (io.is_open())
	{
		int state = 0;
		char buffer[2048] = "\0";
		int buffer_count = 0;
		while (io.good())
		{
			char charactor;
			io.read(&charactor, 1);
			switch (state)
			{
			case 0:
				if ()
					break;
			default:
				break;
			}

		}
		return true;
	}
	else
		return false;
}