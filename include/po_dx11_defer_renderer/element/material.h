#pragma once
#include "interface.h"
#include "../../po/tool/scene.h"
namespace PO
{
	namespace Dx11
	{
		namespace Material
		{
			class merga_gbuffer : public material_interface
			{
			public:
				struct gbuffer : public property_interface
				{
					shader_resource_view srv;
					sample_state ss;
					gbuffer();
				};
				merga_gbuffer();
				virtual void init(creator&, raw_scene&) override;
				virtual bool update(property_interface&, pipeline&, creator&);
				virtual const std::set<std::type_index>& acceptance() const;
			};

			class test_texcoord : public material_interface
			{
				
			public:

				struct texture : property_interface
				{
					shader_resource_view srv;
					sample_state ss;
					friend class material_testing_texcoord;
					texture();
				};
				test_texcoord();
				virtual void init(creator&, raw_scene&) override;
				virtual bool update(property_interface&, pipeline&, creator&);
				virtual const std::set<std::type_index>& acceptance() const;
			};
		}
	}
}
