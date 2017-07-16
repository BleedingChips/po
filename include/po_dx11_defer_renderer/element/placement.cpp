#include "placement.h"
/*
#include "property.h"

namespace PO
{
	namespace Dx11
	{
		namespace Placement
		{
			screen_square::screen_square() : placement_interface(typeid(screen_square)) {}

			void screen_square::init(creator& c)
			{
				if (!load_vs(u"build_in_placement_screen_square_vs.cso", c))
					throw 1;
			}

			static_3d::static_3d() : placement_interface(typeid(static_3d)) {}
			void static_3d::init(creator& c)
			{
				if (!load_vs(u"build_in_placement_static_3d_vs.cso", c))
					throw 1;
			}

			const std::set<std::type_index>& static_3d::acceptance() const
			{
				static const std::set<std::type_index> static_ = { typeid(Property::renderer_3d), typeid(Property::transfer_3d_static) };
				return static_;
			}

			bool static_3d::update(property_interface& pi, pipeline& p)
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

			static_2d::static_2d() : placement_interface(typeid(static_2d)) {}
			void static_2d::init(creator& c)
			{
				if (!load_vs(u"build_in_placement_static_2d_vs.cso", c))
					throw 1;
			}

			bool static_2d::update(property_interface& pi, pipeline& p)
			{
				return pi.cast([this](Property::transfer_2d_static& ip) {
					vs << ip.get_buffer()[0];
				})|| pi.cast([this](Property::renderer_3d& r3) {
					vs << r3.cb[1];
				});
			}

			const std::set<std::type_index>& static_2d::acceptance() const
			{
				static const std::set<std::type_index> acc = { typeid(Property::transfer_2d_static), typeid(Property::renderer_3d) };
				return acc;
			}


		}
	}
}
*/