#include "dx11_element.h"
#include "../po/tool/scene.h"
namespace {
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

		namespace Implement
		{
			base_interface::base_interface(std::type_index ti) : id_info(ti) {}
			base_interface::~base_interface() {}
		}

		void property_storage::update_imp(pipeline& p) {  }
		void property_storage::update(pipeline& p) {
			if (need_to_be_update)
			{
				need_to_be_update = false;
				update_imp(p);
			}
		}

		property_storage& property_mapping::create(const typename property_interface::acception_t::value_type& vt)
		{
			auto ite = mapping.find(vt.first);
			if (ite == mapping.end())
			{
				auto ptr = vt.second();
				mapping.insert({ ptr->id(), ptr });
				return *ptr;
			}
			return *(ite->second);
		}
		property_storage& property_mapping::recreate(const typename property_interface::acception_t::value_type& vt)
		{
			auto ptr = vt.second();
			mapping.insert({ ptr->id(), ptr });
			return *ptr;
		}
		bool property_mapping::create_and_construct(const typename property_interface::acception_t::value_type& vt, property_interface& pi, creator& c)
		{
			auto& ref = create(vt);
			return pi.construct(ref, c);
		}
		bool property_mapping::recreate_and_construct(const typename property_interface::acception_t::value_type& vt, property_interface& pi, creator& c)
		{
			auto& ref = recreate(vt);
			return pi.construct(ref, c);
		}
		void property_mapping::clear() { mapping.clear(); }
		bool property_mapping::have(std::type_index ti) const
		{
			auto ite = mapping.find(ti);
			return ite != mapping.end();
		}
		bool property_mapping::insert(std::shared_ptr<property_storage> sp)
		{
			if (sp)
			{
				auto id = sp->id();
				mapping.insert({ id, std::move(sp) });
				return true;
			}
			return false;
		}
		void property_mapping::update(pipeline& p)
		{
			for (auto& it : mapping)
				(it.second)->update(p);
		}

		namespace Implement
		{
			bool stage_interface::update_imp(property_storage&, pipeline&) { return false; }
			auto stage_interface::acceptance() const -> const acceptance_t&
			{
				static const acceptance_t acce{};
				return acce;
			}
			bool stage_interface::update(pipeline& p, property_mapping& pi, property_mapping& pm)
			{
				for (auto& acce_ite : acceptance())
				{
					return pi.find(acce_ite, [&, this](property_storage& ps) {
						ps.update(p);
						return update_imp(ps, p);
					}) || pm.find(acce_ite, [&, this](property_storage& ps) {
						ps.update(p);
						return update_imp(ps, p);
					}) || default_mapping.find(acce_ite, [&, this](property_storage& ps) {
						ps.update(p);
						return update_imp(ps, p);
					});
				}
				return true;
			}
			bool stage_interface::update(pipeline& p, property_mapping& pi)
			{
				for (auto& acce_ite : acceptance())
				{
					return pi.find(acce_ite, [&, this](property_storage& ps) {
						ps.update(p);
						return update_imp(ps, p);
					}) || default_mapping.find(acce_ite, [&, this](property_storage& ps) {
						ps.update(p);
						return update_imp(ps, p);
					});
				}
				return true;
			}
			bool stage_interface::check(const property_mapping& pm) const
			{
				auto& acce = acceptance();
				for (auto& ite : acce)
				{
					if (!pm.have(ite) && !default_mapping.have(ite))
						return false;
				}
				return false;
			}
			bool stage_interface::check(const property_mapping& pm, const property_mapping& pm2) const
			{
				auto& acce = acceptance();
				for (auto& ite : acce)
				{
					if (!pm.have(ite) && !pm2.have(ite) && !default_mapping.have(ite))
						return false;
				}
				return false;
			}
		}

		bool placement_interface::load_vs(std::u16string p, creator& c)
		{
			path = std::move(p);
			return get_shader_scene().lock([&, this](PO::scene& s) mutable {
				return s.load(path, true, [&, this](std::shared_ptr<PO::Dx::shader_binary> b) mutable {
					stage_vs << c.create_vertex_shader(std::move(b));
				});
			});
		}
		void placement_interface::apply(pipeline& p)
		{
			p << stage_vs;
		}

		void geometry_interface::apply(pipeline& p)
		{
			p << stage_rs << stage_ia;
		}
		bool geometry_interface::set_placement(std::shared_ptr<placement_interface> ptr)
		{
			if (ptr && ptr->id() == std::get<0>(requirement()))
			{
				placement_ptr = std::move(ptr);
				return true;
			}
			return false;
		}
		bool geometry_interface::update_placement(pipeline& p, property_mapping& pm)
		{
			if (placement_ptr)
			{
				return placement_ptr->update(p, pm);
			}
			return false;
		}
		bool geometry_interface::update_placement(pipeline& p, property_mapping& pm, property_mapping& pm2)
		{
			if (placement_ptr)
			{
				return placement_ptr->update(p, pm, pm2);
			}
			return false;
		}
		auto geometry_interface::acceptance_placement() const -> const acceptance_t&
		{
			if (placement_ptr)
			{
				return placement_ptr->acceptance();
			}
			else {
				static const acceptance_t acce{};
				return acce;
			}
		}
		void geometry_interface::apply_placement(pipeline& p)
		{
			if(placement_ptr)
				placement_ptr->apply(p);
		}

		bool geometry_interface::check(const property_mapping& pm) const
		{
			if (placement_ptr)
				return placement_ptr->check(pm) && stage_interface::check(pm);
			return stage_interface::check(pm);
			
		}
		bool geometry_interface::check(const property_mapping& pm, const property_mapping& pm2) const
		{
			if (placement_ptr)
				return placement_ptr->check(pm, pm2) && stage_interface::check(pm, pm2);
			return stage_interface::check(pm, pm2);
		}

		bool material_interface::load_ps(std::u16string p, creator& c)
		{
			path = std::move(p);
			return get_shader_scene().lock([&, this](PO::scene& s) mutable {
				return s.load(path, true, [&, this](const PO::Dx::shader_binary& b) mutable {
					stage_ps << c.create_pixel_shader(b);
				});
			});
		}
		void material_interface::apply(pipeline& p)
		{
			p << stage_ps << stage_bs;
		}

		bool compute_interface::load_cs(std::u16string p, creator& c)
		{
			path = std::move(p);
			return get_shader_scene().lock([&, this](PO::scene& s) mutable {
				return s.load(path, true, [&, this](const PO::Dx::shader_binary& b) mutable {
					stage_cs << c.create_compute_shader(b);
				});
			});
		}
		void compute_interface::apply(pipeline& p)
		{
			p << stage_cs;
		}


		namespace Implement
		{
			bool element_implement::construct_imp(property_interface& pi, creator& c)
			{
				auto& pi_ref = pi.acception();
				for (auto& ite : pi_ref)
				{
					for (auto& ite2 : compute_vector)
					{
						auto& ref = ite2->acceptance();
						if (ref.find(ite.first) != ref.end())
							return mapping.create_and_construct(ite, pi, c);
					}
					if (material_ptr)
					{
						auto& geo_ref = material_ptr->acceptance();
						if (geo_ref.find(ite.first) != geo_ref.end())
							return mapping.create_and_construct(ite, pi, c);
					}
					if (geometry_ptr)
					{
						auto& ref = geometry_ptr->acceptance();
						if (ref.find(ite.first) != ref.end())
							return mapping.create_and_construct(ite, pi, c);
						auto& reff = geometry_ptr->acceptance_placement();
						if (reff.find(ite.first) != reff.end())
							return mapping.create_and_construct(ite, pi, c);
					}
				}
				return false;
			}

			bool element_implement::reconstruct_imp(property_interface& pi, creator& c)
			{
				auto& pi_ref = pi.acception();
				for (auto& ite : pi_ref)
				{
					for (auto& ite2 : compute_vector)
					{
						auto& ref = ite2->acceptance();
						if (ref.find(ite.first) != ref.end())
							return mapping.recreate_and_construct(ite, pi, c);
					}
					if (material_ptr)
					{
						auto& geo_ref = material_ptr->acceptance();
						if (geo_ref.find(ite.first) != geo_ref.end())
							return mapping.recreate_and_construct(ite, pi, c);
					}
					if (geometry_ptr)
					{
						auto& ref = geometry_ptr->acceptance();
						if (ref.find(ite.first) != ref.end())
							return mapping.recreate_and_construct(ite, pi, c);
						auto& reff = geometry_ptr->acceptance_placement();
						if (reff.find(ite.first) != reff.end())
							return mapping.recreate_and_construct(ite, pi, c);
					}
					
				}
				return false;
			}

			void element_implement::clear_unuesd_property()
			{
				mapping.remove_if([this](property_storage& ps) {
					for (auto& compu : compute_vector)
					{
						auto& re = compu->acceptance();
						if (re.find(ps.id()) != re.end())
							return false;
					}
					if (material_ptr)
					{
						auto& re = material_ptr->acceptance();
						if (re.find(ps.id()) != re.end())
							return false;
					}
					if (geometry_ptr)
					{
						auto& re = geometry_ptr->acceptance();
						if (re.find(ps.id()) != re.end())
							return false;
						auto& ref = geometry_ptr->acceptance_placement();
						if (ref.find(ps.id()) != ref.end())
							return false;
					}
					return true;
				});
			}
			bool element_implement::check() const
			{
				for (auto& ite : compute_vector)
					if (!ite->check(mapping))
						return false;
				if (geometry_ptr)
					if (!geometry_ptr->check(mapping))
						return false;
				if (material_ptr)
					if (!material_ptr->check(mapping))
						return false;
				return true;
			}
			bool element_implement::check(const property_mapping& pm) const
			{
				for (auto& ite : compute_vector)
					if (!ite->check(mapping, pm))
						return false;
				if (geometry_ptr)
					if (!geometry_ptr->check(mapping, pm))
						return false;
				if (material_ptr)
					if (!material_ptr->check(mapping, pm))
						return false;
				return true;
			}

			void element_implement::clear_all()
			{
				clear_property();
				clear_compute();
				geometry_ptr.reset();
				material_ptr.reset();
			}

			element_implement& element_implement::operator=(std::shared_ptr<geometry_interface> p)
			{
				geometry_ptr = std::move(p);
				return *this;
			}
			element_implement& element_implement::operator=(std::shared_ptr<material_interface> p)
			{
				material_ptr = std::move(p);
				return *this;
			}
			element_implement& element_implement::operator=(std::shared_ptr<compute_interface> p)
			{
				if (p)
					compute_vector.push_back(std::move(p));
				return *this;
			}

			void element_implement::draw(pipeline& p)
			{
				for (auto& compute_ite : compute_vector)
				{
					if (compute_ite->update(p, mapping))
						compute_ite->dispath(p);
				}
				if (geometry_ptr && material_ptr)
				{
					if (material_ptr->update(p, mapping) && geometry_ptr->update_placement(p, mapping) && geometry_ptr->update(p, mapping))
					{
						geometry_ptr->apply(p);
						geometry_ptr->apply_placement(p);
						material_ptr->apply(p);
						geometry_ptr->draw(p);
					}
				}
			}

			void element_implement::draw(pipeline& p, property_mapping& map)
			{
				for (auto& compute_ite : compute_vector)
				{
					if (compute_ite->update(p, mapping, map))
						compute_ite->dispath(p);
				}
				if (geometry_ptr && material_ptr)
				{
					if (material_ptr->update(p, mapping, map) && geometry_ptr->update_placement(p, mapping, map) && geometry_ptr->update(p, mapping, map))
					{
						geometry_ptr->apply(p);
						geometry_ptr->apply_placement(p);
						material_ptr->apply(p);
						geometry_ptr->draw(p);
					}
				}
			}

		}

		void element::draw(pipeline& p)
		{
			if (element_ptr)
				element_ptr->draw(p);
		}
		void element::draw(pipeline& p, property_mapping& mapping)
		{
			if (element_ptr)
				element_ptr->draw(p, mapping);
		}
		

		/******************************************************************************************************/
	}
}

