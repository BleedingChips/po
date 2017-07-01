#include "placement.h"
#include "property.h"

namespace PO
{
	namespace Dx11
	{
		namespace Placement
		{
			screen_square::screen_square() : placement_interface(typeid(screen_square)) {}

			void screen_square::init(creator& c, raw_scene& s)
			{
				auto po = s.find(typeid(PO::binary), u"shader_lib\\build_in_placement_screen_square_vs.cso", true);
				if (!po || !po->able_cast<PO::binary>()) throw 1;
				vs << c.create_vertex_shader(po->cast<PO::binary>());
			}

			static_3d::static_3d() : placement_interface(typeid(static_3d)) {}
			void static_3d::init(creator& c, raw_scene& s)
			{
				auto po = s.find(typeid(PO::binary), u"shader_lib\\build_in_placement_static_3d_vs.cso", true);
				if (!po || !po->able_cast<PO::binary>()) throw 1;
				vs << c.create_vertex_shader(po->cast<PO::binary>());
			}

			const std::set<std::type_index>& static_3d::acceptance() const
			{
				static const std::set<std::type_index> static_ = { typeid(Property::renderer_3d), typeid(Property::transfer_3d_static) };
				return static_;
			}

			bool static_3d::update(property_interface& pi, pipeline& p, creator& c)
			{
				using namespace PO::Dx11::Property;
				if (pi.is<renderer_3d>())
				{
					vs << pi.cast<renderer_3d>().cb[0];
					return true;
				}
				else if (pi.is<transfer_3d_static>())
				{
					vs << pi.cast<transfer_3d_static>().cb[1];
					return true;
				}
				return false;
			}
		}
	}
}