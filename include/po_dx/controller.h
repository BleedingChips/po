#pragma once
#include "dx_math.h"
#include "../po/frame/define.h"
namespace PO
{
	namespace Dx
	{
		class showcase
		{
			float3 roll_speed = float3{ 0.01, 0.01, 0.01 };
			float translation_speed;
			bool x_clockwise : 1;
			bool x_anticlockwise : 1;
			bool y_clockwise : 1;
			bool y_anticlockwise : 1;
			bool z_clockwise : 1;
			bool z_anticlockwise : 1;
			bool translation_add_state : 1;
			bool translation_del_state : 1;
		public:
			enum class State
			{
				X_CW,
				X_ACW,
				Y_CW,
				Y_ACW,
				Z_CW,
				Z_ACW,
				T_FRONT,
				T_BACK
			};
		private:
			std::map<KeyValue, State> key_mapping;
		public:
			void binding(KeyValue, State);
			void remove_binding(KeyValue);
			Respond respond(event& e);
			void lose_focus();
			void apply(duration da, transfer3D& t3);
		};
	}
}