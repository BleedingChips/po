#pragma once
#include <chrono>
#include "..\tool\tool.h"
#include "..\tool\thread_tool.h"
#include <stdint.h>
namespace PO
{
	using duration = std::chrono::duration<long long, std::ratio<1, 1000>>;
	using time_point = std::chrono::time_point<std::chrono::system_clock>;
	inline decltype(auto) get_time_now() { return std::chrono::system_clock::now(); }
	struct time_calculator
	{
		time_point record_point;
		duration require_duration;
	public:
		time_calculator(time_calculator&) = default;
		time_calculator(duration du = duration(10)) : record_point(get_time_now()), require_duration(du) {}
		void set_duration(duration du) { require_duration = du; }
		bool tick(time_point tp, duration& dua)
		{
			dua = std::chrono::duration_cast<duration>(tp - record_point);
			if (dua >= require_duration)
			{
				record_point = tp;
				return true;
			}
			return false;
		}
		template<typename T>
		bool tick(time_point tp, T&& t)
		{
			duration dua = std::chrono::duration_cast<duration>(tp - record_point);
			if (dua >= require_duration)
			{
				record_point = tp;
				t(dua);
				return true;
			}
			return false;
		}
	};

	enum class EventType : std::int8_t
	{
		E_CLICK_L,
		E_CLICK_R,
		E_CLICK_M,
		E_WHEEL,
		E_MOVE,
		E_KEY,
		E_CLOSE
	};

	enum class ButtonState
	{
		BS_UP = 0,
		BS_DOWN =1,
	};

	enum class KeyState : int8_t
	{
		KS_SHIFT = 0x1,
		KS_ALT = 0x2,
		KS_CTRL = 0x3,
	};

	struct click_type
	{
		EventType type;
		ButtonState button_state : 1;
		KeyState key_state : 3;
		int : 0;
		int16_t location_x;
		int16_t location_y;
		bool is_up() { return button_state == ButtonState::BS_UP; }
		bool is_down() { return button_state == ButtonState::BS_DOWN; }
		int16_t get_x() { return location_x; }
		int16_t get_y() { return location_y; }
	};

	struct move_type
	{
		EventType type;
		int16_t location_x;
		int16_t location_y;
		int16_t get_x() { return location_x; }
		int16_t get_y() { return location_y; }
	};
	
	union event
	{
		EventType type;
		click_type click;
		move_type move;
		bool is_quit() { return type == EventType::E_CLOSE; }
		bool is_click_L() { return type == EventType::E_CLICK_L; }
		bool is_click_M() { return type == EventType::E_CLICK_M; }
		bool is_click_R() { return type == EventType::E_CLICK_R; }
		bool is_click() { return is_click_L() || is_click_M() || is_click_R(); }
		bool is_move() { return type == EventType::E_MOVE; }
	};
	
}