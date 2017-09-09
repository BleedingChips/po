#pragma once
#include "po_dx11\dx11_element.h"

using namespace PO;
using namespace PO::Dx;
using namespace PO::Dx11;

class UE4_cube_static
{
	primitive_topology ele;
	index_buffer index;
	vertex_buffer vertex;
	layout_view lv;
	raterizer_state rs;
public:
	const layout_view& geometry_layout_view() { return lv; }
	UE4_cube_static(creator& c);
	void geometry_apply(stage_context& sc);
	static bool geometry_update(stage_context& sc, property_interface& pi);
	static const std::set<std::type_index>& geometry_requirement();
};