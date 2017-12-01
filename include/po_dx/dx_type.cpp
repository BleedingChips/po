#include "dx_type.h"
namespace PO
{
	namespace Dx
	{
		shader_binary::shader_binary(std::fstream& f)
		{
			f.seekg(0, std::ios::end);
			auto end_poi = f.tellg();
			f.seekg(0, std::ios::beg);
			auto sta_poi = f.tellg();
			size = end_poi - sta_poi;
			char* da = new char[size];
			f.read(da, size);
			data = da;
			/*
			static bool p = false;
			if (!p)
			{
				std::fstream pp("debug.bin", std::ios::out);
				pp.write(da, size);
				pp.close();
				p = true;
			}*/
			

		}

		shader_binary::~shader_binary() { delete[](data); }
	}
}