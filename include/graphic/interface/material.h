#pragma once
#include "../../po/tool/tool.h"
#include "../../po/tool/path_system.h"
#include "property.h"
namespace PO::Graphic
{

	struct material_binding
	{
		void bind() {}
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

	enum class VertexFormat
	{
		UINT32_4,
		
		FLOAT4
	};

	enum class Primitive
	{
		TRA
	};

	struct vertex_scriprion
	{
		VertexFormat fromat;
		const char* semantics;
		size_t solt;
		size_t offset;
	};

	struct grid
	{
		std::array<std::unique_ptr<std::byte[]>, 32> data;
		size_t element_count;
		std::vector<vertex_scriprion> format;
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