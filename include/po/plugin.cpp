#include "plugin.h"
namespace PO
{
	void plugins_implement::tick(viewer& v, duration da)
	{

		depute_renderer_function.lock([&, this](renderer_depute_f& tank) {
			if (tank)
			{
				//to-do call plugin to replace renderer
				renderer_ptr = tank(om);
				if(renderer_ptr)
					for (auto& ite : raw_plugin_tank) {
						auto ite22 = ite->mapping.find(renderer_ptr->id());
						if (ite22 != ite->mapping.end())
							renderer_ptr->insert(ite22->first, ite->self_ptr, ite22->second.init, ite22->second.tick, *this, v);
					}
				tank = {};
			}
		});

		if (extension_delegate_function.lock([this](decltype(extension_delegate_function)::type& t) {
			if (!t.empty()) return (std::swap(inside_extension_delegate_function, t), true);
			return false;
		}))
		{
			for (auto& ite : inside_extension_delegate_function)
			{
				if (ite)
				{
					auto ptr = ite(om);
					auto id = ptr->id();
					extension_map.insert({ id, std::move(ptr) });
				}
			}
			inside_extension_delegate_function = {};
		}

		if (depute_plugin_tank.lock([this](plugin_tank_t& tank) {
			if (!tank.empty()) return (std::swap(raw_plugin_tank, tank), true);
			return false;
		})) {
			if (renderer_ptr)
				for (auto& ite : raw_plugin_tank) {
					auto ite22 = ite->mapping.find(renderer_ptr->id());
					if (ite22 != ite->mapping.end())
						renderer_ptr->insert(ite22->first, ite->self_ptr, ite22->second.init, ite22->second.tick, *this, v);
				}
			plugin_tank.insert(plugin_tank.end(), std::make_move_iterator(raw_plugin_tank.begin()), std::make_move_iterator(raw_plugin_tank.end()));
			raw_plugin_tank.clear();
		}
		
		plugin_tank.erase(std::remove_if(plugin_tank.begin(), plugin_tank.end(), [&, this](std::unique_ptr<Implement::plugin_interface>& i) {
			if (*i)
				return (i->tick(*this, v, da), false);
			return true;
		}), plugin_tank.end());

		if (renderer_ptr)
			renderer_ptr->tick(*this, v, da);

		for (auto& ite : extension_map)
		{
			ite.second->tick(da, v);
		}

	}

	plugins::~plugins() {
	}

	Respond plugins_implement::respond(const event& ev, viewer& v)
	{
		// to-do add renderer_respond
		Respond re = Respond::Pass;

		for (auto& ite : extension_map)
		{
			ite.second->handle_respond(ev, v);
		}

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