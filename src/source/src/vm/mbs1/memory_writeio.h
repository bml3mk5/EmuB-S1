/** @file memory_writeio.h

	HITACHI MB-S1 Emulator 'EmuB-S1'
	Skelton for retropc emulator

	@author Sasaji
	@date   2016.02.01 -

	@brief [ memory write io ]
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
				addr_map[REG_ADDRSEG][addr & 0xf] = data & 0xff;
			}
			break;
		case 0xefe10:
			// fuse system / user mode
			if (NOW_SYSTEM_MODE) {
				REG_FUSE = data & 0xcf;
#ifdef WRITE_SIGNAL
				d_board->WRITE_SIGNAL(SIG_FUSE_INTR_MASK, data, FUSE_INTR_MASK);
#endif
#ifdef _DEBUG_TRACE_COUNTER
				logging->out_debugf(_T("fuse w: 0x%02x"), REG_FUSE);
#endif
			}
			break;
		case 0xefe11:
			// address segment
			if (IOPORT_USE_OS9BD && NOW_SYSTEM_MODE) {
				REG_ADDRSEG = data & 0x3f;
			}
			break;
		case 0xefe18:
			// trap flag (cannot write)
			break;
		case 0xefe19:
			// bus control
#if defined(USE_Z80B_CARD)
#ifdef WRITE_SIGNAL
			d_z80b_card->WRITE_SIGNAL(SIG_CPU_BUSREQ, data, 0x80);
#endif
#elif defined(USE_MPC_68008)
#ifdef WRITE_SIGNAL
			d_mpc_68008->WRITE_SIGNAL(SIG_CPU_BUSREQ, data, 0x80);
#endif
#else
			REG_BUSCTRL = (data & BUSCTRL_SIGNAL);
#endif
			break;
#if defined(USE_MPC_68008)
		case 0xefe1a:
			// PRO-CONTROL on MPC-68008
		case 0xefe1b:
			// ACC-CONTROL on MPC-68008
			if (!SIG_MBC_IS_ON) {
				d_mpc_68008->WRITE_MEMORY_MAPPED_IO8(addr, data);
			}
			break;
#endif
		case 0xefe20:
		case 0xefe21:
		case 0xefe22:
		case 0xefe23:
		case 0xefe24:
		case 0xefe25:
		case 0xefe26:
		case 0xefe27:
		case 0xefe28:
		case 0xefe29:
		case 0xefe2a:
		case 0xefe2b:
		case 0xefe2c:
		case 0xefe2d:
		case 0xefe2e:
		case 0xefe2f:
			// graphic related register
			d_disp->WRITE_IO8(addr, data);
			break;
		case 0xefe40:
		case 0xefe41:
			// pia a port (joystick)
			d_pia->WRITE_IO8(addr - 0xefe40, data);
			break;
		case 0xefe42:
		case 0xefe43:
			// pia b (printer)
			d_pia->WRITE_IO8(addr - 0xefe40, data);
			break;
	}

#if !defined(DEBUG_READ_OK) && (defined(USE_Z80B_CARD) || defined(USE_MPC_68008))
	// If MBC is on, disable accessing to common I/O on extended interface.
	if (SIG_MBC_IS_ON && addr_comio < 0xeffc0) {
		break;
	}
#endif

	// memory mapped i/o (common area : 0xeff00 - 0xeffef, 0xffe00 - 0xfffef)
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
				d_fdd->WRITE_IO8(addr, data);
			}
			break;
		case 0xeff08:
			// halt (1MB fdd only)
			if (IOPORT_USE_5FDD) {
				d_fdd->WRITE_IO8(addr, data);
			}
			break;
		case 0xeff0c:
			// type sel (cannot write)
			break;
		case 0xeff10:
		case 0xeff11:
		case 0xeff12:
		case 0xeff13:
			// fdc (ex)
		case 0xeff14:
			// fdd (ex) drive select
			if (IOPORT_USE_5FDD) {
				d_fdd->WRITE_IO8(addr, data);
			}
			break;
		case 0xeff16:
			// fm opn
			if (IOPORT_USE_FMOPN && config.type_of_fmopn >= Config::CHIP_YM2608_4MHZ) {
				d_fmopn->WRITE_IO8(3, data);

//				logging->out_debugf("fmopn a:%04x d:%02x",addr,data);
			}
			break;
		case 0xeff17:
			// fm opn
			if (IOPORT_USE_FMOPN && config.type_of_fmopn >= Config::CHIP_YM2608_4MHZ) {
				d_fmopn->WRITE_IO8(2, data);

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
				d_fdc3->WRITE_IO8(addr - 0xeff18, data);

//				logging->out_debugf("fdc3w a:%04x d:%02x",addr,data);
			}
			break;
		case 0xeff1e:
			// fdc
			if (IOPORT_USE_3FDD) {
				d_fdc3->WRITE_IO8(addr - 0xeff18, data);

//				logging->out_debugf("fdc3w a:%04x d:%02x",addr,data);
			}
			// fm opn
			if (IOPORT_USE_FMOPN) {
				d_fmopn->WRITE_IO8(1, data);

//				logging->out_debugf("fmopn a:%04x d:%02x",addr,data);
			}
			break;
		case 0xeff1f:
			// fdc
			if (IOPORT_USE_3FDD) {
				d_fdc3->WRITE_IO8(addr - 0xeff18, data);

//				logging->out_debugf("fdc3w a:%04x d:%02x",addr,data);
			}
			// fm opn
			if (IOPORT_USE_FMOPN) {
				d_fmopn->WRITE_IO8(0, data & (config.type_of_fmopn >= Config::CHIP_YM2203_2MHZ ? 0xff : 0x0f));

//				logging->out_debugf("fmopn a:%04x d:%02x",addr,data);
			}
			break;
		case 0xeff20:
			// fdd drive select
			if (IOPORT_USE_3FDD) {
				d_fdd->WRITE_IO8(0, data);

//				logging->out_debugf("fdd3w a:%04x d:%02x",addr,data);
			}
			break;
#endif
		case 0xeff30:
		case 0xeff31:
		case 0xeff32:
		case 0xeff33:
			// psg exboard
			if (IOPORT_USE_PSG6) {
				d_psg3->WRITE_IO8(addr - 0xeff30, data);
//				logging->out_debugf("psgw a:%04x d:%02x",addr,data);
			}
			break;
#ifdef USE_RTC
		case 0xeff38:
		case 0xeff39:
			// rtc
			if (IOPORT_USE_RTC) {
				d_rtc->WRITE_IO8(addr & 3, data);
			}
			break;
#endif
		case 0xeff3c:
		case 0xeff3d:
		case 0xeff3e:
		case 0xeff3f:
			// extended LPT board
			if (IOPORT_USE_EXPIA) {
				d_pia_ex->WRITE_IO8(addr - 0xeff3c, data);
			}
			break;

		case 0xeff40:
			// extended COM board
			if (IOPORT_USE_EXACIA) {
				d_comm1->WRITE_IO8(addr - 0xeff40, data);
			}
			// through
		case 0xeff41:
			// extended COM board
			if (IOPORT_USE_EXACIA) {
				d_acia_ex->WRITE_IO8(addr - 0xeff40, data);
			}
			break;
		case 0xeff70:
		case 0xeff71:
		case 0xeff72:
		case 0xeff73:
		case 0xeff74:
		case 0xeff78:
		case 0xeff79:
		case 0xeff7a:
		case 0xeff7b:
		case 0xeff7c:
		case 0xeff7d:
		case 0xeff7e:
			// psg9
			if (IOPORT_USE_PSG9) {
				d_psg9->WRITE_IO8(addr - 0xeff70, data);
//				logging->out_debugf("psg9w a:%04x d:%02x",addr,data);
			}
			break;
		case 0xeff75:
		case 0xeff76:
			// psg9
			if (IOPORT_USE_PSG9) {
				d_psg9->WRITE_IO8(addr - 0xeff70, data);
//				logging->out_debugf("psg9w a:%04x d:%02x",addr,data);
			}
			// kanji rom
			if (IOPORT_USE_KANJI) {
				d_kanji->WRITE_IO8(addr - 0xeff75, data);
//				logging->out_debugf("kanjiw a:%04x d:%02x",addr,data);
			}
			break;
		case 0xeff77:
			// psg9
			if (IOPORT_USE_PSG9) {
				d_psg9->WRITE_IO8(addr - 0xeff70, data);
//				logging->out_debugf("psg9w a:%04x d:%02x",addr,data);
			}
			// communication rom
			if (IOPORT_USE_CM01) {
				cm01rom_access_ok = ((data & 0xf) == 1);
				change_cm01_membank();
			}
			break;
		case 0xeff7f:
			// psg9
			if (IOPORT_USE_PSG9) {
				d_psg9->WRITE_IO8(addr - 0xeff70, data);
//				logging->out_debugf("psg9w a:%04x d:%02x",addr,data);
			}
#if defined(USE_Z80B_CARD)
			d_z80b_card->WRITE_MEMORY_MAPPED_IO8(addr, data);
#endif
			break;

		case 0xeffc0:
			// membank (L3 only)
			if (!mem_bank_mask) {
				mem_bank_reg1 = data & 0xce;
				change_l3_memory_bank();
			}
			break;

		case 0xeffc1:
			mem_bank_mask = ((data & 4) == 0);
			break;

		case 0xeffc2:
		case 0xeffc3:
			// pia b (printer)
			d_pia->WRITE_IO8(addr - 0xeffc0, data);
			break;

		case 0xeffc4:
			// acia
			d_comm->WRITE_IO8(addr - 0xeffc4, data);
			// pass through
		case 0xeffc5:
			// acia
			d_acia->WRITE_IO8(addr - 0xeffc4, data);
			break;

		case 0xeffc6:
		case 0xeffc7:
			// crtc
			d_crtc->WRITE_IO8(addr - 0xeffc6, data);
//			d_disp->WRITE_IO8(addr, data);
			break;

		case 0xeffc8:
			// keyborad nmi (cannot write)
			break;
		case 0xeffca:
			// timer irq
			break;
		case 0xeffcb:
			// light pen flag (cannot write)
			break;
		case 0xeffd0:
			// mode select
			REG_MODE_SEL = (data & 0xff);
			d_disp->WRITE_IO8(addr, data);
			d_comm->write_signal(COMM::SIG_COMM_RS, data, 0x20);
			break;
		case 0xeffd1:
			// trace counter
#ifdef WRITE_SIGNAL
			fetch_trace_counter(*wait);
#endif
			break;
		case 0xeffd2:
			// remote switch
			d_cmt->write_signal(CMT::SIG_CMT_REMOTE, data, 0x80);
			break;
		case 0xeffd3:
			// music sel
//			logging->out_debugf("ms %02x",data);
			d_sound->write_signal(SOUND::SIG_SOUND_ON, 1, 1);
			d_sound->write_signal(SOUND::SIG_SOUND_SIGNAL, data, 0x80);
			break;
		case 0xeffd4:
			// time mask
			d_timer->WRITE_IO8(addr, data & 0x83);
			break;
		case 0xeffd5:
			// light pen bl
//			logging->out_debugf(_T("lpengl: %02x"),data);
			d_key->WRITE_IO8(addr, data & 0x80);
			break;
		case 0xeffd6:
			// interace sel
			d_disp->WRITE_IO8(addr, data & 0x08);
			break;
		case 0xeffd7:
			// baud sel
			d_cmt->write_signal(CMT::SIG_CMT_BAUD_SEL, data, 0x0f);
			break;
		case 0xeffd8:
			// color register
#ifdef _DEBUG_CRAM
			logging->out_debugf("cw c%02x->%02x",color_reg,data);
#endif
			color_reg = (data & 0xff);
			break;
		case 0xeffd9:
			// trq-sel casette control
			// not implemented
			break;
		case 0xeffdc:
		case 0xeffdd:
		case 0xeffde:
		case 0xeffdf:
			// mouse
			if (IOPORT_USE_MOUSE) {
				d_mouse->WRITE_IO8(addr, data);
			}
			break;
		case 0xeffe0:
			// keyboard mode register
			d_key -> WRITE_IO8(addr, data);
			break;
		case 0xeffe1:
			// kb type (cannot write)
			break;
		case 0xeffe2:
			// tv super inpose
			REG_TVSUPER = (data & 0x17);
			break;
		case 0xeffe4:
			// psg data
			d_psgst->WRITE_IO8(1, data & 0xff);
//			logging->out_debugf("psgw dat:%02x",data);
			break;
		case 0xeffe5:
			// psg reg
			d_psgst->WRITE_IO8(0, data & 0x0f);
//			logging->out_debugf("psgw reg:%02x",data);
			break;
		case 0xeffe6:
			// ex psg data
			if (IOPORT_USE_EXPSG) {
				d_psgex->WRITE_IO8(1, data);
//				logging->out_debugf("psg2w dat:%02x",data);
			}
			break;
		case 0xeffe7:
			// ex psg reg
			if (IOPORT_USE_EXPSG) {
				d_psgex->WRITE_IO8(0, data & (config.type_of_expsg >= Config::CHIP_YM2203_2MHZ ? 0xff : 0x0f));
//				logging->out_debugf("psg2w reg:%02x",data);
			}
			break;
		case 0xeffe8:
			// bank register
			mem_bank_reg2 = data & 0xff;
			change_l3_memory_bank();
			break;
		case 0xeffe9:
			// IG mode register
			set_igmode(0x100 | data);
			break;
		case 0xeffea:
			// IG en register
			ig_enreg = data & 0x07;
			break;
		case 0xeffeb:
			// system mode
			set_cpu_speed(data & 0xff);
			break;
		case 0xeffee:
			// YM2608 mode on ex psg 
			if (IOPORT_USE_EXPSG && config.type_of_expsg >= Config::CHIP_YM2608_4MHZ) {
				d_psgex->WRITE_IO8(3, data);
//				logging->out_debugf("psg2w dat:%02x",data);
			}
			break;
		case 0xeffef:
			// YM2608 mode on ex psg reg
			if (IOPORT_USE_EXPSG && config.type_of_expsg >= Config::CHIP_YM2608_4MHZ) {
				d_psgex->WRITE_IO8(2, data);
//				logging->out_debugf("psg2w reg:%02x",data);
			}
			break;
	}
} while(0);
