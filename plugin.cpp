#include "plugin.h"
namespace PO
{
	void plugins::tick(viewer& v, duration da)
	{

		if (depute_renderer_tank.lock([this](renderer_tank_t& tank) {
			if (!tank.empty()) return (std::swap(raw_renderer_tank, tank), true);
			return false;
		})) {
			for (auto& ite : raw_renderer_tank)
			{
				ite->init(om);
				for (auto& ite2 : plugin_tank)
					ite->plugin_register(ite2->self_ptr, ite2->mapping);
			}
			renderer_tank.insert(renderer_tank.end(), std::make_move_iterator(raw_renderer_tank.begin()), std::make_move_iterator(raw_renderer_tank.end()));
			raw_renderer_tank.clear();
		}
		

		if (depute_plugin_tank.lock([this](plugin_tank_t& tank) {
			if (!tank.empty()) return (std::swap(raw_plugin_tank, tank), true);
			return false;
		})) {
			for (auto& ite : raw_plugin_tank) {
				for (auto& ite2 : renderer_tank) {
					ite2->plugin_register(ite->self_ptr, ite->mapping);
				}
			}
			plugin_tank.insert(plugin_tank.end(), std::make_move_iterator(raw_plugin_tank.begin()), std::make_move_iterator(raw_plugin_tank.end()));
		}
		
		plugin_tank.erase(std::remove_if(plugin_tank.begin(), plugin_tank.end(), [&, this](std::unique_ptr<Implement::plugin_interface>& i) {
			if (*i)
				return (i->tick(*this, v, da), false);
			return true;
		}), plugin_tank.end());

		std::for_each(renderer_tank.begin(), renderer_tank.end(), [&, this](std::unique_ptr<Implement::renderer_interface>& i) {
			i->tick(*this, v, da);
		});

	}

	Respond plugins::respond(event& ev, viewer& v)
	{
		Respond re = Respond::Pass;
		for (auto& po : plugin_tank) {
			if (*po) {
				re = po->respond(ev, *this, v);
				if (re != Respond::Pass)
					break;
			}
		}
		return re;
	}
}