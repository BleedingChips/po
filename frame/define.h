#pragma once
#include <chrono>
#include <stdint.h>
#include "..\tool\tool.h"
#include "..\tool\thread_tool.h"

#ifdef _WIN32
#include <Windows.h>
namespace PO
{
	std::string utf16_to_asc(const std::u16string& data);

	std::u16string asc_to_utf16(const std::string& data);

}
#endif

inline wchar_t const* operator"" _wc
(
	std::conditional_t<sizeof(wchar_t) == sizeof(char16_t), char16_t const *, char32_t const *> ca,
	size_t l
)
{
	return reinterpret_cast<wchar_t const*>(ca);
}

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

	class event_touch
	{
	public:
	};







	enum class EventType : std::int8_t
	{
		E_CLICK,
		E_WHEEL,
		E_MOVE,
		E_KEY,
		E_CLOSE
	};

	enum class ButtonState : int8_t
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

	enum class KeyValue : int8_t
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
		bool is_up() { return button_state == ButtonState::BS_UP; }
		bool is_down() { return button_state == ButtonState::BS_DOWN; }
		bool is_left() { return key_value == KeyValue::K_LBUTTON; }
		bool is_middle() { return key_value == KeyValue::K_MBUTTON; }
		bool is_right() { return key_value == KeyValue::K_RBUTTON; }
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

	struct key_type
	{
		EventType type;
		ButtonState button_state;
		KeyValue value;
		char asc_key;
		//key_type(EventType et, ButtonState bs, KeyValue kv) : type(et), button_state(bs), value(kv), asc_key(kv) {}
		bool is_up() { return button_state == ButtonState::BS_UP; }
		bool is_down() { return button_state == ButtonState::BS_DOWN; }
		KeyValue get_value() { return value; }
		char get_asc() { return asc_key; }
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
		

		bool is_quit() { return type == EventType::E_CLOSE; }
		bool is_click() { return type == EventType::E_CLICK; }
		bool is_move() { return type == EventType::E_MOVE; }
		bool is_key() { return type == EventType::E_KEY; }
	};

	enum class Respond
	{
		Truncation,
		Pass,
		Return
	};

	class binary
	{
		std::shared_ptr<std::vector<char>> ptr;
	public:
		class weak_ref
		{
			std::weak_ptr<std::vector<char>> ptr;
			friend class binary;
		public:
			weak_ref() = default;
			weak_ref(const binary& b) : ptr(b.ptr) {}
			weak_ref(const weak_ref&) = default;
			weak_ref(weak_ref&&) = default;
			weak_ref& operator=(const weak_ref&) = default;
			weak_ref& operator=(weak_ref&&) = default;
			operator bool() const { return !ptr.expired(); }
			binary lock() const { binary tem; tem.ptr = ptr.lock(); return tem; }
			bool operator ==(const weak_ref& b) const { return ptr.owner_before(b.ptr); }
		};

		binary() {}
		binary(size_t s) { ptr = std::make_shared<std::vector<char>>(s, '\0'); }

		bool operator ==(const binary& b) const { return ptr == b.ptr; }

		binary(const binary& b) = default;
		binary(binary&& b) = default;
		binary& operator=(binary&&) = default;
		binary& operator=(const binary&) = default;
		void reset() { ptr.reset(); }
		uint64_t size() const
		{ 
			return ptr ? ptr->size() : 0;
		}
		operator char* () const 
		{
			if (ptr)
				return ptr->data();
			return nullptr; 
		}
		operator void* () const 
		{ 
			if (ptr)
				return reinterpret_cast<void*>(ptr->data());
			return nullptr; 
		}
		operator bool() const { return static_cast<bool>(ptr); }
	};
}