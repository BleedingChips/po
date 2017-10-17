#include "worley_test.h"
#include "DirectXTex.h"
#include <math.h>
#include <random>
using namespace PO;
using namespace PO::Dx11;

void generator_worley_noise()
{
	std::array<uint8_t, 256 * 256 * 4> data;
	std::array<float2, 500> point;
	std::mt19937 mt(123444);
	std::mt19937 mt2(1234442);
	std::normal_distribution<float> nd(0.0f, 0.5f);
	point[0] = float2(0, 0);
	for (size_t i = 1; i < point.size(); ++i)
	{
		point[i] = float2(nd(mt), nd(mt2));
	}
		

	std::atomic_uint count = 0;

	auto thread_func = [&](size_t max_index, size_t current_index, size_t width, size_t height) {
		size_t index_width = height / max_index + 1; 
		size_t start_height = current_index * index_width;
		for (size_t y = start_height; y < start_height + index_width && y < height; ++y)
		{
			for (size_t x = 0; x < width; ++x)
			{
				float2 f_loc = float2(float(x) / width * 2.0 - 1.0, float(y) / height * 2.0 - 1.0);
				size_t index = (y * width + x) * 4;
				float min = 1.0;
				for (auto& ite : point)
				{
					float2 p = ite - f_loc;
					float dis = sqrtf(p.x * p.x + p.y * p.y) * 10.0f;
					if (dis > 1.0) dis = 0.99;
					if (dis < min)
						min = dis;
				}
				data[index] = static_cast<uint8_t>((1.0 - min) * 255);
				data[index + 1] = 100;
				data[index + 2] = 100;
				data[index + 3] = 255;
			}
			++count;
		}
	};

	{
		std::thread fun1(thread_func, 4, 0, 256, 256);
		std::thread fun2(thread_func, 4, 1, 256, 256);
		std::thread fun3(thread_func, 4, 2, 256, 256);
		std::thread fun4(thread_func, 4, 3, 256, 256);

		uint last_count = 0;

		while (last_count != 256)
		{
			if (count != last_count)
			{
				std::cout << last_count << std::endl;
				last_count++;
			}
			std::this_thread::sleep_for(duration(1));
		}

		std::cout << "finish generator" << std::endl;

		fun1.join();
		fun2.join();
		fun3.join();
		fun4.join();
	}
	
	count = 0;
	

	decltype(data) fbm_data;
	float frequence = 1.0, amplitude = 0.5, Lacunarity = 1.8715, Gain = 0.5;
	size_t Octaves = 3;

	auto thread_func_fbm = [&](size_t max_index, size_t current_index, size_t width, size_t height) {
		size_t index_width = height / max_index + 1;
		size_t start_height = current_index * index_width;
		for (size_t y = start_height; y < start_height + index_width && y < height; ++y)
		{
			for (size_t x = 0; x < width; ++x)
			{
				float fre = frequence, amp = amplitude, Lac = Lacunarity, G = Gain;
				float total = 0;
				size_t this_index = (y * width + x) * 4;
				for (size_t i = 0; i < Octaves; ++i)
				{
					size_t pre_x = size_t(fre * x);
					pre_x = pre_x % width;
					size_t pre_y = size_t(fre * y);
					pre_y = pre_y % width;
					size_t index = (pre_y * width + pre_x) * 4;
					total += data[index] * amp;
					fre *= Lac; amp *= G;
				}
				if (total > 255) total = 255;
				fbm_data[this_index] = uint8_t(total);
			}
			++count;
		}
	};

	{

		std::cout << "start fbm" << std::endl;

		std::thread fun1(thread_func_fbm, 4, 0, 256, 256);
		std::thread fun2(thread_func_fbm, 4, 1, 256, 256);
		std::thread fun3(thread_func_fbm, 4, 2, 256, 256);
		std::thread fun4(thread_func_fbm, 4, 3, 256, 256);

		uint last_count = 0;

		while (last_count != 256)
		{
			if (count != last_count)
			{
				std::cout << last_count << std::endl;
				last_count++;
			}
			std::this_thread::sleep_for(duration(1));
		}

		std::cout << "finish fbm" << std::endl;

		fun1.join();
		fun2.join();
		fun3.join();
		fun4.join();
	}

	struct Image
	{
		size_t      width;
		size_t      height;
		DXGI_FORMAT format;
		size_t      rowPitch;
		size_t      slicePitch;
		uint8_t*    pixels;
	};


	{
		DirectX::Image SI{ 256, 256, DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM, 256 * 4, 256 * 256 * 4,  data.data() };
		assert(SUCCEEDED(DirectX::SaveToDDSFile(SI, 0, L"Worley.dds")));
	}

	{
		DirectX::Image SI{ 256, 256, DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM, 256 * 4, 256 * 256 * 4,  fbm_data.data() };
		assert(SUCCEEDED(DirectX::SaveToDDSFile(SI, 0, L"Worley2.dds")));
	}

	

	

	
}