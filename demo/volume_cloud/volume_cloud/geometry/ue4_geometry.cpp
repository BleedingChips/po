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
	multy_50()
	{
		for (auto& ite : ::cube_static_3d2)
			ite.position = ite.position * 50;
	}
};
multy_50 staticl;

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
		c, layout_type<buffer_layout<syntax<position, 0, float3>, syntax<texcoord, 0, float2>>>{}
		)
{
	index.create_index(c, cube_static_3d_index);
	vertex.create_vertex(c, cube_static_3d2);
}

void UE4_cube_static::apply(stage_context& sc)
{
	geometry_resource::apply(sc);
	sc << index << vertex[0] << index_call{ static_cast<uint32_t>(cube_static_3d_index.size()), 0, 0};
}