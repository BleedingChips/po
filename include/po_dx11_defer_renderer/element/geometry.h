#pragma once
#include "interface.h"
#include "../../po/tool/scene.h"
namespace PO
{
	namespace Dx11
	{
		namespace Geometry
		{
			class screen_square : public geometry_interface
			{
			public:
				void draw(pipeline& p);
				screen_square();
				virtual void init(creator& c, raw_scene& rs, interface_storage&) override;
			};

			class cube_static_3d : public geometry_interface
			{
			public:
				UINT index_draw;
				float width;
				void draw(pipeline& p);
				cube_static_3d();
				virtual void init(creator& c, raw_scene& rs, interface_storage&) override;
			};
		}
	}
}
