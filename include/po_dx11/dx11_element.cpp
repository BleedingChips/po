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
				if (allready_update == false)
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

		std::shared_ptr<Implement::property_map> property_proxy_map::push(creator& c)
		{
			inside_map->proxy_mapping.clear();
			for (auto& ite : mapping)
			{
				ite.second->push(c);
				inside_map->proxy_mapping.insert(ite);
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

			bool stage_ptr::update(stage_context& sc, const std::type_index& ti, Tool::stack_list<property_map>* sl)
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
			bool stage_ptr::update(stage_context& sc, Tool::stack_list<property_map>* sl)
			{
				for (auto& ite : requirement())
					if (!update(sc, ite, sl))
						return false;
				return true;
			}

			void element_dispatch_request::dispatch(stage_context& sc, Tool::stack_list<property_map> * sl)
			{
				Tool::stack_list<property_map> tem{ *mapping, sl };
				if (compute->update(sc, &tem))
				{
					compute->apply(sc);
					sc.call();
				}
				
			}

			void element_draw_request::draw(stage_context& sc, Tool::stack_list<property_map> * sl)
			{
				Tool::stack_list<property_map> tem{ *mapping, sl };
				if (placemenet->update(sc, &tem) && geometry->update(sc, &tem) && material->update(sc, &tem))
				{
					placemenet->apply(sc);
					geometry->apply_implement(sc, placemenet->id());
					material->apply(sc);
					sc.call();
				}
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

		pipeline_interface::pipeline_interface(const std::type_index& ti) : type_info(ti) {}
		pipeline_interface::~pipeline_interface() {}
		void pipeline_interface::push(creator& c) {
			property_mapping.push(c);
		}

		void pipeline_interface::execute(stage_context& sc, element_renderer_storage& esb, Tool::stack_list<Implement::property_map>* pml)
		{
			property_mapping.inside_map->update(sc);
			Tool::stack_list<Implement::property_map> tem{ *(property_mapping.inside_map), pml };
			execute_implement(sc, esb, &tem);
		}

		/******************************************************************************************************/
	}
}

