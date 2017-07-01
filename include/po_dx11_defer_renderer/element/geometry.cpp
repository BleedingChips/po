#include "geometry.h"
#include "../../po_dx11/dx11_vertex.h"
#include "placement.h"
namespace
{
	struct SCREEN_INDEX
	{
		const char* operator()() { return "SCREEN_INDEX"; }
	};

	struct POSITION
	{
		const char* operator()() { return "POSITION"; }
	};

	struct TEXCOORD
	{
		const char* operator()() { return "TEXCOORD"; }
	};

	std::array<uint16_t, 6> screen_vertex = { 0, 1, 2, 2, 3, 0 };

}

namespace PO
{
	namespace Dx11
	{
		namespace Geometry
		{
			void screen_square::draw(pipeline& pl)
			{
				pl << ia << ra;
				pl.draw_vertex(6, 0);
			}

			screen_square::screen_square() : geometry_interface(typeid(screen_square)) {}

			void screen_square::init(creator& c, raw_scene& rs, interface_storage& eis)
			{
				ia << c.create_vertex(screen_vertex, layout_type<syntax<SCREEN_INDEX, 0, uint16_t>>{})[0];
				decltype(ra)::dscription des = decltype(ra)::default_dscription;
				des.CullMode = decltype(des.CullMode)::D3D11_CULL_NONE;
				ra = c.create_raterizer_state(des);
				set_placement<Placement::screen_square>(eis, c);
			}
		}

	}
}

namespace
{
	using namespace PO::Dx;
	using namespace PO::Dx11;
	struct cube_vertex
	{
		float3 poi;
		float2 tex;
		using type = layout_type<syntax<POSITION, 0, float3>, syntax<TEXCOORD, 0, float2>>;
	};

	std::array<cube_vertex, 24> cube_poi =
	{
		cube_vertex{ { -1.0f, -1.0f, 1.0f },{ 0.0f, 1.0f } },
		{ { 1.0, -1.0, 1.0 },{ 1.0, 1.0 } },
		{ { 1.0, 1.0, 1.0 },{ 1.0, 0.0 } },
		{ { -1.0, 1.0, 1.0 },{ 0.0, 0.0 } },

		{ { 1.0, -1.0, 1.0 },{ 0.0, 1.0 } },
		{ { 1.0, -1.0, -1.0 },{ 1.0, 1.0 } },
		{ { 1.0, 1.0, -1.0 },{ 1.0, 0.0 } },
		{ { 1.0, 1.0, 1.0 },{ 0.0, 0.0 } },


		{ { 1.0, -1.0, -1.0 },{ 0.0, 1.0 } },
		{ { -1.0, -1.0, -1.0 },{ 1.0, 1.0 } },
		{ { -1.0, 1.0, -1.0 },{ 1.0, 0.0 } },
		{ { 1.0, 1.0, -1.0 },{ 0.0, 0.0 } },


		{ { -1.0, -1.0, -1.0 },{ 0.0, 1.0 } },
		{ { -1.0, -1.0, 1.0 },{ 1.0, 1.0 } },
		{ { -1.0, 1.0, 1.0 },{ 1.0, 0.0 } },
		{ { -1.0, 1.0, -1.0 },{ 0.0, 0.0 } },

		{ { -1.0, 1.0, 1.0 },{ 0.0, 1.0 } },
		{ { 1.0, 1.0, 1.0 },{ 1.0, 1.0 } },
		{ { 1.0, 1.0, -1.0 },{ 1.0, 0.0 } },
		{ { -1.0, 1.0, -1.0 },{ 0.0, 0.0 } },

		{ { -1.0, -1.0, -1.0 },{ 0.0, 1.0 } },
		{ { 1.0, -1.0, -1.0 },{ 1.0, 1.0 } },
		{ { 1.0, -1.0, 1.0 },{ 1.0, 0.0 } },
		{ { -1.0, -1.0, 1.0 },{ 0.0, 0.0 } }
	};

	std::array<uint16_t, 36> cube_ind =
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

namespace PO
{
	namespace Dx11
	{
		namespace Geometry
		{
			cube_static_3d::cube_static_3d() : geometry_interface(typeid(cube_static_3d)) {}
			void cube_static_3d::draw(pipeline& p)
			{
				p << ia << ra;
				p.draw_index(36, 0, 0);
			}

			void cube_static_3d::init(creator& c, raw_scene& rs, interface_storage& eis)
			{
				ia << c.create_vertex(cube_poi, cube_vertex::type{})[0]
					<< c.create_index(cube_ind);
				auto des = raterizer_state::default_dscription;
				//des.FrontCounterClockwise = TRUE;
				ra = c.create_raterizer_state(des);
				set_placement<Placement::static_3d>(eis, c);
			}
		}
	}
}