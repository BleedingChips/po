#include "ue4_geometry.h"
#include "po_dx11\dx11_vertex.h"
#include "po_dx11\dx11_buildin_element.h"

namespace
{
	struct position
{
	const char* operator()() const { return "POSITION"; }
};
struct texcoord
{
	const char* operator()() const { return "TEXCOORD"; }
};

struct cube_static_3d_t
{
	float3 position;
	float2 texturecoord;
};

std::array<cube_static_3d_t, 24> cube_static_3d2 =
{
	cube_static_3d_t
{ { -1.0f, -1.0f, 1.0f },{ 1.0f, 1.0f } },
{ { 1.0, -1.0, 1.0 },{ 0.0, 1.0 } },
{ { 1.0, 1.0, 1.0 },{ 0.0, 0.0 } },
{ { -1.0, 1.0, 1.0 },{ 1.0, 0.0 } },

{ { 1.0, -1.0, 1.0 },{ 1.0, 1.0 } },
{ { 1.0, -1.0, -1.0 },{ 0.0, 1.0 } },
{ { 1.0, 1.0, -1.0 },{ 0.0, 0.0 } },
{ { 1.0, 1.0, 1.0 },{ 1.0, 0.0 } },


{ { 1.0, -1.0, -1.0 },{ 1.0, 1.0 } },
{ { -1.0, -1.0, -1.0 },{ 0.0, 1.0 } },
{ { -1.0, 1.0, -1.0 },{ 0.0, 0.0 } },
{ { 1.0, 1.0, -1.0 },{ 1.0, 0.0 } },


{ { -1.0, -1.0, -1.0 },{ 1.0, 1.0 } },
{ { -1.0, -1.0, 1.0 },{ 0.0, 1.0 } },
{ { -1.0, 1.0, 1.0 },{ 0.0, 0.0 } },
{ { -1.0, 1.0, -1.0 },{ 1.0, 0.0 } },

{ { 1.0, 1.0, -1.0 },{ 1.0, 1.0 } },
{ { -1.0, 1.0, -1.0 },{ 0.0, 1.0 } },
{ { -1.0, 1.0, 1.0 },{ 0.0, 0.0 } },
{ { 1.0, 1.0, 1.0 },{ 1.0, 0.0 } },

{ { -1.0, -1.0, -1.0 },{ 1.0, 1.0 } },
{ { 1.0, -1.0, -1.0 },{ 0.0, 1.0 } },
{ { 1.0, -1.0, 1.0 },{ 0.0, 0.0 } },
{ { -1.0, -1.0, 1.0 },{ 1.0, 0.0 } }
};

struct multy_50
{
	std::array<cube_static_3d_t, 24> data;
	multy_50()
	{
		for (size_t i = 0; i < cube_static_3d2.size() && i < data.size(); ++i)
		{
			data[i].position = cube_static_3d2[i].position * 50;
			data[i].texturecoord = cube_static_3d2[i].texturecoord;
		}
	}
};
multy_50 staticl;

struct multy_40_20_10
{
	std::array<cube_static_3d_t, 24> data;
	multy_40_20_10()
	{
		for (size_t i = 0; i < cube_static_3d2.size() && i < data.size(); ++i)
		{
			float3 poi = cube_static_3d2[i].position;
			data[i].position = float3(poi.x * 80, poi.y * 80, poi.z * 10);
			data[i].texturecoord = cube_static_3d2[i].texturecoord;
		}

	}
};
multy_40_20_10 staticl_2;

std::array<uint16_t, 36> cube_static_3d_index =
{
	0,1,2,
	2,3,0,

	4,5,6,
	6,7,4,

	8,9,10,
	10,11,8,

	12,13,14,
	14,15,12,

	16,17,18,
	18,19,16,

	20,21,22,
	22,23,20
};
}



UE4_cube_static::UE4_cube_static(creator& c) : 
	geometry_resource(
		c, layout_type<buffer_layout<syntax<position, 0, float3>, syntax<texcoord, 0, float2>>>{},
		raterizer_state::description{
D3D11_FILL_MODE::D3D11_FILL_SOLID,
D3D11_CULL_MODE::D3D11_CULL_FRONT,
FALSE,
0,
0.0f,
0.0f,
true,
false,
false,
false
}
		)
{
	index.create_index(c, cube_static_3d_index);
	vertex.create_vertex(c, ::staticl.data);
}

void UE4_cube_static::apply(stage_context& sc)
{
	geometry_resource::apply(sc);
	sc << index << vertex[0] << index_call{ static_cast<uint32_t>(cube_static_3d_index.size()), 0, 0};
}

uint32_t3 UE4_cube_static::size()
{
	return uint32_t3{ 50, 50, 50 };
}

UE4_cube_static_Frame::UE4_cube_static_Frame(creator& c) :
	geometry_resource(
		c, layout_type<buffer_layout<syntax<position, 0, float3>, syntax<texcoord, 0, float2>>>{},
		raterizer_state::description{
	D3D11_FILL_MODE::D3D11_FILL_WIREFRAME,
	D3D11_CULL_MODE::D3D11_CULL_FRONT,
	FALSE,
	0,
	0.0f,
	0.0f,
	true,
	false,
	false,
	false
}
)
{
	index.create_index(c, cube_static_3d_index);
	vertex.create_vertex(c, ::staticl.data);
}

void UE4_cube_static_Frame::apply(stage_context& sc)
{
	geometry_resource::apply(sc);
	sc << index << vertex[0] << index_call{ static_cast<uint32_t>(cube_static_3d_index.size()), 0, 0 };
}


UE4_cubiods_static::UE4_cubiods_static(creator& c) :
	geometry_resource(
		c, layout_type<buffer_layout<syntax<position, 0, float3>, syntax<texcoord, 0, float2>>>{},
		raterizer_state::description{
	D3D11_FILL_MODE::D3D11_FILL_SOLID,
	D3D11_CULL_MODE::D3D11_CULL_FRONT,
	FALSE,
	0,
	0.0f,
	0.0f,
	true,
	false,
	false,
	false
}
)
{
	index.create_index(c, cube_static_3d_index);
	vertex.create_vertex(c, ::staticl_2.data);
}

void UE4_cubiods_static::apply(stage_context& sc)
{
	geometry_resource::apply(sc);
	sc << index << vertex[0] << index_call{ static_cast<uint32_t>(cube_static_3d_index.size()), 0, 0 };
}

uint32_t3 UE4_cubiods_static::size()
{
	return uint32_t3{ 80, 80, 10 };
}


std::array<cube_static_3d_t, 8> cube_simple =
{
	cube_static_3d_t
{ { -1.0f, -1.0f, 1.0f },{ 1.0f, 1.0f } },
{ { 1.0, -1.0, 1.0 },{ 0.0, 1.0 } },
{ { 1.0, 1.0, 1.0 },{ 0.0, 0.0 } },
{ { -1.0, 1.0, 1.0 },{ 1.0, 0.0 } },


{ { -1.0, -1.0, -1.0 },{ 1.0, 1.0 } },
{ { 1.0, -1.0, -1.0 },{ 0.0, 1.0 } },
{ { 1.0, 1.0, -1.0 },{ 0.0, 0.0 } },
{ { -1.0, 1.0, -1.0 },{ 1.0, 0.0 } },

};

CubeFrame::CubeFrame(creator& c) : geometry_resource(
	c,
	layout_type<buffer_layout<syntax<position, 0, float3>, syntax<texcoord, 0, float2>>>{},
	raterizer_state::description{
	D3D11_FILL_MODE::D3D11_FILL_WIREFRAME,
	D3D11_CULL_MODE::D3D11_CULL_NONE,
	FALSE,
	0,
	0.0f,
	0.0f,
	true,
	false,
	false,
	false
}
)
{
	index.create_index(c, cube_static_3d_index);
	vertex.create_vertex(c, cube_static_3d2);
}

void CubeFrame::apply(stage_context& sc) {
	geometry_resource::apply(sc);
	sc << index << vertex[0] << index_call{ static_cast<UINT>(cube_static_3d_index.size()), 0, 0 };
}

UE4_cubiods_static_Frame::UE4_cubiods_static_Frame(creator& c) : geometry_resource(
	c, layout_type<buffer_layout<syntax<position, 0, float3>, syntax<texcoord, 0, float2>>>{},
	raterizer_state::description{
	D3D11_FILL_MODE::D3D11_FILL_WIREFRAME,
	D3D11_CULL_MODE::D3D11_CULL_NONE,
	FALSE,
	0,
	0.0f,
	0.0f,
	true,
	false,
	false,
	false
}
)
{
	index.create_index(c, cube_static_3d_index);
	vertex.create_vertex(c, ::staticl_2.data);
}

void UE4_cubiods_static_Frame::apply(stage_context& sc)
{
	geometry_resource::apply(sc);
	sc << index << vertex[0] << index_call{ static_cast<uint32_t>(cube_static_3d_index.size()), 0, 0 };
}