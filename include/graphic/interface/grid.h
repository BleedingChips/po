#pragma once
#include "format.h"
#include <array>
#include <memory>
#include <vector>
namespace PO::Graphic
{
	struct vertex_scriprion
	{
		//FromatPixel fromat;
		void* semantics;
		size_t solt;
		size_t offset;
	};

	struct grid
	{
		std::array<std::unique_ptr<std::byte[]>, 32> data;
		size_t element_count;
		std::vector<vertex_scriprion> format;
	};

}