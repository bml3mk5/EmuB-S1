/** @file win_input.h

	Skelton for retropc emulator

	@author Takeda.Toshiya
	@date   2006.08.18 -

	@note
	Modified for BML3MK5/MBS1 by Sasaji at 2011.06.17

	@brief [ win32 input ]
*/

#ifndef _WIN_INPUT_H_
#define _WIN_INPUT_H_

#include "win_emu.h"
#include "../../vm/vm.h"
#include "../../keycode.h"

#ifdef USE_SHIFT_NUMPAD_KEY
static const int numpad_table[256] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x65, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//	0x00, 0x69, 0x63, 0x61, 0x67, 0x64, 0x68, 0x66, 0x62, 0x00, 0x00, 0x00, 0x00, 0x60, 0x6e, 0x00,
	0x00, 0x69, 0x63, 0x61, 0x67, 0x64, 0x68, 0x66, 0x62, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00,	// remove shift + comma
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
#endif

/// mapping from VK_* virtual key to original keycode
const uint16_t vkkey2keycode[256] = {
	/* 0x00 - 0x0f */
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	KEYCODE_BACKSPACE,
	KEYCODE_TAB,
	0,
	0,
	KEYCODE_CLEAR,
	KEYCODE_RETURN,
	0,
	0,
	/* 0x10 - 0x1f */
	0,			//	KEYCODE_SHIFT,
	0,			//	KEYCODE_CONTROL,
	0,			//	KEYCODE_ALT,
	KEYCODE_PAUSE,
	KEYCODE_CAPSLOCK,
	KEYCODE_KATAHIRA,
	0,
	0,
	0,
	KEYCODE_GRAVE,
	0,
	KEYCODE_ESCAPE,
	KEYCODE_HENKAN,
	KEYCODE_MUHENKAN,
	0,
	0,
	/* 0x20 - 0x2f */
	KEYCODE_SPACE,
	KEYCODE_PAGEUP,
	KEYCODE_PAGEDOWN,
	KEYCODE_END,
	KEYCODE_HOME,
	KEYCODE_LEFT,
	KEYCODE_UP,
	KEYCODE_RIGHT,
	KEYCODE_DOWN,
	KEYCODE_SELECT,
	KEYCODE_PRINT,
	0,
	0,
	KEYCODE_INSERT,
	KEYCODE_DELETE,
	KEYCODE_HELP,
	/* 0x30 - 0x3f */
	KEYCODE_0,
	KEYCODE_1,
	KEYCODE_2,
	KEYCODE_3,
	KEYCODE_4,
	KEYCODE_5,
	KEYCODE_6,
	KEYCODE_7,
	KEYCODE_8,
	KEYCODE_9,
	0,
	0,
	0,
	0,
	0,
	0,
	/* 0x40 - 0x4f */
	0,
	KEYCODE_A,
	KEYCODE_B,
	KEYCODE_C,
	KEYCODE_D,
	KEYCODE_E,
	KEYCODE_F,
	KEYCODE_G,
	KEYCODE_H,
	KEYCODE_I,
	KEYCODE_J,
	KEYCODE_K,
	KEYCODE_L,
	KEYCODE_M,
	KEYCODE_N,
	KEYCODE_O,
	/* 0x50 - 0x5f */
	KEYCODE_P,
	KEYCODE_Q,
	KEYCODE_R,
	KEYCODE_S,
	KEYCODE_T,
	KEYCODE_U,
	KEYCODE_V,
	KEYCODE_W,
	KEYCODE_X,
	KEYCODE_Y,
	KEYCODE_Z,
	KEYCODE_LSUPER,
	KEYCODE_RSUPER,
	KEYCODE_MENU,
	0,
	0, // KEYCODE_SLEEP
	/* 0x60 - 0x6f */
	KEYCODE_KP_0,
	KEYCODE_KP_1,
	KEYCODE_KP_2,
	KEYCODE_KP_3,
	KEYCODE_KP_4,
	KEYCODE_KP_5,
	KEYCODE_KP_6,
	KEYCODE_KP_7,
	KEYCODE_KP_8,
	KEYCODE_KP_9,
	KEYCODE_KP_MULTIPLY,
	KEYCODE_KP_PLUS,
	0,
	KEYCODE_KP_MINUS,
	KEYCODE_KP_PERIOD,
	KEYCODE_KP_DIVIDE,
	/* 0x70 - 0x7f */
	KEYCODE_F1,
	KEYCODE_F2,
	KEYCODE_F3,
	KEYCODE_F4,
	KEYCODE_F5,
	KEYCODE_F6,
	KEYCODE_F7,
	KEYCODE_F8,
	KEYCODE_F9,
	KEYCODE_F10, 
	KEYCODE_F11,
	KEYCODE_F12,
	KEYCODE_F13,
	KEYCODE_F14,
	KEYCODE_F15,
	KEYCODE_F16,
	/* 0x80 - 0x8f */
	KEYCODE_F17,
	KEYCODE_F18,
	KEYCODE_F19,
	KEYCODE_F20,
	KEYCODE_F21,
	KEYCODE_F22,
	KEYCODE_WORLD_0,
	KEYCODE_WORLD_1,
	KEYCODE_WORLD_2,
	KEYCODE_WORLD_3,
	KEYCODE_WORLD_4,
	KEYCODE_WORLD_5,
	KEYCODE_WORLD_6,
	KEYCODE_WORLD_7,
	KEYCODE_WORLD_8,
	KEYCODE_WORLD_9,
	/* 0x90 - 0x9f */
	KEYCODE_NUMLOCK,
	KEYCODE_SCROLLLOCK,
	KEYCODE_KP_EQUALS,
	KEYCODE_JISHO,
	KEYCODE_MASSHOU,
	KEYCODE_TOUROKU,
	KEYCODE_LOYAYUBI,
	KEYCODE_ROYAYUBI,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	/* 0xa0 - 0xaf */
	KEYCODE_LSHIFT,
	KEYCODE_RSHIFT,
	KEYCODE_LCTRL,
	KEYCODE_RCTRL,
	KEYCODE_LALT,
	KEYCODE_RALT,
	KEYCODE_BROWSER_BACK,
	KEYCODE_BROWSER_FORWARD,
	KEYCODE_BROWSER_REFRESH,
	KEYCODE_BROWSER_STOP,
	KEYCODE_BROWSER_SEARCH,
	KEYCODE_BROWSER_FAVOR, 
	KEYCODE_BROWSER_HOME, 
	KEYCODE_VOLUME_MUTE, 
	KEYCODE_VOLUME_DOWN, 
	KEYCODE_VOLUME_UP,
	/* 0xb0 - 0xbf */
	0,
	0,
	0,
	0, 
	0,
	0, 
	0, 
	0,
	0,
	0, 
	KEYCODE_COLON,
	KEYCODE_SEMICOLON,
	KEYCODE_COMMA,
	KEYCODE_MINUS, 
	KEYCODE_PERIOD, 
	KEYCODE_SLASH,
	/* 0xc0 - 0xcf */
	KEYCODE_AT,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0, 
	0,
	0,
	0, 
	0,
	0,
	0,
	0,
	/* 0xd0 - 0xdf */
	0,
	0,
	0, 
	0,
	0,
	0, 
	0, 
	0,
	0,
	0,
	0,
	KEYCODE_LBRACKET,
	KEYCODE_BACKSLASH,
	KEYCODE_RBRACKET,
	KEYCODE_CARET,
	0,
	/* 0xe0 - 0xef */
	0,
	0,
	KEYCODE_UNDERSCORE,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0, 
	0, 
	0,
	0,
	0,
	0,
	/* 0xf0 - 0xff */
	KEYCODE_CAPSLOCK | KEYCODE_KEEP_FRAMES,
	KEYCODE_MUHENKAN,
	KEYCODE_KATAHIRA | KEYCODE_KEEP_FRAMES,
	KEYCODE_GRAVE | KEYCODE_KEEP_FRAMES,
	KEYCODE_GRAVE | KEYCODE_KEEP_FRAMES,
	0, 
	0,
	0,
	0,
	0,
	0,
	0,
	0, 
	0, 
	0,
	0
};

static const uint8_t scancode2keycode47[16] = {
	/* 0x47 - 0x56 */
	KEYCODE_KP_7,
	KEYCODE_KP_8,
	KEYCODE_KP_9,
	0,
	KEYCODE_KP_4,
	KEYCODE_KP_5,
	KEYCODE_KP_6,
	0,
	KEYCODE_KP_1,
	KEYCODE_KP_2,
	KEYCODE_KP_3,
	KEYCODE_KP_0,
	KEYCODE_KP_PERIOD,
	0,
	0,
	0
};

static const uint8_t scancode2keycode70[16] = {
	/* 0x70 - 0x7f */
	KEYCODE_KATAHIRA,	// Katakana / Hiragana (JP)
	0,
	0,
	KEYCODE_UNDERSCORE,	// _
	0,
	0,
	0,
	0,
	0,
	KEYCODE_HENKAN,		// Henkan (JP)
	0,
	KEYCODE_MUHENKAN,	// Muhenkan (JP)
	0,
	KEYCODE_BACKSLASH,	// Yen
	0,
	0
};

#ifdef USE_DIRECTINPUT
/// mapping from dinput key to original keycode
/// MSB is set when keeping frame
const uint16_t dikey2keycode[256] = {
	/* 0x00 - 0x0f */
	0,
	KEYCODE_ESCAPE,
	KEYCODE_1,
	KEYCODE_2,
	KEYCODE_3,
	KEYCODE_4,
	KEYCODE_5,
	KEYCODE_6,
	KEYCODE_7,
	KEYCODE_8,
	KEYCODE_9,
	KEYCODE_0,
	KEYCODE_MINUS,
	KEYCODE_EQUALS,
	KEYCODE_BACKSPACE,
	KEYCODE_TAB,
	/* 0x10 - 0x1f */
	KEYCODE_Q,
	KEYCODE_W,
	KEYCODE_E,
	KEYCODE_R,
	KEYCODE_T,
	KEYCODE_Y,
	KEYCODE_U,
	KEYCODE_I,
	KEYCODE_O,
	KEYCODE_P,
	KEYCODE_LBRACKET,
	KEYCODE_RBRACKET,
	KEYCODE_RETURN,
	KEYCODE_LCTRL,
	KEYCODE_A,
	KEYCODE_S,
	/* 0x20 - 0x2f */
	KEYCODE_D,
	KEYCODE_F,
	KEYCODE_G,
	KEYCODE_H,
	KEYCODE_J,
	KEYCODE_K,
	KEYCODE_L,
	KEYCODE_SEMICOLON,
	KEYCODE_COLON,
	KEYCODE_GRAVE,
	KEYCODE_LSHIFT,
	KEYCODE_UNDERSCORE,
	KEYCODE_Z,
	KEYCODE_X,
	KEYCODE_C,
	KEYCODE_V,
	/* 0x30 - 0x3f */
	KEYCODE_B,
	KEYCODE_N,
	KEYCODE_M,
	KEYCODE_COMMA,
	KEYCODE_PERIOD,
	KEYCODE_SLASH,
	KEYCODE_RSHIFT,
	KEYCODE_KP_MULTIPLY,
	KEYCODE_LALT,
	KEYCODE_SPACE,
	0,				//	KEYCODE_CAPSLOCK | 0x8000,
	KEYCODE_F1,
	KEYCODE_F2,
	KEYCODE_F3,
	KEYCODE_F4,
	KEYCODE_F5,
	/* 0x40 - 0x4f */
	KEYCODE_F6,
	KEYCODE_F7,
	KEYCODE_F8,
	KEYCODE_F9,
	KEYCODE_F10,
	KEYCODE_NUMLOCK,
	KEYCODE_SCROLLLOCK,
	KEYCODE_KP_7,
	KEYCODE_KP_8,
	KEYCODE_KP_9,
	KEYCODE_KP_MINUS,
	KEYCODE_KP_4,
	KEYCODE_KP_5,
	KEYCODE_KP_6,
	KEYCODE_KP_PLUS,
	KEYCODE_KP_1,
	/* 0x50 - 0x5f */
	KEYCODE_KP_2,
	KEYCODE_KP_3,
	KEYCODE_KP_0,
	KEYCODE_KP_PERIOD,
	0,
	0,
	KEYCODE_BACKSLASH,
	KEYCODE_F11,
	KEYCODE_F12,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	/* 0x60 - 0x6f */
	0,
	0,
	0,
	0,
	KEYCODE_F13,
	KEYCODE_F14,
	KEYCODE_F15,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	/* 0x70 - 0x7f */
	0,			//	KEYCODE_KATAHIRA | 0x8000,
	0,
	0,
	KEYCODE_QUESTION,
	0,
	0,
	0,
	0,
	0,
	KEYCODE_HENKAN,
	0,
	KEYCODE_MUHENKAN,
	0,
	KEYCODE_BACKSLASH,
	KEYCODE_KP_PERIOD,
	0,
	/* 0x80 - 0x8f */
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	KEYCODE_KP_EQUALS,
	0,
	0,
	/* 0x90 - 0x9f */
	KEYCODE_CARET,
	KEYCODE_AT,
	KEYCODE_COLON,
	KEYCODE_UNDERSCORE,
	0,			//	KEYCODE_GRAVE,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	KEYCODE_KP_ENTER,
	KEYCODE_RCTRL,
	0,
	0,
	/* 0xa0 - 0xaf */
	KEYCODE_VOLUME_MUTE, 
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	KEYCODE_VOLUME_DOWN, 
	0,
	/* 0xb0 - 0xbf */
	KEYCODE_VOLUME_UP,
	0,
	0,
	KEYCODE_BROWSER_HOME,
	0,
	KEYCODE_KP_DIVIDE,
	0,
	KEYCODE_SYSREQ,
	KEYCODE_RALT,
	0,
	0,
	0,
	0,
	0,
	0,
	0, 
	/* 0xc0 - 0xcf */
	0, 
	0,
	0,
	0, 
	0, 
	KEYCODE_PAUSE,
	0,
	KEYCODE_HOME,
	KEYCODE_UP,
	KEYCODE_PAGEUP,
	0,
	KEYCODE_LEFT,
	0,
	KEYCODE_RIGHT,
	0,
	KEYCODE_END,
	/* 0xd0 - 0xdf */
	KEYCODE_DOWN,
	KEYCODE_PAGEDOWN,
	KEYCODE_INSERT,
	KEYCODE_DELETE,
	0,
	0,
	0,
	0,
	0, 
	0,
	0,
	KEYCODE_LSUPER, 
	KEYCODE_RSUPER,
	KEYCODE_POWER,
	0,				//	KEYCODE_SLEEP,
	0,				//	KEYCODE_WAKE,
	/* 0xe0 - 0xef */
	0,
	0,
	0, 
	0,
	0,
	KEYCODE_BROWSER_SEARCH, 
	KEYCODE_BROWSER_FAVOR, 
	KEYCODE_BROWSER_REFRESH,
	KEYCODE_BROWSER_STOP,
	KEYCODE_BROWSER_FORWARD,
	KEYCODE_BROWSER_BACK,
	0,
	0,
	0,
	0,
	0,
	/* 0xf0 - 0xff */
	0,
	0,
	0,
	0, 
	0,
	0,
	0,
	0,
	0,
	0,
	0, 
	0,
	0,
	0,
	0,
	0
};
#endif

#endif	// _WIN32_INPUT_H_
