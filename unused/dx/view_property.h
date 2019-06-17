#pragma once
#include "transformation.h"
namespace PO
{
	namespace Dx
	{

		struct view_perspective_3d
		{
			float4 perspective_property;
			float4x4 projection;
			float4 view_size;
		};
	}
}