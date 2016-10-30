#pragma once
#include <chrono>
#include "..\tool\tool.h"
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

	enum class Event_Type
	{
		E_MOUSE_BUTTON,
		E_MOUSE_WHEEL,
		E_MOUSE_MOVE,
		E_KEY
	};

	//enum class E_Mou

	enum class Sys_Mouse_Button_Event_Type
	{
		//EM_
	};

	struct mouse
	{
		struct mouse_flag
		{
			//Sys_Event_Type type;
			Sys_Mouse_Button_Event_Type button;
		}flag;
		int16_t location_x;
		int16_t loaction_y;
	};

	
	union event_data
	{
		Event_Type type;
	};
	
	struct event
	{
		time_point time;
		event_data data;
	};
	
}