#pragma once
#include "tool\event.h"
#include "tool\math.h"
#include "frame\context.h"

namespace PO
{
	using component_res = PO::ECSFramework::component_res;
	using system_res = PO::ECSFramework::system_res;
	using context = PO::ECSFramework::context;
	using entity = PO::ECSFramework::entity;
	using context_implement = PO::ECSFramework::context_implement;
	using duration_ms = std::chrono::milliseconds;
}
