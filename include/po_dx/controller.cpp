#include "controller.h"
namespace PO
{
	namespace Dx
	{
		void showcase::binding(KeyValue kv, State s)
		{
			key_mapping[kv] = s;
		}

		void showcase::binding(std::initializer_list<std::pair<KeyValue, State>> i)
		{
			for (auto ite : i)
				binding(ite.first, ite.second);
		}

		void showcase::remove_binding(KeyValue kv)
		{
			auto ite = key_mapping.find(kv);
			if (ite == key_mapping.end())
				key_mapping.erase(ite);
		}

		Respond showcase::respond(event& e)
		{
			if (e.is_key())
			{
				auto ite = key_mapping.find(e.key.value);
				if (ite != key_mapping.end())
				{
					uint8_t key_state = static_cast<uint8_t>(ite->second);
					if (e.key.is_down())
						state |= key_state;
					else if (e.key.is_up())
						state &= ~key_state;
					return Respond::Truncation;
				}
			}
			return Respond::Pass;
		}

		void showcase::lose_focus()
		{
			state = 0;
		}

		void showcase::apply(duration da, transfer3D& t3)
		{
			if (static_cast<bool>(state & static_cast<uint8_t>(State::X_CW)) != static_cast<bool>(state & static_cast<uint8_t>(State::X_ACW)))
			{
				t3.qua = t3.qua * PO::Dx::rotation_axis{ da.count() * roll_speed.x / ((state & static_cast<uint8_t>(State::X_CW)) ? 1000.0f : -1000.0f) ,{ 1.0, 0.0, 0.0 } };
			}

			if (static_cast<bool>(state & static_cast<uint8_t>(State::Y_CW)) != static_cast<bool>(state & static_cast<uint8_t>(State::Y_ACW)))
			{
				t3.qua = t3.qua * PO::Dx::rotation_axis{ da.count() * roll_speed.x / ((state & static_cast<uint8_t>(State::Y_CW)) ? 1000.0f : -1000.0f) ,{ 0.0, 1.0, 0.0 } };
			}

			if (static_cast<bool>(state & static_cast<uint8_t>(State::Z_CW)) != static_cast<bool>(state & static_cast<uint8_t>(State::Z_ACW)))
			{
				t3.qua = t3.qua * PO::Dx::rotation_axis{ da.count() * roll_speed.x / ((state & static_cast<uint8_t>(State::Z_CW)) ? 1000.0f : -1000.0f) ,{ 0.0, 0.0, 1.0 } };
			}

			if (static_cast<bool>(state & static_cast<uint8_t>(State::T_FR)) != static_cast<bool>(state & static_cast<uint8_t>(State::T_BA)))
			{
				t3.poi = t3.poi + float3(0.0, 0.0, da.count() * roll_speed.x / ((state & static_cast<uint8_t>(State::T_FR)) ? 1000.0f : -1000.0f));
				//t3.qua = t3.qua * PO::Dx::rotation_axis{ da.count() * roll_speed.x / ((state & static_cast<uint8_t>(State::Z_CW)) ? 1000.0f : -1000.0f) ,{ 0.0, 11.0, 0.0 } };
			}
		}
	}
}