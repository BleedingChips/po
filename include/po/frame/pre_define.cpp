#include "pre_define.h"
namespace PO::ECSFramework
{
	namespace Implement 
	{
		bool type_index_view::operator ==(type_index_view tiv) const noexcept
		{
			if (view_count == tiv.view_count)
			{
				if (view == view)
					return true;
				else
				{
					for (size_t i = 0; i < view_count; ++i)
						if (view[i] != tiv.view[i])
							return false;
					return true;
				}
			}
			return false;
		}

		bool type_index_view::operator < (type_index_view tiv) const noexcept
		{
			if (view_count < tiv.view_count) return true;
			else if (view_count == tiv.view_count)
			{
				for (size_t i = 0; i < view_count; ++i)
				{
					if (view[i] < tiv.view[i])
						return true;
					else if (view[i] > tiv.view[i])
						return false;
				}
			}
			return false;
		}

		bool type_index_view::have(std::type_index ti) const noexcept
		{
			if (view_count == 0)
				return false;
			if (view[0] > ti || view[view_count - 1] < ti)
				return false;
			size_t Start = 0;
			size_t End = view_count;
			for (; Start != End; )
			{
				size_t C = (Start + End) / 2;
				if (view[C] == ti)
					return true;
				else if (view[C] < ti)
				{
					Start = C + 1;
				}
				else
				{
					End = C;
				}
			}
			return false;
		}

		bool type_index_view::is_collided(const type_index_view& ti) const noexcept
		{
			if (view_count != 0 && ti.view_count != 0 && view[view_count - 1] >= ti.view[0] && view[0] <= ti.view[ti.view_count - 1])
			{
				for (size_t index = 0; index <= view_count; ++index)
				{
					if (ti.have(view[index]))
						return true;
				}
			}
			return false;
		}

	}
}