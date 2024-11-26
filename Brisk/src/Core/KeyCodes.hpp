#pragma once

#include <string>
#include <iostream>

namespace Brisk
{
	typedef enum class KeyCode : uint16_t
	{
		// From glfw3.h
		Space = 32,
		Apostrophe = 39, /* ' */
		Comma = 44, /* , */
		Minus = 45, /* - */
		Period = 46, /* . */
		Slash = 47, /* / */

		D0 = 48, /* 0 */
		D1 = 49, /* 1 */
		D2 = 50, /* 2 */
		D3 = 51, /* 3 */
		D4 = 52, /* 4 */
		D5 = 53, /* 5 */
		D6 = 54, /* 6 */
		D7 = 55, /* 7 */
		D8 = 56, /* 8 */
		D9 = 57, /* 9 */

		Semicolon = 59, /* ; */
		Equal = 61, /* = */

		A = 65,
		B = 66,
		C = 67,
		D = 68,
		E = 69,
		F = 70,
		G = 71,
		H = 72,
		I = 73,
		J = 74,
		K = 75,
		L = 76,
		M = 77,
		N = 78,
		O = 79,
		P = 80,
		Q = 81,
		R = 82,
		S = 83,
		T = 84,
		U = 85,
		V = 86,
		W = 87,
		X = 88,
		Y = 89,
		Z = 90,

		LeftBracket = 91,  /* [ */
		Backslash = 92,  /* \ */
		RightBracket = 93,  /* ] */
		GraveAccent = 96,  /* ` */

		World1 = 161, /* non-US #1 */
		World2 = 162, /* non-US #2 */

		/* Function keys */
		Escape = 256,
		Enter = 257,
		Tab = 258,
		Backspace = 259,
		Insert = 260,
		Delete = 261,
		Right = 262,
		Left = 263,
		Down = 264,
		Up = 265,
		PageUp = 266,
		PageDown = 267,
		Home = 268,
		End = 269,
		CapsLock = 280,
		ScrollLock = 281,
		NumLock = 282,
		PrintScreen = 283,
		Pause = 284,
		F1 = 290,
		F2 = 291,
		F3 = 292,
		F4 = 293,
		F5 = 294,
		F6 = 295,
		F7 = 296,
		F8 = 297,
		F9 = 298,
		F10 = 299,
		F11 = 300,
		F12 = 301,
		F13 = 302,
		F14 = 303,
		F15 = 304,
		F16 = 305,
		F17 = 306,
		F18 = 307,
		F19 = 308,
		F20 = 309,
		F21 = 310,
		F22 = 311,
		F23 = 312,
		F24 = 313,
		F25 = 314,

		/* Keypad */
		KP0 = 320,
		KP1 = 321,
		KP2 = 322,
		KP3 = 323,
		KP4 = 324,
		KP5 = 325,
		KP6 = 326,
		KP7 = 327,
		KP8 = 328,
		KP9 = 329,
		KPDecimal = 330,
		KPDivide = 331,
		KPMultiply = 332,
		KPSubtract = 333,
		KPAdd = 334,
		KPEnter = 335,
		KPEqual = 336,

		LeftShift = 340,
		LeftControl = 341,
		LeftAlt = 342,
		LeftSuper = 343,
		RightShift = 344,
		RightControl = 345,
		RightAlt = 346,
		RightSuper = 347,
		Menu = 348
	} Key;

	inline std::ostream& operator<<(std::ostream& os, KeyCode keyCode)
	{
		os << static_cast<int32_t>(keyCode);
		return os;
	}
}

// From glfw3.h
#define Brisk_KEY_SPACE           ::Brisk::Key::Space
#define Brisk_KEY_APOSTROPHE      ::Brisk::Key::Apostrophe    /* ' */
#define Brisk_KEY_COMMA           ::Brisk::Key::Comma         /* , */
#define Brisk_KEY_MINUS           ::Brisk::Key::Minus         /* - */
#define Brisk_KEY_PERIOD          ::Brisk::Key::Period        /* . */
#define Brisk_KEY_SLASH           ::Brisk::Key::Slash         /* / */
#define Brisk_KEY_0               ::Brisk::Key::D0
#define Brisk_KEY_1               ::Brisk::Key::D1
#define Brisk_KEY_2               ::Brisk::Key::D2
#define Brisk_KEY_3               ::Brisk::Key::D3
#define Brisk_KEY_4               ::Brisk::Key::D4
#define Brisk_KEY_5               ::Brisk::Key::D5
#define Brisk_KEY_6               ::Brisk::Key::D6
#define Brisk_KEY_7               ::Brisk::Key::D7
#define Brisk_KEY_8               ::Brisk::Key::D8
#define Brisk_KEY_9               ::Brisk::Key::D9
#define Brisk_KEY_SEMICOLON       ::Brisk::Key::Semicolon     /* ; */
#define Brisk_KEY_EQUAL           ::Brisk::Key::Equal         /* = */
#define Brisk_KEY_A               ::Brisk::Key::A
#define Brisk_KEY_B               ::Brisk::Key::B
#define Brisk_KEY_C               ::Brisk::Key::C
#define Brisk_KEY_D               ::Brisk::Key::D
#define Brisk_KEY_E               ::Brisk::Key::E
#define Brisk_KEY_F               ::Brisk::Key::F
#define Brisk_KEY_G               ::Brisk::Key::G
#define Brisk_KEY_H               ::Brisk::Key::H
#define Brisk_KEY_I               ::Brisk::Key::I
#define Brisk_KEY_J               ::Brisk::Key::J
#define Brisk_KEY_K               ::Brisk::Key::K
#define Brisk_KEY_L               ::Brisk::Key::L
#define Brisk_KEY_M               ::Brisk::Key::M
#define Brisk_KEY_N               ::Brisk::Key::N
#define Brisk_KEY_O               ::Brisk::Key::O
#define Brisk_KEY_P               ::Brisk::Key::P
#define Brisk_KEY_Q               ::Brisk::Key::Q
#define Brisk_KEY_R               ::Brisk::Key::R
#define Brisk_KEY_S               ::Brisk::Key::S
#define Brisk_KEY_T               ::Brisk::Key::T
#define Brisk_KEY_U               ::Brisk::Key::U
#define Brisk_KEY_V               ::Brisk::Key::V
#define Brisk_KEY_W               ::Brisk::Key::W
#define Brisk_KEY_X               ::Brisk::Key::X
#define Brisk_KEY_Y               ::Brisk::Key::Y
#define Brisk_KEY_Z               ::Brisk::Key::Z
#define Brisk_KEY_LEFT_BRACKET    ::Brisk::Key::LeftBracket   /* [ */
#define Brisk_KEY_BACKSLASH       ::Brisk::Key::Backslash     /* \ */
#define Brisk_KEY_RIGHT_BRACKET   ::Brisk::Key::RightBracket  /* ] */
#define Brisk_KEY_GRAVE_ACCENT    ::Brisk::Key::GraveAccent   /* ` */
#define Brisk_KEY_WORLD_1         ::Brisk::Key::World1        /* non-US #1 */
#define Brisk_KEY_WORLD_2         ::Brisk::Key::World2        /* non-US #2 */

/* Function keys */
#define Brisk_KEY_ESCAPE          ::Brisk::Key::Escape
#define Brisk_KEY_ENTER           ::Brisk::Key::Enter
#define Brisk_KEY_TAB             ::Brisk::Key::Tab
#define Brisk_KEY_BACKSPACE       ::Brisk::Key::Backspace
#define Brisk_KEY_INSERT          ::Brisk::Key::Insert
#define Brisk_KEY_DELETE          ::Brisk::Key::Delete
#define Brisk_KEY_RIGHT           ::Brisk::Key::Right
#define Brisk_KEY_LEFT            ::Brisk::Key::Left
#define Brisk_KEY_DOWN            ::Brisk::Key::Down
#define Brisk_KEY_UP              ::Brisk::Key::Up
#define Brisk_KEY_PAGE_UP         ::Brisk::Key::PageUp
#define Brisk_KEY_PAGE_DOWN       ::Brisk::Key::PageDown
#define Brisk_KEY_HOME            ::Brisk::Key::Home
#define Brisk_KEY_END             ::Brisk::Key::End
#define Brisk_KEY_CAPS_LOCK       ::Brisk::Key::CapsLock
#define Brisk_KEY_SCROLL_LOCK     ::Brisk::Key::ScrollLock
#define Brisk_KEY_NUM_LOCK        ::Brisk::Key::NumLock
#define Brisk_KEY_PRINT_SCREEN    ::Brisk::Key::PrintScreen
#define Brisk_KEY_PAUSE           ::Brisk::Key::Pause
#define Brisk_KEY_F1              ::Brisk::Key::F1
#define Brisk_KEY_F2              ::Brisk::Key::F2
#define Brisk_KEY_F3              ::Brisk::Key::F3
#define Brisk_KEY_F4              ::Brisk::Key::F4
#define Brisk_KEY_F5              ::Brisk::Key::F5
#define Brisk_KEY_F6              ::Brisk::Key::F6
#define Brisk_KEY_F7              ::Brisk::Key::F7
#define Brisk_KEY_F8              ::Brisk::Key::F8
#define Brisk_KEY_F9              ::Brisk::Key::F9
#define Brisk_KEY_F10             ::Brisk::Key::F10
#define Brisk_KEY_F11             ::Brisk::Key::F11
#define Brisk_KEY_F12             ::Brisk::Key::F12
#define Brisk_KEY_F13             ::Brisk::Key::F13
#define Brisk_KEY_F14             ::Brisk::Key::F14
#define Brisk_KEY_F15             ::Brisk::Key::F15
#define Brisk_KEY_F16             ::Brisk::Key::F16
#define Brisk_KEY_F17             ::Brisk::Key::F17
#define Brisk_KEY_F18             ::Brisk::Key::F18
#define Brisk_KEY_F19             ::Brisk::Key::F19
#define Brisk_KEY_F20             ::Brisk::Key::F20
#define Brisk_KEY_F21             ::Brisk::Key::F21
#define Brisk_KEY_F22             ::Brisk::Key::F22
#define Brisk_KEY_F23             ::Brisk::Key::F23
#define Brisk_KEY_F24             ::Brisk::Key::F24
#define Brisk_KEY_F25             ::Brisk::Key::F25

/* Keypad */
#define Brisk_KEY_KP_0            ::Brisk::Key::KP0
#define Brisk_KEY_KP_1            ::Brisk::Key::KP1
#define Brisk_KEY_KP_2            ::Brisk::Key::KP2
#define Brisk_KEY_KP_3            ::Brisk::Key::KP3
#define Brisk_KEY_KP_4            ::Brisk::Key::KP4
#define Brisk_KEY_KP_5            ::Brisk::Key::KP5
#define Brisk_KEY_KP_6            ::Brisk::Key::KP6
#define Brisk_KEY_KP_7            ::Brisk::Key::KP7
#define Brisk_KEY_KP_8            ::Brisk::Key::KP8
#define Brisk_KEY_KP_9            ::Brisk::Key::KP9
#define Brisk_KEY_KP_DECIMAL      ::Brisk::Key::KPDecimal
#define Brisk_KEY_KP_DIVIDE       ::Brisk::Key::KPDivide
#define Brisk_KEY_KP_MULTIPLY     ::Brisk::Key::KPMultiply
#define Brisk_KEY_KP_SUBTRACT     ::Brisk::Key::KPSubtract
#define Brisk_KEY_KP_ADD          ::Brisk::Key::KPAdd
#define Brisk_KEY_KP_ENTER        ::Brisk::Key::KPEnter
#define Brisk_KEY_KP_EQUAL        ::Brisk::Key::KPEqual

#define Brisk_KEY_LEFT_SHIFT      ::Brisk::Key::LeftShift
#define Brisk_KEY_LEFT_CONTROL    ::Brisk::Key::LeftControl
#define Brisk_KEY_LEFT_ALT        ::Brisk::Key::LeftAlt
#define Brisk_KEY_LEFT_SUPER      ::Brisk::Key::LeftSuper
#define Brisk_KEY_RIGHT_SHIFT     ::Brisk::Key::RightShift
#define Brisk_KEY_RIGHT_CONTROL   ::Brisk::Key::RightControl
#define Brisk_KEY_RIGHT_ALT       ::Brisk::Key::RightAlt
#define Brisk_KEY_RIGHT_SUPER     ::Brisk::Key::RightSuper
#define Brisk_KEY_MENU            ::Brisk::Key::Menu