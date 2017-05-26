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
	}
}