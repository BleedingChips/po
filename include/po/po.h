#pragma once
#include "tool\event.h"
#include "tool\math.h"
#include "frame\context.h"

namespace PO
{
	//using component_res = PO::ECSFramework::component_res;
	using system_default_define = PO::ECSFramework::system_default_define;
	using context = PO::ECSFramework::context;
	using entity = PO::ECSFramework::entity;
	//using context_implement = PO::ECSFramework::context_implement;
	using duration_ms = std::chrono::milliseconds;
	template<typename ...T> using filter = PO::ECSFramework::filter<T...>;
}
