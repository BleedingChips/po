#include "renderer.h"
#include "plugin.h"
namespace PO
{
	namespace Implement {

		void renderer_interface::tick(plugins& p, viewer& v, duration da)
		{
			for (auto& i : init_proxy_list)
			{
				if (*i.self_ptr)
					i.ptr(*i.self_ptr, p, v);
			}
			init_proxy_list.clear();


			pre_tick(da);
			tick_proxy_list.erase(std::remove_if(tick_proxy_list.begin(), tick_proxy_list.end(), [da, &p, &v, this](tick_proxy& i) {
				if (*i.self_ptr)
					return (i.ptr(*i.self_ptr, p, v, da), false);
				return true;
			}), tick_proxy_list.end());
			pos_tick(da);
		}


	}
}