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

		void property_interface::update(stage_context& p) {
			if (update_function)
			{
				update_function(p);
				update_function = std::function<void(stage_context& p)>{};
			}
		}

		void property_mapping::clear() { mapping.clear(); }
		bool property_mapping::have(std::type_index ti) const
		{
			auto ite = mapping.find(ti);
			return ite != mapping.end();
		}
		bool property_mapping::insert(std::shared_ptr<property_interface> sp)
		{
			if (sp)
			{
				auto id = sp->id();
				mapping.insert({ id, std::move(sp) });
				return true;
			}
			return false;
		}
		void property_mapping::update(stage_context& p)
		{
			for (auto& it : mapping)
				(it.second)->update(p);
		}

		namespace Implement
		{
			bool stage_interface::update(property_interface&, stage_context&) { return false; }
			auto stage_interface::acceptance() const -> const acceptance_t&
			{
				static const acceptance_t acce{};
				return acce;
			}

			bool stage_interface::update_acceptance_implement(stage_context& p, const std::type_index& ti, property_mapping& pm)
			{
				bool result;
				return pm.find(ti, [&](property_interface& pi) {
					result = update(pi, p);
				}) && result;
			}

			pipeline_interface::pipeline_interface(const std::type_index& ti) : type_info(ti) {}
			pipeline_interface::~pipeline_interface() {}
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

		material_interface::material_interface(const std::type_index& material_type, const std::type_index& pipeline_type) : Implement::stage_interface(material_type), Implement::pipeline_interface(pipeline_type) {}

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


		namespace Implement
		{

			void element_compute_implement::clear_unused_property()
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

			void element_implement::update_layout(creator& c)
			{
				if (placemenet_ptr && geometry_ptr)
					layout = c.create_layout(geometry_ptr->ia(), placemenet_ptr->vs());
			}


		}

		/******************************************************************************************************/
	}
}

