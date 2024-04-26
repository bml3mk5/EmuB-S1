/** @file display.cpp

	HITACHI MB-S1 Emulator 'EmuB-S1'
	Skelton for retropc emulator

	@author Sasaji
	@date   2011.06.08 -

	@brief [ display ]
*/

#include "display.h"
#include <stdlib.h>
#include <time.h>
#include "../../emu.h"
#include "../hd46505.h"
#include "../../fileio.h"
#include "../../config.h"
#include "../../utility.h"

#define CURSOR_MAXPOS 0xffffff

#ifdef _DEBUG
//#define DEBUG_CRTMON
//#define DEBUG_CRTMON2
#endif

#define COLOR16A(bb,rr,gg,ii) color16[(bb) | ((rr) << 1) | ((gg) << 2) | ((ii) << 3)]
#define COLOR64A(bb0,rr0,gg0,ii0,bb1,rr1,gg1,ii1) color64[(bb0) | ((rr0) << 1) | ((gg0) << 2) | ((ii0) << 3)][(bb1) | ((rr1) << 1) | ((gg1) << 2) | ((ii1) << 3)]

#define DECODE_TO_PALETTE16_NUM(code, outc) { \
	outc.b.b = (code & 1) ? 1 : 0; \
	outc.b.r = (code & 2) ? 1 : 0; \
	outc.b.g = (code & 4) ? 1 : 0; \
	outc.b.i = (code & 8) ? 1 : 0; \
}

#define DECODE_TO_PALETTE64_NUM(code, outc) { \
	outc.b.b = (code & 1) ? ((code & 8) ? 3 : 2) : 0; \
	outc.b.r = (code & 2) ? ((code & 8) ? 3 : 2) : 0; \
	outc.b.g = (code & 4) ? ((code & 8) ? 3 : 2) : 0; \
}

#define EXPAND_BRG_1BIT(code, in8, outc) { \
	if (code & 1) { outc.b.b |= in8; } \
	if (code & 2) { outc.b.r |= in8; } \
	if (code & 4) { outc.b.g |= in8; } \
	if (code & 8) { outc.b.i |= in8; } \
}

#define EXPAND_BRG_2BIT(code, in8, outc) { \
	if (code & 1) { outc[0].b.b |= in8; } \
	if (code & 2) { outc[0].b.r |= in8; } \
	if (code & 4) { outc[0].b.g |= in8; } \
	if (code & 8) { outc[0].b.i |= in8; } \
	outc[1].i = outc[0].i; \
}

#define EXPAND_BRG_2BIT_I(code, in8, outc) { \
	wk1 = in8; wk2 = (code & 8); \
	if (code & 1) { outc[0].b.b |= wk1; outc[1].b.b |= (wk2 ? wk1 : 0); } \
	if (code & 2) { outc[0].b.r |= wk1; outc[1].b.r |= (wk2 ? wk1 : 0); } \
	if (code & 4) { outc[0].b.g |= wk1; outc[1].b.g |= (wk2 ? wk1 : 0); } \
}

#define INIT_BRG_1(data,out) \
	out.i = data;

#define INIT_BRG_2(data,out) \
	out[0].i = out[1].i = data;

void DISPLAY::initialize()
{
	memset(font, 0, sizeof(font));
	font_rom_loaded = 0;
	l3font_rom_loaded = 0;
	font_rom_loaded_at_first = false;

	vm_pause = emu->get_pause_ptr();

	// load rom images
	load_font_rom_file();

	emu->get_rgbaformat(&Rmask, &Gmask, &Bmask, &Amask, &Rshift, &Gshift, &Bshift, &Ashift);

	// create pc palette
	for(int i = 0; i < 2; i++) {
		for(int g = 0; g < 2; g++) {
			for(int r = 0; r < 2; r++) {
				for(int b = 0; b < 2; b++) {
					int bb = b * (i ? 0xff : 0xaf);
					int rr = r * (i ? 0xff : 0xaf);
					int gg = g * (i ? 0xff : 0xaf);
					COLOR16A(b, r, g, i) = emu->map_rgbcolor(rr,gg,bb);
				}
			}
		}
	}
	const uint8_t mm[16] = {
		0x00, 0x00, 0x00, 0x00,
		0x1f, 0x3f, 0x1f, 0x3f,
		0x5f, 0x5f, 0x7f, 0x7f,
		0x9f, 0xbf, 0xdf, 0xff
	};
	for(int i = 0; i < 4; i++) {
		for(int g = 0; g < 4; g++) {
			for(int r = 0; r < 4; r++) {
				for(int b = 0; b < 4; b++) {
					int bb = mm[b * 4 + i];
					int rr = mm[r * 4 + i];
					int gg = mm[g * 4 + i];
					if (bb < 0) bb = 0;
					if (rr < 0) rr = 0;
					if (gg < 0) gg = 0;
					if (bb > 0xff) bb = 0xff;
					if (rr > 0xff) rr = 0xff;
					if (gg > 0xff) gg = 0xff;
					COLOR64A(b >> 1, r >> 1, g >> 1, i >> 1, b & 1, r & 1, g & 1, i & 1) = emu->map_rgbcolor(rr,gg,bb);
#ifdef DEBUG_CRTMON
					logging->out_debugf(_T("palette_64[%02d][%02d] b:%d r:%d g:%d i:%d :%06x")
						,(b >> 1) | ((r >> 1) << 1) | ((g >> 1) << 2) | ((i >> 1) << 3)
						,(b & 1) | ((r & 1) << 1) | ((g & 1) << 2) | ((i & 1) << 3)
						,b,r,g,i,
						COLOR64A(b >> 1, r >> 1, g >> 1, i >> 1, b & 1, r & 1, g & 1, i & 1));
#endif
				}
			}
		}
	}

	screen_left = 0;
	screen_top = 0;
	screen_right = SCREEN_WIDTH + screen_left;
	screen_bottom = SCREEN_HEIGHT + screen_top;
	scrnline1_offset = 0;

	mode_sel = 0;
	ig_mask = true;
	disable_ig = 0xff;

	cursor_addr = 0;

	update_chr_clocks(0);

	bg_color = 0;
	crt_mon_bg_color = 0;
	dws_b_p.i = 0;

	hireso = 0;
	width_sel = 0;
	page_no = 0;
	gwidth_sel = 0;
	gpage_no = 0;

	// synchronizable vertical range
	v_total_min[0] = (262 - 12);
	v_total_max[0] = (262 + 18);
	v_total_min[1] = (486 - 12);
	v_total_max[1] = (486 + 18);
	v_total_min[2] = (518 - 12);
	v_total_max[2] = (518 + 18);

	for(int i=0; i<VRAM_BUF_HEIGHT; i++) {
		crt_mon_vline[i].left_col = 0;
#ifdef USE_LIGHTPEN
		crt_mon_vline[i].right_col = 0;
#endif
		crt_mon_vline[i].flags = 0;
	}
	for(int i=0; i<SCREEN_HEIGHT; i++) {
		crt_mon_vinfo[i].row = 0;
		crt_mon_vinfo[i].raster = 0;
		crt_mon_vinfo[i].cursor_st_col = 255;
		crt_mon_vinfo[i].cursor_ed_col = 0;
	}
	dws_left_diff = 0;
	// synchronizable horizontal range
	for(int i=0; i<4; i++) {
		h_total_min[i] = CHARS_PER_LINE * (i+1) - 3;
		h_total_max[i] = CHARS_PER_LINE * (i+1) + 2;
	}

	sw = 0;

	//
	memset(crt_mon, 0, sizeof(crt_mon));
	crt_mon_col = 0;
	crt_mon_row = 0;
	crt_mon_v_count = 0;
	crt_mon_v_start = 0;
	crt_mon_l3vaddr = L3_ADDR_VRAM_START;
	crt_mon_s1vtaddr = 0;
	crt_mon_s1vgaddr = 0;
#ifdef USE_KEEPIMAGE
	crt_mon_page = 0;
#endif
#ifdef USE_LIGHTPEN
	crt_mon_disptmg_top = 0;
	crt_mon_disptmg_bottom = 0;
#endif

	disp_page_brg_mask.i = 0;
	disp_page_changed.i = 0;

	skip_frame_count = 0;

	// CRTC 2MHz default
	update_chr_clocks(1);

	for(int i=0; i<8; i++) {
		ppd[i]=(crtc_ppch == 8) ? 1 : 2;	// pixel per 1dot
		dof[i]=(0x80 >> i);
	}

	update_config();
}

/// power on reset
///
void DISPLAY::reset()
{
	load_font_rom_file();

	REG_INTERLACE_SEL = 0;
	REG_IGENABLE = 0;
	ig_mask = true;
	disable_ig = (IOPORT_USE_DISIG ? 0xdf : 0xff);

	now_cpu_halt = 0;

	//
	memset(crt_mon, 0, sizeof(crt_mon));

	// set bg color
	srand((unsigned int)time(NULL));
	int md = (rand() % 16);
	REG_MODE_SEL |= (md == 1 || md == 3 || md == 7 ? md + 8 : 0); 
	write_io8(0xffd0, REG_MODE_SEL);

	change_palette();
}

void DISPLAY::update_config()
{
	scanline = pConfig->scan_line;
	afterimage = pConfig->afterimage;
#ifdef USE_KEEPIMAGE
	keepimage = pConfig->keepimage;
#endif
	buf_stepcols = (scanline <= 2 ? 1 : 2);
	crt_mon_stepcols = (scanline <= 2 ? 1 : 2);

	// scanline
//	dws_ys = (scanline ? 2 : 1);	// skip step
	dws_ys = (scanline > 1 ? 2 : 1);	// skip step

	// color filter for afterimage
	memset(scrnflag,  0x00, sizeof(scrnflag));
	if (afterimage == 0) {
		memset(scrnline1, 0x00, sizeof(scrntype) * SCREEN_WIDTH);
		scrnline1_offset = 0;
	} else if (afterimage == 1) {
		scrnline1_offset = SCREEN_WIDTH;

		Rloss[0] = (uint32_t)pConfig->rloss[0] * (scanline <= 1 ? 1 : scanline);
		Rloss[1] = (uint32_t)pConfig->rloss[1] * (scanline <= 1 ? 1 : scanline);
		Gloss[0] = (uint32_t)pConfig->gloss[0] * (scanline <= 1 ? 1 : scanline);
		Gloss[1] = (uint32_t)pConfig->gloss[1] * (scanline <= 1 ? 1 : scanline);
		Bloss[0] = (uint32_t)pConfig->bloss[0] * (scanline <= 1 ? 1 : scanline);
		Bloss[1] = (uint32_t)pConfig->bloss[1] * (scanline <= 1 ? 1 : scanline);
		Rboad = (128 << Rshift);
		Gboad = (128 << Gshift);
		Bboad = (128 << Bshift);
		for(int i=0; i<2; i++) {
			if (Rloss[i] > 255) Rloss[i] = 255;
			if (Gloss[i] > 255) Gloss[i] = 255;
			if (Bloss[i] > 255) Bloss[i] = 255;
			Rloss[i] <<= Rshift;
			Gloss[i] <<= Gshift;
			Bloss[i] <<= Bshift;
		}
	} else if (afterimage == 2) {
		scrnline1_offset = SCREEN_WIDTH;
	}

#ifdef USE_KEEPIMAGE
	if (keepimage > 0) {
		memset(scrntop, 0, sizeof(scrntop));
	}
#endif

	update_dws_params();
}

void DISPLAY::update_dws_params()
{
//	crt_mon_disptmg_left = (*disptmg_skew) >= 4 ? VRAM_BUF_WIDTH
//		: crtc_regs[0]-crtc_regs[2]-(crtc_regs[3] & 0x0f)-(crtc_chr_clocks ? 14 : 5) + (*disptmg_skew & 3) - pConfig->disptmg_skew;

//	crt_mon_top = crtc_regs[4] + crtc_regs[5] - ((crtc_regs[3] & 0xf0) >> 4) - crtc_regs[7];

	crt_mon_crtc_hsync = ((h_total_min[0] <= crtc_regs[0] && crtc_regs[0] <= h_total_max[0])
	 || (h_total_min[1] <= crtc_regs[0] && crtc_regs[0] <= h_total_max[1])
	 || (h_total_min[2] <= crtc_regs[0] && crtc_regs[0] <= h_total_max[2])
	 || (h_total_min[3] <= crtc_regs[0] && crtc_regs[0] <= h_total_max[3]));

	crtc_vs_mid = (*crtc_vs_start) + 2;

#ifdef DEBUG_CRTMON
	logging->out_debugf(_T("DISPLAY:update_dws_params: clk:%d r0:%d r1:%d r2:%d r3:%d left_col:% 4d")
		,crtc_chr_clocks
		,crtc_regs[0],crtc_regs[1],crtc_regs[2],(crtc_regs[3] & 0xf)
		,crt_mon_disptmg_left
	);
#endif
}

void DISPLAY::update_chr_clocks(int clk)
{
	crtc_chr_clocks = (uint8_t)clk;
	crtc_ppch = (crtc_chr_clocks ? 8 : 16);
}

void DISPLAY::load_font_rom_file()
{
	const _TCHAR *app_path, *rom_path[2];

	rom_path[0] = pConfig->rom_path.Get();
	rom_path[1] = emu->application_path();

	for(int i=0; i<2; i++) {
		app_path = rom_path[i];

		if (!font_rom_loaded && !l3font_rom_loaded) {
			l3font_rom_loaded = emu->load_data_from_file(app_path, _T("FONT.ROM"), font, 0x1000, (const uint8_t *)"\x00\x00\x00\x00", 4);
		}
		if (!font_rom_loaded && !l3font_rom_loaded) {
			font_rom_loaded = emu->load_data_from_file(app_path, _T("S1FONT.ROM"), font, 0x2000, (const uint8_t *)"\x00\x00\x00\x00", 4);
		}
		if (font_rom_loaded || l3font_rom_loaded) break;
	}

	if (!l3font_rom_loaded) {
		if (!font_rom_loaded_at_first) {
			logging->out_logf_x(LOG_WARN, CMsg::VSTR_couldn_t_be_loaded, _T("FONT.ROM"));
		}
		if (!font_rom_loaded) {
			if (!font_rom_loaded_at_first) {
				logging->out_logf_x(LOG_WARN, CMsg::VSTR_couldn_t_be_loaded, _T("S1FONT.ROM"));
			}
			memset(chrfont, 0x10, sizeof(chrfont));
		} else {
			// convert font data to useful array data
			set_font_data();
			conv_to_l3font_data();
			font_rom_loaded = 0;
			l3font_rom_loaded = 0;
		}
	} else {
		// convert font data to useful array data
		set_l3font_data();
	}
	font_rom_loaded_at_first = true;
}

void DISPLAY::write_io8(uint32_t addr, uint32_t data)
{
	addr &= 0xffff;
	switch(addr) {
	case 0xfe20:
		// bmsk color graphic color
		REG_BMSK_COLOR = (data & 0x87);
		SIG_BMSK_DW = (BMSK_COLOR_BMSK ? (SIG_BMSK_DW | 2) : (SIG_BMSK_DW & ~2));
		break;
	case 0xfe21:
		// active graphic plane
		REG_ACTIVE_PLANE = (data & 0x0f);
		SIG_BMSK_DW = (ACTIVE_PLANE_DW ? (SIG_BMSK_DW | 1) : (SIG_BMSK_DW & ~1));
		break;
	case 0xfe23:
		// display page number
		if (DISP_PAGE_GRAPHIC_BRG == 0) {
			// latch timing is late when disp turn on
			disp_page_changed.b.b = (data & DISP_PAGE_B_MASK ? 0xff : 0);
			disp_page_changed.b.r = (data & DISP_PAGE_R_MASK ? 0xff : 0);
			disp_page_changed.b.g = (data & DISP_PAGE_G_MASK ? 0xff : 0);
			disp_page_changed.b.i = 0xff;
		}

		REG_DISP_PAGE = (data & 0xff);

		set_disp_page();
		break;
	case 0xfe24:
		// scrn mode
		REG_SCRN_MODE = (data & 0xff);

		if (NOW_S1_MODE) {
			set_s1_screen_mode();
		}
		break;
	case 0xfe25:
	case 0xfe26:
	case 0xfe27:
		// graphic color
		REG_GCOLOR[addr - 0xfe25] = (data & 0xff);
		break;
	case 0xfe28:
	case 0xfe29:
	case 0xfe2a:
	case 0xfe2b:
	case 0xfe2c:
	case 0xfe2d:
	case 0xfe2e:
	case 0xfe2f:
		// palette
		REG_PALETTE[addr - 0xfe28] = (data & 0x0f);

//		logging->out_debugf("dispw a:%04x d:%02x",addr,data);
		break;
	case 0xffd0:
		// mode sel
		mode_sel = REG_MODE_SEL;

		bg_color = mode_sel & 0x0f;				// bg color

		if (NOW_L3_MODE) {
			set_l3_screen_mode();
		}
		d_crtc->write_signal(HD46505::SIG_HD46505_CHR_CLOCKS, CLOCKS_1MHZ * (NOW_S1_MODE || (mode_sel & 0x80) ? 2 : 1), 0xffffffff);
		write_signal(SIG_DISPLAY_CHR_CLOCKS, CLOCKS_1MHZ * (NOW_S1_MODE || (mode_sel & 0x80) ? 2 : 1), 0xffffffff);

		break;
	case 0xffd6:
		// interlace sel
		REG_INTERLACE_SEL = ((data & 0x08) >> 3);	// 0:noninterlace char  1:interlace char
		break;
	case 0xffe9:
		// ig mode reg
		REG_IGENABLE = (data & 0x01);
		ig_mask &= (((data & 0x100) == 0) | (IOPORT_USE_DISIG != 0));
		break;
	}
}

uint32_t DISPLAY::read_io8(uint32_t addr)
{
	uint32_t data = 0xff;
	switch(addr & 0xffff) {
	case 0xfe28:
	case 0xfe29:
	case 0xfe2a:
	case 0xfe2b:
	case 0xfe2c:
	case 0xfe2d:
	case 0xfe2e:
	case 0xfe2f:
		// palette cannot be read
		// when hsync and vsync is off, send HALT signal 
		if (!HVSYNC_ON && !now_cpu_halt) {
			d_board->write_signal(SIG_CPU_HALT, SIG_HALT_PALETTE_MASK, SIG_HALT_PALETTE_MASK);
			now_cpu_halt = 1;
//			logging->out_debugf(_T("palette halt fired: %d"),data);
		}
		break;
	}
	return data;
}

void DISPLAY::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
#ifdef USE_LIGHTPEN
		case SIG_DISPLAY_LPSTB:
			{
			int width = crtc_regs[1];
			int dotx = (data >> 16) & 0xffff;
			int doty = (data & 0xffff);
			int x = dotx;
			int y = doty;
			int offset = (width_sel ? 5 : 4);
			uint32_t newaddr;

			x /= crtc_ppch;
			y /= 2;

			if (y < crt_mon_disptmg_top) {
				y = crt_mon_disptmg_top;
			}
			if (y > crt_mon_disptmg_bottom) {
				y = crt_mon_disptmg_bottom;
			}
			t_crt_mon_vline *vline = &crt_mon_vline[y];
			if (x < vline->left_col) {
				x = vline->right_col;
				y--;
			}
			if (x > vline->right_col) { 
				x = vline->left_col;
				y++;
			}

			x = (x - vline->left_col);
			y = (y - crt_mon_disptmg_top) / 8;

			newaddr = ((crtc_regs[12] << 8)+crtc_regs[13]) + y * width + x + offset;
			crtc_regs[16] = (newaddr >> 8) & 0x3f;
			crtc_regs[17] = newaddr & 0xff;

#ifdef DEBUG_CRTMON
			logging->out_debugf("lp dot(%03d,%03d) t:%03d b:%03d ch(%02d,%02d) start:%02x%02x ma:%04x pos:%02x%02x"
				,dotx,doty
				,crt_mon_disptmg_top,crt_mon_disptmg_bottom
				,x,y
				,crtc_regs[12],crtc_regs[13]
				,*crtc_ma
				,crtc_regs[16],crtc_regs[17]);
#endif
			}
			break;
#endif
		case SIG_DISPLAY_DISPTMG:
			// display area
			REG_IGMODE = ((data & mask) ? (REG_IGMODE | DISPTMG_MASK) : (REG_IGMODE & ~DISPTMG_MASK)) | REG_IGMODE_UNUSE;
//			logging->out_debugf(_T("disptmg %06lld IGM:%02x  %d:%d"), current_clock() & 0xffffff, REG_IGMODE, data, mask);
			break;
		case SIG_DISPLAY_HSYNC:
			// hsync
			REG_IGMODE = ((data & mask) ? (REG_IGMODE | HSYNC_MASK) : (REG_IGMODE & ~HSYNC_MASK)) | REG_IGMODE_UNUSE;
			// release halt signal
			if (now_cpu_halt) {
				now_cpu_halt = 0;
				d_board->write_signal(SIG_CPU_HALT, 0, SIG_HALT_PALETTE_MASK);
//				logging->out_debugf(_T("hsync halt released: %d %d"),data,mask);
			}
			// TODO: PAE flag is different from real machine
			REG_IGMODE = (HVSYNC_ON ? REG_IGMODE | PAE_MASK : REG_IGMODE & ~PAE_MASK) | REG_IGMODE_UNUSE;

//			logging->out_debugf(_T("hsync   %06lld IGM:%02x  %d:%d"), current_clock() & 0xffffff, REG_IGMODE, data, mask);
			break;
		case SIG_DISPLAY_VSYNC:
			if ((data & mask) == 0) {
				REG_IGMODE = (REG_IGMODE & ~VSYNC_MASK) | REG_IGMODE_UNUSE;

#ifdef DEBUG_CRTMON
				logging->out_debugf(_T("DISPLAY:SIG_DISPLAY_VSYNC_OFF: crtc:%d nras:%d cmr:%d cmc:%d cmras:%d top:%d btm:%d vtop:%d vbtm:%d")
					,(*crtc_vt_count), crt_mon_now_raster
					,crt_mon_row, crt_mon_col, (*crtc_ra)
					,top_raster, bottom_raster, vsync_top_raster, vsync_bottom_raster
				);
#endif
			} else {
				REG_IGMODE = (REG_IGMODE | VSYNC_MASK) | REG_IGMODE_UNUSE;

				// synchronize display vertical raster
				for(int i=0; i<3; i++) {
					if (v_total_min[i] <= crt_mon_v_count && crt_mon_v_count <= v_total_max[i]) {
						crt_mon_v_start = 0;
						crt_mon_v_count = 0;
						crt_mon_row = CRT_MON_HEIGHT - 2;
						break;
					}
				}

				// clear changed flag
				disp_page_changed.i = 0;

			}
			// release halt signal
			if (now_cpu_halt) {
				now_cpu_halt = 0;
				d_board->write_signal(SIG_CPU_HALT, 0, SIG_HALT_PALETTE_MASK);
//				logging->out_debugf(_T("vsync halt released: %d %d"),data,mask);
			}
			// TODO: PAE flag is different from real machine
			REG_IGMODE = (HVSYNC_ON ? REG_IGMODE | PAE_MASK : REG_IGMODE & ~PAE_MASK) | REG_IGMODE_UNUSE;

//			logging->out_debugf(_T("vsync   %06lld IGM:%02x  %d:%d ras:%d"), current_clock() & 0xffffff, REG_IGMODE, data, mask, now_raster);
			break;
		case SIG_DISPLAY_CHR_CLOCKS:
			update_chr_clocks(data == CLOCKS_1MHZ ? 0 : 1);
			update_dws_params();
			break;
		case SIG_DISPLAY_WRITE_REGS:
			update_dws_params();
			break;
		case SIG_CPU_RESET:
			now_reset = (data & mask) ? true : false;
			if (now_reset) {
				if (NOW_S1_MODE) {
					set_s1_screen_mode();
				} else {
					set_l3_screen_mode();
				}
				change_palette();
			}
			break;
	}
}

void DISPLAY::set_display_size(int left, int top, int right, int bottom)
{
	screen_left = left;
	screen_right = right;
	screen_top = top;
	screen_bottom = bottom;
}

void DISPLAY::draw_screen()
{
	if ((afterimage == 0 && skip_frame_count >= 1)
	||  (afterimage > 0 && skip_frame_count >= 2)) {
		// clear cursor
#ifdef DEBUG_CRTMON2
		logging->out_debugf(_T("draw_screen:skip sw:%d skip_frame_count:%d afterimage:%d pause:%d")
			,sw , skip_frame_count, afterimage, (*vm_pause)
		);
#endif
		return;
	}
#ifdef DEBUG_CRTMON2
		logging->out_debugf(_T("draw_screen:draw sw:%d skip_frame_count:%d afterimage:%d pause:%d")
			,sw , skip_frame_count, afterimage, (*vm_pause)
		);
#endif

//	if (*vm_pause) return;

	crt_mon_bg_color = bg_color;

	// render screen
	if (afterimage == 0) draw_screen_sub();
	else if (afterimage == 1) draw_screen_sub_afterimage1();
	else draw_screen_sub_afterimage2();

	// sw bit0 : even/odd line (y axis)  bit1: even/odd char (x axis)
	sw = ((sw + 1) & 3);

	if (scanline <= 1 && *crtc_odd_line && skip_frame_count == 0) {
		sw = *crtc_odd_line;
	}
}

uint8_t DISPLAY::get_font_data(int interlace, int chr_code, int y)
{
	int code = 0;
	int offset = 0;
	if (interlace == 1) {
		// interlace mode char
		code = (chr_code | 0x100) << 4;	// 16byte per 1char
		offset = (y & 0xf);	// 0 - 15
	} else {
		// non interlace mode char
		code = chr_code << 4;	// 16byte per 1char
		offset = (y & 0x7);	// 0 - 7
	}
	return font[code + offset];
}

void DISPLAY::set_font_data()
{
	for(int i = 0; i < 2; i++) {
		for(int c = 0; c < 256; c++) {
			for(int l = 0; l < 16; l++) {
				chrfont[i][c][l] = get_font_data(i, c, l);
			}
		}
	}
}

uint8_t DISPLAY::get_l3font_data(int interlace, int chr_code, int y)
{
	int code = 0;
	int offset = 0;
	if (interlace == 1) {
		// interlace mode char
		if (chr_code < 0x80) {
			code = chr_code << 4;	// 16byte per 1char
			offset = (y & 0x0e);	// 0 0 2 2 4 4 6 6 8 8 a a c c e e
		} else {
			code = chr_code << 4;	// 16byte per 1char
			offset = (y & 0x0f);	// 0 1 2 3 4 5 6 7 8 9 a b c d e f
		}
	} else {
		// non interlace mode char
		if (chr_code < 0x80) {
			// ascii code
			code = chr_code << 4;	// 16byte per 1char
		} else {
			code = ((chr_code - 0x80) << 4) + 1;
		}
		offset = ((y << 1) & 0x0e);	// 0 2 4 6 8 a c e 0 2 4 6 8 a c e
	}
	return font[code + offset];
}

void DISPLAY::set_l3font_data()
{
	for(int i = 0; i < 2; i++) {
		for(int c = 0; c < 256; c++) {
			for(int l = 0; l < 16; l++) {
				chrfont[i][c][l] = get_l3font_data(i, c, l);
			}
		}
	}
}

void DISPLAY::conv_to_l3font_data()
{
	int code = 0;
	int offset = 0;
	memset(font, 0, sizeof(font));
	for(int chr_code = 0; chr_code < 256; chr_code++) {
		for(int y=0; y<8; y++) {
			// non interlace mode char
			if (chr_code < 0x80) {
				// ascii code
				code = chr_code << 4;	// 16byte per 1char
			} else {
				code = ((chr_code - 0x80) << 4) + 1;
			}
			offset = ((y << 1) & 0x0e);	// 0 2 4 6 8 a c e 0 2 4 6 8 a c e
			font[code + offset] = chrfont[0][chr_code][y];
		}
	}
	for(int chr_code = 128; chr_code < 256; chr_code++) {
		for(int y=0; y<16; y++) {
			// interlace mode char
			code = chr_code << 4;	// 16byte per 1char
			offset = (y & 0x0f);	// 0 1 2 3 4 5 6 7 8 9 a b c d e f
			font[code + offset] = chrfont[1][chr_code][y];
		}
	}
}

#define COLOR_REVERSE(fg,bg) { \
	uint8_t c; \
	c = (bg); \
	(bg) = (fg); \
	(fg) = c; \
}

#define SET_SCRNLINE2M(ii) dws_scrnline2m = ((ii) ? 0 : 0xffffffff) 
#define SET_SCRN_DOT(xx,dd) dws_scrn[xx] = (dd | (dws_scrnline1p[xx] & dws_scrnline2m))
#define SET_SCRN_DOT_COLOR16(xx,bb,rr,gg,ii) dws_scrn[xx] = (COLOR16A(bb,rr,gg,ii) | (dws_scrnline1p[xx] & dws_scrnline2m))
#define SET_SCRN_FILTER(xx) dws_scrnf[xx] &= dws_scrnline2m

#define DISP_OUT_OF_DISPTMG_S(bx,xx,pp) { \
	SET_SCRNLINE2M(color16[pp]); \
	for(dws_bt = 0; dws_bt < crtc_ppch; dws_bt++) { \
		SET_SCRN_DOT(xx + dws_bt, color16[pp]); \
		SET_SCRN_FILTER(xx); \
	} \
}

#define DISP_TEXT_CHAR_ONE_DOT(pos, code) { \
	dws_p_p = (code & dof[pos]) ? dws_t_p : dws_b_p; \
	dws_dot = COLOR16A(dws_p_p.b.b,dws_p_p.b.r,dws_p_p.b.g,dws_p_p.b.i); \
	SET_SCRNLINE2M(dws_p_p.i & BRG_COLOR_IMASK); \
	dws_dtm = dws_x+dws_bi+ppd[pos]; \
	if (dws_dtm > SCREEN_WIDTH) dws_dtm = SCREEN_WIDTH; \
	for(dws_dt = dws_x+dws_bi; dws_dt < dws_dtm; dws_dt++) { \
		SET_SCRN_DOT(dws_dt,dws_dot); \
		SET_SCRN_FILTER(dws_dt); \
	} \
}

#define DISP_TEXT_CHAR_MAIN(code) { \
	dws_bi = 0; \
	for(dws_bt = 0; dws_bt < 8; dws_bt++) { \
		DISP_TEXT_CHAR_ONE_DOT(dws_bt, code); \
		dws_bi += ppd[dws_bt]; \
	} \
}

#define DISP_L3_TEXT_CHAR(code) { \
	dws_pattern = chrfont[REG_INTERLACE_SEL][code][(dws_raster >> dws_videomode_n) & 0xf]; \
\
	if (dws_disp_cursor_char) dws_pattern = ~dws_pattern; \
	if (dws_text_cram & 8) dws_pattern = ~dws_pattern; \
	DISP_TEXT_CHAR_MAIN(dws_pattern) \
}

#define DISP_IG_PATTERN_ONE_DOT(ci, bt) { \
	if (FLG_ORIG_CURIG && dws_disp_cursor_char) { \
		dws_p_p = dws_t_p; \
	} else { \
		dws_pattern = ((ci[0].b.g & dof[bt]) ? 4 : 0) | ((ci[0].b.r & dof[bt]) ? 2 : 0) | ((ci[0].b.b & dof[bt]) ? 1 : 0); \
		DECODE_TO_PALETTE16_NUM(dws_pattern, dws_p_p); \
		if (dws_p_p.i == 0) dws_p_p = dws_b_p; \
		dws_p_p.b.i = 1; \
	} \
	dws_dot = COLOR16A(dws_p_p.b.b,dws_p_p.b.r,dws_p_p.b.g,dws_p_p.b.i); \
	SET_SCRNLINE2M(dws_p_p.i & BRG_COLOR_IMASK); \
	dws_dtm = dws_x+dws_bi+ppd[bt]; \
	if (dws_dtm > SCREEN_WIDTH) dws_dtm = SCREEN_WIDTH; \
	for(dws_dt = dws_x+dws_bi; dws_dt < dws_dtm; dws_dt++) { \
		SET_SCRN_DOT(dws_dt,dws_dot); \
		SET_SCRN_FILTER(dws_dt); \
	} \
}

#define DISP_IG_PATTERN_MAIN(ci) { \
	dws_bi = 0; \
	for(dws_bt = 0; dws_bt < 8; dws_bt++) { \
		DISP_IG_PATTERN_ONE_DOT(ci, dws_bt) \
		dws_bi += ppd[dws_bt]; \
	} \
}

#define DISP_L3_IG_PATTERN(code) { \
	dws_raster2 = dws_raster >> dws_int_vid; \
	dws_ct[0].b.b = ig_ram[code*8+dws_raster2]; \
	dws_ct[0].b.r = ig_ram[code*8+dws_raster2+IGRAM_SIZE]; \
	dws_ct[0].b.g = ig_ram[code*8+dws_raster2+IGRAM_SIZE+IGRAM_SIZE]; \
	if (FLG_ORIG_CURIG && (dws_text_cram & 8)) dws_ct[0].i = ~dws_ct[0].i; \
\
	DISP_IG_PATTERN_MAIN(dws_ct) \
}

#define DISP_WHITE_SPACE { \
	dws_c_f = 15; \
	DECODE_TO_PALETTE16_NUM(dws_c_f, dws_p_p); \
	dws_dot = COLOR16A(dws_p_p.b.b,dws_p_p.b.r,dws_p_p.b.g,dws_p_p.b.i); \
	SET_SCRNLINE2M(dws_p_p.i & BRG_COLOR_IMASK); \
	dws_dtm = dws_x+ppch; \
	if (dws_dtm > SCREEN_WIDTH) dws_dtm = SCREEN_WIDTH; \
	for(dws_dt = dws_x; dws_dt < dws_dtm; dws_dt++) { \
		SET_SCRN_DOT(dws_dt,dws_dot); \
		SET_SCRN_FILTER(dws_dt); \
	} \
}

#define	DISP_L3_GRAPHIC_HIRESO_MAIN(code) { \
	dws_bi = 0; \
	for(dws_bt = 0; dws_bt < 8; dws_bt++) { \
		dws_p_p = (code & dof[dws_bt]) ? dws_t_p : dws_b_p; \
		dws_dot = COLOR16A(dws_p_p.b.b,dws_p_p.b.r,dws_p_p.b.g,dws_p_p.b.i); \
		SET_SCRNLINE2M(dws_p_p.i & BRG_COLOR_IMASK); \
		dws_dtm = dws_x+dws_bi+ppd[dws_bt]; \
		if (dws_dtm > SCREEN_WIDTH) dws_dtm = SCREEN_WIDTH; \
		for(dws_dt = dws_x+dws_bi; dws_dt < dws_dtm; dws_dt++) { \
			SET_SCRN_DOT(dws_dt,dws_dot); \
			SET_SCRN_FILTER(dws_dt); \
		} \
		dws_bi += ppd[dws_bt]; \
	} \
}

#define	DISP_L3_GRAPHIC_HIRESO(code) \
	DISP_L3_GRAPHIC_HIRESO_MAIN(code)


#define DISP_L3_GRAPHIC_NORMAL_MAIN(code) { \
	dws_bi = 0; \
	for (dws_bt = 0; dws_bt < 8; dws_bt += 4) { \
		dws_p_p = (code & ((0x10 >> dws_bt) << ((dws_raster >> (1 + dws_int_vid)) & 3))) ? dws_t_p : dws_b_p; \
		dws_dot = COLOR16A(dws_p_p.b.b,dws_p_p.b.r,dws_p_p.b.g,dws_p_p.b.i); \
		SET_SCRNLINE2M(dws_p_p.i & BRG_COLOR_IMASK); \
		dws_dtm = dws_x+dws_bi+ppd[dws_bt]+ppd[dws_bt+1]+ppd[dws_bt+2]+ppd[dws_bt+3]; \
		if (dws_dtm > SCREEN_WIDTH) dws_dtm = SCREEN_WIDTH; \
		for(dws_dt = dws_x+dws_bi; dws_dt < dws_dtm; dws_dt++) { \
			SET_SCRN_DOT(dws_dt,dws_dot); \
			SET_SCRN_FILTER(dws_dt); \
		} \
		dws_bi += ppd[dws_bt] + ppd[dws_bt + 1] + ppd[dws_bt + 2] + ppd[dws_bt + 3]; \
	} \
}

#define DISP_L3_GRAPHIC_NORMAL(code) \
	DISP_L3_GRAPHIC_NORMAL_MAIN(code)

#define ENCODE_COLOR_ONE_DOT_1(pos, inc, outc) { \
	iwk1.i = ((inc.i & (0x01010101 << pos)) >> pos); \
	outc[pos] = iwk1.b.b | (iwk1.b.r << 1) | (iwk1.b.g << 2); \
}

#define ENCODE_PALETTE_ONE_DOT_1(p_vline, pos, inc, outc) { \
	iwk1.i = ((inc.i & (0x01010101 << pos)) >> pos); \
	outc[pos] = p_vline->pal[iwk1.b.b | (iwk1.b.r << 1) | (iwk1.b.g << 2)]; \
}

#define ENCODE_COLOR_ONE_DOT_2(pos, inc, outc) { \
	iwk1.i = ((inc[0].i & (0x01010101 << pos)) >> pos); \
	outc[0][pos] = iwk1.b.b | (iwk1.b.r << 1) | (iwk1.b.g << 2); \
	iwk1.i = ((inc[1].i & (0x01010101 << pos)) >> pos); \
	outc[1][pos] = iwk1.b.b | (iwk1.b.r << 1) | (iwk1.b.g << 2); \
}

#define ENCODE_PALETTE_ONE_DOT_2(p_vline, pos, inc, outc) { \
	iwk1.i = ((inc[0].i & (0x01010101 << pos)) >> pos); \
	outc[0][pos] = p_vline->pal[iwk1.b.b | (iwk1.b.r << 1) | (iwk1.b.g << 2)]; \
	iwk1.i = ((inc[1].i & (0x01010101 << pos)) >> pos); \
	outc[1][pos] = p_vline->pal[iwk1.b.b | (iwk1.b.r << 1) | (iwk1.b.g << 2)]; \
}

#define DECODE_BRG_2BIT_I(pos, inc, outc) { \
	iwk2.i = (inc.i << pos); \
	if (inc.i == 0x1000000) { \
		outc[1].i |= (0x010101 << pos); \
	} else { \
		outc[0].i |= iwk2.i; \
		if (inc.b.i) outc[1].i |= iwk2.i; \
	} \
}

#define CREATE_MASK_0(inc,outc) { \
	outc.b.b = outc.b.r = outc.b.g = outc.b.i = inc; \
}

#define CREATE_MASK_1(inc,outc) { \
	wk1u8 = ~(inc.b.b | inc.b.r | inc.b.g | inc.b.i); \
	outc.b.b = outc.b.r = outc.b.g = outc.b.i = wk1u8; \
}

#define CREATE_MASK_2(inc,outc) { \
	wk1u8 = ~(inc[0].b.b | inc[0].b.r | inc[0].b.g | inc[0].b.i | inc[1].b.b | inc[1].b.r | inc[1].b.g | inc[1].b.i); \
	outc.b.r = outc.b.g = outc.b.b = outc.b.i = wk1u8; \
}

#define COPY_BRG_1(inc,outc) { \
	outc.i = inc.i; \
}

#define COPY_BRG_2(inc,outc) { \
	outc[0].i = inc[0].i; outc[1].i = inc[1].i; \
}

#define COPY_BRG_1_WITH_MASK(inc,outc,msk) { \
	outc.i = (inc.i & msk.i); \
}

#define COPY_BRG_2_WITH_MASK(inc,outc,msk) { \
	outc[0].i = (inc[0].i & msk.i); outc[1].i = (inc[1].i & msk.i); \
}

#define ADD_BRG_1(inc,outc) { \
	outc.i |= inc.i; \
}

#define ADD_BRG_2(inc,outc) { \
	outc[0].i |= inc[0].i; outc[1].i |= inc[1].i; \
}

#define ADD_BRG_1_WITH_MASK(inc,outc,msk) { \
	outc.i |= (inc.i & msk.i); \
}

#define ADD_BRG_2_WITH_MASK(inc,outc,msk) { \
	outc[0].i |= (inc[0].i & msk.i); outc[1].i |= (inc[1].i & msk.i); \
}

#define REVERSE_BRG_1(outc) { \
	outc.i = ~outc.i; \
}

#define REVERSE_BRG_2(outc) { \
	outc[0].i = ~outc[0].i;	outc[1].i = ~outc[1].i; \
}

#define DECODE_BRG_1BIT_TO_PALETTE16(pos,inc,outc) { \
	iwk1.i = (0x80808080 >> pos); iwk2.i = (7 - pos); \
	outc.i = ((inc[0].i & iwk1.i) >> iwk2.i); \
}

#define DECODE_BRG_2BIT_TO_PALETTE64_R(pos,inc,outc) { \
	iwk1.i = (0x80808080 >> pos); iwk2.i = (7 - pos); \
	outc.i = (((inc[0].i & iwk1.i) >> iwk2.i) | (((inc[1].i & iwk1.i) >> iwk2.i) << 1)); \
}

#define DECODE_BRG_2BIT_TO_PALETTE64(pos,inc,outc) { \
	iwk1.i = (0x80808080 >> pos); iwk2.i = (7 - pos); \
	outc.i = ((((inc[0].i & iwk1.i) >> iwk2.i) << 1) | ((inc[1].i & iwk1.i) >> iwk2.i)); \
}

#define EXPAND_UINT8_LEFT(inp,outp) { \
	outp = ((inp) & 0x01) | (((inp) & 0x02) << 1) | (((inp) & 0x04) << 2) | (((inp) & 0x08) << 3); \
	outp |= ((outp) << 1); \
}							
#define EXPAND_UINT8_RIGHT(inp,outp) { \
	outp = (((inp) & 0x10) >> 4) | (((inp) & 0x20) >> 3) | (((inp) & 0x40) >> 2) | (((inp) & 0x80) >> 1); \
	outp |= ((outp) << 1); \
}

#define EXPAND_UINT32_LEFT(inp,outp) { \
	outp = ((inp) & 0x01010101) | (((inp) & 0x02020202) << 1) | (((inp) & 0x04040404) << 2) | (((inp) & 0x08080808) << 3); \
	outp |= ((outp) << 1); \
}							
#define EXPAND_UINT32_RIGHT(inp,outp) { \
	outp = (((inp) & 0x10101010) >> 4) | (((inp) & 0x20202020) >> 3) | (((inp) & 0x40404040) >> 2) | (((inp) & 0x80808080) >> 1); \
	outp |= ((outp) << 1); \
}

#undef SET_SCRNLINE2M
#undef SET_SCRN_DOT
#undef SET_SCRN_DOT_COLOR16
#undef SET_SCRN_FILTER

#define INIT_SCRN_DOT(scrn,yy) scrn = dws_scrn0 + dws_scrn_offset * (yy)
#define INIT_SCRN_FILTER(yy)
#define INIT_SCRNLINE1(yy)
#define SET_SCRNLINE2M(ii)
#define SET_SCRN_DOT(xx,dd) dws_scrn[xx] = (dd)
#define SET_SCRN_DOT_COLOR16(xx,bb,rr,gg,ii) dws_scrn[xx] = COLOR16A(bb,rr,gg,ii)
#define SET_SCRN_FILTER(xx)
#define ADD_SCRN_DOT(scrn) scrn += dws_scrn_offset * dws_ys
#define ADD_SCRN_FILTER
#define ADD_SCRNLINE1

void DISPLAY::draw_screen_sub()
{
	dws_disp_cursor_line = false;
	dws_disp_cursor_char = false;
	dws_scrn_offset = emu->screen_buffer_offset();
	dws_scrn0 = emu->screen_buffer(0);
	dws_dot = 0;
	dws_buf_x = 0;

#include "displaysub.h"
}

#undef INIT_SCRN_FILTER
#undef INIT_SCRNLINE1
#undef SET_SCRNLINE2M
#undef SET_SCRN_DOT
#undef SET_SCRN_DOT_COLOR16
#undef SET_SCRN_FILTER
#undef ADD_SCRNLINE1
#undef ADD_SCRN_FILTER

#define SCRNTYPE_SELDOT(newdot, olddot) \
	(((newdot & Rmask) > (olddot & Rmask) ? (newdot & Rmask) : (olddot & Rmask)) \
	| ((newdot & Gmask) > (olddot & Gmask) ? (newdot & Gmask) : (olddot & Gmask)) \
	| ((newdot & Bmask) > (olddot & Bmask) ? (newdot & Bmask) : (olddot & Bmask)) \
	| Amask)

#define INIT_SCRNLINE1(yy)   dws_scrnline1p = scrnline1[0] + scrnline1_offset * (yy)
#define INIT_SCRN_FILTER(yy)
#define SET_SCRNLINE2M(ii)
#define SET_SCRN_DOT(xx,dd) dws_scrn[xx] = SCRNTYPE_SELDOT(dd, dws_scrnline1p[xx])
#define SET_SCRN_DOT_COLOR16(xx,bb,rr,gg,ii) dws_scrn[xx] = SCRNTYPE_SELDOT(COLOR16A(bb,rr,gg,ii), dws_scrnline1p[xx])
#define SET_SCRN_FILTER(xx)
#define ADD_SCRNLINE1   dws_scrnline1p += scrnline1_offset * dws_ys
#define ADD_SCRN_FILTER

void DISPLAY::draw_screen_sub_afterimage1()
{
	dws_disp_cursor_line = false;
	dws_disp_cursor_char = false;
	dws_scrn_offset = emu->screen_buffer_offset();
	dws_scrn0 = emu->screen_buffer(0);
	dws_dot = 0;
	dws_buf_x = 0;

	// color filter for afterimage
	dws_scrn = dws_scrn0 + dws_scrn_offset * screen_top;
	for(dws_y = screen_top; dws_y < screen_bottom; dws_y++) {
		dws_scrnline1p = scrnline1[dws_y];
		for(dws_x = screen_left; dws_x < screen_right; dws_x++) {
			dws_dot = dws_scrn[dws_x];
			dws_r = (dws_dot & Rmask);
			dws_g = (dws_dot & Gmask);
			dws_b = (dws_dot & Bmask);

			if (dws_r > Rboad) dws_r = dws_r - Rloss[0];
			else if (dws_r > Rloss[1]) dws_r = dws_r - Rloss[1];
			else dws_r = 0;
			if (dws_g > Gboad) dws_g = dws_g - Gloss[0];
			else if (dws_g > Gloss[1]) dws_g = dws_g - Gloss[1];
			else dws_g = 0;
			if (dws_b > Bboad) dws_b = dws_b - Bloss[0];
			else if (dws_b > Bloss[1]) dws_b = dws_b - Bloss[1];
			else dws_b = 0;

			dws_dot = dws_r | dws_g | dws_b;

			dws_scrnline1p[dws_x] = dws_dot;
		}
		dws_scrn += dws_scrn_offset;
	}

#include "displaysub.h"
}

#undef INIT_SCRN_FILTER
#undef INIT_SCRNLINE1
#undef SET_SCRNLINE2M
#undef SET_SCRN_DOT
#undef SET_SCRN_DOT_COLOR16
#undef SET_SCRN_FILTER
#undef ADD_SCRNLINE1
#undef ADD_SCRN_FILTER

#define INIT_SCRNLINE1(yy)   dws_scrnline1p = scrnline1[0] + scrnline1_offset * (yy)
#define INIT_SCRN_FILTER(yy) dws_scrnf = scrnflag[0] + dws_scrn_offset * (yy)
#define SET_SCRNLINE2M(ii) dws_scrnline2m = ((ii & (Rmask | Gmask | Bmask)) ? 0 : 0xffffffff) 
#define SET_SCRN_DOT(xx,dd) dws_scrn[xx] = (dd | (dws_scrnline1p[xx] & dws_scrnline2m))
#define SET_SCRN_DOT_COLOR16(xx,bb,rr,gg,ii) dws_scrn[xx] = (COLOR16A(bb,rr,gg,ii) | (dws_scrnline1p[xx] & dws_scrnline2m))
//#define SET_SCRN_FILTER(xx) dws_scrnf[xx] &= dws_scrnline2m
#define SET_SCRN_FILTER(xx) dws_scrnf[xx] = ((uint8_t)dws_scrnline2m + 1)
#define ADD_SCRNLINE1   dws_scrnline1p += scrnline1_offset * dws_ys
#define ADD_SCRN_FILTER dws_scrnf += dws_scrn_offset * dws_ys

void DISPLAY::draw_screen_sub_afterimage2()
{
	dws_disp_cursor_line = false;
	dws_disp_cursor_char = false;
	dws_scrn_offset = emu->screen_buffer_offset();
	dws_scrn0 = emu->screen_buffer(0);
	dws_dot = 0;
	dws_buf_x = 0;

	// color filter for afterimage2
	dws_scrnline2m = 0;
	dws_scrn = dws_scrn0 + dws_scrn_offset * screen_top;
	for(dws_y = screen_top; dws_y < screen_bottom; dws_y++) {
		dws_scrnline1p = scrnline1[dws_y];
		dws_scrnf = scrnflag[dws_y];
		for(dws_x = screen_left; dws_x < screen_right; dws_x++) {
			dws_dot = dws_scrn[dws_x];

			if (dws_scrnf[dws_x] > 0) {
				dws_scrnf[dws_x]--;
			} else {
				dws_dot = 0;
			}

			dws_scrnline1p[dws_x] = dws_dot;
		}
		dws_scrn += dws_scrn_offset;
	}

#include "displaysub.h"
}

#define SET_CRT_MON(row, name, value) \
	crt_mon[row][crt_mon_col].name = (value);

#define ADD_CRT_MON(row, name, value) \
	crt_mon[row][crt_mon_col].name |= (value);

#define GET_CRT_MON(row, name) \
	crt_mon[row][crt_mon_col].name

#define P_SET_CRT_MON(ptr, name, value) \
	ptr->name = (value);

#define P_ADD_CRT_MON(ptr, name, value) \
	ptr->name |= (value);

#define P_GET_CRT_MON(ptr, name) \
	ptr->name

void DISPLAY::update_display(int v, int clock)
{
	static int crtc_vt_count_prev = -1;
	static int addr;
	static int addr_offset;
	static uint8_t text,color,val,val2;
	static int vtaddr;
	static int vtadiv;
	static int vgaddr;
	static int vgaddr2;
	static int vgadiv;
	static bool now_skip_frame;
	static uint8_t disptmg;
	static int disptmg_left[2];
	static int disptmg_right[2];
	static int disptmg_st;
	static int hsync_left[2];
	static int hsync_right[2];
	static int addr_left;
	static bool now_hsync;
	static bool now_vsync;
	static int cursor_pos;
	static int videomode_n;
	static uint8_t raster;
	static int crt_mon_col_st;
	static t_crt_mon *crt_mon_bufy;
	static t_crt_mon *crt_mon_bufx;
	static t_crt_mon_vline *crt_mon_vline_y;
	static t_crt_mon_vinfo *crt_mon_vinfo_y;
	static uint8_t cursor_col;
	static int cursor_st_ras;
	static int cursor_ed_ras;
	static int cursor_skew;
	static int width_sel_n;
	static int hireso_mask;
	static int ig_mask_mask;

	if ((*crtc_vt_count) == crtc_vt_count_prev) {
		return;
	}

	videomode_n = 1 - (*videomode);
	disptmg_st = (crt_mon_stepcols - 1) & (crt_mon_row + (sw >> 1));
	crt_mon_col = disptmg_st;

	if ((*crtc_vt_count) == 0) {
		// disptmg on
#ifdef USE_LIGHTPEN
		crt_mon_disptmg_top = crt_mon_row;
#endif
		// synchronize horizontal total
		if (crt_mon_crtc_hsync) {
			dws_left_diff = 0;
		}
#ifdef USE_KEEPIMAGE
		// top most page ?
		crt_mon_page = ((keepimage == 1 && (*crtc_ma) < 0x0800) || (keepimage == 2 && (*crtc_ma) >= 0x0800)) ? 0x80 : 0;
#endif
	}

	// pixel of width
	if (*crtc_vs_start <= *crtc_vt_count && *crtc_vt_count < crtc_vs_mid) {
		now_vsync = true;
	} else {
		now_vsync = false;
	}
#ifdef USE_LIGHTPEN
	if (*crtc_vt_count == *crtc_vt_disp) {
		crt_mon_disptmg_bottom = crt_mon_row;
	}
#endif
	crt_mon_disptmg_left = (*disptmg_skew) >= 4 ? VRAM_BUF_WIDTH
		: crtc_regs[0]-crtc_regs[2]-(crtc_regs[3] & 0x0f)-(crtc_chr_clocks ? 14 : 5) + (*disptmg_skew & 3) - pConfig->disptmg_skew;

	if (*crtc_vt_count >= *crtc_vt_disp) {
		disptmg_left[0] = crt_mon_disptmg_left + dws_left_diff;
		disptmg_right[0] = 0;
		disptmg_left[1] = VRAM_BUF_WIDTH;
		disptmg_right[1] = 0;
	} else {
		disptmg_left[0] = crt_mon_disptmg_left + dws_left_diff;
		disptmg_right[0] = disptmg_left[0] + crtc_regs[1];
		disptmg_left[1] = disptmg_left[0] + (crtc_chr_clocks ? crtc_regs[0] + 1 : 0);
		disptmg_right[1] = disptmg_right[0] + (crtc_chr_clocks ? crtc_regs[0] + 1 : 0);
	}

	if (disptmg_left[0] < 0) {
		addr_left = - disptmg_left[0];
	} else {
		addr_left = 0;
	}
	crt_mon_col_st = (crt_mon_stepcols - 1) & (disptmg_left[0] ^ disptmg_st);

	hsync_left[0] = disptmg_left[0] + crtc_regs[2];
	hsync_right[0] = hsync_left[0] + (crtc_regs[3] & 0x0f);
	hsync_left[1] = hsync_left[0] + (crtc_chr_clocks ? crtc_regs[0] + 1 : 0);
	hsync_right[1] = hsync_right[0] + (crtc_chr_clocks ? crtc_regs[0] + 1 : 0);

#ifdef DEBUG_CRTMON
	if (crt_mon_row < 2) {
		logging->out_debugf(_T("DISPLAY:update_vram: sw:%d crtc:cnt:%d ra:%d cmnr:%d cmr:%d cmsr:%d cmc:%d top:%d btm:%d vtop:%d vbtm:%d")
			,sw
			,(*crtc_vt_count), (*crtc_ra), crt_mon_now_raster
			,crt_mon_row, crt_mon_start_row,  crt_mon_col
			,top_raster, bottom_raster, vsync_top_raster, vsync_bottom_raster
		);
	}
#endif

	// diff horizontal total display and crtc
	if (!crt_mon_crtc_hsync) {
		dws_left_diff += (crtc_regs[0] + 1 - (1 << (crtc_chr_clocks + 6)));
		if (dws_left_diff >= VRAM_BUF_WIDTH) dws_left_diff = -VRAM_BUF_WIDTH + 1;
		else if (dws_left_diff <= -VRAM_BUF_WIDTH) dws_left_diff = VRAM_BUF_WIDTH - 1;
	}

	// now skip frame ?
	if (v == 0) {
		if (emu->now_skip_frame()) {
			skip_frame_count++;
		} else {
			skip_frame_count = 0;
		}
		if ((afterimage == 0 && skip_frame_count >= 1)
		||  (afterimage > 0 && skip_frame_count >= 2)) {
			// skip copy vram data
			now_skip_frame = true;
		} else {
			now_skip_frame = false;
		}
	}

	// copy vram data to buffer
	if (crt_mon_row < VRAM_BUF_HEIGHT && !now_skip_frame) { // && *crtc_vt_count < *crtc_vt_disp) {
		crt_mon_vline_y = &crt_mon_vline[crt_mon_row];
		crt_mon_vline_y->left_col = disptmg_left[0];
#ifdef USE_LIGHTPEN
		crt_mon_vline_y->right_col = disptmg_right[0];
#endif
		crt_mon_vline_y->flags = *crtc_odd_line;

		raster = (*crtc_ra) << videomode_n;

		crt_mon_vinfo_y = &crt_mon_vinfo[(crt_mon_row << 1) | (*videomode & *crtc_odd_line)];
		crt_mon_vinfo_y->raster = raster;
		crt_mon_vinfo_y->row = crt_mon_row;

		cursor_col = 254;
		cursor_st_ras = 255;
		cursor_ed_ras = 0;

		// calcrate address
		if (NOW_S1_MODE) {
			// S1
			if (SCRN_MODE_IS_640x400) {
				addr_offset = ((*crtc_ra) & 0xe) << 11; // 0x800;
			} else {
				addr_offset = (((*crtc_ra) >> REG_INTERLACE_SEL) & 0x7) << (10 + gwidth_sel);	// 0x400 or 0x800 
			}
			crt_mon_s1vtaddr = (((*crtc_ma) & 0x7ff) >> (1 - width_sel)) + (width_sel & crt_mon_col_st);
			crt_mon_s1vgaddr = ((*crtc_ma) >> (1 - gwidth_sel)) + (gwidth_sel & crt_mon_col_st);
			crt_mon_s1vgaddr += addr_offset;
			crt_mon_s1vtaddr += addr_left;
			crt_mon_s1vgaddr += addr_left;

			vtaddr = (crt_mon_s1vtaddr + (width_sel ? 0 : page_no) * (S1_VTRAM_SIZE >> 1));
			vgaddr = (crt_mon_s1vgaddr + ((gwidth_sel || SCRN_MODE_IS_64COLOR) ? 0 : gpage_no) * (S1_VGRAM_SIZE_ONE >> 1));

			// cursor
			cursor_addr = (((crtc_regs[14] << 8) | crtc_regs[15]) + (*disptmg_skew & 3) - pConfig->disptmg_skew + (width_sel & crt_mon_col_st));
			cursor_addr >>= (1 - width_sel); 
			cursor_addr += addr_left;
			cursor_addr &= 0x3fff;

			cursor_pos = (int)cursor_addr - vtaddr;
			width_sel_n = (1 - width_sel);
			cursor_pos <<= width_sel_n;
			cursor_skew = (*curdisp_skew) - 1;
			cursor_pos += cursor_skew;
			if (cursor_skew && width_sel_n) cursor_pos = -2;

			if ((*crtc_curdisp) != 0
			&& cursor_pos >= 0 && disptmg_left[0] + cursor_pos < disptmg_right[0]
			) {
				cursor_col = disptmg_left[0] + cursor_pos;
				cursor_st_ras = (crtc_regs[10] & 0x1f) << videomode_n;
				cursor_ed_ras = (crtc_regs[11] + 1) << videomode_n;
//				logging->out_debugf(_T("DISPLAY: v:%d row:%d ra:%d pos:%d col:%d ras:%d-%d"), (*videomode), crt_mon_row, *crtc_ra, cursor_pos, crt_mon_vline_y->cursor_col, crt_mon_vline_y->cursor_st_ras, crt_mon_vline_y->cursor_ed_ras);
			}
		} else {
			// L3
			addr_offset = hireso * (((*crtc_ra) >> REG_INTERLACE_SEL) & 0x7) << (10 + width_sel);	// 0x400 or 0x800
			crt_mon_l3vaddr = *crtc_ma;
			crt_mon_l3vaddr += crt_mon_col_st;
			crt_mon_l3vaddr += addr_offset;
			crt_mon_l3vaddr += addr_left;
			crt_mon_l3vaddr &= L3_VRAM_SIZE_1;
			if (crt_mon_l3vaddr < L3_ADDR_VRAM_START) {
				crt_mon_l3vaddr += L3_VRAM_SIZE;
			}

			// cursor
			cursor_addr = (((crtc_regs[14] << 8) | crtc_regs[15]) + (*disptmg_skew & 3) - pConfig->disptmg_skew);
			cursor_addr += crt_mon_col_st;
			cursor_addr += addr_offset;
			cursor_addr += addr_left;
			cursor_addr &= L3_VRAM_SIZE_1;
			if (cursor_addr < L3_ADDR_VRAM_START) {
				cursor_addr += L3_VRAM_SIZE;
			}

			cursor_pos = (int)cursor_addr - crt_mon_l3vaddr;
			width_sel_n = 0;
			cursor_skew = (*curdisp_skew) + pConfig->curdisp_skew - 1;

			if ((*crtc_curdisp) != 0
			&& cursor_pos >= 0 && disptmg_left[0] + cursor_pos < disptmg_right[0]
			) {
				cursor_col = disptmg_left[0] + cursor_pos + cursor_skew;
				cursor_st_ras = (crtc_regs[10] & 0x1f) << videomode_n;
				cursor_ed_ras = (crtc_regs[11] + 1) << videomode_n;
//				logging->out_debugf(_T("DISPLAY: v:%d row:%d ra:%d pos:%d col:%d ras:%d-%d"), (*videomode), crt_mon_row, *crtc_ra, cursor_pos, crt_mon_vline_y->cursor_col, crt_mon_vline_y->cursor_st_ras, crt_mon_vline_y->cursor_ed_ras);
			}
		}

		if (cursor_st_ras <= raster && raster < cursor_ed_ras) {
			crt_mon_vinfo_y->cursor_st_col = cursor_col;
			crt_mon_vinfo_y->cursor_ed_col = cursor_col + width_sel_n;
		} else {
			crt_mon_vinfo_y->cursor_st_col = 254;
		}

		if (raster & 1) {
			crt_mon_vinfo_y--;
			raster--;
		} else {
			crt_mon_vinfo_y++;
			raster++;
		}
		crt_mon_vinfo_y->raster = raster;
		crt_mon_vinfo_y->row = crt_mon_row;
		if (cursor_st_ras <= raster && raster < cursor_ed_ras) {
			crt_mon_vinfo_y->cursor_st_col = cursor_col;
			crt_mon_vinfo_y->cursor_ed_col = cursor_col + width_sel_n;
		} else {
			crt_mon_vinfo_y->cursor_st_col = 254;
		}

		now_hsync = false;

		if (NOW_S1_MODE) {
			// S1

			// save palette
			memcpy(crt_mon_vline_y->pal, REG_PALETTE, sizeof(crt_mon_vline_y->pal));

			vtadiv = 1;
			vgaddr2 = -1;
			vgadiv = 1;

			if (SCRN_MODE_IS_64COLOR) {
				// 64color mode
				vgaddr2 = vgaddr + S1_VGRAM_SIZE_HALF;
			} else if (SCRN_MODE_IS_640x400) {
				// 640x400 mode
				vgaddr2 = vgaddr + 0x800;
				vgaddr2 &= S1_VGRAM_SIZE_TWO_1;
				crt_mon_vline_y->flags |= 0x80;
			}

			crt_mon_bufy = crt_mon[crt_mon_row];

			for(; crt_mon_col < VRAM_BUF_WIDTH; crt_mon_col += crt_mon_stepcols) {
				crt_mon_bufx = &crt_mon_bufy[crt_mon_col];
				if ((disptmg_left[0] <= crt_mon_col && crt_mon_col < disptmg_right[0])
					|| (disptmg_left[1] <= crt_mon_col && crt_mon_col < disptmg_right[1])) {
					if (now_hsync) {
						d_crtc->update_raster();
						if (SCRN_MODE_IS_640x400) {
							addr_offset += (1 << 11); // 0x800;
						} else {
							addr_offset += (1 << (10 + gwidth_sel));	// 0x400 or 0x800 
						}
						crt_mon_s1vtaddr = (((*crtc_ma) & 0x7ff) >> (1 - width_sel)) + (width_sel & crt_mon_col_st);
						crt_mon_s1vgaddr = ((*crtc_ma) >> (1 - gwidth_sel)) + (gwidth_sel & crt_mon_col_st);
						crt_mon_s1vgaddr += addr_offset;

						vtaddr = (crt_mon_s1vtaddr + (width_sel ? 0 : page_no) * (S1_VTRAM_SIZE >> 1));
						vgaddr = (crt_mon_s1vgaddr + ((gwidth_sel || SCRN_MODE_IS_64COLOR) ? 0 : gpage_no) * (S1_VGRAM_SIZE_ONE >> 1));

						if (SCRN_MODE_IS_64COLOR) {
							// 64color mode
							vgaddr2 = vgaddr + S1_VGRAM_SIZE_HALF;
						} else if (SCRN_MODE_IS_640x400) {
							// 640x400 mode
							vgaddr2 = vgaddr + 0x800;
							vgaddr2 &= S1_VGRAM_SIZE_TWO_1;
						}
						now_hsync = false;
					}
					P_SET_CRT_MON(crt_mon_bufx, disptmg, 1);
					if (vtaddr >= S1_VTRAM_SIZE) {
						vtaddr -= S1_VTRAM_SIZE;
					}
					if (SCRN_MODE_IS_640x400) {
						if (vgaddr >= S1_VGRAM_SIZE_TWO) {
							vgaddr -= S1_VGRAM_SIZE_TWO;
						}
						if (vgaddr2 >= S1_VGRAM_SIZE_TWO) {
							vgaddr2 -= S1_VGRAM_SIZE_TWO;
						}
					} else {
						if (vgaddr >= S1_VGRAM_SIZE_ONE) {
							vgaddr -= S1_VGRAM_SIZE_ONE;
						}
					}
					// text vram
					text = s1vtram[vtaddr];
					P_SET_CRT_MON(crt_mon_bufx, text, text);
					// text color
					// bit 0-6: color ram  bit 7:width 0:40 1:80
					color = ((s1vcram[vtaddr] & 0x7f) | (width_sel << 7)) & disable_ig;

					P_SET_CRT_MON(crt_mon_bufx, color, color);
					if (color & 0x20) {
						// ig
						addr = (text << 3)+((*crtc_ra) >> (*videomode));
						P_SET_CRT_MON(crt_mon_bufx, c.b.b, ig_ram[addr]);
						P_SET_CRT_MON(crt_mon_bufx, c.b.r, ig_ram[addr+IGRAM_SIZE]);
						P_SET_CRT_MON(crt_mon_bufx, c.b.g, ig_ram[addr+IGRAM_SIZE+IGRAM_SIZE]);
						P_SET_CRT_MON(crt_mon_bufx, c.b.i, 0);
					} else {
						P_SET_CRT_MON(crt_mon_bufx, c.b.b, (color & 1) ? 0xff : 0);
						P_SET_CRT_MON(crt_mon_bufx, c.b.r, (color & 2) ? 0xff : 0);
						P_SET_CRT_MON(crt_mon_bufx, c.b.g, (color & 4) ? 0xff : 0);
						P_SET_CRT_MON(crt_mon_bufx, c.b.i, 0);
					}

					// graphic vram
					if (SCRN_MODE_IS_640x400) {
						// 640x400 mono
						if (*videomode) {
							// interlace only
							// even line
							P_SET_CRT_MON(crt_mon_bufx, g[0].i, 0);
							if (vgaddr < S1_VGRAM_SIZE_ONE) {
								val = s1vgrram[vgaddr];
								P_SET_CRT_MON(crt_mon_bufx, g[0].b.r, val | disp_page_changed.b.r);
							} else {
								val = s1vggram[vgaddr-S1_VGRAM_SIZE_ONE];
								P_SET_CRT_MON(crt_mon_bufx, g[0].b.g, val | disp_page_changed.b.g);
							}
							// odd line
							P_SET_CRT_MON(crt_mon_bufx, g[1].i, 0);
							if (vgaddr2 < S1_VGRAM_SIZE_ONE) {
								val2 = s1vgrram[vgaddr2];
								P_SET_CRT_MON(crt_mon_bufx, g[1].b.r, val2 | disp_page_changed.b.r);
							} else {
								val2 = s1vggram[vgaddr2-S1_VGRAM_SIZE_ONE];
								P_SET_CRT_MON(crt_mon_bufx, g[1].b.g, val2 | disp_page_changed.b.g);
							}
						} else {
							// non interlace ?
							P_SET_CRT_MON(crt_mon_bufx, g[0].i, 0);
							P_SET_CRT_MON(crt_mon_bufx, g[1].i, 0);
							if (vgaddr < S1_VGRAM_SIZE_ONE) {
								val = s1vgrram[vgaddr];
								P_SET_CRT_MON(crt_mon_bufx, g[0].b.r, val | disp_page_changed.b.r);
								P_SET_CRT_MON(crt_mon_bufx, g[1].b.r, val | disp_page_changed.b.r);
							} else {
								val = s1vggram[vgaddr-S1_VGRAM_SIZE_ONE];
								P_SET_CRT_MON(crt_mon_bufx, g[0].b.g, val | disp_page_changed.b.g);
								P_SET_CRT_MON(crt_mon_bufx, g[1].b.g, val | disp_page_changed.b.g);
							}
						}
						// attribute
						P_SET_CRT_MON(crt_mon_bufx, brg_mask, disp_page_brg_mask);
					} else {
						// normal
						P_SET_CRT_MON(crt_mon_bufx, g[0].b.r, s1vgrram[vgaddr] | disp_page_changed.b.r);
						P_SET_CRT_MON(crt_mon_bufx, g[0].b.g, s1vggram[vgaddr] | disp_page_changed.b.g);
						P_SET_CRT_MON(crt_mon_bufx, g[0].b.b, s1vgbram[vgaddr] | disp_page_changed.b.b);
						P_SET_CRT_MON(crt_mon_bufx, g[0].b.i, disp_page_changed.b.i);
						if (vgaddr2 >= 0) {
							// 320x200 64color
							P_SET_CRT_MON(crt_mon_bufx, g[1].b.r, s1vgrram[vgaddr2] | disp_page_changed.b.r);
							P_SET_CRT_MON(crt_mon_bufx, g[1].b.g, s1vggram[vgaddr2] | disp_page_changed.b.g);
							P_SET_CRT_MON(crt_mon_bufx, g[1].b.b, s1vgbram[vgaddr2] | disp_page_changed.b.b);
							P_SET_CRT_MON(crt_mon_bufx, g[1].b.i, disp_page_changed.b.i);
						}
						// attribute
						P_SET_CRT_MON(crt_mon_bufx, brg_mask, disp_page_brg_mask);
					}
					//   bit0-1:scrn_mode  bit5-7:disp graphic plane b g r
					P_SET_CRT_MON(crt_mon_bufx, attr, ((DISP_PAGE_GRAPHIC_BRG) | (SCRN_MODE_GRAPHIC)));

					vtadiv = crt_mon_stepcols - vtadiv;
					vtaddr += (width_sel ? crt_mon_stepcols : vtadiv);
					vgadiv = crt_mon_stepcols - vgadiv;
					vgaddr += (gwidth_sel ? crt_mon_stepcols : vgadiv);
					if (vgaddr2 >= 0) vgaddr2 += (gwidth_sel ? crt_mon_stepcols : vgadiv);

					crt_mon_s1vtaddr += vtaddr;
					crt_mon_s1vgaddr += vgaddr;
				} else {
					// out of disptmg area
					disptmg = 0;
					P_SET_CRT_MON(crt_mon_bufx, text, 0);
					if ((hsync_left[0] <= crt_mon_col && crt_mon_col < hsync_right[0])
					 || (hsync_left[1] <= crt_mon_col && crt_mon_col < hsync_right[1])) {
						P_SET_CRT_MON(crt_mon_bufx, color, 0);
						if (!now_hsync) {
							disptmg |= 0x02;
							now_hsync = true;
						}
					} else if (now_vsync) {
						P_SET_CRT_MON(crt_mon_bufx, color, 0);
					} else {
						P_SET_CRT_MON(crt_mon_bufx, color, crt_mon_bg_color);
					}
					P_SET_CRT_MON(crt_mon_bufx, disptmg, disptmg);
					P_SET_CRT_MON(crt_mon_bufx, g[0].i, 0);
					P_SET_CRT_MON(crt_mon_bufx, g[1].i, 0);
					P_SET_CRT_MON(crt_mon_bufx, attr, 0);
				}
			  }

		} else {
			// L3
			crt_mon_bufy = crt_mon[crt_mon_row];

			hireso_mask = (hireso << 6);
			ig_mask_mask = (ig_mask ? 0xdf : 0xff);

			for(; crt_mon_col < VRAM_BUF_WIDTH; crt_mon_col += crt_mon_stepcols) {
				crt_mon_bufx = &crt_mon_bufy[crt_mon_col];

				if ((disptmg_left[0] <= crt_mon_col && crt_mon_col < disptmg_right[0])
					|| (disptmg_left[1] <= crt_mon_col && crt_mon_col < disptmg_right[1])) {
					if (now_hsync) {
						d_crtc->update_raster();
						addr_offset += (hireso << (10 + width_sel));	// 0x400 or 0x800
						crt_mon_l3vaddr = *crtc_ma;
						crt_mon_l3vaddr += crt_mon_col_st;
						crt_mon_l3vaddr += addr_offset;
						crt_mon_l3vaddr &= L3_VRAM_SIZE_1;
						if (crt_mon_l3vaddr < L3_ADDR_VRAM_START) {
							crt_mon_l3vaddr += L3_VRAM_SIZE;
						}
						now_hsync = false;
					}
					P_SET_CRT_MON(crt_mon_bufx, disptmg, 1);
					if (crt_mon_l3vaddr >= L3_ADDR_VRAM_END) {
						crt_mon_l3vaddr -= L3_VRAM_SIZE;
					}
					P_SET_CRT_MON(crt_mon_bufx, text, l3vram[crt_mon_l3vaddr]);
					// bit 0-5: color ram   bit 6: hireso  bit 7:top_most
#ifdef USE_KEEPIMAGE
					P_SET_CRT_MON(crt_mon_bufx, color, (color_ram[crt_mon_l3vaddr - L3_ADDR_VRAM_START] | hireso_mask | crt_mon_page) & ig_mask_mask);
#else
					P_SET_CRT_MON(crt_mon_bufx, color, (color_ram[crt_mon_l3vaddr - L3_ADDR_VRAM_START] | hireso_mask) & ig_mask_mask);
#endif
					P_SET_CRT_MON(crt_mon_bufx, attr, crt_mon_bg_color);
					crt_mon_l3vaddr += crt_mon_stepcols;
				} else {
					// out of disptmg area
					disptmg = 0;
					P_SET_CRT_MON(crt_mon_bufx, text, 0);
					if ((hsync_left[0] <= crt_mon_col && crt_mon_col < hsync_right[0])
					 || (hsync_left[1] <= crt_mon_col && crt_mon_col < hsync_right[1])) {
						P_SET_CRT_MON(crt_mon_bufx, attr, 0);
						if (!now_hsync) {
							disptmg |= 0x02;
							now_hsync = true;
						}
					} else if (now_vsync) {
						P_SET_CRT_MON(crt_mon_bufx, attr, 0);
					} else {
						P_SET_CRT_MON(crt_mon_bufx, attr, crt_mon_bg_color);
					}
					P_SET_CRT_MON(crt_mon_bufx, disptmg, disptmg);
				}
			}
		}
	}

	crt_mon_row++;
	if (crt_mon_row >= CRT_MON_HEIGHT) {
		crt_mon_row = 0;
	}
	crt_mon_v_count++;
	if (crt_mon_v_count >= (LINES_PER_FRAME * 2 + 16)) {
		crt_mon_v_count = crt_mon_v_start;
		crt_mon_v_start += 96;
		if (crt_mon_v_start >= LINES_PER_FRAME) crt_mon_v_start = LINES_PER_FRAME-1;
	}

	crtc_vt_count_prev = *crtc_vt_count;
}

// ----------------------------------------------------------------------------
void DISPLAY::set_disp_page()
{
	page_no = (DISP_PAGE_TEXT) ? 1 : 0;
	gpage_no = (DISP_PAGE_GRAPHIC) ? 1 : 0;
	disp_page_brg_mask.b.b = (DISP_PAGE_GRAPHIC_B ? 0xff : 0);
	disp_page_brg_mask.b.r = (DISP_PAGE_GRAPHIC_R ? 0xff : 0);
	disp_page_brg_mask.b.g = (DISP_PAGE_GRAPHIC_G ? 0xff : 0);
	disp_page_brg_mask.b.i = 0xff;
}

void DISPLAY::set_s1_screen_mode()
{
	// for text
	width_sel = SCRN_MODE_TEXT_WIDTH ? 1 : 0;	// 0:w.40  1:w.80

	// for graphic
	gwidth_sel = SCRN_MODE_GRAPHIC_WIDTH ? 1 : 0;	// 640 or 320
}

void DISPLAY::set_l3_screen_mode()
{
	bg_color |= 0x08;					// bright

	hireso = (mode_sel & 0x40) ? 0 : 1;		// 0:normal 1:hireso
	width_sel = (mode_sel & 0x80) ? 1 : 0;	// 0:w.40  1:w.80

	for(int i=0; i<8; i++) {
		ppd[i]=width_sel ? 1 : 2;	// pixel per 1dot
		dof[i]=(0x80 >> i);
	}
}

void DISPLAY::change_palette()
{
	if (NOW_S1_MODE && FLG_ORIG_PAL8) {
		// dark gray
		COLOR16A(0,0,0,1) = emu->map_rgbcolor(0x1f,0x1f,0x1f);
	} else {
		// black 
		COLOR16A(0,0,0,1) = emu->map_rgbcolor(0x0,0x0,0x0);
	}
}

// ----------------------------------------------------------------------------
// event handler
// ----------------------------------------------------------------------------

void DISPLAY::event_frame()
{
}

// ----------------------------------------------------------------------------

void DISPLAY::save_state(FILEIO *fio)
{
	struct vm_state_st vm_state;

	//
	vm_state_ident.version = Uint16_LE(0x41);
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(vm_state));

	// copy values
	memset(&vm_state, 0, sizeof(vm_state));
	vm_state.ig_enable = REG_IGENABLE;
	vm_state.ig_modereg = REG_IGMODE;
	vm_state.ig_mask = ig_mask ? 1 : 0;

	vm_state.sys_mode = REG_SYS_MODE;
	vm_state.mode_sel = REG_MODE_SEL;

	vm_state.chr_clocks = crtc_chr_clocks;
	vm_state.now_cpu_halt = now_cpu_halt;
	vm_state.interlace_sel = REG_INTERLACE_SEL;
	vm_state.tvsuper = REG_TVSUPER;

	//
	vm_state.bmsk_color = REG_BMSK_COLOR;
	vm_state.active_plane = REG_ACTIVE_PLANE;
	vm_state.dummy1 = 0;
	vm_state.disp_page = REG_DISP_PAGE;
	vm_state.scrn_mode = REG_SCRN_MODE;
	memcpy(vm_state.gcolor, REG_GCOLOR, sizeof(vm_state.gcolor));
	memcpy(vm_state.palette, REG_PALETTE, sizeof(vm_state.palette));

	fio->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);
	fio->Fwrite(&vm_state, sizeof(vm_state), 1);
}

bool DISPLAY::load_state(FILEIO *fio)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;
	uint8_t mode_sel = 0;

	READ_STATE_CHUNK(fio, vm_state_i, vm_state);

	REG_IGENABLE = vm_state.ig_enable;
	ig_mask = vm_state.ig_mask ? true : false;

	if (Uint16_LE(vm_state_i.version) >= 2) {
		update_chr_clocks(vm_state.chr_clocks);
	}

	if (Uint16_LE(vm_state_i.version) >= 0x41) {
		REG_SYS_MODE = vm_state.sys_mode;
		mode_sel = vm_state.mode_sel;

		REG_IGMODE = vm_state.ig_modereg;
		now_cpu_halt = vm_state.now_cpu_halt;
		REG_INTERLACE_SEL = vm_state.interlace_sel;
		REG_TVSUPER = vm_state.tvsuper;

		REG_BMSK_COLOR = vm_state.bmsk_color;
		REG_ACTIVE_PLANE = vm_state.active_plane;
		REG_DISP_PAGE = vm_state.disp_page;
		REG_SCRN_MODE = vm_state.scrn_mode;
		memcpy(REG_GCOLOR, vm_state.gcolor, sizeof(REG_GCOLOR));
		memcpy(REG_PALETTE, vm_state.palette, sizeof(REG_PALETTE));

		SIG_BMSK_DW = (BMSK_COLOR_BMSK ? (SIG_BMSK_DW | 2) : (SIG_BMSK_DW & ~2));
		SIG_BMSK_DW = (ACTIVE_PLANE_DW ? (SIG_BMSK_DW | 1) : (SIG_BMSK_DW & ~1));

		set_disp_page();

		if (NOW_S1_MODE) {
			set_s1_screen_mode();
		}

		write_io8(0xffd0, mode_sel);
	}

	// clear
	crt_mon_row = 9999;
	crt_mon_col = 0;

	return true;
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER

enum en_gnames {
	GN_IG_A = 0,
	GN_IG_B,
	GN_IG_R,
	GN_IG_G,
	GN_GR_A,
	GN_GR_B,
	GN_GR_R,
	GN_GR_G
};

const struct st_gnames {
	const _TCHAR *name;
} c_gnames[] = {
	_T("IG RAM"),
	_T("IG RAM (blue)"),
	_T("IG RAM (red)"),
	_T("IG RAM (green)"),
	_T("GRAPHIC RAM"),
	_T("GRAPHIC RAM (blue)"),
	_T("GRAPHIC RAM (red)"),
	_T("GRAPHIC RAM (green)"),
	NULL
};

uint32_t DISPLAY::debug_read_io8(uint32_t addr)
{
	return 0xff;
}

int DISPLAY::get_debug_graphic_memory_size(int num, int type, int *width, int *height)
{
	switch(type) {
	case GN_IG_A:
	case GN_IG_B:
	case GN_IG_R:
	case GN_IG_G:
		// IG
		*width = (16 * 8);
		*height = (16 * 8);
		break;
	case GN_GR_A:
	case GN_GR_B:
	case GN_GR_R:
	case GN_GR_G:
		{
			// GVRAM
			int w = (SCRN_MODE_GRAPHIC_WIDTH ? 80 : 40);
			*width = (w * 8);
			*height = (25 * 8);
		}
		break;
	default:
		return -1;
	}
	return 0;
}

bool DISPLAY::debug_graphic_type_name(int type, _TCHAR *buffer, size_t buffer_len)
{
	for(int i=0; c_gnames[i].name; i++) {
		if (type == i) {
			UTILITY::tcscpy(buffer, buffer_len, c_gnames[i].name);
			return true;
		}
	}
	return false;
}

bool DISPLAY::debug_draw_graphic(int type, int width, int height, scrntype *buffer)
{

	int size = width * height;
	switch (type) {
	case GN_IG_A:
	case GN_IG_B:
	case GN_IG_R:
	case GN_IG_G:
		// IG
		for(int x=0; x<16; x++) {
		for(int y=0; y<16; y++) {
			for(int li=0; li<8; li++) {
			for(int bt=0; bt<8; bt++) {
				int pos = bt + (y + (li + x * 8) * 16) * 8;
				if (pos < size) {
					int rampos = (y*16 + x) * 8 + li;
					uint8_t msk = (0x80 >> bt);
					buffer[pos]=emu->map_rgbcolor(
							((type & 3) == 0 || (type & 3) == 2) && (ig_ram[rampos+0x0800] & msk) ? 0xff : 0,	// r
							((type & 3) == 0 || (type & 3) == 3) && (ig_ram[rampos+0x1000] & msk) ? 0xff : 0,	// g
							((type & 3) == 0 || (type & 3) == 1) && (ig_ram[rampos] & msk) ? 0xff : 0	// b
						);
				}
			}
			}
		}
		}
		break;
	case GN_GR_A:
	case GN_GR_B:
	case GN_GR_R:
	case GN_GR_G:
		{
			// GVRAM
			int w = (SCRN_MODE_GRAPHIC_WIDTH ? 80 : 40);
			int s = (w == 80 ? 0x800 : 0x400);
			int lim = 8; //(SCRN_MODE_IS_640x400 ? 16 : 8);
			for(int x=0; x<w; x++) {
			for(int y=0; y<25; y++) {
				for(int li=0; li<lim; li++) {
				for(int bt=0; bt<8; bt++) {
					int pos = bt + (x + (y * lim + li) * w) * 8;
					if (pos < size) {
						int rampos = x + y * w + li * s;
						uint8_t msk = (0x80 >> bt);
						buffer[pos]=emu->map_rgbcolor(
								((type & 3) == 0 || (type & 3) == 2) && (s1vgrram[rampos] & msk) ? 0xff : 0,	// r
								((type & 3) == 0 || (type & 3) == 3) && (s1vggram[rampos] & msk) ? 0xff : 0,	// g
								((type & 3) == 0 || (type & 3) == 1) && (s1vgbram[rampos] & msk) ? 0xff : 0	// b
							);
					}
				}
				}
			}
			}
		}
		break;
	default:
		return false;
	}
	return true;
}

bool DISPLAY::debug_dump_graphic(int type, int width, int height, uint16_t *buffer)
{

	int size = width * height;
	switch (type) {
	case GN_IG_A:
	case GN_IG_B:
	case GN_IG_R:
	case GN_IG_G:
		// IG
		for(int x=0; x<16; x++) {
		for(int y=0; y<16; y++) {
			for(int li=0; li<8; li++) {
			for(int bt=0; bt<8; bt++) {
				int pos = bt + (y + (li + x * 8) * 16) * 8;
				if (pos < size) {
					int rampos = (y*16 + x) * 8 + li;
					uint8_t msk = (0x80 >> bt);
					buffer[pos]=(
						  (((type & 3) == 0 || (type & 3) == 2) && (ig_ram[rampos+0x0800] & msk) ? 2 : 0)
						| (((type & 3) == 0 || (type & 3) == 3) && (ig_ram[rampos+0x1000] & msk) ? 4 : 0)
						| (((type & 3) == 0 || (type & 3) == 1) && (ig_ram[rampos] & msk) ? 1 : 0)
						);
				}
			}
			}
		}
		}
		break;
	case GN_GR_A:
	case GN_GR_B:
	case GN_GR_R:
	case GN_GR_G:
		{
			// GVRAM
			int w = (SCRN_MODE_GRAPHIC_WIDTH ? 80 : 40);
			int s = (w == 80 ? 0x800 : 0x400);
			int lim = 8; //(SCRN_MODE_IS_640x400 ? 16 : 8);
			for(int x=0; x<w; x++) {
			for(int y=0; y<25; y++) {
				for(int li=0; li<lim; li++) {
				for(int bt=0; bt<8; bt++) {
					int pos = bt + (x + (y * lim + li) * w) * 8;
					if (pos < size) {
						int rampos = x + y * w + li * s;
						uint8_t msk = (0x80 >> bt);
						buffer[pos]=(
							  (((type & 3) == 0 || (type & 3) == 2) && (s1vgrram[rampos] & msk) ? 2 : 0)
							| (((type & 3) == 0 || (type & 3) == 3) && (s1vggram[rampos] & msk) ? 4 : 0)
							| (((type & 3) == 0 || (type & 3) == 1) && (s1vgbram[rampos] & msk) ? 1 : 0)
							);
					}
				}
				}
			}
			}
		}
		break;
	default:
		return false;
	}
	return true;
}

#endif

