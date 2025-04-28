/** @file memory_readio.h

	HITACHI MB-S1 Emulator 'EmuB-S1'
	Skelton for retropc emulator

	@author Sasaji
	@date   2016.02.01 -

	@brief [ memory read io ]
*/

do {
	// memory mapped i/o (s1 only area : 0xefe00 - 0xefeff)
	switch(addr & 0xfffff) {
		case 0xefe00:
		case 0xefe01:
		case 0xefe02:
		case 0xefe03:
		case 0xefe04:
		case 0xefe05:
		case 0xefe06:
		case 0xefe07:
		case 0xefe08:
		case 0xefe09:
		case 0xefe0a:
		case 0xefe0b:
		case 0xefe0c:
		case 0xefe0d:
		case 0xefe0e:
		case 0xefe0f:
			// address mapping
			if (NOW_SYSTEM_MODE) {
				data = addr_map[REG_ADDRSEG][addr & 0xf];
			}
			break;
		case 0xefe10:
			// fuse (cannot read)
#ifdef DEBUG_READ_OK
			data = REG_FUSE;
#endif
			break;
		case 0xefe11:
			// segment (cannot read)
#ifdef DEBUG_READ_OK
			if (IOPORT_USE_OS9BD) {
				data = REG_ADDRSEG;
			}
#endif
			break;
		case 0xefe18:
			// trap flag
			data = (REG_TRAPF & TRAPF_SIGNAL);
#ifdef WRITE_SIGNAL
			REG_TRAPF &= ~TRAPF_SIGNAL;
//			d_board->WRITE_SIGNAL(SIG_CPU_NMI, 0, SIG_NMI_TRAP_MASK);
#endif
			break;
		case 0xefe19:
			// bus control
			data = (REG_BUSCTRL | ~BUSCTRL_SIGNAL) & 0xff;
			break;
#if defined(USE_MPC_68008)
		case 0xefe1a:
			// PRO-CONTROL on MPC-68008
			if (!SIG_MBC_IS_ON) {
				data = d_mpc_68008->READ_MEMORY_MAPPED_IO8(addr);
			}
			break;
#endif
		case 0xefe20:
			// bmsk color graphic color (cannot read)
#ifdef DEBUG_READ_OK
			data = REG_BMSK_COLOR;
#endif
			break;
		case 0xefe21:
			// active graphic plane (cannot read)
#ifdef DEBUG_READ_OK
			data = REG_ACTIVE_PLANE;
#endif
			break;
		case 0xefe23:
			// display page (cannot read)
#ifdef DEBUG_READ_OK
			data = REG_DISP_PAGE;
#endif
			break;
		case 0xefe24:
			// screen page (cannot read)
#ifdef DEBUG_READ_OK
			data = REG_SCRN_MODE;
#endif
			break;
		case 0xefe25:
		case 0xefe26:
		case 0xefe27:
			// graphic color
			data = REG_GCOLOR[addr - 0xefe25];
			break;
		case 0xefe28:
		case 0xefe29:
		case 0xefe2a:
		case 0xefe2b:
		case 0xefe2c:
		case 0xefe2d:
		case 0xefe2e:
		case 0xefe2f:
			// palette (cannot read, but send halt signal when disptmg is on)
#ifndef DEBUG_READ_OK
			data = d_disp->READ_IO8(addr);
#else
			data = REG_PALETTE[addr & 0x7];
#endif
			break;
		case 0xefe40:
		case 0xefe41:
			// pia a port (joystick)
			data = d_pia->READ_IO8(addr - 0xefe40);
			break;
		case 0xefe42:
		case 0xefe43:
			// pia b (printer)
			data = d_pia->READ_IO8(addr - 0xefe40);
			break;
	}

#if !defined(DEBUG_READ_OK) && (defined(USE_Z80B_CARD) || defined(USE_MPC_68008))
	// If MBC is on, disable accessing to common I/O on extended interface.
	if (SIG_MBC_IS_ON && addr_comio < 0xeffc0) {
		break;
	}
#endif

	// memory mapped i/o (common area : 0xeff00 - 0xeffef, 0xfff00 - 0xfffef)
	switch(addr_comio) {
#ifdef USE_FD1
		case 0xeff00:
		case 0xeff01:
		case 0xeff02:
		case 0xeff03:
			// fdc
		case 0xeff04:
			// fdd drive select
			if (IOPORT_USE_5FDD) {
				data = d_fdd->READ_IO8(addr);
			}
			break;
		case 0xeff08:
			// now halt (1MB fdd only) (cannot read)
			break;
		case 0xeff0c:
			// type sel (1MB fdd only)
			if (IOPORT_USE_5FDD) {
				data = d_fdd->READ_IO8(addr);
			}
			break;
		case 0xeff10:
		case 0xeff11:
		case 0xeff12:
		case 0xeff13:
			// fdc (ex)
		case 0xeff14:
			// fdd (ex) drive select
			if (IOPORT_USE_5FDD) {
				data = d_fdd->READ_IO8(addr);
			}
			break;
		case 0xeff16:
			// fm opn
			if (IOPORT_USE_FMOPN && pConfig->type_of_fmopn >= Config::CHIP_YM2608_4MHZ) {
				data = d_fmopn->READ_IO8(3);
//				logging->out_debugf("fmopn a:%04x d:%02x",addr,data);
			}
			break;
		case 0xeff17:
			// fm opn
			if (IOPORT_USE_FMOPN && pConfig->type_of_fmopn >= Config::CHIP_YM2608_4MHZ) {
				data = d_fmopn->READ_IO8(2);
//				logging->out_debugf("fmopn a:%04x d:%02x",addr,data);
			}
			break;
		case 0xeff18:
		case 0xeff19:
		case 0xeff1a:
		case 0xeff1b:
		case 0xeff1c:
		case 0xeff1d:
			// fdc
			if (IOPORT_USE_3FDD) {
				data = d_fdc3->READ_IO8(addr - 0xeff18);

//				logging->out_debugf("fdc3r a:%04x d:%02x",addr,data);
			}
			break;
		case 0xeff1e:
			// fdc
			if (IOPORT_USE_3FDD) {
				data = d_fdc3->READ_IO8(addr - 0xeff18);

//				logging->out_debugf("fdc3r a:%04x d:%02x",addr,data);
			}
			// fm opn
			if (IOPORT_USE_FMOPN) {
				data = d_fmopn->READ_IO8(1);
//				logging->out_debugf("fmopn a:%04x d:%02x",addr,data);
			}
			break;
		case 0xeff1f:
			// fdc
			if (IOPORT_USE_3FDD) {
				data = d_fdc3->READ_IO8(addr - 0xeff18);

//				logging->out_debugf("fdc3r a:%04x d:%02x",addr,data);
			}
			// fm opn (cannot read on PSG mode)
			if (IOPORT_USE_FMOPN && pConfig->type_of_fmopn >= Config::CHIP_YM2203_2MHZ) {
				data = d_fmopn->READ_IO8(0);
//				logging->out_debugf("fmopn a:%04x d:%02x",addr,data);
			}
			break;
		case 0xeff20:
		case 0xeff21:
		case 0xeff22:
		case 0xeff23:
		case 0xeff24:
		case 0xeff25:
		case 0xeff26:
		case 0xeff27:
			// fdd drive select
			if (IOPORT_USE_3FDD) {
				data = d_fdd->READ_IO8(0);

//				logging->out_debugf("fdd3r a:%04x d:%02x",addr,data);
			}
			break;
#endif
		case 0xeff30:
		case 0xeff31:
		case 0xeff32:
		case 0xeff33:
			// psg exboard
			if (IOPORT_USE_PSG6) {
				data = d_psg3->READ_IO8(addr - 0xeff30);
//				logging->out_debugf("psgr a:%04x d:%02x",addr,data);
			}
			break;
#ifdef USE_RTC
		case 0xeff3a:
			// rtc read
			if (IOPORT_USE_RTC) {
				data = d_rtc->READ_IO8(addr & 3);
			}
			break;
#endif
		case 0xeff3c:
		case 0xeff3d:
		case 0xeff3e:
		case 0xeff3f:
			// extended LPT board
			if (IOPORT_USE_EXPIA) {
				data = d_pia_ex->READ_IO8(addr - 0xeff3c);
			}
			break;

		case 0xeff40:
		case 0xeff41:
			// extended COM board
			if (IOPORT_USE_EXACIA) {
				data = d_acia_ex->READ_IO8(addr - 0xeff40);
			}
			break;
		case 0xeff70:
		case 0xeff71:
		case 0xeff78:
		case 0xeff79:
		case 0xeff7a:
		case 0xeff7b:
		case 0xeff7c:
		case 0xeff7d:
		case 0xeff7e:
			// psg9
			if (IOPORT_USE_PSG9) {
				data = d_psg9->READ_IO8(addr - 0xeff70);
//				logging->out_debugf("psg9r a:%04x d:%02x",addr,data);
			}
			break;
		case 0xeff72:
		case 0xeff73:
		case 0xeff74:
			// psg9
			if (IOPORT_USE_PSG9) {
				data = d_psg9->READ_IO8(addr - 0xeff70);
//				logging->out_debugf("psg9r a:%04x d:%02x",addr,data);
			}
			// kanji rom (JIS2)
			if (IOPORT_USE_KANJI2) {
				data = d_kanji->READ_IO8(addr - 0xeff70);
//				logging->out_debugf("kanjir a:%04x d:%02x",addr,data);
			}
			break;
		case 0xeff75:
		case 0xeff76:
			// psg9
			if (IOPORT_USE_PSG9) {
				data = d_psg9->READ_IO8(addr - 0xeff70);
//				logging->out_debugf("psg9r a:%04x d:%02x",addr,data);
			}
			// kanji rom
			if (IOPORT_USE_KANJI) {
				data = d_kanji->READ_IO8(addr - 0xeff70);
//				logging->out_debugf("kanjir a:%04x d:%02x",addr,data);
			}
			break;
		case 0xeff77:
			// psg9
			if (IOPORT_USE_PSG9) {
				data = d_psg9->READ_IO8(addr - 0xeff70);
//				logging->out_debugf("psg9r a:%04x d:%02x",addr,data);
			}
			// communication rom
			if (IOPORT_USE_CM01 && cm01rom_access_ok) {
				data = (cm01eeprom_writing ? 0x0e : 0x8e);
				cm01eeprom_writing = false;
			}
			break;
		case 0xeff7f:
			// psg9
			if (IOPORT_USE_PSG9) {
				data = d_psg9->READ_IO8(addr - 0xeff70);
//				logging->out_debugf("psg9r a:%04x d:%02x",addr,data);
			}
#if defined(USE_Z80B_CARD)
			data = d_z80b_card->READ_MEMORY_MAPPED_IO8(addr);
#endif
			break;

		case 0xeffc0:
			// membank (L3 mode)
			data = mem_bank_reg1;
			break;

		case 0xeffc1:
			break;

		case 0xeffc2:
		case 0xeffc3:
			// pia b (printer)
			data = d_pia->READ_IO8(addr - 0xeffc0);
			break;

		case 0xeffc4:
		case 0xeffc5:
			// acia
			data = d_acia->READ_IO8(addr - 0xeffc4);
			break;

		case 0xeffc6:
		case 0xeffc7:
			// crtc
			data = d_crtc->READ_IO8(addr - 0xeffc6);
			break;

		case 0xeffc8:
			// keyboard nmi
			data = d_key->READ_IO8(addr);
			break;
		case 0xeffc9:
			// newon number
			data = pConfig->dipswitch;
			break;
		case 0xeffca:
			// timer irq
			data = d_timer->READ_IO8(addr);
			break;
		case 0xeffcb:
			// light pen flag 
			data = d_key->READ_IO8(addr);
			break;
		case 0xeffce:
			// disable booting from rom basic (Limelight)
			data = IOPORT_USE_DISROMB ? 0x7f : 0xff;
			break;
		case 0xeffd0:
			// mode select
#ifdef DEBUG_READ_OK
			data = REG_MODE_SEL;
#endif
			break;
		case 0xeffd1:
			// trace counter
			break;
		case 0xeffd2:
			// remote switch
#ifdef DEBUG_READ_OK
			data = REG_REMOTE;
#endif
			break;
		case 0xeffd3:
			// music sel
#ifdef DEBUG_READ_OK
			data = REG_MUSIC_SEL;
#endif
			break;
		case 0xeffd4:
			// time mask
#ifdef DEBUG_READ_OK
			data = REG_TIME_MASK;
#endif
			break;
		case 0xeffd5:
			// light pen bl
			break;
		case 0xeffd6:
			// interace sel
#ifdef DEBUG_READ_OK
			data = (REG_INTERLACE_SEL << 3);
#endif
			break;
		case 0xeffd7:
			// baud sel
#ifdef DEBUG_READ_OK
			data = REG_BAUD_SEL;
#endif
			break;
		case 0xeffd8:
			// color register sel
#ifdef _DEBUG_CRAM
			logging->out_debugf("cr c%02x",color_reg);
#endif
			data = (color_reg & 0xff);
			break;
		case 0xeffd9:
			// trq-sel casette control
			data = 0xf;
			break;
		case 0xeffdc:
		case 0xeffdd:
		case 0xeffde:
		case 0xeffdf:
			// mouse
			if (IOPORT_USE_MOUSE) {
				data = d_mouse->READ_IO8(addr);
			}
			break;
		case 0xeffe0:
			// keyboard scan data
			data = d_key->READ_IO8(addr);
			break;
		case 0xeffe1:
			// kb type
			data = (IOPORT_USE_KEYBD ? 1 : 0);
			break;
		case 0xeffe2:
			// tv super inpose (cannot read)
#ifdef DEBUG_READ_OK
			data = REG_TVSUPER;
#endif
			break;
		case 0xeffe4:
			// psg data
			data = d_psgst->READ_IO8(1);
			break;
		case 0xeffe5:
			// psg reg (cannot read)
			break;
		case 0xeffe6:
			// ex psg data
			if (IOPORT_USE_EXPSG) {
				data = d_psgex->READ_IO8(1);
			}
			break;
		case 0xeffe7:
			// ex psg reg (cannot read except OPN mode)
			if (IOPORT_USE_EXPSG && pConfig->type_of_expsg >= Config::CHIP_YM2203_2MHZ) {
				data = d_psgex->READ_IO8(0);
			}
			break;
		case 0xeffe8:
			// bank register
#ifdef DEBUG_READ_OK
			data = mem_bank_reg2;
#endif
			break;
		case 0xeffe9:
			// IG mode register timing status
			data = REG_IGMODE;
			break;
		case 0xeffea:
			// IG en register
			break;
		case 0xeffeb:
			// system mode
			data = REG_SYS_MODE & 0x07;
			break;
		case 0xeffee:
			// YM2608 mode on ex psg
			if (IOPORT_USE_EXPSG && pConfig->type_of_expsg >= Config::CHIP_YM2608_4MHZ) {
				data = d_psgex->READ_IO8(3);
			}
			break;
		case 0xeffef:
			// YM2608 mode on ex psg
			if (IOPORT_USE_EXPSG && pConfig->type_of_expsg >= Config::CHIP_YM2608_4MHZ) {
				data = d_psgex->READ_IO8(2);
			}
	}
} while(0);
