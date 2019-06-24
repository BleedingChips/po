#pragma once
#include "../graphic/event.h"
#include <Windows.h>
#include <optional>
namespace PO::Win32
{
	std::optional<event> translate_message_to_event(const MSG& msg);
	inline bool translate_message_to_event(const MSG& msg, event& e) 
	{
		auto res = translate_message_to_event(msg);
		if (res)
		{
			e = *res;
			return true;
		}
		return false;
	}
}
