#include "interface.h"
#include "../../po/tool/scene.h"
namespace {
	const std::set<std::type_index> default_acceptance_set{};

	PO::Tool::scope_lock<PO::scene>& get_shader_scene()
	{
		static PO::Tool::scope_lock<PO::scene> scene;
		return scene;
	}

}


namespace PO
{
	namespace Dx11
	{
		bool add_shader_path(std::type_index ti, const std::u16string& path)
		{
			auto& sce = ::get_shader_scene();
			return sce.lock([&](PO::scene& t) {
				return t.add_path(ti, path);
			});
		}

		void property_interface::push(creator& c) {}
		void property_interface::update(pipeline& p) {}
		property_interface::property_interface(std::type_index ti) : id_info(std::move(ti)), vision_for_update(0), is_need_to_push(true) {}
		bool property_interface::update_vision(pipeline& p, uint64_t v)
		{
			if (vision_for_update == v)
				return false;
			vision_for_update = v;
			update(p);
			return true;
		}
		property_interface::~property_interface() {}
		bool property_interface::push_implmenet(creator& c)
		{
			if (is_need_to_push)
			{
				push(c);
				is_need_to_push = false;
				return true;
			}
			return false;
			void force_update();
		}

		void property_interface::force_update(pipeline& p) { update(p); }



		placement_interface::~placement_interface() {}
		placement_interface::placement_interface(std::type_index ti) : id_info(ti) {}
		bool placement_interface::update(property_interface&, pipeline&) { return false; }
		const std::set<std::type_index>& placement_interface::acceptance() const { return default_acceptance_set; }
		void placement_interface::apply(pipeline& p) { p << vs; }
		bool placement_interface::load_vs(const std::u16string& path, creator& c)
		{
			return get_shader_scene().lock([&, this] (PO::scene& s) mutable {
				return s.load(path, true, [&, this](std::shared_ptr<PO::Dx::shader_binary> b) mutable {
					vs << c.create_vertex_shader(std::move(b));
				});
			});
		}

		geometry_interface::geometry_interface(std::type_index ti) : id_info(ti) {}
		geometry_interface::~geometry_interface() {}
		bool geometry_interface::update(property_interface&, pipeline&) { return false; }
		const std::set<std::type_index>& geometry_interface::acceptance() const { return default_acceptance_set; }
		void geometry_interface::update_layout(creator& c) {
			if (placement_ptr)
				c.update_layout(ia, placement_ptr->vs);
		}

		material_interface::~material_interface() {}
		bool material_interface::update(property_interface&, pipeline&) { return false; }
		void material_interface::apply(pipeline& c) { c << ps << bs; }
		material_interface::material_interface(std::type_index ti, renderer_order o) : id_info(ti), order(o) {}
		const std::set<std::type_index>& material_interface::acceptance() const { return default_acceptance_set; }
		bool material_interface::load_ps(std::u16string p, creator& c)
		{
			path = std::move(p);
			return get_shader_scene().lock([&, this](PO::scene& s) mutable {
				return s.load(path, true, [&, this](const PO::Dx::shader_binary& b) mutable {
					ps << c.create_pixel_shader(b);
				});
			});
		}

		compute_interface::compute_interface(std::type_index ti) : id_info(ti) {}
		compute_interface::~compute_interface() {}
		const std::set<std::type_index>& compute_interface::acceptance() const { return default_acceptance_set; }
		bool compute_interface::update(property_interface&, pipeline&) { return false; }
		bool compute_interface::load_cs(const std::u16string& path, creator& c)
		{
			return get_shader_scene().lock([&, this](PO::scene& s) mutable {
				return s.load(path, true, [&, this](const PO::Dx::shader_binary& b) mutable {
					cs << c.create_compute_shader(b);
				});
			});
		}

		namespace Implement
		{
			bool property_storage::have(std::type_index ti) const
			{
				return mapping.find(ti) == mapping.end();
			}
			void property_storage::clear() { mapping.clear(); }
			property_storage::~property_storage() {}
			/*
			void property_storage::update(pipeline& p, uint64_t u)
			{
				for (auto& ite : mapping)
					if(ite.second->update_vision(u))
						ite.second->update(p);
			}
			*/
			void property_storage::push(creator& c)
			{
				for (auto& ite : mapping)
					ite.second->push_implmenet(c);
			}
			void property_storage::push(std::shared_ptr<property_interface> p)
			{
				if (p)
				{
					auto index = p->id();
					mapping[index] = std::move(p);
				}
			}

			element_implement& element_implement::operator=(std::shared_ptr<geometry_interface> s) { geometry = std::move(s); return *this; }
			element_implement& element_implement::operator=(std::shared_ptr<material_interface> s) { 
				material = std::move(s); 
				if (material)
					order = material->get_order();
				return *this; 
			}
			element_implement& element_implement::operator=(std::shared_ptr<compute_interface> s) { compute.push_back(std::move(s)); return *this; }
			void element_implement::clear_compute() { compute.clear(); }

			bool element_implement::dispatch(pipeline& p, property_storage& out_mapping, uint64_t vision)
			{
				for (auto& com : compute)
				{
					auto& c_set = com->acceptance();
					for (auto& ite_set : c_set)
					{
						auto ite = property_map.mapping.find(ite_set);
						if (ite != property_map.mapping.end())
						{
							ite->second->update_vision(p, vision);
							if (!com->update(*ite->second, p)) return false;
						}
						else {
							auto ite2 = out_mapping.mapping.find(ite_set);
							if (ite2 != out_mapping.mapping.end())
							{
								ite2->second->update_vision(p, vision);
								if (!com->update(*ite2->second, p)) return false;
							}
							else
								return false;
						}
					}
					if (!com->draw(p)) return false;
				}
				return true;
			}

			bool element_implement::draw(pipeline& p, property_storage& out_mapping, uint64_t vision)
			{
				if (geometry && *geometry && material)
				{

					auto& g_set = geometry->acceptance();
					for (auto& ite_set : g_set)
					{
						auto ite = property_map.mapping.find(ite_set);
						if (ite != property_map.mapping.end())
						{
							ite->second->update_vision(p, vision);
							if (!geometry->update(*ite->second, p)) return false;
						}
						else {
							auto ite2 = out_mapping.mapping.find(ite_set);
							if (ite2 != out_mapping.mapping.end())
							{
								ite2->second->update_vision(p, vision);
								if (!geometry->update(*ite2->second, p)) return false;
							}else 
								return false;
						}
					}

					auto& gp_set = geometry->acceptance_placement();
					for (auto& ite_set : gp_set)
					{
						auto ite = property_map.mapping.find(ite_set);
						if (ite != property_map.mapping.end())
						{
							ite->second->update_vision(p, vision);
							if (!geometry->update_placement(*ite->second, p)) return false;
						}
						else {
							auto ite2 = out_mapping.mapping.find(ite_set);
							if (ite2 != out_mapping.mapping.end())
							{
								ite2->second->update_vision(p, vision);
								if (!geometry->update_placement(*ite2->second, p)) return false;
							}
							else
								return false;
						}
					}

					auto& m_set = material->acceptance();
					for (auto& ite_set : m_set)
					{
						auto ite = property_map.mapping.find(ite_set);
						if (ite != property_map.mapping.end())
						{
							ite->second->update_vision(p, vision);
							if (!material->update(*ite->second, p)) return false;
						}
						else {
							auto ite2 = out_mapping.mapping.find(ite_set);
							if (ite2 != out_mapping.mapping.end())
							{
								ite2->second->update_vision(p, vision);
								if (!material->update(*ite2->second, p)) return false;
							}
							else
								return false;
						}
					}
					material->apply(p);
					geometry->apply(p);
					geometry->draw(p);
					return true;
				}
				return false;
			}

			element_implement_storage::element_implement_storage() :vision_for_update(1) {}

			void element_implement_storage::clear_element()
			{
				for (auto& ite : element_ptr)
					ite.second.clear();
			}

			void element_implement_storage::update() {
				if (vision_for_update != 0xffffffffffffffff)
					++vision_for_update += 1;
				else
					vision_for_update = 1;
				//property_map.update(p, vision_for_update);
			}

			bool element_implement_storage::push_back(std::shared_ptr<element_implement> p, creator& c)
			{
				if (p)
				{
					p->push(c);
					renderer_order o = p->order;
					element_ptr[o].push_back(std::move(p));
					return true;
				}
				return false;
			}

			bool element_implement_storage::dispatch(pipeline& p)
			{
				for (auto& mapp : element_ptr)
				{
					for (auto& mapp_ite : mapp.second)
					{
						if (!mapp_ite->dispatch(p, property_map, vision_for_update)) return false;
					}
				}
				return true;
			}

			bool element_implement_storage::draw(renderer_order or , pipeline& p)
			{
				bool re = true;
				auto po = element_ptr.find(or);
				if (po != element_ptr.end())
					for (auto& i : po->second)
						if (!(re = (re && i &&  i->draw(p, property_map, vision_for_update)))) break;
				return re;
			}

			bool element_implement_storage::direct_draw(element_implement& p, pipeline& p2)
			{
				return p.draw(p2, property_map, vision_for_update);
			}
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

		element::operator Implement::element_implement&()
		{
			if (!ptr)
				ptr = std::make_shared<Implement::element_implement>();
			return *ptr;
		}

		void element::push(creator& c)
		{
			if (ptr)
				ptr->property_map.push(c);
		}
	}
}

