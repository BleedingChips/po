#pragma once
#include "../graphic/format.h"
#include <dxgi.h>
#include <exception>
#include <Atlbase.h>
#include <Atlbase.h>
#include <vector>
#include <map>
#include <functional>
#include <dxgi.h>
namespace PO::DXGI
{
	using Format = DXGI_FORMAT;
	Format translate(PO::Graphic::FormatPixel FP) noexcept;
	Graphic::FormatPixel inversen_translate(Format) noexcept;
	uint8_t calculate_pixel_size(Format format);
	inline Format translate(PO::Graphic::FormatRT FP) noexcept { return translate(Graphic::translate_FormatRT(FP)); }
	const char* format_to_utf8(Format) noexcept;

	//std::string DXGI_FORMAT_to_s(DXGI_FORMAT);
	//std::string DXGI_ADAPTER_FLAG_to_s(DXGI_ADAPTER_FLAG);
	//inline std::string DXGI_ADAPTER_FLAG_to_s(UINT u) { return DXGI_ADAPTER_FLAG_to_s(DXGI_ADAPTER_FLAG(u)); }
}