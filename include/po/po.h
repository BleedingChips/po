#pragma once
#include "tool\event.h"
#include "tool\math.h"
#include "frame\context.h"

namespace PO
{
	//using component_res = PO::ECSFramework::component_res;
	using system_default = PO::ECSFramework::system_default;
	using context = PO::ECSFramework::context;
	using entity = PO::ECSFramework::entity;
	//using context_implement = PO::ECSFramework::context_implement;
	using duration_ms = std::chrono::milliseconds;
	template<typename ...component> using filter = PO::ECSFramework::filter<std::remove_reference_t<component>...>;
	template<typename ...component> using pre_filter = PO::ECSFramework::pre_filter<std::remove_reference_t<component>...>;
	template<typename component> using provider = PO::ECSFramework::provider<component>;
	template<typename component> using receiver = PO::ECSFramework::receiver<component>;
	using SystemSequence = PO::ECSFramework::SystemSequence;
	using SystemLayout = PO::ECSFramework::SystemLayout;
}
