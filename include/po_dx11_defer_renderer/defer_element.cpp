#include "defer_element.h"

namespace PO
{
	namespace Dx11
	{
		
		blend_state::description one_to_zero = blend_state::description{
			FALSE, FALSE, D3D11_RENDER_TARGET_BLEND_DESC{ TRUE, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL }
		};
		blend_state::description s_alpha_to_inv_s_alpha = blend_state::description{
			FALSE, FALSE, D3D11_RENDER_TARGET_BLEND_DESC{ TRUE, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL }
		};
		blend_state::description one_to_one = blend_state::description{
			FALSE, FALSE, D3D11_RENDER_TARGET_BLEND_DESC{ TRUE, D3D11_BLEND_ONE, D3D11_BLEND_ONE, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL }
		};

		defer_material_interface::defer_material_interface(std::type_index ti, render_order ro) : material_interface(ti), order_(ro) {}

		namespace Implement
		{
			void defer_element_implement::clear_all()
			{
				order_ = render_order::NotSet;
				element_implement::clear_all();
			}

			void defer_element_implement::set(std::shared_ptr<defer_material_interface> p)
			{
				if (p)
					order_ = p->order();
				else
					order_ = render_order::NotSet;
				material_ptr = std::move(p);
			}
		}

		void defer_element::clear_all()
		{
			if (element_ptr)
				element_ptr->clear_all();
		}

		bool defer_element::check(const property_mapping& pm) const 
		{ 
			if (element_ptr) 
				return element_ptr->check(pm); 
			return true; 
		}

		Implement::stage_interface::acceptance_t defer_element::lack_acceptance() const
		{
			if (element_ptr)
				return element_ptr->lack_acceptance();
			return {};
		}
		Implement::stage_interface::acceptance_t defer_element::lack_acceptance(const property_mapping& pm) const
		{
			if (element_ptr)
				return element_ptr->lack_acceptance(pm);
			return {};
		}

		void defer_element::check_ptr()
		{
			if (!element_ptr) element_ptr = std::make_shared<Implement::defer_element_implement>();
		}

		template<typename M, typename F> auto find_and_create(creator& c, const std::type_index& ti, M& mapping, const F& f)
		{
			auto ite = mapping.find(ti);
			if (ite == mapping.end() || ite->second.expired())
			{
				auto ptr = f();
				ptr->init(c);
				mapping[ptr->id()] = ptr;
				return ptr;
			}
			else
				return ite->second.lock();
		}

		void interface_storage::make_geometry_placement(creator& c, defer_element& d,
			const std::type_index& ti1, const std::function<std::shared_ptr<geometry_interface>()>& f1,
			const std::type_index& ti2, const std::function<std::shared_ptr<placement_interface>()>& f2
		)
		{
			d.check_ptr();
			bool update = false;
			auto& p1 = (d.element_ptr)->geometry_ptr;
			if (!p1 || p1->id() != ti1)
			{
				update = true;
				p1 = find_and_create(c, ti1, geometry_ptr, f1);
			}


			auto& p2 = (d.element_ptr)->placemenet_ptr;
			if (!p2 || p2->id() != ti1)
			{
				update = true;
				p2 = find_and_create(c, ti2, placement_ptr, f2);
			}

			if (update)
				(d.element_ptr)->layout = c.create_layout(p1->ia(), p2->vs());
		}

		void interface_storage::make_material(creator& c, defer_element& d, const std::type_index& ti1, const std::function<std::shared_ptr<defer_material_interface>()>& f1)
		{
			d.check_ptr();
			auto& p1 = (d.element_ptr)->material_ptr;
			if (!p1 || p1->id() != ti1)
			{
				auto p = find_and_create(c, ti1, material_ptr, f1);
				(d.element_ptr)->set(p);
			}
		}
		std::shared_ptr<compute_interface> interface_storage::make_compute(creator& c, defer_element& d, const std::type_index& t, const std::function<std::shared_ptr<compute_interface>()>& f)
		{
			d.check_ptr();
			auto ptr = find_and_create(c, t, compute_ptr, f);
			(d.element_ptr)->compute_vector.push_back(ptr);
			return ptr;
		}

		void defer_element_implement_storage::draw(render_order or , pipeline& p)
		{
			for (auto& ite : element_ptr[or ])
				ite->draw(p, mapping);
		}
		void defer_element_implement_storage::draw(defer_element& de, pipeline& p)
		{
			if (de.element_ptr)
				(de.element_ptr)->draw(p, mapping);
		}
		bool defer_element_implement_storage::insert(const defer_element& de)
		{
			if (de.element_ptr)
			{
				render_order ro = (de.element_ptr)->order();
				element_ptr[ro].push_back(de.element_ptr);
			}
			return false;
		}
		void defer_element_implement_storage::clear()
		{
			element_ptr.clear();
		}
		Implement::stage_interface::acceptance_t defer_element_implement_storage::check_acceptance(const defer_element& p) const
		{
			if (p.element_ptr)
				return (p.element_ptr)->lack_acceptance(mapping);
			return {};
		}

		/*
		void property_constructor::push(creator& c) {}
		void property_constructor::update(pipeline& p) {}
		property_constructor::property_constructor(std::type_index ti) : id_info(std::move(ti)), vision_for_update(0), is_need_to_push(true) {}
		bool property_constructor::update_vision(pipeline& p, uint64_t v)
		{
			if (vision_for_update == v)
				return false;
			vision_for_update = v;
			update(p);
			return true;
		}
		property_constructor::~property_constructor() {}
		bool property_constructor::push_implmenet(creator& c)
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

		void property_constructor::force_update(pipeline& p) { update(p); }



		placement_interface::~placement_interface() {}
		placement_interface::placement_interface(std::type_index ti) : id_info(ti) {}
		bool placement_interface::update(property_constructor&, pipeline&) { return false; }
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
		void geometry_interface::pre_push(property_constructor&) {}
		bool geometry_interface::update(property_constructor&, pipeline&) { return false; }
		const std::set<std::type_index>& geometry_interface::acceptance() const { return default_acceptance_set; }
		void geometry_interface::update_layout(creator& c) {
			if (placement_ptr)
				c.update_layout(ia, placement_ptr->vs);
		}
		void geometry_interface::pre_undate(property_constructor&, pipeline&) {}

		material_interface::~material_interface() {}
		bool material_interface::update(property_constructor&, pipeline&) { return false; }
		void material_interface::apply(pipeline& c) { c << ps << bs; }
		material_interface::material_interface(std::type_index ti, render_order o) : id_info(ti), order(o) {}
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
		bool compute_interface::update(property_constructor&, pipeline&) { return false; }
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
			bool property_interface::have(std::type_index ti) const
			{
				return mapping.find(ti) != mapping.end();
			}
			void property_interface::clear() { mapping.clear(); }
			property_interface::~property_interface() {}
			
			void property_interface::update(pipeline& p, uint64_t u)
			{
				for (auto& ite : mapping)
					if(ite.second->update_vision(u))
						ite.second->update(p);
			}
			
			void property_interface::push(creator& c)
			{
				for (auto& ite : mapping)
					ite.second->push_implmenet(c);
			}

			void property_interface::insert(std::shared_ptr<property_constructor> p)
			{
				if (p)
				{
					auto index = p->id();
					mapping[index] = std::move(p);
				}
			}

			void property_interface::pre_push(geometry_interface& gi)
			{
				auto& sett = gi.acceptance_placement();
				for (auto& ite : sett)
				{
					auto ite2 = mapping.find(ite);
					if (ite2 != mapping.end())
						gi.pre_push(*(ite2->second));
				}
			}

			void element_implement::dispatch_imp(pipeline& p, property_interface& out_mapping, uint64_t vision)
			{
				if (!dispatch(p, out_mapping, vision))
					state = render_state::Fail;
			}

			void element_implement::draw_imp(pipeline& p, property_interface& out_mapping, uint64_t vision)
			{
				if (state == render_state::AtList)
					state = draw(p, out_mapping, vision) ? render_state::Success : render_state::Fail;
			}

			std::set<std::type_index> element_implement::ckeck(const property_interface& p) const
			{
				std::set<std::type_index> result;
				for (auto& i : compute)
				{
					auto& ioset = i->acceptance();
					for (auto& ti : ioset)
					{
						if (!(property_map.have(ti) || p.have(ti) ))
							result.insert(ti);
					}
				}
				if (geometry && *geometry)
				{
					auto& set_ref = geometry->acceptance();
					for (auto& ti : set_ref)
					{
						if (!(property_map.have(ti) || p.have(ti)))
							result.insert(ti);
					}
					auto& set_ref2 = geometry->acceptance_placement();
					for (auto& ti : set_ref2)
					{
						if (!(property_map.have(ti) || p.have(ti)))
							result.insert(ti);
					}
				}
				if (material)
				{
					auto& set_ref = material->acceptance();
					for (auto& ti : set_ref)
					{
						if (!(property_map.have(ti) || p.have(ti)))
							result.insert(ti);
					}
				}
				return std::move(result);
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

			bool element_implement::dispatch(pipeline& p, property_interface& out_mapping, uint64_t vision)
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

			bool element_implement::draw(pipeline& p, property_interface& out_mapping, uint64_t vision)
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

			void element_implement::push(creator& c)
			{
				if (geometry && *geometry)
					property_map.pre_push(*geometry);
				property_map.push(c);
				state = render_state::AtList;
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
					render_order o = p->order;
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

			bool element_implement_storage::draw(render_order or , pipeline& p)
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
				p.push(p2.get_creator());
				return p.draw(p2, property_map, vision_for_update);
			}

			std::set<std::type_index> element_implement_storage::check(const std::shared_ptr<element_implement>& p) const
			{
				if (p)
					return p->ckeck(property_map);
				else return {};
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
		}*/

		/*
		void element::push(creator& c)
		{
			if (ptr)
				ptr->property_map.push(c);
		}
		*/
	}
}

