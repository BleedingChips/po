#include "texture_3d_viewer.h"
#include <random>
#undef max
using namespace PO::Dx;
using namespace PO::Dx11; 
using namespace PO;
texture_3d_viewer::texture_3d_viewer() : material_interface(typeid(texture_3d_viewer), renderer_order::Post) {}
const std::set<std::type_index>& texture_3d_viewer::acceptance() const
{
	static const std::set<std::type_index> acc = {typeid(input_property)};
	return acc;
}

void texture_3d_viewer::init(PO::Dx11::creator& c)
{
	if (!load_ps(u"texture_3d_viewer_ps.cso", c)) throw 1;
}
constant_buffer cb666;

bool texture_3d_viewer::update(PO::Dx11::property_interface& pi, PO::Dx11::pipeline& p)
{
	if (pi.is<input_property>())
	{
		auto& ref = pi.cast<input_property>();
		ps << ref.ss[0] << ref.srv[0] << ref.cb[0] << cb666[1];
		return true;
	}
	return false;
}



void texture_3d_viewer::input_property::update(PO::Dx11::pipeline& p)
{
	auto& c = p.get_creator();
	if (!ss)
		ss = c.create_sample_state();

	if (layer_change)
	{

		struct layout_t
		{
			alignas(16) float layer;
			alignas(16) float4 filter;
		}lt{layer, filter};

		if (!cb)
			cb = c.create_constant_buffer(&lt, true);
		else
			p.write_constant_buffer(cb, [&, this](void* data) {
				*static_cast<layout_t*>(data) = lt;
			});

		layer_change = false;
	}

	if (!cb666)
	{
		std::mt19937 mtx(123);
		std::mt19937 mty(456);
		std::mt19937 mtz(789);
		std::normal_distribution<double> nd(0.0f, 0.2f);

		aligned_storage<uint32_t3, aligned_array<float3, 100>, aligned_array<float3, 4>> bp233;

		for (size_t i = 0; i < 4; ++i)
		{
			(bp233.get<2>())[i] = float3{
				mtz() / static_cast<float>(std::mt19937::max()),
				mtx() / static_cast<float>(std::mt19937::max()),
				mty() / static_cast<float>(std::mt19937::max())
			};
			std::cout << bp233.get<2>()[i] << std::endl;
		}

		std::cout << "end" << std::endl;

		for (size_t i = 0; i < 100; ++i)
		{
			(bp233.get<1>())[i] = float3{
				mtz() / static_cast<float>(std::mt19937::max()),
				mtx() / static_cast<float>(std::mt19937::max()),
				mty() / static_cast<float>(std::mt19937::max())
			};
			std::cout << bp233.get<1>()[i] << std::endl;
		}

		cb666 = p.get_creator().create_constant_buffer(&bp233);
	}

	if (t_buffer)
	{
		srv = c.cast_shader_resource_view(t_buffer);
		t_buffer = PO::Dx11::tex3{};
	}
		
}

texture_3d_viewer::input_property::input_property() : property_interface(typeid(input_property)), layer_change(false), layer(0.0f), filter(float4{ 1.0f, 1.0f, 1.0f, 1.0f }) {}