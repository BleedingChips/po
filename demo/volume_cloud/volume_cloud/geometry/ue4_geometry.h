#pragma once
#include "po_dx11\dx11_element.h"

using namespace PO;
using namespace PO::Dx;
using namespace PO::Dx11;

class UE4_cube_static : public geometry_resource
{
	buffer_index index;
	buffer_vertex vertex;
public:
	UE4_cube_static(creator& c);
	void apply(stage_context& sc);
};

class UE4_cubiods_static : public geometry_resource
{
	buffer_index index;
	buffer_vertex vertex;
public:
	UE4_cubiods_static(creator& c);
	void apply(stage_context& sc);
};