#pragma once
#include "material.h"
namespace PO::Graphic
{
	struct model
	{
		std::shared_ptr<pixel_generator> generator;
		std::unique_ptr<material_interface> material;
	};
}