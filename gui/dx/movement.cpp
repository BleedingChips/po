#include "movement.h"
namespace PO
{
	namespace Dx
	{

		int opposite_direct::final_direction() const
		{
			if (direction == 1) return -1;
			if (direction == 2) return 1;
			return 0;
		}

		movement_interpolation::operator float4x4() const
		{
			float sx = sinf(eul.x), sy = sinf(eul.y), sz = sinf(eul.z);
			float cx = cosf(eul.x), cy = cosf(eul.y), cz = cosf(eul.z);
			return float4x4{
				cy * cz * sca.x, (cz*sx*sy - cx * sz) * sca.x, (sx*sz + cx*cz*sy) * sca.x, 0.0,
				cy*sz *sca.y, (cx*cz + sx*sy*sz) * sca.y, (cx*sy*sz - cz*sx) * sca.y, 0.0,
				(-sy) * sca.z, cy*sx*sca.z, cx*cy * sca.z, 0.0,
				poi.x, poi.y, poi.z, 1.0
			};
		}

		movement_interpolation::operator matrix() const
		{
			float sx = sinf(eul.x), sy = sinf(eul.y), sz = sinf(eul.z);
			float cx = cosf(eul.x), cy = cosf(eul.y), cz = cosf(eul.z);
			return matrix{
				cy * cz * sca.x, (cz*sx*sy - cx * sz) * sca.x, (sx*sz+cx*cz*sy) * sca.x, 0.0,
				cy*sz *sca.y, (cx*cz+sx*sy*sz) * sca.y, (cx*sy*sz - cz*sx) * sca.y, 0.0,
				(-sy) * sca.z, cy*sx*sca.z, cx*cy * sca.z, 0.0,
				poi.x, poi.y, poi.z, 1.0
			};
		}
	}
}