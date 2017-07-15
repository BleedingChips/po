#include "material.h"
namespace PO
{
	namespace Dx11
	{
		namespace Material
		{
			merga_gbuffer::gbuffer::gbuffer() : property_interface(typeid(gbuffer)) {}

			merga_gbuffer::merga_gbuffer() : material_interface(typeid(merga_gbuffer), render_order::Post) {}

			void merga_gbuffer::init(creator& c)
			{
				if (!load_ps(u"build_in_material_merga_gbuffer_ps.cso", c))
					throw 1;
				auto des = blend_state::default_description;
				des.RenderTarget[0].BlendEnable = FALSE;
				bs = c.create_blend_state(des);
			}
			
			bool merga_gbuffer::update(property_interface& pi, pipeline& p)
			{
				if (pi.is<gbuffer>())
				{
					auto& ref = pi.cast<gbuffer>();
					ps << ref.ss[0] << ref.srv[0];

					aligned_storage<uint32_t4, float, aligned_array<float3, 300>> bp{ uint32_t4{100, 100, 100, 100}, 2.0f, aligned_array<float3, 300>{ { {1.0, 1.0, 1.0}}} };
					constant_buffer cb = p.get_creator().create_constant_buffer(&bp);
					ps << cb[1];


					return true;
				}
				return false;
			}

			const std::set<std::type_index>& merga_gbuffer::acceptance() const
			{
				static const std::set<std::type_index> accept = { typeid(gbuffer) };
				return accept;
			}

			test_texcoord::test_texcoord() : material_interface(typeid(test_texcoord), render_order::Defer) {}
			test_texcoord::texture::texture() : property_interface(typeid(texture)) {}

			void test_texcoord::init(creator& c)
			{
				if (!load_ps(u"build_in_material_text_texcoord_ps.cso", c))
					throw 1;
				auto des = blend_state::default_description;
				des.RenderTarget[0].BlendEnable = FALSE;
				bs = c.create_blend_state(des);
			}

			bool test_texcoord::update(property_interface& mpi, pipeline& p)
			{
				if (mpi.is<texture>())
				{
					auto& ref = mpi.cast<texture>();
					ps << ref.ss[0] << ref.srv[0];
					return true;
				}
				return false;
			}

			const std::set<std::type_index>& test_texcoord::acceptance() const
			{
				static const std::set<std::type_index> accept = { typeid(texture) };
				return accept;
			}

		}

	}
}