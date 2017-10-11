#include <iostream>
#include <array>
#include "DirectXTex.h"
int main(int argc, char *argv[])
{

	/*
	std::array<uint8_t, 256 * 256 * 4> PixelData;

	for (size_t x = 0; x < 256; ++x)
	{
		for (size_t y = 0; y < 256; ++y)
		{
			size_t loc = (y * 256 + x) * 4;
			PixelData[loc] = x;
			PixelData[loc + 1] = y;
			PixelData[loc + 2] = 255;
			PixelData[loc + 3] = 255;
		}
	}


	DirectX::Image Im;
	Im.format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
	Im.height = 256;
	Im.width = 256;
	Im.pixels = PixelData.data();
	Im.rowPitch = 256 * 4;
	Im.slicePitch = 256 * 256;

	DirectX::SaveToTGAFile(Im, L"OutPutTest.tga");
	std::cout << "finish" << std::endl;
	*/
}






