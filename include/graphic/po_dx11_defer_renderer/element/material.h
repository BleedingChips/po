#pragma once
/*
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
				virtual void init(creator&) override;
				virtual bool update(property_interface&, pipeline&) override;
				virtual const std::set<std::type_index>& acceptance() const override;
			};

			class test_texcoord : public material_interface
			{
				
			public:

				struct texture : property_interface
				{
					tex2 text;
					sample_state::description des = sample_state::default_description;

					shader_resource_view srv;
					sample_state ss;
					friend class material_testing_texcoord;
					texture();
					void set_texture(tex2 t) { text = std::move(t); need_push(); }
					void push(creator& c) {
						srv = c.cast_shader_resource_view(text);
						ss = c.create_sample_state(des);
					}
				};
				test_texcoord();
				
				virtual void init(creator&) override;
				virtual bool update(property_interface&, pipeline&) override;
				virtual const std::set<std::type_index>& acceptance() const override;
			};
		}
	}
}
*/
