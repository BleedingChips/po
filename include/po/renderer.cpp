#include "renderer.h"
namespace PO
{
	namespace Implement {

		bool renderer_interface::insert(const std::type_index& ti, std::weak_ptr<self> ptr, adapter_store_t::init_t& it, adapter_store_t::tick_t tf, plugins& p, viewer& v)
		{
			if (ti == id())
			{
				auto ptr_s = ptr.lock();
				if (ptr_s)
				{
					if(it)
						it(*ptr_s, p, v, *this);
					if (tf)
					{
						tick_proxy_list.emplace_back(std::move(ptr), std::move(tf));
						return true;
					}
				}
			}
			return false;
		}

		void renderer_interface::tick(plugins& p, viewer& v, duration da)
		{
			pre_tick(da);
			tick_proxy_list.erase(std::remove_if(tick_proxy_list.begin(), tick_proxy_list.end(), [da, &p, &v, this](tick_proxy& i) {
				return !i(p, v, *this, da);
			}), tick_proxy_list.end());
			pos_tick(da);
		}


		bool tick_proxy::operator()(plugins & p, viewer & v, renderer_interface & r, duration d)
		{
			auto ptr = self_ptr.lock();
			if (ptr && tick)
			{
				tick(*ptr, p, v, r, d);
				return true;
			}
			return false;
		}

		renderer_interface::~renderer_interface() {}

	}
}