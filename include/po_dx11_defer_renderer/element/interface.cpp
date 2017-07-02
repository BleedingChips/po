#include "interface.h"
namespace {
	const std::set<std::type_index> default_acceptance_set{};
}



namespace PO
{
	namespace Dx11
	{

		void property_interface::init(creator& c) {}
		void property_interface::update(pipeline& p) {}
		property_interface::property_interface(std::type_index ti) : id_info(std::move(ti)) {}
		property_interface::~property_interface() {}


		placement_interface::~placement_interface() {}
		placement_interface::placement_interface(std::type_index ti) : id_info(ti) {}
		bool placement_interface::update(property_interface&, pipeline&, creator&) { return false; }
		const std::set<std::type_index>& placement_interface::acceptance() const { return default_acceptance_set; }
		void placement_interface::apply(pipeline& p) { p << vs; }

		geometry_interface::geometry_interface(std::type_index ti) : id_info(ti) {}
		geometry_interface::~geometry_interface() {}
		bool geometry_interface::update(property_interface&, pipeline&, creator&) { return false; }
		const std::set<std::type_index>& geometry_interface::acceptance() const { return default_acceptance_set; }
		void geometry_interface::update_layout(creator& c) {
			if (placement_interface)
				c.update_layout(ia, placement_interface->vs);
		}

		material_interface::~material_interface() {}
		bool material_interface::update(property_interface&, pipeline&, creator&) { return false; }
		void material_interface::apply(pipeline& c) { c << ps << bs; }
		material_interface::material_interface(std::type_index ti, renderer_order o) : id_info(ti), order(o) {}
		const std::set<std::type_index>& material_interface::acceptance() const { return default_acceptance_set; }

		namespace Implement
		{
			bool property_storage::have(std::type_index ti) const
			{
				return mapping.find(ti) == mapping.end();
			}
			void property_storage::clear() { mapping.clear(); }
			property_storage::~property_storage() {}
			void property_storage::update(pipeline& p)
			{
				for (auto& ite : mapping)
					ite.second->update(p);
			}
			void property_storage::init(creator& c)
			{
				for (auto& ite : mapping)
					ite.second->init(c);
			}
			void property_storage::push(std::shared_ptr<property_interface> p)
			{
				if (p)
				{
					auto index = p->id();
					mapping[index] = std::move(p);
				}
			}

			bool element_implement::call(pipeline& p, creator& c, const property_storage& out_mapping)
			{
				if (geometry && *geometry && material)
				{
					auto& g_set = geometry->acceptance();
					for (auto& ite_set : g_set)
					{
						auto ite = property_map.mapping.find(ite_set);
						if (ite != property_map.mapping.end())
						{
							ite->second->update(p);
							if (!geometry->update(*ite->second, p, c)) return false;
						}
						else {
							auto ite2 = out_mapping.mapping.find(ite_set);
							if (!(ite2 != out_mapping.mapping.end() && geometry->update(*ite2->second, p, c))) return false;
						}
					}

					auto& gp_set = geometry->acceptance_placement();
					for (auto& ite_set : gp_set)
					{
						auto ite = property_map.mapping.find(ite_set);
						if (ite != property_map.mapping.end())
						{
							auto ite_ = g_set.find(ite_set);
							if (ite_ == g_set.end())
								ite->second->update(p);
							if (!geometry->update_placement(*ite->second, p, c)) return false;
						}
						else {
							auto ite2 = out_mapping.mapping.find(ite_set);
							if (!(ite2 != out_mapping.mapping.end() && geometry->update_placement(*ite2->second, p, c))) return false;
						}
					}

					auto& m_set = material->acceptance();
					for (auto& ite_set : m_set)
					{
						auto ite = property_map.mapping.find(ite_set);
						if (ite != property_map.mapping.end())
						{
							auto ite_ = g_set.find(ite_set);
							if (ite_ == g_set.end())
							{
								auto ite__ = gp_set.find(ite_set);
								if(ite__ == gp_set.end())
									ite->second->update(p);
							}
							if (!material->update(*ite->second, p, c)) return false;
						}
						else {
							auto ite2 = out_mapping.mapping.find(ite_set);
							if (!(ite2 != out_mapping.mapping.end() && material->update(*ite2->second, p, c))) return false;
						}
					}
					material->apply(p);
					geometry->apply(p);
					geometry->draw(p);
					return true;
				}
				return false;
			}

			bool element_implement::direct_call(pipeline& p, creator& c)
			{
				if (geometry && *geometry && material)
				{
					auto& g_set = geometry->acceptance();
					for (auto& ite_set : g_set)
					{
						auto ite = property_map.mapping.find(ite_set);
						if (ite != property_map.mapping.end())
						{
							ite->second->update(p);
							if (!geometry->update(*ite->second, p, c)) return false;
						}
					}

					auto& gp_set = geometry->acceptance_placement();
					for (auto& ite_set : gp_set)
					{
						auto ite = property_map.mapping.find(ite_set);
						if (ite != property_map.mapping.end())
						{
							auto ite_ = g_set.find(ite_set);
							if (ite_ == g_set.end())
								ite->second->update(p);
							if (!geometry->update_placement(*ite->second, p, c)) return false;
						}
					}

					auto& m_set = material->acceptance();
					for (auto& ite_set : m_set)
					{
						auto ite = property_map.mapping.find(ite_set);
						if (ite != property_map.mapping.end())
						{
							auto ite_ = g_set.find(ite_set);
							if (ite_ == g_set.end())
							{
								auto ite__ = gp_set.find(ite_set);
								if (ite__ == gp_set.end())
									ite->second->update(p);
							}
							if (!material->update(*ite->second, p, c)) return false;
						}
					}

					material->apply(p);
					geometry->apply(p);
					geometry->draw(p);
					return true;
				}
				return false;
			}

			void update(pipeline& pl);

			void element_implement_storage::clear_element()
			{
				for (auto& ite : element_ptr)
					ite.second.clear();
			}

			bool element_implement_storage::push_back(std::shared_ptr<element_implement> p)
			{
				if (p)
				{
					renderer_order o = p->order;
					element_ptr[o].push_back(std::move(p));
					return true;
				}
				return false;
			}

			bool element_implement_storage::call(renderer_order or , pipeline& p, creator& c)
			{
				bool re = true;
				auto po = element_ptr.find(or);
				if (po != element_ptr.end())
					for (auto& i : po->second)
						if (!(re = (re && i && i->call(p, c, property_map)))) break;
				return re;
			}

		}

		bool element::direct_call(pipeline& p, creator& c)
		{
			if (!ptr) return false;
			return ptr->direct_call(p, c);
		}

		void element::clear()
		{
			ptr.reset();
		}

		void element::clear_property()
		{
			if (ptr)
				ptr->property_map.clear();
		}

		void element::init(creator& c)
		{
			if (ptr)
				ptr->property_map.init(c);
		}
	}
}

