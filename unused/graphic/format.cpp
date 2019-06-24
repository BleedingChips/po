#include "format.h"
namespace PO::Graphic
{

	uint8_t calculate_pixel_size(FormatPixel format)
	{
		switch (format)
		{
		case PO::Graphic::FormatPixel::RGBA_F32:
			return 16;
		case PO::Graphic::FormatPixel::RGB_F32:
			return 12;
		case PO::Graphic::FormatPixel::RGBA_F16:
			return 8;
		case PO::Graphic::FormatPixel::UI32:
		case PO::Graphic::FormatPixel::F32:
		case PO::Graphic::FormatPixel::RGBA_I8:
		case PO::Graphic::FormatPixel::RGBA_UI8:
		case PO::Graphic::FormatPixel::RGBA_U8:
			return 4;
		case PO::Graphic::FormatPixel::F16:
		case PO::Graphic::FormatPixel::UI16:
			return 2;
		default:
			return 0;
		}
	}

	uint8_t tex_dimension(uint3 size)
	{
		if (size.x != 0)
		{
			if (size.y != 0)
			{
				if (size.z != 0)
					return 3;
				else
					return 2;
			}
			else {
				if (size.z == 0)
					return 1;
			}
		}
		return 0;
	}

	/*
	uint8_t calculate_pixel_size(FormatIndex format)
	{
		switch (format)
		{
		case PO::Graphic::FormatIndex::U16:
			return 2;
		case PO::Graphic::FormatIndex::U32:
			return 4;
		default:
			return 0;
		}
	}
	*/
}