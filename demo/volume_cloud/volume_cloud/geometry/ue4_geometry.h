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

class CubeSimpleX : public geometry_resource
{
	buffer_vertex vertex;
	buffer_index index[6];
public:

	struct property
	{
		size_t index_count = 0;
		struct renderer_data
		{
			buffer_index index;
		};
		void set_index(size_t i) { index_count = i; if (index_count > 5) index_count = 5; }
		void update(creator& c, renderer_data& rd);
	};

	CubeSimpleX(creator& c);
	const element_requirement& requirement() const;
	void apply(stage_context& sc);
};

class CubeFrame: public geometry_resource
{
	buffer_vertex vertex;
	buffer_index index;
public:

	CubeFrame(creator& c);
	void apply(stage_context& sc);
};

class UE4_cubiods_static_Frame : public geometry_resource
{
	buffer_index index;
	buffer_vertex vertex;
public:
	UE4_cubiods_static_Frame(creator& c);
	void apply(stage_context& sc);
};