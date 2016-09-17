#pragma once

namespace PO
{
	enum class Event_Type
	{
		Mouse,
		Mouse_LButton_Down,
		Mouse_LButton_Up,
		Mouse_MButton_Down,
		Mouse_MButton_Up,
		Mouse_RButton_Down,
		Mouse_RButton_Up,
		Mouse_Move,
		Mouse_Wheel,
		Key,
		Key_Down,
		Key_Up
	};

	struct event
	{
		int msg;
	};

}