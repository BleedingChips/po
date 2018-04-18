#pragma once
#include "../../po/tool/tool.h"
#include "../../po/tool/path_system.h"
#include "property.h"
#include "grid.h"
namespace PO::Graphic
{

	struct material_binding
	{
		virtual void bind() = 0;
	};

	enum class BlendMode
	{
		OPAQUE,
		ALPHA_ADD,
		ALPHA_MUL
	};

	enum class DepthMode
	{
		WRITE,
		DONWRITE
	};

	enum class Primitive
	{
		TRA
	};

	struct pixel_generator
	{
		std::shared_ptr<grid> grid_ptr;
		Tool::path v_shader;
		Tool::path d_shader;
		Tool::path h_shader;
		virtual void operator()() {}
	};

	struct material_interface
	{
		BlendMode Mode = BlendMode::OPAQUE;
		Tool::path shader_path;
		virtual void operator()(material_binding&) {};
	};



}