#include "volume_cloud_material.h"

HeightMap2D::HeightMap2D(creator& c) : material_resource(c, u"HeightMap2D.cso", blend_state::description{
	FALSE, FALSE, D3D11_RENDER_TARGET_BLEND_DESC{ TRUE, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL }
}) {

	depth_stencil_state::description des = depth_stencil_state::default_description;
	des.DepthEnable = FALSE;
	dss.create(c, des);
}

const element_requirement& HeightMap2D::requirement() const
{
	return make_element_requirement(
		[](stage_context& sc, property_wrapper_t<property_viewport_transfer>& pvt) {
		sc.PS() << pvt.viewport[2];
	}, [](stage_context& sc, property_wrapper_t<property>& rd) {
		sc.PS() << rd.BaseShapeTex[0] << rd.ss[0];
	}, [](stage_context& sc, property_wrapper_t<defer_renderer_default::property_linear_z>& pp)
	{
		sc.PS() << pp.ss[2] << pp.z_buffer[2];
	}, [](stage_context& sc, property_wrapper_t<property_local_transfer>& rd)
	{
		sc.PS() << rd.transfer[1];
	}, [](stage_context& sc, property_wrapper_t<property_volumecloud_debug_value>& pw)
	{
		sc.PS() << pw.bc[0];
	}
	);
}

SignedDistanceField3D::SignedDistanceField3D(creator& c) : material_resource(
	c, u"SignedDistanceField3D.cso", blend_state::description{
	FALSE, FALSE, D3D11_RENDER_TARGET_BLEND_DESC{ TRUE, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL }
})
{
	depth_stencil_state::description des = depth_stencil_state::default_description;
	des.DepthEnable = FALSE;
	dss.create(c, des);
}

const element_requirement& SignedDistanceField3D::requirement() const
{
	return make_element_requirement(
		[](stage_context& sc, property_wrapper_t<property>& rd) {
		sc.PS() << rd.tex[1] << rd.ss[1];
	}, [](stage_context& sc, property_wrapper_t<property_viewport_transfer>& pvt) {
		sc.PS() << pvt.viewport[2];
	}, [](stage_context& sc, property_wrapper_t<defer_renderer_default::property_linear_z>& pp)
	{
		sc.PS() << pp.ss[0] << pp.z_buffer[0];
	}, [](stage_context& sc, property_wrapper_t<property_local_transfer>& rd)
	{
		sc.PS() << rd.transfer[1];
	}, [](stage_context& sc, property_wrapper_t<property_volumecloud_debug_value>& pw)
	{
		sc.PS() << pw.bc[0];
	}
	);
}

Density3DEdge2DDensity2D::Density3DEdge2DDensity2D(creator& c) : material_resource(
	c, u"Density3DEdge2DDensity2D.cso", blend_state::description{
	FALSE, FALSE, D3D11_RENDER_TARGET_BLEND_DESC{ TRUE, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL }
})
{
	depth_stencil_state::description des = depth_stencil_state::default_description;
	des.DepthEnable = FALSE;
	dss.create(c, des);
}

const element_requirement& Density3DEdge2DDensity2D::requirement() const
{
	return make_element_requirement(
		[](stage_context& sc, property_wrapper_t<property>& rd) {
		sc.PS() << rd.Edge[1] << rd.ss[1] << rd.Height[2] << rd.DensityMap[3];
	}, [](stage_context& sc, property_wrapper_t<property_viewport_transfer>& pvt) {
		sc.PS() << pvt.viewport[2];
	}, [](stage_context& sc, property_wrapper_t<defer_renderer_default::property_linear_z>& pp)
	{
		sc.PS() << pp.ss[0] << pp.z_buffer[0];
	}, [](stage_context& sc, property_wrapper_t<property_local_transfer>& rd)
	{
		sc.PS() << rd.transfer[1];
	}, [](stage_context& sc, property_wrapper_t<property_local_transfer>& rd)
	{
		sc.PS() << rd.transfer[1];
	}
	);
}