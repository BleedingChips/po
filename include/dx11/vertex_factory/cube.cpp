#include "cube.h"
#include "../dx11_vertex.h"
using namespace PO::Dx;
using namespace PO::Dx11;
struct position
{
	const char* operator()() { return "POSITION"; }
};

struct texcoord
{
	const char* operator()() { return "TEXCOORD"; }
};

struct diffuse
{
	const char* operator()() { return "DIFFUSE"; }
};

struct cube_ver
{
	float3 poi;
	float3 col;
	using type = PO::Dx11::layout_type<PO::Dx11::syntax<position, 0, float3>, PO::Dx11::syntax<diffuse, 0, float3>>;
};

std::vector<cube_ver> poi =
{
	{ { -0.5, -0.5, 0.5 },{ 1.0, 0.0, 0.0 } },
	{ { 0.5, -0.5, 0.5 },{ 1.0, 0.0, 0.0 } },
	{ { 0.5, 0.5, 0.5 },{ 1.0, 0.0, 0.0 } },
	{ { -0.5, 0.5, 0.5 },{ 1.0, 0.0, 0.0 } },

	{ { 0.5, -0.5, 0.5 },{ 0.0, 1.0, 0.0 } },
	{ { 0.5, -0.5, -0.5 },{ 0.0, 1.0, 0.0 } },
	{ { 0.5, 0.5, -0.5 },{ 0.0, 1.0, 0.0 } },
	{ { 0.5, 0.5, 0.5 },{ 0.0, 1.0, 0.0 } },


	{ { 0.5, -0.5, -0.5 },{ 0.0, 0.0, 1.0 } },
	{ { -0.5, -0.5, -0.5 },{ 0.0, 0.0, 1.0 } },
	{ { -0.5, 0.5, -0.5 },{ 0.0, 0.0, 1.0 } },
	{ { 0.5, 0.5, -0.5 },{ 0.0, 0.0, 1.0 } },

	{ { -0.5, -0.5, -0.5 },{ 1.0, 0.0, 1.0 } },
	{ { -0.5, -0.5, 0.5 },{ 1.0, 0.0, 1.0 } },
	{ { -0.5, 0.5, 0.5 },{ 1.0, 0.0, 1.0 } },
	{ { -0.5, 0.5, -0.5 },{ 1.0, 0.0, 1.0 } },

	{ { -0.5, 0.5, 0.5 },{ 1.0, 1.0, 0.0 } },
	{ { 0.5, 0.5, 0.5 },{ 1.0, 1.0, 0.0 } },
	{ { 0.5, 0.5, -0.5 },{ 1.0, 1.0, 0.0 } },
	{ { -0.5, 0.5, -0.5 },{ 1.0, 1.0, 0.0 } },

	{ { -0.5, -0.5, -0.5 },{ 0.0, 1.0, 1.0 } },
	{ { 0.5, -0.5, -0.5 },{ 0.0, 1.0, 1.0 } },
	{ { 0.5, -0.5, 0.5 },{ 0.0, 1.0, 1.0 } },
	{ { -0.5, -0.5, 0.5 },{ 0.0, 1.0, 1.0 } }
};

const std::vector<uint16_t> ind =
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


const UINT draw_count = static_cast<UINT>(ind.size());

namespace PO
{
	namespace Dx11
	{
		void cube_vertex_factor::init(creator& i)
		{
			vertex_factory::init(i);
			ia << i.create_vertex(poi, layout_type<syntax<position, 0, float3>, syntax<diffuse, 0, float3>>{})[0]
				<< i.create_index(ind);
			draw_index = draw_count;
		}
	}
}