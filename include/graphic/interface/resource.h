#pragma once
#include "format.h"
#include "../../po/tool/tool.h"
#include "../../po/tool/path_system.h"
#include <atomic>
#include <variant>
namespace PO::Graphic
{

	struct resource_map
	{
		uint64_t allocate_resource(std::byte* data, uint64_t i);
	};

	// buffer setting *********************************************
	enum class BufUsage
	{
		UNCHANGE,
		SWAP,
		OUTPUT
	};

	namespace Implement
	{

	}

	struct buffer
	{
		uint64_t vision;
		BufUsage usage;
		uint32_t size;
		uint64_t resource_id;
	};

	struct buffer_structure
	{
		BufUsage usage;
		uint32_t size;
		uint64_t resource_id;
	};

	// texture *************************************************
	enum class TexUsage
	{
		UNCHANGE,
		SWAP,
		RENDER_TARGET,
		DEPTH,
		OUTPUT
	};

	enum class SampleState
	{
		LINE,
	};

	struct tex_size
	{
		uint16_t x, y, z;
		tex_size() : x(0), y(0), z(0) {}
		tex_size(uint16_t x_size = 0, uint16_t y_size = 0, uint16_t z_size = 0) : x(x_size), y(y_size), z(z_size) {}
		bool is_1d() const noexcept { return x != 0 && y == 0 && z == 0; }
		bool is_2d() const noexcept { return x != 0 && y != 0 && z == 0; }
		bool is_3d() const noexcept { return x != 0 && y != 0 && z != 0; }
	};

	namespace Implement
	{

		struct texture_scription
		{
			FormatPixel format;
			tex_size size;
			uint16_t mipmap;
			std::shared_ptr<std::byte[]> pixel;
		};

		struct texture_delegate
		{

		};

		struct texture_swap : Tool::atomic_reference_count
		{
			uint64_t vision;
			TexUsage usage;
			SampleState sampler;
			uint64_t resource_id;
		};

		struct texture_resource : Tool::atomic_reference_count
		{
			uint64_t vision;
			TexUsage usage;
			SampleState sampler;
			size_t resource;
			std::variant<texture_scription, texture_delegate> scription;
			void update(texture_swap& ts);
		};

		
	}

	struct texture_agent
	{
		//virtual bool insert_agent(std::shared_ptr<Implement::texture_request_implement>) = 0;
		virtual bool direct_load(const Tool::path&, Implement::texture_scription&) = 0;
		virtual ~texture_agent() = default;
	};

	class texture
	{
		Tool::intrusive_ptr<Implement::texture_resource> scription;
		Tool::intrusive_ptr<Implement::texture_swap> swap;
	public:
		void load(texture_agent& ta, TexUsage usage, Tool::path path);
		void create(TexUsage usage, tex_size size, size_t mipmap, std::shared_ptr<std::byte[]> pixel);
	};

}