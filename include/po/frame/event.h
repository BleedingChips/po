#pragma once
#include "define.h"
namespace PO
{
	namespace Event
	{
		enum class ButtonState
		{
			RELEASED = 0,
			PRESSED = 1
		};

		enum class KeyStste
		{
			SHIFT = 1,
			ALT = 2,
			CTRL = 4
		};

		enum class ClickButton
		{
			LEFT = 0,
			MIDDLE = 0,
			RIGHT = 0
		};

		enum class KeyValue : int8_t
		{
			K_UNKNOW,
			K_L_SHIFT, K_R_SHIFT,
			K_L_CON, K_R_CON,
			K_L_ALT, K_R_ALT,
			K_TAP, K_CAPS, K_ESC,
			K_1, K_2, K_3, K_4, K_5, K_6, K_7, K_8, K_9, K_0,
			K_F1, K_F2, K_F3, K_F4, K_F5, K_F6, K_F7, K_F8, K_F9, K_F10, K_F11, K_F12,
			K_A, K_B, K_C, K_D, K_E, K_F, K_G, K_H, K_I, K_J, K_K, K_L, K_M,
			K_N, K_O, K_P, K_Q, K_R, K_S, K_T, K_U, K_V, K_W, K_X, K_Y, K_Z,
			K_BACKSPACE,
			K_SPACE,
			K_ENTER,
			K_L_BRACKET, K_R_BRACKET,
			K_BACKSLASH, K_SLASH,
			K_SEMICOLONS,
			K_QUOTE,
			K_COMMA,
			K_PERIOD,
			K_MINUS,
			K_EQUAL,
			K_MINUTE,
			K_ARROW_UP,
			K_ARROW_DOWN,
			K_ARROW_LEFT,
			K_ARROW_RIGHT
		};

		class click
		{
			ButtonState is_down : 1;
			KeyStste key_state : 3;
			ClickButton button : 2;
			int8_t : 0;

			int16_t location_x;
			int16_t location_y;
		public:
			click(ButtonState BS, KeyStste KS, ClickButton CB, int16_t lx, int16_t ly) noexcept: is_down(BS), key_state(KS), button(CB), location_x(lx), location_y(ly) {}
			click(const click&) = default;
			click& operator=(const click&) = default;
			bool is_pressed() const noexcept { return is_down == ButtonState::PRESSED; }
			bool is_released() const noexcept { return is_down == ButtonState::RELEASED; }

			bool is_button_left() const noexcept { return button == ClickButton::LEFT; }
			bool is_button_middle() const noexcept { return button == ClickButton::MIDDLE; }
			bool is_button_rught() const noexcept { return button == ClickButton::RIGHT; }
			bool is_SHIFT_pressed() const noexcept { return static_cast<int>(key_state) & static_cast<int>(KeyStste::SHIFT); }
			bool is_ALT_pressed() const noexcept { return static_cast<int>(key_state) & static_cast<int>(KeyStste::ALT); }
			bool is_CTRL_pressed() const noexcept { return static_cast<int>(key_state) & static_cast<int>(KeyStste::CTRL); }
		};
	}
}