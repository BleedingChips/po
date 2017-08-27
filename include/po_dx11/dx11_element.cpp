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
		property_interface::~property_interface() {}
		namespace Implement
		{
			
			property_proxy_interface::property_proxy_interface(const std::type_index& original, const std::type_index& real, const std::type_index& asso)
				: base_interface<property_proxy_implement>(original, real), associate_info(asso){}
			property_proxy_interface::~property_proxy_interface() {}

			void property_map::update(stage_context& sc)
			{
				if (allready_update = false)
				{
					associate_mapping.clear();
					for (auto& ite : proxy_mapping)
					{
						ite.second->update(sc);
						associate_mapping.insert({ ite.second->associate_id(), ite.second });
					}
					allready_update = true;
				}
			}
		}

		std::shared_ptr<Implement::property_map> property_proxy_map::push(creator& c, const std::set<std::type_index>& require)
		{
			inside_map->proxy_mapping.clear();
			for (auto& ite : mapping)
			{
				auto id = ite.second->associate_id();
				if (require.find(id) != require.end())
				{
					ite.second->push(c);
					inside_map->proxy_mapping.insert(ite);
				}
			}
			inside_map->allready_update = false;
			return inside_map;
		}

		namespace Implement
		{
			compute_shader stage_load_cs(const std::u16string& path, creator& c)
			{
				compute_shader temporary;
				get_shader_scene().lock([&](PO::scene& s) mutable {
					s.load(path, true, [&](const PO::Dx::shader_binary& b) mutable {
						temporary = c.create_compute_shader(b);
					});
				});
				return temporary;
			}

			vertex_shader stage_load_vs(const std::u16string& path, creator& c)
			{
				vertex_shader temporary;
				get_shader_scene().lock([&](PO::scene& s) mutable {
					s.load(path, true, [&](std::shared_ptr<PO::Dx::shader_binary> b) mutable {
						temporary = c.create_vertex_shader(std::move(b));
					});
				});
				return temporary;
			}

			pixel_shader stage_load_ps(const std::u16string& path, creator& c)
			{
				pixel_shader temporary;
				get_shader_scene().lock([&](PO::scene& s) mutable {
					s.load(path, true, [&](const PO::Dx::shader_binary& b) mutable {
						temporary = c.create_pixel_shader(b);
					});
				});
				return temporary;
			}

			bool stage_interface::update(stage_context& sc, const std::type_index& ti, Tool::stack_list<property_map>* sl)
			{
				if (sl == nullptr)
					return false;
				else
				{
					bool result = false;
					return update(sc, ti, sl->front) || sl->type_ref.find_associate(ti, [&, this](property_interface& pi) {
						result = update_implement(sc, pi);
					}) && result;
				}
			}
			bool stage_interface::update(stage_context& sc, Tool::stack_list<property_map>* sl)
			{
				for (auto& ite : requirement())
					if (!update(sc, ite, sl))
						return false;
				return true;
			}

			void element_dispatch_request::dispatch(stage_context& sc, Tool::stack_list<property_map> * sl)
			{
				Tool::stack_list<property_map> tem{ *mapping, sl };
				compute->update(sc, &tem);
				compute->apply(sc);
				sc.call();
			}

			void element_draw_request::draw(stage_context& sc, Tool::stack_list<property_map> * sl)
			{
				Tool::stack_list<property_map> tem{ *mapping, sl };
				placemenet->update(sc, &tem);
				geometry->update(sc, &tem);
				material->update(sc, &tem);

				placemenet->apply(sc);
				geometry->apply_implement(sc, placemenet->id());
				material->apply(sc);
				sc.call();
			}
		}

		void element_logic_storage::push(element_swap_block& esb, creator& c)
		{
			if (esb.swap_lock.try_lock())
			{
				esb.dispatch_request.clear();
				for (auto& ite : esb.draw_request)
					ite.second.clear();
				
				for (auto & ite : element_store)
				{
					std::set<std::type_index> temporary;

					for (auto& ite2 : ite->compute)
					{
						auto& info = ite2->requirement();
						temporary.insert(info.begin(), info.end());
					}

					if (ite->placement)
					{
						auto& info = ite->placement->requirement();
						temporary.insert(info.begin(), info.end());
					}

					if (ite->geometry)
					{
						auto& info = ite->geometry->requirement();
						temporary.insert(info.begin(), info.end());
					}

					for (auto& ite2 : ite->material)
					{
						auto& info = ite2.second->requirement();
						temporary.insert(info.begin(), info.end());
					}

					auto map = ite->mapping.push(c, temporary);

					for (auto& ite2 : ite->compute)
					{
						esb.dispatch_request.emplace_back(Implement::element_dispatch_request{ ite2, map });
					}

					if (ite->placement && ite->geometry)
					{
						ite->geometry->update_layout(*(ite->placement), c);
						for (auto& ite2 : ite->material)
						{
							(esb.draw_request)[ite2.second->pipeline()].emplace_back(Implement::element_draw_request{ ite->placement, ite->geometry, ite2.second, map });
						}
					}
				}
				esb.swap_lock.unlock();
				element_store.clear();
			}
		}

		void element_renderer_storage::get(element_swap_block& esb, stage_context& sc)
		{
			{
				std::lock_guard<decltype(esb.swap_lock)> lg(esb.swap_lock);
				std::swap(dispatch_request, esb.dispatch_request);
				std::swap(draw_request, esb.draw_request);
			}
			
			for (auto& ite : dispatch_request)
				ite.update(sc);
			for (auto& ite : draw_request)
				for (auto& ite2 : ite.second)
					ite2.update(sc);
		}

		/*

		void property_interface::update_implement(stage_context& p) {
			lock_t l(this);
			if (is_update_require)
			{
				update(p);
				is_update_require = false;
			}
		}

		namespace Implement
		{
			void property_map_implement::clear()
			{
				mapping.clear();
			}

			bool property_map_implement::have(std::type_index ti) const
			{
				auto ite = mapping.find(ti);
				return ite != mapping.end();
			}

			bool property_map_implement::insert(std::shared_ptr<property_interface> sp)
			{
				if (sp)
				{
					auto id = sp->id();
					mapping.insert({ id, std::move(sp) });
					return true;
				}
				return false;
			}
		}

		bool acceptance_t::check_acceptance(Tool::stack_list<const property_map>* sl) const
		{
			for (auto& ele : acceptance)
			{
				bool finded = false;
				while (!finded && sl != nullptr)
				{
					finded = sl->type_ref.lock([&](const Implement::property_map_implement& t) {return t.have(ele.first); });
					sl = sl->front;
				}
				if (!finded)
					return false;
			}
			return true;
		}

		std::set<std::type_index> acceptance_t::lack_acceptance(Tool::stack_list<const property_map>* sl) const
		{
			std::set<std::type_index> temporary;
			for (auto& ele : acceptance)
			{
				bool finded = false;
				while (!finded && sl != nullptr)
				{
					finded = sl->type_ref.lock([&](const Implement::property_map_implement& t) {return t.have(ele.first); });
					sl = sl->front;
				}
				if (!finded)
					temporary.insert(ele.first);
			}
			return temporary;
		}

		bool acceptance_t::update_implement(stage_context& sc, typename map_t::value_type& vt, Tool::stack_list<property_map>* sl)
		{
			if (sl == nullptr)
				return false;
			else {
				return update_implement(sc, vt, sl->front) || sl->type_ref.lock([&](auto& t) {
					bool result = false;
					return t.find(vt.first, [&](property_interface& pi) {
						result = vt.second(pi, sc);
					}) && result;
				});
			}
		}

		bool acceptance_t::update(stage_context& sc, Tool::stack_list<property_map>* sl)
		{
			for (auto& ite : acceptance)
				if (!update_implement(sc, ite, sl))
					return false;
			return true;
		}

		namespace Implement
		{

			pipeline_interface::pipeline_interface(const std::type_index& ti) : type_info(ti) {}
			pipeline_interface::~pipeline_interface() {}
			void pipeline_interface::execute(stage_context&, Tool::stack_list<property_map>* pml) {}
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
		void placement_interface::apply(stage_context& p)
		{
			p << stage_vs;
		}

		void geometry_interface::apply(stage_context& p)
		{
			p << stage_rs << stage_ia;
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

		void material_interface::apply(stage_context& p)
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
		void compute_interface::apply(stage_context& p)
		{
			p << stage_cs;
		}

		/*
		namespace Implement
		{

			void element_entity::update_layout(creator& c)
			{
				if (placemenet_ptr && geometry_ptr)
					layout = c.create_layout(geometry_ptr->ia(), placemenet_ptr->vs());
			}

			void element_compute::clear_unused_property()
			{
				if (compute_ptr)
				{
					const auto& re = compute_ptr->acceptance();
					mapping.remove_if([&](property_interface& pi) {
						return re.find(pi.id()) == re.end();
					});
				}
				else
					mapping.clear();
			}

			bool element_compute::dispatch(stage_context& p, property_mapping_list* pml = nullptr)
			{
				if (compute_ptr)
				{
					property_mapping_list temporary(mapping, pml);
					if (compute_ptr->update_implement(p, &temporary))
						return (compute_ptr->apply(p), compute_ptr->dispath(p), true);
				}
				return false;
			}

			bool element_implement::draw(stage_context& p, property_mapping_list* pml)
			{
				if (placemenet_ptr && geometry_ptr && material_ptr)
				{
					property_mapping_list temporary{ mapping, pml };
					if (
						placemenet_ptr->update_implement(p, &temporary)
						&& geometry_ptr->update_implement(p, &temporary)
						&& material_ptr->update_implement(p, &temporary)
						)
					{
						placemenet_ptr->apply(p);
						geometry_ptr->apply(p);
						material_ptr->apply(p);
						geometry_ptr->draw(p);
						return true;
					}
				}
				return false;
			}

			void element_implement::clear_unused_property()
			{
				mapping.remove_if([this](property_interface& ps) {
					if (material_ptr)
					{
						auto& re = material_ptr->acceptance();
						if (re.find(ps.id()) != re.end())
							return false;
					}
					if (placemenet_ptr)
					{
						auto& ref = placemenet_ptr->acceptance();
						if (ref.find(ps.id()) != ref.end())
							return false;
					}
					if (geometry_ptr)
					{
						auto& re = geometry_ptr->acceptance();
						if (re.find(ps.id()) != re.end())
							return false;
					}
					return true;
				});
			}

			bool element_implement::check_acceptance() const 
			{ 
				return (placemenet_ptr ? placemenet_ptr->check_acceptance(mapping) : true)
					&& (geometry_ptr ? geometry_ptr->check_acceptance(mapping) : true)
					&& (material_ptr ? material_ptr->check_acceptance(mapping) : true);
			}

			stage_interface::acceptance_t element_implement::lack_acceptance() const
			{
				stage_interface::acceptance_t tem = placemenet_ptr ? placemenet_ptr->lack_acceptance(mapping) : stage_interface::acceptance_t{};
				if (geometry_ptr)
				{
					auto tem2 = geometry_ptr->lack_acceptance(mapping);
					tem.insert(tem2.begin(), tem2.end());
				}
				if (material_ptr)
				{
					auto tem2 = material_ptr->lack_acceptance(mapping);
					tem.insert(tem2.begin(), tem2.end());
				}
			}
			*/

		/******************************************************************************************************/
	}
}

