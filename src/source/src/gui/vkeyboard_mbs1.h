/** @file vkeyboard_mbs1.h

	HITACHI MB-S1 Emulator 'EmuB-S1'
	Skelton for retropc emulator

	@author Sasaji
	@date   2016.12.21 -

	@brief [ virtual keyboard layout for mbs1 ]
*/

#ifndef VKEYBOARD_MBS1_H
#define VKEYBOARD_MBS1_H

#include "vkeyboard.h"

namespace Vkbd {

const struct stBitmap_t cBmpParts[] = {
	{ BITMAPIDS_LED_PARTS, 7 * 16, 0, 13, 6 },	// LED Red Horizontal
	{ BITMAPIDS_LED_PARTS, 9 * 16, 0, 13, 6 },	// LED Green Horizontal
	{ BITMAPIDS_LED_PARTS, 1 * 16, 0, 6, 13 },	// LED Red Vertical
	{ BITMAPIDS_VKEY_MODE,     0, 0, 16, 30 },	// MODE Switch
	{ BITMAPIDS_VKEY_BREAK,    0, 0, 49, 32 },	// BREAK Key
	{ BITMAPIDS_VKEY_POWER,    0, 0, 19, 43 },	// POWER Switch
	{ BITMAPIDS_VKEY_RESET,    0, 0, 16, 30 },	// RESET Switch
	{ -1, 0, 0, 0, 0 }
};

const struct stLedPos_t cLedPos[] = {
	{ BITMAPPARTS_LED_RH, 457, 263 },	// KATAKANA Led
	{ BITMAPPARTS_LED_GH, 457, 277 },	// HIRAGANA Led
	{ BITMAPPARTS_LED_RV,  18, 230 },	// CAPSLOCK Led
	{ BITMAPPARTS_MODE,   507,  11 },	// MODE Switch
	{ BITMAPPARTS_POWER,  622,  10 },	// POWER Switch
	{ -1, 0, 0 }
};

const Hori_t cvKeyHori0[] = {
	{455, 16, 0x7f, 0, -1, BITMAPPARTS_RESET},	// RESET
	{507, 16, 0x7d, KEYKIND_NOANIME, -1, -1},	// MODE
	{623, 18, 0x7e, KEYKIND_NOANIME, -1, -1},	// POWER
	{  0,  0,    0, 0, -1, -1}
};

const Hori_t cvKeyHori1[] = {
	{ 12, 49, 0x80, 0, -1, BITMAPPARTS_BREAK},	// BREAK
	{ 88, 49, 0x50, 0, -1, -1},	// PF1
	{138, 49, 0x51, 0, -1, -1},	// PF2
	{188, 49, 0x52, 0, -1, -1},	// PF3
	{238, 49, 0x53, 0, -1, -1},	// PF4
	{288, 49, 0x54, 0, -1, -1},	// PF5
	{358, 49, 0x68, 0, -1, -1},	// COPY
	{408, 49, 0x65, 0, -1, -1},	// INS
	{458, 49, 0x6f, 0, -1, -1},	// DEL
	{516, 32, 0x2e, 0, -1, -1},	// HOME/CLS
	{549, 32, 0x01, 0, -1, -1},	// UP
	{582, 32, 0x02, 0, -1, -1},	// num ?
	{615, 32, 0x45, 0, -1, -1},	// num /
	{  0,  0,    0, 0, -1, -1}
};

const Hori_t cvKeyHori2[] = {
	{516, 32, 0x03, 0, -1, -1},	// LEFT
	{549, 32, 0x04, 0, -1, -1},	// DOWN
	{582, 32, 0x05, 0, -1, -1},	// RIGHT
	{615, 32, 0x0f, 0, -1, -1},	// num *
	{  0,  0,    0, 0, -1, -1}
};

const Hori_t cvKeyHori3[] = {
	{ 13, 32, 0x0c, 0, -1, -1},	// ESC
	{ 46, 32, 0x1a, 0, -1, -1},	// 1
	{ 79, 32, 0x1b, 0, -1, -1},	// 2
	{112, 32, 0x17, 0, -1, -1},	// 3
	{145, 32, 0x11, 0, -1, -1},	// 4
	{178, 32, 0x19, 0, -1, -1},	// 5
	{211, 32, 0x12, 0, -1, -1},	// 6
	{244, 32, 0x10, 0, -1, -1},	// 7
	{277, 32, 0x13, 0, -1, -1},	// 8
	{310, 32, 0x1c, 0, -1, -1},	// 9
	{343, 32, 0x14, 0, -1, -1},	// 0
	{376, 32, 0x16, 0, -1, -1},	// -
	{409, 32, 0x15, 0, -1, -1},	// ^
	{442, 32, 0x1f, 0, -1, -1},	// YEN
	{475, 32, 0x1e, 0, -1, -1},	// BS
	{  0,  0,    0, 0, -1, -1}
};

const Hori_t cvKeyHori4[] = {
	{ 13, 49, 0x6c, 0, -1, -1},	// TAB
	{ 63, 32, 0x28, 0, -1, -1},	// Q 
	{ 96, 32, 0x2a, 0, -1, -1},	// W
	{129, 32, 0x2b, 0, -1, -1},	// E 
	{162, 32, 0x21, 0, -1, -1},	// R
	{195, 32, 0x29, 0, -1, -1},	// T
	{228, 32, 0x22, 0, -1, -1},	// Y 
	{261, 32, 0x20, 0, -1, -1},	// U
	{294, 32, 0x23, 0, -1, -1},	// I
	{327, 32, 0x2c, 0, -1, -1},	// O
	{360, 32, 0x24, 0, -1, -1},	// P
	{393, 32, 0x26, 0, -1, -1},	// @
	{426, 32, 0x25, 0, -1, -1},	// [
	{459,  8, 0x2f, KEYKIND_ARRAY, ARRAYKEYS_RETURN, -1},	// RETURN(left peninsula)
	{516, 32, 0x1d, 0, -1, -1},	// num 7
	{549, 32, 0x0d, 0, -1, -1},	// num 8
	{582, 32, 0x0e, 0, -1, -1},	// num 9
	{615, 32, 0x3f, 0, -1, -1},	// num -
	{  0,  0,    0, 0, -1, -1}
};

const Hori_t cvKeyHori5[] = {
	{467, 32, 0x2f, KEYKIND_ARRAY, ARRAYKEYS_RETURN, -1},	// RETURN(land)
	{  0,  0,    0, 0, -1, -1}
};

const Hori_t cvKeyHori6[] = {
	{  5, 32, 0x09, 0, -1, -1},	// CAPS
	{ 38, 32, 0x06, KEYKIND_TOGGLE, TOGGLEKEYS_CTRL, -1},	// CTRL
	{ 71, 32, 0x38, 0, -1, -1},	// A
	{104, 32, 0x3a, 0, -1, -1},	// S
	{137, 32, 0x3b, 0, -1, -1},	// D
	{170, 32, 0x31, 0, -1, -1},	// F
	{203, 32, 0x39, 0, -1, -1},	// G
	{236, 32, 0x32, 0, -1, -1},	// H
	{269, 32, 0x30, 0, -1, -1},	// J
	{302, 32, 0x33, 0, -1, -1},	// K
	{335, 32, 0x3c, 0, -1, -1},	// L
	{368, 32, 0x34, 0, -1, -1},	// ;
	{401, 32, 0x36, 0, -1, -1},	// :
	{434, 32, 0x35, 0, -1, -1},	// ]
	{516, 32, 0x37, 0, -1, -1},	// num 4
	{549, 32, 0x3d, 0, -1, -1},	// num 5
	{582, 32, 0x3e, 0, -1, -1},	// num 6
	{615, 32, 0x4f, 0, -1, -1},	// num +
	{  0,  0,    0, 0, -1, -1}
};

const Hori_t cvKeyHori7[] = {
	{ 37, 49, 0x07, KEYKIND_TOGGLE, TOGGLEKEYS_SHIFT, -1},	// LSHIFT 
	{ 87, 32, 0x48, 0, -1, -1},	// Z
	{120, 32, 0x4a, 0, -1, -1},	// X
	{153, 32, 0x4b, 0, -1, -1},	// C
	{186, 32, 0x41, 0, -1, -1},	// V
	{219, 32, 0x49, 0, -1, -1},	// B
	{252, 32, 0x42, 0, -1, -1},	// N
	{285, 32, 0x40, 0, -1, -1},	// M
	{318, 32, 0x43, 0, -1, -1},	// ,
	{351, 32, 0x4c, 0, -1, -1},	// .
	{384, 32, 0x44, 0, -1, -1},	// /
	{417, 32, 0x46, 0, -1, -1},	// _
	{450, 49, 0x07, KEYKIND_TOGGLE, TOGGLEKEYS_SHIFT, -1},	// RSHIFT
	{516, 32, 0x47, 0, -1, -1},	// num 1
	{549, 32, 0x4d, 0, -1, -1},	// num 2
	{582, 32, 0x4e, 0, -1, -1},	// num 3
	{  0,  0,    0, 0, -1, -1}
};

const Hori_t cvKeyHori8[] = {
	{615, 32, 0x2f, 0, -1, -1},	// num RETURN
	{  0,  0,    0, 0, -1, -1}
};

const Hori_t cvKeyHori9[] = {
	{ 87,  32, 0x0b, KEYKIND_TOGGLE, TOGGLEKEYS_GRAPH, -1},	// GRAPH 
	{120, 263, 0x00, 0, -1, -1},	// SPACE 
	{384,  32, 0x67, 0, -1, -1},	// HENKAN 
	{417,  32, 0x0a, 0, -1, -1},	// KATA/HIRA 
	{516,  32, 0x27, 0, -1, -1},	// num 0
	{549,  32, 0x6d, 0, -1, -1},	// num ,
	{582,  32, 0x2d, 0, -1, -1},	// num .
	{  0,   0,    0, 0, -1, -1}
};

const Pos_t cVkbdKeyPos[] = {
	{  11, 30, cvKeyHori0 },	// POWER/RESET/MODE
	{  77, 32, cvKeyHori1 },	// PF Key/BREAK
	{ 110, 32, cvKeyHori2 },	// numpad line 2
	{ 125, 32, cvKeyHori3 },	// line 1
	{ 158, 32, cvKeyHori4 },	// line 2
	{ 158, 65, cvKeyHori5 },	// RETURN(land)
	{ 191, 32, cvKeyHori6 },	// line 3
	{ 224, 32, cvKeyHori7 },	// line 4
	{ 224, 65, cvKeyHori8 },	// numpad RETURN
	{ 257, 32, cvKeyHori9 },	// line 5
	{ 0, 0, NULL }
};

} /* namespace Vkbd */

#endif /* VKEYBOARD_MBS1_H */
