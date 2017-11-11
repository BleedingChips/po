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
			property_interface::~property_interface() {}
			
			property_proxy_interface::property_proxy_interface(const std::type_index& original, const std::type_index& real, const std::type_index& asso)
				: base_interface<property_proxy_implement>(original, real), associate_info(asso){}
			property_proxy_interface::~property_proxy_interface() {}

			void property_map::swap_to_renderer()
			{
				if (allready_update == false)
				{
					renderer_mapping.clear();
					for (auto& ite : swap_mapping)
					{
						ite.second->swap_to_renderer();
						renderer_mapping.insert(ite);
					}
					allready_update = true;
				}
			}
		}

		void property_proxy_map::logic_to_swap(creator& c)
		{
			inside_map->swap_mapping.clear();
			for (auto& ite : mapping)
			{
				ite.second->logic_to_swap(c);
				inside_map->swap_mapping.insert({ite.second->associate_id(), ite.second});
			}
			inside_map->allready_update = false;
		}

		bool property_proxy_map::shared_property_to(const std::type_index& id, property_proxy_map& ppm) const
		{
			auto ite = mapping.find(id);
			if (ite != mapping.end())
			{
				auto ptr = ite->second;
				ppm.mapping.insert_or_assign(id, ptr);
				return true;
			}
			return false;
		}

		void property_proxy_map::logic_to_renderer(creator& c)
		{
			inside_map->renderer_mapping.clear();
			for (auto& ite : mapping)
			{
				ite.second->logic_to_renderer(c);
				inside_map->renderer_mapping.insert({ ite.second->associate_id(), ite.second });
			}
			inside_map->allready_update = true;
		}

		namespace Implement
		{
			bool stage_resource::apply_property(stage_context& sc, property_interface& pi) { return true; }
			stage_resource::~stage_resource() {}
			const element_requirement& stage_resource::requirement() const
			{
				static element_requirement re{};
				return re;
			}
		}

		geometry_resource::geometry_resource(creator& c, layout_view v, std::optional<raterizer_state::description> o, primitive_topology t) : view(v), primitive(t)
		{
			if (o)
				state.create(c, *o);
		}

		void geometry_resource::apply(stage_context& sc)
		{
			sc << primitive << state;
		}

		void compute_resource::apply(stage_context& sc) { sc << shader; }
		compute_resource::compute_resource(creator& c, std::u16string cs_pacth): patch(std::move(cs_pacth))
		{
			if (!patch.empty())
			{
				shader = Implement::stage_load_cs(patch, c);
				assert(shader);
			}
		}

		void material_resource::apply(stage_context& sc)
		{
			sc << bs << shader;
		}

		material_resource::material_resource(creator& c, std::u16string ps_patch, std::optional<blend_state::description> des) : patch(std::move(ps_patch))
		{
			if (!patch.empty())
			{
				shader = Implement::stage_load_ps(patch, c);
				assert(shader);
			}
			if (des)
				bs.create(c, *des);
		}

		void placement_resource::apply(stage_context& sc)
		{
			sc << shader;
		}

		placement_resource::placement_resource(creator& c, std::u16string ps_patch) : patch(std::move(ps_patch))
		{
			if (!patch.empty())
			{
				shader = Implement::stage_load_vs(patch, c);
				assert(shader);
			}
		}

		bool element_requirement::apply_property_implement(Implement::stage_resource& sr, stage_context& sc, const map_t::value_type& map_ite, Tool::stack_list<Implement::property_map>* sl)
		{
			if (sl != nullptr)
			{
				bool result = false;
				return apply_property_implement(sr, sc, map_ite, sl->front) || (sl->type_ref.find_associate(map_ite.first, [&](Implement::property_interface& pi) {
					auto& ref = (map_ite.second);
					result = (*std::get<0>(ref))(sc, pi, std::get<1>(ref)) && sr.apply_property(sc, pi);
				}) && result);
			}else
				return false;
		}

		bool element_requirement::apply_property(Implement::stage_resource& sr, stage_context& sc, Tool::stack_list<Implement::property_map>* sl) const
		{
			for (auto& ite : mapping)
				if (!apply_property_implement(sr, sc, ite, sl))
				{
					return false;
				}
			return true;
		}

		namespace Implement
		{
			shader_compute stage_load_cs(const std::u16string& path, creator& c)
			{
				shader_compute temporary;
				get_shader_scene().lock([&](PO::scene& s) mutable {
					s.load(path, true, [&](const PO::Dx::shader_binary& b) mutable {
						temporary.create(c, b);
					});
				});
				return temporary;
			}

			shader_vertex stage_load_vs(const std::u16string& path, creator& c)
			{
				shader_vertex temporary;
				get_shader_scene().lock([&](PO::scene& s) mutable {
					s.load(path, true, [&](std::shared_ptr<PO::Dx::shader_binary> b) mutable {
						temporary.create(c, b);
					});
				});
				return temporary;
			}

			shader_pixel stage_load_ps(const std::u16string& path, creator& c)
			{
				shader_pixel temporary;
				get_shader_scene().lock([&](PO::scene& s) mutable {
					s.load(path, true, [&](const PO::Dx::shader_binary& b) mutable {
						temporary.create(c, b);
					});
				});
				return temporary;
			}

			void element_dispatch_request::dispatch(stage_context& sc, Tool::stack_list<property_map> * sl)
			{
				Tool::stack_list<property_map> tem{ *mapping, sl };
				if (compute->apply_property(sc, &tem))
				{
					compute->apply_stage(sc);
					sc.call();
				}
			}

			void element_draw_request::draw(stage_context& sc, const depth_stencil_state& ss, Tool::stack_list<property_map> * sl)
			{
				Tool::stack_list<property_map> tem{ *mapping, sl };
				if (placemenet->apply_property(sc, &tem) && geometry->apply_property(sc, &tem) && material->apply_property(sc, &tem))
				{
					placemenet->apply_stage(sc);
					geometry->apply_stage(sc);
					geometry->apply_layout(sc, *placemenet);
					material->apply_stage(sc);
					material->apply_depth_stencil_state(sc, ss);
					sc.call();
				}
			}
		}

		void element_compute_logic_storage::logic_to_swap(element_compute_swap_block& esb, creator& c)
		{
			if (esb.swap_lock.try_lock())
			{
				esb.dispatch_request.clear();
				for (auto& ite : element_compute_store)
				{
					if (ite->compute)
					{
						ite->mapping.logic_to_swap(c);
						esb.dispatch_request.emplace_back(Implement::element_dispatch_request{ ite->compute, ite->mapping.map() });
					}
				}
				esb.swap_lock.unlock();
				element_compute_store.clear();
			}
		}

		void element_draw_logic_storage::logic_to_swap(element_draw_swap_block& esb, creator& c)
		{
			if (esb.swap_lock.try_lock())
			{
				esb.draw_request.clear();

				for (auto& ite : element_draw_store)
				{
					if (ite->geometry && ite->material && ite->placement)
					{
						ite->mapping.logic_to_swap(c);
						esb.draw_request.emplace_back(Implement::element_draw_request{ ite->placement, ite->geometry, ite->material, ite->mapping.map() });
					}
				}
				esb.swap_lock.unlock();
				element_draw_store.clear();
			}
		}

		void element_compute_renderer_storage::swap_to_renderer(element_compute_swap_block& esb, stage_context& sc)
		{
			{
				std::lock_guard<decltype(esb.swap_lock)> lg(esb.swap_lock);
				std::swap(dispatch_request, esb.dispatch_request);
			}

			for (auto& ite : dispatch_request)
				ite.swap_to_renderer();
		}



		void element_draw_renderer_storage::swap_to_renderer(element_draw_swap_block& esb, stage_context& sc)
		{
			{
				std::lock_guard<decltype(esb.swap_lock)> lg(esb.swap_lock);
				std::swap(draw_request, esb.draw_request);
			}
			
			for (auto& ite : draw_request)
				ite.swap_to_renderer();
		}

		/******************************************************************************************************/
	}
}

