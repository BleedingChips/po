#pragma once
#include "po_dx11\dx11_element.h"

using namespace PO;
using namespace PO::Dx;
using namespace PO::Dx11;

class UE4_cube_static : public geometry_resource
{
	index_buffer index;
	vertex_buffer vertex;
public:
	UE4_cube_static(creator& c);
	void apply(stage_context& sc);
};