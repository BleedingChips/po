#pragma once
#include <stdint.h>
namespace PO
{
	class event_touch
	{
	public:
	};


	enum class EventType : uint8_t
	{
		E_CLICK,
		E_WHEEL,
		E_MOVE,
		E_KEY,
		E_CLOSE
	};

	enum class ButtonState : uint8_t
	{
		BS_UP = 0,
		BS_DOWN = 1,
	};

	enum class KeyState : uint8_t
	{
		KS_SHIFT = 0x1,
		KS_ALT = 0x2,
		KS_CTRL = 0x3,
	};

	enum class KeyValue : uint8_t
	{
		K_UNKNOW,
		K_LBUTTON,
		K_MBUTTON,
		K_RBUTTON,
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

	char translate_key_value_to_asc_char_pair(KeyValue kv, bool upper);

	struct click_type
	{
		EventType type;
		ButtonState button_state;
		KeyState key_state;
		KeyValue key_value;
		int16_t location_x;
		int16_t location_y;
		bool is_up() const { return button_state == ButtonState::BS_UP; }
		bool is_down() const { return button_state == ButtonState::BS_DOWN; }
		bool is_left()const { return key_value == KeyValue::K_LBUTTON; }
		bool is_middle()const { return key_value == KeyValue::K_MBUTTON; }
		bool is_right() const { return key_value == KeyValue::K_RBUTTON; }
		int16_t get_x()const { return location_x; }
		int16_t get_y() const { return location_y; }
	};

	struct move_type
	{
		EventType type;
		int16_t location_x;
		int16_t location_y;
		int16_t get_x() const { return location_x; }
		int16_t get_y() const { return location_y; }
	};

	struct key_type
	{
		EventType type;
		ButtonState button_state;
		KeyValue value;
		char asc_key;
		//key_type(EventType et, ButtonState bs, KeyValue kv) : type(et), button_state(bs), value(kv), asc_key(kv) {}
		bool is_up() const { return button_state == ButtonState::BS_UP; }
		bool is_down() const { return button_state == ButtonState::BS_DOWN; }
		KeyValue get_value() const { return value; }
		char get_asc() const { return asc_key; }
	};

	union event_data
	{
		EventType type;
		click_type click;
		move_type move;
		key_type key;
	};

	/*
	template<typename T>
	struct key_event
	{
	key_type& ref;
	bool
	};
	*/

	union event
	{
		event(PO::EventType et) : type(et) {}
		event() {}
		event(const click_type& ct) : click(ct) {}
		event(const move_type& ct) : move(ct) {}
		event(const key_type& ct) : key(ct) {}

		EventType type;
		click_type click;
		move_type move;
		key_type key;


		bool is_quit() const { return type == EventType::E_CLOSE; }
		bool is_click() const { return type == EventType::E_CLICK; }
		bool is_move() const { return type == EventType::E_MOVE; }
		bool is_key() const { return type == EventType::E_KEY; }
	};

	enum class Respond
	{
		Truncation,
		Pass,
		Return
	};

}