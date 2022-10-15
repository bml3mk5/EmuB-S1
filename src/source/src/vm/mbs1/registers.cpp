/** @file registers.cpp

	HITACHI MB-S1 Emulator 'EmuB-S1'
	Skelton for retropc emulator

	@author Sasaji
	@date   2015.09.01 -

	@brief [ registers on circit ]
*/

#include "registers.h"

REGISTERS registers;

REGISTERS::REGISTERS()
{
	initialize();
}

void REGISTERS::initialize()
{
	reg_fuse = 0;
	reg_addrseg = 0;
	reg_trapf = 0;
	reg_busctrl = 0;
	reg_bmsk_color = 0;
	reg_active_plane = 0;
	bmsk_dw = 0;
	reg_disp_page = 0;
	reg_scrn_mode = 0;
	memset(reg_gcolor, 0, sizeof(reg_gcolor));
	memset(reg_palette, 0, sizeof(reg_palette));
	reg_mode_sel = 0;
	reg_remote = 0;
	reg_music_sel = 0;
	reg_time_mask = 0;
	reg_interlace_sel = 0;
	reg_baud_sel = 0;
	reg_igenable = 0;
	reg_igmode = 0;
	sig_mbc = 0;
}
