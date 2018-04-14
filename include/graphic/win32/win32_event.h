#pragma once
#include "../interface/event.h"
#include <Windows.h>
#include <optional>
namespace PO::Win32
{
	std::optional<event> translate_message_to_event(const MSG& msg);
}
