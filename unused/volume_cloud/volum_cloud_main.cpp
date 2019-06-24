#include <iostream>
#include <limits>
#include <random>
#include "po/po.h"
#include "po_dx11/dx11_form.h"
#include <fstream>
#include <algorithm>
#include "DirectXTex.h"
#include <sstream>
#include "po_dx11/dx11_renderer.h"
#include "po_dx\controller.h"
#include "../../po/include/po/po.h"
#include "compute\volume_cloud_compute.h"
#include "material\volume_cloud_material.h"
#include "geometry\ue4_geometry.h"
#include "imagefileio.h"
#include "po\tool\utf_support.h"
using namespace PO;
PO::Dx11::device_ptr dev;
/*
struct DrawEnity : public entity_res
{
	PO::Dx::transfer3D t1;
	PO::Dx::showcase sc;
	float4 input_value = float4{0.0f, 0.1f, 0.1f, 0.1f};
	float4 input_mulity = float4{ 0.1f, 0.0f, 0.1f, 0.1f };
	float4 input_shift = float4{ 0.01f, 0.001f, 0.01f, 0.01f };
	buffer_structured random_point;
	
	void init(entity_self& es, context& c)
	{
		point_count = 1000;
		
		sc.binding({
			{ KeyValue::K_D, showcase::State::Y_CW },
			{ KeyValue::K_A, showcase::State::Y_ACW },
			{ KeyValue::K_S, showcase::State::X_CW },
			{ KeyValue::K_W, showcase::State::X_ACW },
			{ KeyValue::K_E, showcase::State::Z_CW },
			{ KeyValue::K_Q, showcase::State::Z_ACW },
			{ KeyValue::K_R, showcase::State::T_FR },
			{ KeyValue::K_F, showcase::State::T_BA }
			});
		sc.lose_focus();
		sc.set_translation_speed(4.0);

		uint4 fbm_simulate_size = { 32, 32, 4, 2 };
		uint4 density_simulate_size = { 64, 64, 4, 4 };

		t1.poi = float3(0.0, 0.0, 5.0);

		float Max = density_simulate_size.x > density_simulate_size.y ?  density_simulate_size.x : density_simulate_size.y;
		Max = Max > density_simulate_size.z * density_simulate_size.w * 4.0 ? Max : density_simulate_size.z * density_simulate_size.w * 4.0;


		t1.sca = float3(0.02f * (density_simulate_size.x / Max), 0.02f * (density_simulate_size.y / Max), 0.02f * (density_simulate_size.z * density_simulate_size.w * 4.0 / Max));

		{
			std::vector<float3> Point{ point_count, float3{ 0.5, 0.5, 0.5 } };
			std::mt19937 mx(124323423);
			std::mt19937 my(3232433);
			std::mt19937 mz(323223124433);
			std::uniform_real_distribution<float> nd(0.0f, 1.0f);
			//std::normal_distribution<float> nd2(0.2f, 0.3f);
			std::uniform_real_distribution<float> nd2(0.3f, 0.7f);
			for (auto& ite : Point)
				ite = float3{ nd(mx), nd(my), nd2(mz) };
			random_point.create_unordered_access(dev, Point);
		}



		{
			auto ite2 = c.create_componenet<PO::event_receiver>();
			es.insert(ite2);
			ite2.instance([this](PO::event_receiver& er) {
				er.filter = [this](const event& e) {
					if (sc.respond(e) == PO::Respond::Pass)
					{
						if (e.is_key() && e.key.is_down())
						{
							switch (e.key.value)
							{

							case PO::KeyValue::K_U:
								input_value.x += input_mulity.x * input_value.x + input_shift.x;
								std::cout << input_value << std::endl;
								break;
							case PO::KeyValue::K_I:
								input_value.y += input_mulity.y * input_value.y + input_shift.y;
								std::cout << input_value << std::endl;
								break;
							case PO::KeyValue::K_O:
								input_value.z += input_mulity.z * input_value.z + input_shift.z;
								std::cout << input_value << std::endl;
								break;
							case PO::KeyValue::K_P:
								input_value.w += input_mulity.w * input_value.w + input_shift.w;
								std::cout << input_value << std::endl;
								break;
							case PO::KeyValue::K_J:
								input_value.x -= input_mulity.x * input_value.x + input_shift.x;
								std::cout << input_value << std::endl;
								break;
							case PO::KeyValue::K_K:
								input_value.y -= input_mulity.y * input_value.y + input_shift.y;
								std::cout << input_value << std::endl;
								break;
							case PO::KeyValue::K_L:
								input_value.z -= input_mulity.z * input_value.z + input_shift.z;
								std::cout << input_value << std::endl;
								break;
							case PO::KeyValue::K_SEMICOLONS:
								input_value.w -= input_mulity.w * input_value.w + input_shift.w;
								std::cout << input_value << std::endl;
								break;

							default:
								break;
							}
						}
						else if (e.is_quit())
							destory();
					}
					return true; };
			});
		}

		tex2 fbmSimulate;
		{
			tex3 fbm;
			fbm.create_unordered_access(dev, DXGI_FORMAT_R16_FLOAT, { fbm_simulate_size.x, fbm_simulate_size.y, fbm_simulate_size.z * fbm_simulate_size.w * 4 });
			auto ite = c.create_componenet<PO::Dx11::defer_renderer_compute_component>();
			es.insert(ite);
			ite.instance([&](PO::Dx11::defer_renderer_compute_component& cc) {
				cc.ele
					<< PO::Dx11::stage_instance_instance().create_compute<fbmWorleyNoiseGenerator>()
					<< [&](fbmWorleyNoiseGenerator::property& p)
				{
					p.Block = { 10.0, 10.0, 10.0 };
					p.fbmOutput = fbm.cast_unordered_access_view(dev);
					p.Frequence = 1.8;
					p.Gain = 0.5;
					p.Lacunarity = 2.7;
					p.Mulity = 1.0;
					p.Octaves = 4;
					p.Shift = float3{ 0.34f,0.45f, 0.78f };
					p.texture_size = fbm.size();
				};
				cc.ele.set_task([=](PO::Dx11::stage_context& sc) {
					assert(SaveToDDS(sc.dev, sc.ptr, fbm, u"fbmoutput.DDS"));
				});
			});
			
			fbmSimulate.create_unordered_access(dev, DXGI_FORMAT_R8G8B8A8_TYPELESS, Simulate3DFloatWith2DUnorm4::calculate_texture_size(fbm_simulate_size));
			auto ite2 = c.create_componenet<PO::Dx11::defer_renderer_compute_component>();
			es.insert(ite2);
			ite2.instance([&](PO::Dx11::defer_renderer_compute_component& cc) {
				cc.ele
					<< PO::Dx11::stage_instance_instance().create_compute<Simulate3DFloatWith2DUnorm4>()
					<< [&](Simulate3DFloatWith2DUnorm4::property& p)
				{
					p.Factor = float4{ 1.0, 0.0, 0.0, 0.0 };
					p.input = fbm.cast_shader_resource_view(dev);
					p.output = fbmSimulate.cast_unordered_access_view_as_format(dev, DXGI_FORMAT_R8G8B8A8_UINT);
					p.simulate_size = fbm_simulate_size;
					p.output_size = fbmSimulate.size();
					p.type = decltype(p.type)::Mirro;
				};
				cc.ele.set_task([=](PO::Dx11::stage_context& sc) {
					std::stringstream ss;
					ss << "T_Volume_fbmWorleyNoise_2D4Simulate3D1_"  <<fbm_simulate_size.x << "X" << fbm_simulate_size.y << "X" << fbm_simulate_size.z << "X" << fbm_simulate_size.w << "_V.tga";
					std::string io;
					ss >> io;
					std::cout << io << std::endl;
					auto  po = PO::asc_to_utf16(io);
					assert(SaveToTGA(sc.dev, sc.ptr, fbmSimulate, po, DXGI_FORMAT_R8G8B8A8_UNORM));
				});
			});
		}



		tex2 BaseShapeSimualte;
		{
			tex3 BaseShape;
			tex2 Area;
			assert(LoadFormTGA(dev, Area, u"CloudArea.tga"));
			BaseShape.create_unordered_access(dev, DXGI_FORMAT_R16_FLOAT, { density_simulate_size.x, density_simulate_size.y, density_simulate_size.z * density_simulate_size.w * 4 });
			auto ite3 = c.create_componenet<PO::Dx11::defer_renderer_compute_component>();
			es.insert(ite3);
			ite3.instance([&](PO::Dx11::defer_renderer_compute_component& cc) {
				cc.ele
					<< PO::Dx11::stage_instance_instance().create_compute<GeneratorMetaBall3D>()
					<< [&](GeneratorMetaBall3D::property& p)
				{
					p.shape_output = BaseShape.cast_unordered_access_view(dev);
					p.original_point = random_point.cast_shader_resource_view(dev);
					p.texture_size = BaseShape.size();
					p.area = Area.cast_shader_resource_view(dev);
				}
				;
				cc.ele.set_task([=](stage_context& sc) {
				assert(SaveToDDS(sc.dev, sc.ptr, BaseShape, u"BaseShapeOutoput.DDS"));
				});
			});
			BaseShapeSimualte.create_unordered_access(dev, DXGI_FORMAT_R8G8B8A8_TYPELESS, Simulate3DFloatWith2DUnorm4::calculate_texture_size(density_simulate_size));
			auto ite2 = c.create_componenet<PO::Dx11::defer_renderer_compute_component>();
			es.insert(ite2);
			ite2.instance([&](PO::Dx11::defer_renderer_compute_component& cc) {
				cc.ele
					<< PO::Dx11::stage_instance_instance().create_compute<Simulate3DFloatWith2DUnorm4>()
					<< [&](Simulate3DFloatWith2DUnorm4::property& p)
				{
					p.Factor = float4{ 1.0, 0.0, 0.0, 0.0 };
					p.input = BaseShape.cast_shader_resource_view(dev);
					p.output = BaseShapeSimualte.cast_unordered_access_view_as_format(dev, DXGI_FORMAT_R8G8B8A8_UINT);
					p.simulate_size = density_simulate_size;
					p.output_size = BaseShapeSimualte.size();
					p.type = decltype(p.type)::Clamp;
				};
				cc.ele.set_task([=](PO::Dx11::stage_context& sc) {
					std::stringstream ss;
					ss << "T_Volume_DF_BaseShape_2D4Simulate3D1_" << density_simulate_size.x << "X" << density_simulate_size.y << "X" << density_simulate_size.z << "X" << density_simulate_size.w << "_V.tga";
					std::string io;
					ss >> io;
					std::cout << io << std::endl;
					assert(SaveToTGA(sc.dev, sc.ptr, BaseShapeSimualte, PO::asc_to_utf16(io), DXGI_FORMAT_R8G8B8A8_UNORM));
				});
			});
		}


		PO::Dx11::element_draw ed;
		{
			auto ite = c.create_componenet<PO::Dx11::defer_renderer_static_mesh_component>();
			es.insert(ite);
			
			ite.instance([&, this](PO::Dx11::defer_renderer_static_mesh_component& drsmc) {
				drsmc.ele
					<< PO::Dx11::stage_instance_instance().create_material<material_testing>()
					<< PO::Dx11::stage_instance_instance().create_placement<placement_static_viewport_static>()
					<< PO::Dx11::stage_instance_instance().create_geometry<UE4_cube_static_Frame>()
					<< [&](property_local_transfer& plt) {
					plt.LocalToWorld = t1;
					plt.WorldToLocal = t1.inverse_float4x4();
				};
				ed = drsmc.ele;
				drsmc.is_opaque = false;
			});
		}
		
		{
			tex2 Demo;
			assert(LoadFormTGA(dev, Demo, u"CloudArea.tga"));
			auto ite = c.create_componenet<PO::Dx11::defer_renderer_static_mesh_component>();
			es.insert(ite);
			ite.instance([&, this](PO::Dx11::defer_renderer_static_mesh_component& drsmc) {
				drsmc.ele
					<< PO::Dx11::stage_instance_instance().create_material<AdvantureRenderer>()
					//<< PO::Dx11::stage_instance_instance().create_material<Tex3dSimulateDebuger>()
					<< PO::Dx11::stage_instance_instance().create_placement<placement_static_viewport_static>()
					<< PO::Dx11::stage_instance_instance().create_geometry<UE4_cube_static>()
					<< [&, this](AdvantureRenderer::property& pt) {
					pt.DensityMap = BaseShapeSimualte.cast_shader_resource_view_as_format(dev, DXGI_FORMAT_R8G8B8A8_UNORM);
					pt.DetailMap = fbmSimulate.cast_shader_resource_view_as_format(dev, DXGI_FORMAT_R8G8B8A8_UNORM);
					pt.ss_des2.AddressU = decltype(pt.ss_des2.AddressU)::D3D11_TEXTURE_ADDRESS_MIRROR;
					pt.ss_des2.AddressV = decltype(pt.ss_des2.AddressU)::D3D11_TEXTURE_ADDRESS_MIRROR;
					pt.ss_des2.AddressW = decltype(pt.ss_des2.AddressU)::D3D11_TEXTURE_ADDRESS_MIRROR;
					pt.DensitySimulateSize = density_simulate_size;
					pt.DetailSimulateSize = fbm_simulate_size;
				}
					<< [&, this](property_volumecloud_debug_value& pvdv)
				{
					pvdv.InputValue = input_value;
					pvdv.Density = input_value.x;
					pvdv.XYZSizeOfCube = UE4_cube_static::size();
				}
				;
				ed.ptr->mapping.shared_property_to<property_local_transfer>(drsmc.ele.ptr->mapping);
				drsmc.is_opaque = false;
			});
		}
		
	}
	void tick(entity_self& es, context& c, duration d)
	{
		es.find_component([&](PO::Dx11::defer_renderer_static_mesh_component& drrsmc) {
			if (sc.apply(d, t1))
				drrsmc.ele << [&](property_local_transfer& plt) {
					plt.LocalToWorld = t1;
					plt.WorldToLocal = t1.inverse_float4x4();
				};
			drrsmc.ele << [&](property_volumecloud_debug_value& plt) {
				plt.InputValue = input_value;
				plt.Density = input_value.x;
			} << [&](Tex3dSimulateDebuger::property& p)
			{
				p.InputValue = input_value;
			}
			;
			return true;
		});
	}
	uint point_count;
	DrawEnity() : point_count(2000)
	{
		
		
	}
};
*/


struct window_component : PO::component_res
{
	PO::Dx11::form form;
};

#include <sstream>
int main(int count, const char** parameter)
{
	

#ifdef DEBUG
	PO::Dx11::add_shader_path(u"..\\..\\..\\..\\project\\vs2017\\po_dx11\\lib\\shader\\Debug\\");
	PO::Dx11::add_shader_path(u"..\\..\\..\\..\\..\\project\\vs2017\\po_dx11\\lib\\shader\\Debug\\");
	PO::Dx11::add_shader_path(u"..\\..\\..\\project\\vs2017\\po_dx11\\lib\\shader\\Debug\\");
	PO::Dx11::add_shader_path(u".\\");
#else
	PO::Dx11::add_shader_path(u"..\\..\\..\\..\\project\\vs2017\\po_dx11\\lib\\shader\\Release\\");
	PO::Dx11::add_shader_path(u"..\\..\\..\\..\\..\\project\\vs2017\\po_dx11\\lib\\shader\\Release\\");
	PO::Dx11::add_shader_path(u"..\\..\\..\\project\\vs2017\\po_dx11\\lib\\shader\\Release\\");
	PO::Dx11::add_shader_path(u".\\");
#endif // DEBUG
	

	PO::context_implement imp;

	{
		imp.create([](PO::context& c) {
			c.create_singleton_component<window_component>();
		});
		imp.loop();
	}
	


	system("pause");
	return 0;
}
