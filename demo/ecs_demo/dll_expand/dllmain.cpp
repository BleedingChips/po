// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include "..//..//..//include/po/frame/ecs/interface.h"
#include <mutex>
#include <random>
#include <iostream>
std::mutex* gobal_mutex;
using namespace PO::ECS;

template<typename Type> struct CallRecord
{
	CallRecord()
	{
		std::lock_guard lg(*gobal_mutex);
		std::cout << "thread id<" << std::this_thread::get_id() << "> : " << typeid(Type).name() << " - start" << std::endl;
	}

	~CallRecord()
	{
		std::lock_guard lg(*gobal_mutex);
		std::cout << "thread id<" << std::this_thread::get_id() << "> : " << typeid(Type).name() << " - end" << std::endl;
	}
};

extern "C" {
	void __declspec(dllexport) init(PO::ECS::Context*, std::mutex*);
}

struct Location
{
	float x, y;
};

struct Collision
{
	float Range;
};

struct Velocity
{
	float x, y;
};

struct MoveSystem
{
	void operator()(Filter<Location, Velocity, const Collision>& f, PO::ECS::Context& c)
	{
		CallRecord<MoveSystem> record;
		for (auto ite : f)
		{
			auto& [lo, ve, col] = std::get<1>(ite);
			lo.x += ve.x * c.duration_s();
			lo.y += ve.y * c.duration_s();
			if ((lo.x - col.Range) < -1.0f)
			{
				lo.x = col.Range - 1.0f;
				ve.x = -ve.x;
			}
			else if ((lo.x + col.Range) > 1.0f)
			{
				lo.x = 1.0f - col.Range;
				ve.x = -ve.x;
			}
			if ((lo.y - col.Range) < -1.0f)
			{
				lo.y = col.Range - 1.0f;
				ve.y = -ve.y;
			}
			else if ((lo.y + col.Range) > 1.0f)
			{
				lo.y = 1.0f - col.Range;
				ve.y = -ve.y;
			}

		}
	}
};

struct EventTesting
{
	void operator()(EventViewer<uint64_t>& EV)
	{
		CallRecord<EventTesting> record;
		{
			std::lock_guard lg(*gobal_mutex);
			for (auto& ite : EV)
				std::cout << " Last Frame Event : " << ite << std::endl;
		}
		
	}
};

void init(PO::ECS::Context* context, std::mutex* mutex)
{
	gobal_mutex = mutex;
	std::random_device r_dev;
	std::default_random_engine engine(r_dev());
	std::uniform_real_distribution<float> location(-0.9f, 0.9f);
	std::uniform_real_distribution<float> range(0.03f, 0.05f);
	std::uniform_real_distribution<float> vel(-0.4f, 0.4f);

	for (size_t i = 0; i < 4; ++i)
	{
		auto entity = context->create_entity();
		context->create_component<Location>(entity, location(engine), location(engine));
		context->create_component<Collision>(entity, range(engine));
	}
	context->create_system<MoveSystem>();
	context->create_system<EventTesting>();
	context->create_system([](Context& c) {
		std::lock_guard lg(*gobal_mutex);
		std::cout << "thread id<" << std::this_thread::get_id() <<"> : lambda 函数刷个存在感" << std::endl;
	});
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

