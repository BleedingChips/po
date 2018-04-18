#pragma once
#include "format.h"
#include "../../po/tool/tool.h"
#include <variant>
#include <future>
#include <filesystem>
#include <optional>
namespace PO::Graphic
{

	enum class FormatTex
	{

	};

	enum class FormatTexOP
	{

	};

	enum class FormatTexRT
	{

	};

	enum class FormatTexDS
	{

	};



	struct Tex1
	{
		size_t x;
	};

	struct Tex2
	{
		size_t x, y;
	};

	struct Tex3
	{
		size_t x, y, z;
	};

	enum TextureStyle
	{
		Default,
		Swap,
		RenderTarget,
		Output
	};

	namespace Implement
	{
		struct texture_scription
		{
			TextureStyle style;
			std::variant<Tex1, Tex2, Tex3> size;
			size_t mipmap;
			std::shared_ptr<std::byte[]> data;
		};

		struct texture_request_implement
		{
			std::experimental::filesystem::path path;
			size_t target_mipmap;
			std::promise<texture_scription> request;
		};

		struct texture_request
		{
			TextureStyle style;
			size_t mipmap;
			std::shared_ptr<texture_request_implement> request;
			std::optional<std::future<Implement::texture_scription>> result_furture;
		};
	}

	

	struct texture
	{
		struct resource_id
		{
			size_t id;
		};

		std::shared_ptr<resource_id> resource_ptr;

		std::variant<Implement::texture_scription, Implement::texture_request> scription;
		bool create(FormatTex format, std::variant<Tex1, Tex2, Tex3> size, size_t mipmap, std::shared_ptr<std::byte[]>);
		bool create_rt(FormatTexRT style, std::variant<Tex1, Tex2, Tex3> size, size_t mipmap, std::shared_ptr<std::byte[]>);
		bool create_ds(FormatTexDS style, std::variant<Tex1, Tex2, Tex3> size, size_t mipmap, std::shared_ptr<std::byte[]>);
		bool create_op(FormatTexOP style, std::variant<Tex1, Tex2, Tex3> size, size_t mipmap, std::shared_ptr<std::byte[]>);
		//bool load(texture_agent&,TextureStyle, std::experimental::filesystem::path, size_t mipmap);
	};
}