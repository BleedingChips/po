#pragma once
#include "frame\define.h"

namespace PO
{
	//class 





	class move_responder
	{
		
	public:

		enum class state: uint16_t
		{
			X_CW = 0x0001,
			Y_CW = 0x0002,
			Z_CW = 0x0003,

			X_ACW = 0x0010,
			Y_ACW = 0x0020,
			Z_ACW = 0x0030,

			X_FR = 0x0100,
			Y_FR = 0x0200,
			Z_FR = 0x0300,

			X_BA = 0x1000,
			Y_BA = 0x2000,
			Z_BA = 0x3000,
		};

		struct event_holder {
			union 
			{

			} holder;
		};

	};
}