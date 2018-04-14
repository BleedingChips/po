#pragma once
#include "dx_math.h"
#include "../../po/po.h"
#include "../interface/event.h"
namespace PO
{
	namespace Dx
	{
		class showcase
		{
			float3 roll_speed = float3{ 40.0f, 40.0f, 40.0f };
			float translation_speed = 0.5f;
			uint8_t state;
		public:
			enum class State : uint8_t
			{
				X_CW = 0x01,
				Y_CW = 0x02,
				Z_CW = 0x04,
				T_FR = 0x08,
				X_ACW = 0x10,
				Y_ACW = 0x20,
				Z_ACW = 0x40,
				T_BA = 0x80,
			};
		private:
			std::map<KeyValue, State> key_mapping;
		public:
			void set_roll_spped(float3 s) { roll_speed = s; }
			void set_translation_speed(float s) { translation_speed = s; }
			void binding(KeyValue, State);
			void binding(std::initializer_list<std::pair<KeyValue, State>>);
			void remove_binding(KeyValue);
			Respond respond(const event& e);
			void lose_focus();
			//bool apply(duration da, transfer3D& t3);
		};

		class aircraft
		{
			float3 Speed;
		};

	}
}