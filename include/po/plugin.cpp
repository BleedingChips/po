#include "plugin.h"
namespace PO
{
	void plugins::tick(viewer& v, duration da)
	{

		depute_renderer_f_tank.lock([this](renderer_depute_tank_t& tank) {
			if (!tank.empty())
			{
				renderer_tank.reserve(renderer_tank.size() + tank.size());
				for (auto& f : tank)
					renderer_tank.push_back(std::move(f(om)));
				tank.clear();
			}
		});

		if (depute_plugin_tank.lock([this](plugin_tank_t& tank) {
			if (!tank.empty()) return (std::swap(raw_plugin_tank, tank), true);
			return false;
		})) {
			for (auto& ite : raw_plugin_tank) {
				for (auto& ite2 : renderer_tank) {
					auto ite22 = ite->mapping.find(ite2->id());
					if (ite22 != ite->mapping.end())
						ite2->insert(ite22->first, ite->self_ptr, ite22->second.init, ite22->second.tick, *this, v);
				}
			}
			plugin_tank.insert(plugin_tank.end(), std::make_move_iterator(raw_plugin_tank.begin()), std::make_move_iterator(raw_plugin_tank.end()));
			raw_plugin_tank.clear();
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

	plugins::~plugins() {
	}

	Respond plugins::respond(const event& ev, viewer& v)
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