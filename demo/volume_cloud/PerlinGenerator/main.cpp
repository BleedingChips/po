#include <iostream>
#include <sstream>
#include <array>
#include <string>
#include "po/tool/simple_command_analyzer.h"
#include "po\tool\utf_support.h"
#include "DirectXTex.h"
#include <random>
#include <math.h>

template<typename T, size_t i>
std::ostream& operator << (std::ostream& o, const std::array<T, i>& a)
{
	o << "{";
	for (size_t p = 0; p < i; ++p)
	{
		o << a[p];
		if (p + 1 != i)
			o << ",";
	}
	o << "}";
	return o;
}

int main(int arg, const char** com)
{
	const char* match[] =
	{
		"Seed",
		"Size",
		"Block",
		"Name"
	};

	struct AnalyzeData
	{
		uint32_t random_seed = 0;
		std::array<uint32_t, 4> simulate_size = { 0, 0, 0, 0 };
		std::array<uint32_t, 4> block = {0, 0, 0, 0};
		std::string name;
	}Data;

	auto index = PO::simple_command_analyzer(com, arg, match, 4);
	std::stringstream ss;

	size_t current = 4;
	for (size_t i = 0; i < arg; ++i)
	{
		if (index[i] <= 4)
			current = index[i];
		else if(current != 4){
			switch (current)
			{
			case 1:
				if (index[i] - 5 < 4)
				{
					ss.clear();
					ss << com[i];
					ss >> Data.simulate_size[index[i] - 5];
				}
				break;
			case 0:
				if (index[i] - 5 == 0)
				{
					ss.clear();
					ss << com[i];
					ss >> Data.random_seed;
				}
				break;
			case 2:
				if (index[i] - 5 < 4)
				{
					ss.clear();
					ss << com[i];
					ss >> Data.block[index[i] - 5];
				}
				break;
			case 3:
				if (index[i] - 5 == 0)
				{
					Data.name = com[i];
				}
				break;
			}
		}
	}

	bool check = true;
	for (size_t i = 0; i < 4; ++i)
	{
		if (Data.simulate_size[i] == 0 || Data.block[i] == 0)
		{
			check = false;
			break;
		}
	}

	if (check)
	{
		std::vector<uint8_t> pixel;
		uint32_t size[4] = { Data.simulate_size[0], Data.simulate_size[1], Data.simulate_size[2], Data.simulate_size[3] };
		uint32_t block[4] = { Data.block[0], Data.block[1], Data.block[2], Data.block[3] };
		pixel.resize(size[0] * size[1] * size[2] * size[3] * 4);
		std::vector<float> seed;
		auto cal_block_count = [](uint32_t i) -> uint32_t {return (i + 1) * (i + 1) * (i + 1); };
		size_t point_size = cal_block_count(block[0]) + cal_block_count(block[1]) + cal_block_count(block[2]) + cal_block_count(block[3]);
		seed.reserve(point_size);
		std::mt19937 mt(Data.random_seed);
		std::uniform_real_distribution<float> nd(0.0, 1.0);

		for (size_t i = 0; i < point_size; ++i)
			seed.push_back(nd(mt));

		auto find_point = [](uint32_t input_size[3], uint32_t block) -> size_t {
			return input_size[2] * (block + 1) * (block + 1) + input_size[1] * (block + 1) + input_size[0];
		};

		auto cal_rate = [](float t) -> float { return (t * t * t * (t * (t * 6 - 15) + 10)); };
		auto lerp = [](float f, float p, float r) -> float { return f * (1.0 - r) + p * r; };

		auto cal_perlin_noise = [&](std::array<uint32_t, 3> input_size, uint32_t block, size_t start) -> float {
			float Area[3] = {
				size[0] / float(block),
				size[1] / float(block),
				(size[2] * size[3] * 4) / float(block)
			};
			uint32_t count[3]; float Rate[3];
			for (size_t i = 0; i < 3; ++i)
			{
				count[i] = floorf(input_size[i] / Area[i]);
				Rate[i] = input_size[i] / Area[i];
				Rate[i] = Rate[i] - floorf(Rate[i]);
			}
			uint32_t in[8][3] = {
				{ 0, 0, 0 },
				{ 1, 0, 0 },
				{ 0, 1, 0 },
				{ 1, 1, 0 },
				{ 0, 0, 1 },
				{ 1, 0, 1 },
				{ 0, 1, 1 },
				{ 1, 1, 1 }
			};
			float Result[8];
			for (size_t i = 0; i < 8; ++i)
			{
				uint32_t insize[3];
				for (size_t k = 0; k < 3; ++k)
					insize[k] = count[k] + in[i][k];
				Result[i] = seed[find_point(insize, block) + start];
			}
			float X[4] = {
				lerp(Result[0], Result[1], cal_rate(Rate[0])),
				lerp(Result[2], Result[3], cal_rate(Rate[0])),
				lerp(Result[4], Result[5], cal_rate(Rate[0])),
				lerp(Result[6], Result[7], cal_rate(Rate[0]))
			};

			float Y[2] = {
				lerp(X[0], X[1], cal_rate(Rate[1])),
				lerp(X[2], X[3], cal_rate(Rate[1])),
			};
			return lerp(Y[0], Y[1], cal_rate(Rate[2]));
		};
		for (size_t z = 0; z < size[2] * size[3] * 4; ++z)
		{
			std::cout << "current :"<< z << " / " << size[2] * size[3] * 4 << std::endl;
			for (size_t y = 0; y < size[1]; ++y)
			{
				for (size_t x = 0; x < size[0]; ++x)
				{
					std::array<uint32_t, 3> cur_size = { x, y, z };
					std::array<uint32_t, 2> cur_size2 = { x + ((z % (size[2] * size[3]) ) % size[2]) * size[0],  y + ((z % (size[2] * size[3])) / size[2]) * size[0]};
					uint32_t cur_size3 = (cur_size2[0] + cur_size2[1] * size[2] * size[0]) * 4 + z / (size[2] * size[3]);
					
					pixel[cur_size3] =
						(
							cal_perlin_noise(cur_size, block[0], 0) * 0.5f
							+ cal_perlin_noise(cur_size, block[1], cal_block_count(block[0])) * 0.25f
							+ cal_perlin_noise(cur_size, block[2], cal_block_count(block[0]) + cal_block_count(block[1])) * 0.125f
							+ cal_perlin_noise(cur_size, block[3], cal_block_count(block[0]) + cal_block_count(block[1]) + cal_block_count(block[2])) * 0.125f
							) * 255;
				}
			}
			
		}
		std::string Name = "T_VolumeCloud_Perlin_S.tga";
		DirectX::Image im{size[0] * size[2], size[1] * size[3], DXGI_FORMAT_R8G8B8A8_UNORM, size[0] * size[2] * 4, 0, pixel.data()};
		std::u16string out = PO::asc_to_utf16(Name);
		assert(SUCCEEDED(DirectX::SaveToTGAFile(im, L"T_VolumeCloud_Perlin_S.tga")));
		std::cout << "finish" << std::endl;
		return 0;
	}
	else {
		return -1;
	}
}