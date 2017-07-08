#pragma once
#include "interface.h"
#include "../../po/tool/scene.h"
namespace PO 
{
	namespace Dx11
	{
		namespace Placement
		{
			struct screen_square : placement_interface
			{
				screen_square();
				virtual void init(creator&) override;
			};

			struct static_3d : placement_interface
			{
				static_3d();
				virtual void init(creator&) override;
				virtual bool update(property_interface&, pipeline&) override;
				virtual const std::set<std::type_index>& acceptance() const override;
			};

			struct static_2d : placement_interface
			{
				static_2d();
				virtual void init(creator&) override;
				virtual bool update(property_interface&, pipeline&) override;
				virtual const std::set<std::type_index>& acceptance() const override;
			};
		}
	}
}