#pragma once
#include "po.h"
#include "frame\context_implement.h"
namespace PO
{
	using context_implement = PO::ECSFramework::context_implement;
	namespace Error
	{
		using context_logic_error = ::PO::ECSFramework::Error::context_logic_error;
		using system_dependence_circle = ::PO::ECSFramework::Error::system_dependence_circle;
		using system_dependence_confuse = ::PO::ECSFramework::Error::system_dependence_confuse;
	}
}