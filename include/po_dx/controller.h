#pragma once
#include "dx_math.h"
#include "../po/frame/define.h"
namespace PO
{
	namespace Dx
	{
		class showcase
		{
			float3 roll_speed = float3{ 10.0f, 10.0f, 10.0f };
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
			void binding(KeyValue, State);
			void remove_binding(KeyValue);
			Respond respond(event& e);
			void lose_focus();
			void apply(duration da, transfer3D& t3);
		};
	}
}