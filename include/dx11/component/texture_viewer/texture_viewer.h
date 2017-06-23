#pragma once
#include "../../../po/tool/scene.h"
#include "../../dx11_frame.h"
namespace PO 
{
	namespace Dx11
	{
		struct texture_viewer {
			input_assember_stage is;
			vertex_stage vs;
			pixel_stage ps;
			raw_scene rs;
			float zoom = 1.0;
			void init(creator& c);
			void tick(const unordered_access_view& uav, pipeline& p);
		};
	}
}