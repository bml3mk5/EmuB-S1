/** @file mbs1.cpp

	HITACHI MB-S1 Emulator 'EmuB-S1'
	Skelton for retropc emulator

	@author Sasaji
	@date   2011.06.08 -

	@brief [ virtual machine ]
*/

#include "mbs1.h"
#include "../../config.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../hd46505.h"
#include "../ay38910.h"
#include "../ym2203.h"
#include "../mc6809.h"

#include "board.h"
#include "../pia.h"
#include "../acia.h"
#include "display.h"
#include "keyboard.h"
#include "mouse.h"
#include "memory.h"
#include "sound.h"
#include "comm.h"
#include "cmt.h"
#include "printer.h"
#include "timer.h"
#include "kanji.h"
#include "psgc.h"
#include "../via.h"
#include "psg9c.h"
#ifdef USE_FD1
#include "../mc6843.h"
#include "../mb8866.h"
#include "floppy.h"
#endif
#ifdef USE_RTC
#include "rtc.h"
#include "../msm58321.h"
#endif
#include "keyrecord.h"
#ifdef USE_DEBUGGER
#include "../debugger.h"
#include "../mc6809dasm.h"
#endif
#if defined(USE_Z80B_CARD)
#include "z80b_card.h"
#include "../z80.h"
#elif defined(USE_MPC_68008)
#include "mpc_68008.h"
#include "../mc68000.h"
#endif
#include "../../utility.h"
#include "../../version.h"

// ----------------------------------------------------------------------------

//const int fmopn_clocks[] = {
//	3579545,			// 3.58MHz
//	CLOCKS_1MHZ,		// 1MHz
//	CLOCKS_1MHZ * 2,	// 2MHz
//	CLOCKS_1MHZ * 4,	// 4MHz
//	-1
//};

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU* parent_emu) : emu(parent_emu)
{
	//
	emu->set_parami(ParamFddType, pConfig->fdd_type);
	emu->set_parami(ParamIOPort, pConfig->io_port);
	emu->set_parami(ParamSysMode, pConfig->sys_mode);
	emu->set_parami(ParamExMemNum, pConfig->exram_size_num);
	emu->set_parami(ParamChipTypeOnFmOpn, pConfig->type_of_fmopn);
	emu->set_parami(ParamChipTypeOnExPsg, pConfig->type_of_expsg);

	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu, NULL);	// must be 1st device
	event = new EVENT(this, emu, NULL);	// must be 2nd device
	event->initialize();		// must be initialized first

	crtc = new HD46505(this, emu, NULL);
	psgst = new AY38910(this, emu, "P01");
//	psgex = new AY38910(this, emu, "P02");
	psgex = new YM2203(this, emu, "P02");
	psg3[0] = new AY38910(this, emu, "P31");
	psg3[1] = new AY38910(this, emu, "P32");
	psg9[0] = new AY38910(this, emu, "P91");
	psg9[1] = new AY38910(this, emu, "P92");
	psg9[2] = new AY38910(this, emu, "P93");
	fmopn = new YM2203(this, emu, "F01");

	board = new BOARD(this, emu, NULL);
	pia = new PIA(this, emu, "MA");
	acia = new ACIA(this, emu, "MA");
	display = new DISPLAY(this, emu, NULL);
	key = new KEYBOARD(this, emu, NULL);
	mouse = new MOUSE(this, emu, NULL);
	memory = new MEMORY(this, emu, NULL);
	sound = new SOUND(this, emu, NULL);
	comm[0] = new COMM(this, emu, NULL, 0);
	comm[1] = new COMM(this, emu, "EX1", 1);
	cmt = new CMT(this, emu, NULL);
	printer[0] = new PRINTER(this, emu, NULL, 0);
	printer[1] = new PRINTER(this, emu, "EX1", 1);
	printer[2] = new PRINTER(this, emu, "EX2", 2);
	timer = new TIMER(this, emu, NULL);
	kanji = new KANJI(this, emu, NULL);
	psg3_pia = new PIA(this, emu, "P31");
	psg3c = new PSGC(this, emu, "P31");
	psg9_via = new VIA(this, emu, "P91");
	psg9c = new PSG9C(this, emu, "P91");
#ifdef USE_FD1
	fdc3 = new MC6843(this, emu, NULL);
	fdc5 = new MB8866(this, emu, NULL);
	fdc5ex = new MB8866(this, emu, "EX1");
	fdd = new FLOPPY(this, emu, NULL);
#endif
	pia_ex = new PIA(this, emu, "EX1");
	acia_ex = new ACIA(this, emu, "EX1");
#ifdef USE_RTC
	rtc = new RTC(this, emu, NULL);
	msm58321 = new MSM58321(this, emu, NULL);
#endif
#if defined(USE_Z80B_CARD)
	z80b_card = new Z80B_CARD(this, emu, "B");
	z80 = new Z80(this, emu, "B");
#elif defined(USE_MPC_68008)
	mpc_68008 = new MPC_68008(this, emu, "B");
	mc68k = new MC68008(this, emu, "B");
#endif

	//
	cpu = new MC6809(this, emu, NULL);

#ifdef USE_KEY_RECORD
	reckey = new KEYRECORD(emu);
#endif

	// set contexts
	event->set_context_cpu(cpu, CPU_CLOCKS);
#if defined(USE_Z80B_CARD)
	event->set_context_cpu(z80, Z80B_CLOCKS);
#elif defined(USE_MPC_68008)
	event->set_context_cpu(mc68k, MC68K_CLOCKS);
#endif
	event->set_context_sound(sound);
	event->set_context_sound(psgst);
	event->set_context_sound(psgex);
	event->set_context_sound(psg3[0]);
	event->set_context_sound(psg3[1]);
	event->set_context_sound(psg9[0]);
	event->set_context_sound(psg9[1]);
	event->set_context_sound(psg9[2]);
	event->set_context_sound(cmt);
	event->set_context_sound(fdd);
	event->set_context_sound(fmopn);
	event->set_context_display(display);
#ifdef USE_KEY_RECORD
	reckey->set_context_event(event);
#endif

	// crtc
//	crtc->set_context_vsync(timer, TIMER::SIG_TIMER_VSYNC, 1);
	crtc->set_context_vsync(display, DISPLAY::SIG_DISPLAY_VSYNC, 1);
	crtc->set_context_hsync(display, DISPLAY::SIG_DISPLAY_HSYNC, 1);
	crtc->set_context_disp(display, DISPLAY::SIG_DISPLAY_DISPTMG, 1);
//	crtc->set_context_hsync(key, KEYBOARD::SIG_KEYBOARD_HSYNC, 1);
	crtc->set_context_write_regs(display, DISPLAY::SIG_DISPLAY_WRITE_REGS, 0x0f);

	// display
	display->set_context_crtc(crtc);
	display->set_context_board(board);
	display->set_s1vtram_ptr(memory->get_s1vtram());
	display->set_s1vcram_ptr(memory->get_s1vcram());
	display->set_s1vgrram_ptr(memory->get_s1vgrram());
	display->set_s1vggram_ptr(memory->get_s1vggram());
	display->set_s1vgbram_ptr(memory->get_s1vgbram());
	display->set_l3vram_ptr(memory->get_l3vram());
	display->set_color_ram_ptr(memory->get_color_ram());
	display->set_ig_ram_ptr(memory->get_ig_ram());
	display->set_regs_ptr(crtc->get_regs());
	display->set_crtc_vt_ptr(crtc->get_vt_total_ptr(), crtc->get_vt_count_ptr(), crtc->get_vt_disp_ptr());
	display->set_crtc_ma_ra_ptr(crtc->get_ma_ptr(),crtc->get_ra_ptr());
	display->set_crtc_max_ra_ptr(crtc->get_max_ra_ptr());
	display->set_crtc_odd_line_ptr(crtc->get_video_odd_line_ptr());
	display->set_crtc_reg8_ptr(crtc->get_videomode_ptr(), crtc->get_disptmg_skew_ptr(), crtc->get_curdisp_skew_ptr());
	display->set_crtc_curdisp_ptr(crtc->get_curdisp_ptr());
	display->set_crtc_vsync_ptr(crtc->get_vs_start_ptr(), crtc->get_vs_end_ptr());

	// keyboard
	key->set_context_cpu(cpu);
	key->set_context_disp(display);
	key->set_context_board(board);
#ifdef USE_KEY_RECORD
	key->set_keyrecord(reckey);
//	reckey->set_context(key);
#endif
	key->set_context_pia(pia);

	// mouse
	mouse->set_context_board(board);
#ifdef USE_KEY_RECORD
	mouse->set_keyrecord(reckey);
#endif

	// memory
//	memory->set_context_cpu(cpu);
#if defined(USE_Z80B_CARD)
//	memory->set_context_cpu(z80);
#elif defined(USE_MPC_68008)
//	memory->set_context_cpu(mc68k);
#endif
	memory->set_context_pia(pia);
	memory->set_context_acia(acia);
	memory->set_context_crtc(crtc);
	memory->set_context_display(display);
	memory->set_context_key(key);
	memory->set_context_mouse(mouse);
	memory->set_context_sound(sound);
	memory->set_context_psg(psgst, psgex);
	memory->set_context_psg3(psg3_pia);
	memory->set_context_psg9(psg9_via);
	memory->set_context_fmopn(fmopn);
	memory->set_context_comm(comm[0]);
	memory->set_context_cmt(cmt);
	memory->set_context_timer(timer);
	memory->set_context_kanji(kanji);
#ifdef USE_FD1
	memory->set_context_fdc(fdc3, fdc5, fdc5ex);
	memory->set_context_fdd(fdd);
#endif
	memory->set_context_pia_ex(pia_ex);
	memory->set_context_acia_ex(acia_ex);
	memory->set_context_comm1(comm[1]);
#ifdef USE_RTC
	memory->set_context_rtc(rtc);
#endif
	memory->set_context_board(board);
#if defined(USE_Z80B_CARD)
	memory->set_context_z80bcard(z80b_card);
#elif defined(USE_MPC_68008)
	memory->set_context_mpc_68008(mpc_68008);
#endif
	memory->set_context_event(event);

	memory->set_font(display->get_font());

	// cpu bus
	cpu->set_context_mem(memory);
#if defined(USE_Z80B_CARD)
	z80->set_context_mem(z80b_card);
	z80->set_context_io(z80b_card);
	z80->set_context_busack(z80b_card, SIG_Z80BCARD_BUSACK, 1);
#elif defined(USE_MPC_68008)
	mc68k->set_context_mem(M68K_FC_SUPERVISOR_PROGRAM, mpc_68008);
	mc68k->set_context_bg(mpc_68008, MPC_68008::SIG_BGACK, 0xffffffff);
	mc68k->set_context_fc(mpc_68008, SIG_M68K_FC, 0xffffffff);
#endif
#ifdef USE_DEBUGGER
//	debugger = new DEBUGGER(this, emu, NULL);
	cpu->set_context_debugger(new DEBUGGER(this, emu, NULL));
#if defined(USE_Z80B_CARD)
	z80->set_context_debugger(new DEBUGGER(this, emu, "B"));
#elif defined(USE_MPC_68008)
	DEBUGGER *m68k_dbgr = new DEBUGGER(this, emu, "B");
	m68k_dbgr->set_context_mem(mpc_68008);
	mc68k->set_context_debugger(m68k_dbgr);
#endif
#endif

	// acia
	acia->set_context_txdata(comm[0], ACIA::SIG_ACIA_TXDATA, 0xff);
	acia->set_context_rts(comm[0], ACIA::SIG_ACIA_RTS, 1);
	acia->set_context_dtr(comm[0], ACIA::SIG_ACIA_DTR, 1);
	acia->set_context_res(comm[0], ACIA::SIG_ACIA_RESET, 1);
	acia->set_context_irq(board, SIG_CPU_IRQ, SIG_IRQ_ACIA_MASK);

	// acia_ex
	acia_ex->set_context_txdata(comm[1], ACIA::SIG_ACIA_TXDATA, 0xff);
	acia_ex->set_context_rts(comm[1], ACIA::SIG_ACIA_RTS, 1);
	acia_ex->set_context_dtr(comm[1], ACIA::SIG_ACIA_DTR, 1);
	acia_ex->set_context_res(comm[1], ACIA::SIG_ACIA_RESET, 1);
	acia_ex->set_context_irq(board, SIG_CPU_IRQ, SIG_IRQ_EXACIA_MASK);

	// comm
	comm[0]->set_context_ctrl(acia);
	comm[0]->set_context_cmt(cmt);
	comm[1]->set_context_ctrl(acia_ex);

	// data recorder
	cmt->set_context_ctrl(comm[0]);

	// pia
	pia->set_context_pa(key, KEYBOARD::SIG_KEYBOARD_PIA_PA, 0xff);
	pia->set_context_irqa(board, SIG_CPU_IRQ, SIG_IRQ_PIAA_MASK);
	pia->set_context_pb(printer[0], PRINTER::SIG_PRINTER_PIA_PB, 0xff);
	pia->set_context_cb2(printer[0], PRINTER::SIG_PRINTER_PIA_CB2, 1);
	pia->set_context_irqb(board, SIG_CPU_IRQ, SIG_IRQ_PIAB_MASK);

	// pia ex
	pia_ex->set_context_pa(printer[1], PRINTER::SIG_PRINTER_PIA_PB, 0xff);
	pia_ex->set_context_ca2(printer[1], PRINTER::SIG_PRINTER_PIA_CB2, 1);
	pia_ex->set_context_irqa(board, SIG_CPU_IRQ, SIG_IRQ_EXPIAA_MASK);
	pia_ex->set_context_pb(printer[2], PRINTER::SIG_PRINTER_PIA_PB, 0xff);
	pia_ex->set_context_cb2(printer[2], PRINTER::SIG_PRINTER_PIA_CB2, 1);
	pia_ex->set_context_irqb(board, SIG_CPU_IRQ, SIG_IRQ_EXPIAB_MASK);

	// printer
	printer[0]->set_context_ctrl(pia);
	printer[0]->set_context_cb1(PIA::SIG_PIA_CB1);
	printer[1]->set_context_ctrl(pia_ex);
	printer[1]->set_context_cb1(PIA::SIG_PIA_CA1);
	printer[2]->set_context_ctrl(pia_ex);
	printer[2]->set_context_cb1(PIA::SIG_PIA_CB1);

	// timer
	timer->set_context_board(board);

	// psg ex
	psg3_pia->set_context_pa(psg3c, PSGC::SIG_PSG_PIA_PA, 0xff);
	psg3_pia->set_context_pb(psg3c, PSGC::SIG_PSG_PIA_PB, 0xff);
	psg3c->set_context_psg(psg3[0], psg3[1]);
	psg3c->set_context_pia(psg3_pia);

	// 9voice psg
	psg9_via->set_context_pa(psg9c, PSG9C::SIG_PSG9_VIA_PA, 0xff);
	psg9_via->set_context_pb(psg9c, PSG9C::SIG_PSG9_VIA_PB, 0xff);
	psg9_via->set_context_irq(board, SIG_CPU_IRQ, SIG_IRQ_9PSG_MASK);
	psg9_via->write_signal(VIA::SIG_VIA_CLOCK_UNIT, CPU_CLOCKS / CLOCKS_1MHZ, 0xff);
	psg9c->set_context_psg(psg9[0], psg9[1], psg9[2]);
	psg9c->set_context_via(psg9_via);

	// fm opn
	fmopn->set_chiptype(pConfig->c_type_of_sound[pConfig->type_of_fmopn].chip_type == Config::CHIP_YM2608);
	psgex->set_chiptype(pConfig->c_type_of_sound[pConfig->type_of_expsg].chip_type == Config::CHIP_YM2608);
	if (pConfig->opn_irq > 0) {
		int id = (pConfig->opn_irq == 2 ? SIG_CPU_FIRQ : SIG_CPU_IRQ);
		fmopn->set_context_irq(board, id, SIG_IRQ_FMOPN_MASK);
		psgex->set_context_irq(board, id, SIG_IRQ_FMOPNEX_MASK);
	}

#ifdef USE_FD1
	// fdc for 3inch compact floppy
	fdc3->set_context_irq(board, SIG_CPU_NMI, SIG_NMI_FD_MASK);
	fdc3->set_context_fdd(fdd);

	// fdc for 5inch mini floppy
	fdc5->set_context_irq(fdd, SIG_FLOPPY_IRQ, 1);
	fdc5->set_context_drq(fdd, SIG_FLOPPY_DRQ, 1);
	fdc5->set_context_fdd(fdd);
	fdc5->set_channel(0);

	// fdc for 5inch mini floppy (ex)
	fdc5ex->set_context_irq(fdd, SIG_FLOPPY_IRQ, 0x10001);
	fdc5ex->set_context_drq(fdd, SIG_FLOPPY_DRQ, 0x10001);
	fdc5ex->set_context_fdd(fdd);
	fdc5ex->set_channel(1);

	// fdd
	fdd->set_context_irq(board, SIG_CPU_NMI, SIG_NMI_FD_MASK);
	fdd->set_context_drq(board, SIG_CPU_HALT, SIG_HALT_FD_MASK);
	fdd->set_context_fdc(fdc3, fdc5, fdc5ex);
	fdd->set_context_board(board);
#endif

#ifdef USE_RTC
	rtc->set_context_rtc(msm58321);
#endif

#if defined(USE_Z80B_CARD)
	z80b_card->set_context_cpu(z80);
	z80b_card->set_context_board(board);
#elif defined(USE_MPC_68008)
	mpc_68008->set_context_cpu(mc68k);
	mpc_68008->set_context_board(board);
	mpc_68008->set_context_memory(memory);
#endif

	// main board
	// reset signal
	// send reset to memory at first
	board->set_context_reset(memory, SIG_CPU_RESET, 1);
	board->set_context_reset(display, SIG_CPU_RESET, 1);
	board->set_context_reset(key, SIG_CPU_RESET, 1);
	board->set_context_reset(mouse, SIG_CPU_RESET, 1);
	board->set_context_reset(cmt, SIG_CPU_RESET, 1);
	board->set_context_reset(pia, SIG_CPU_RESET, 1);
	board->set_context_reset(timer, SIG_CPU_RESET, 1);
	board->set_context_reset(psgst, SIG_CPU_RESET, 1);
	board->set_context_reset(psgex, SIG_CPU_RESET, 1);
	board->set_context_reset(psg3[0], SIG_CPU_RESET, 1);
	board->set_context_reset(psg3[1], SIG_CPU_RESET, 1);
	board->set_context_reset(psg3_pia, SIG_CPU_RESET, 1);
	board->set_context_reset(psg9[0], SIG_CPU_RESET, 1);
	board->set_context_reset(psg9[1], SIG_CPU_RESET, 1);
	board->set_context_reset(psg9[2], SIG_CPU_RESET, 1);
	board->set_context_reset(psg9_via, SIG_CPU_RESET, 1);
	board->set_context_reset(fmopn, SIG_CPU_RESET, 1);
#ifdef USE_FD1
	board->set_context_reset(fdd, SIG_CPU_RESET, 1);
#endif
	board->set_context_reset(pia_ex, SIG_CPU_RESET, 1);
#ifdef USE_RTC
	board->set_context_reset(rtc, SIG_CPU_RESET, 1);
#endif
#if defined(USE_Z80B_CARD)
	board->set_context_reset(z80b_card, SIG_CPU_RESET, 1);
//	board->set_context_reset(z80, SIG_CPU_RESET, 1);
#elif defined(USE_MPC_68008)
	board->set_context_reset(mpc_68008, SIG_CPU_RESET, 1);
//	board->set_context_reset(m68k, SIG_CPU_RESET, 1);
#endif
	// send reset to cpu at last
	board->set_context_reset(cpu, SIG_CPU_RESET, 1);

	// nmi signal
	board->set_context_nmi(cpu, SIG_CPU_NMI, 0xffffffff);
#if defined(USE_MPC_68008)
	board->set_context_nmi(mpc_68008, SIG_CPU_NMI, 0xffffffff);
#endif
	// irq signal
	board->set_context_irq(cpu, SIG_CPU_IRQ, 0xffffffff);
	// firq signal
	board->set_context_firq(cpu, SIG_CPU_FIRQ, 0xffffffff);
	// halt signal
	board->set_context_halt(cpu, SIG_CPU_HALT, 0xffffffff);

	board->set_context_cpu(cpu);

	// initialize all devices
	for(DEVICE* device = first_device; device; device = device->get_next_device()) {
		if(device->get_id() != event->get_id()) {
			device->initialize();
		}
	}
}

VM::~VM()
{
	// delete all devices
	for(DEVICE* device = first_device; device; device = device->get_next_device()) {
		device->release();
	}
	for(DEVICE* device = first_device; device;) {
		DEVICE *next_device = device->get_next_device();
		delete device;
		device = next_device;
	}
#ifdef USE_KEY_RECORD
	delete reckey;
#endif
}

DEVICE* VM::get_device(int id)
{
	for(DEVICE* device = first_device; device; device = device->get_next_device()) {
		if(device->get_id() == id) {
			return device;
		}
	}
	return NULL;
}

DEVICE* VM::get_device(char *name, char *identifier)
{
	for(DEVICE* device = first_device; device; device = device->get_next_device()) {
		if(strncmp(name, device->get_class_name(), 12) == 0
		&& strncmp(identifier, device->get_identifier(), 4) == 0) {
			return device;
		}
	}
	return NULL;
}

// ----------------------------------------------------------------------------
// drive virtual machine
// ----------------------------------------------------------------------------

void VM::reset()
{
	// power on / off
	emu->out_info_x(pConfig->now_power_off ? CMsg::PowerOff : CMsg::PowerOn);

	// reset all devices
	for(DEVICE* device = first_device; device; device = device->get_next_device()) {
		device->reset();
	}

	// disable unused devices
	msm58321->enable(IOPORT_USE_RTC != 0);

	// set initial port status
	sound->write_signal(SOUND::SIG_SOUND_ON, 0, 1);
}

void VM::special_reset()
{
}

void VM::warm_reset(int onoff)
{
	// send reset signal
	if (onoff < 0) {
		board->write_signal(SIG_CPU_RESET, 1, 1);
		board->write_signal(SIG_CPU_RESET, 0, 1);
	} else {
		board->write_signal(SIG_CPU_RESET, onoff, 1);
	}
}

void VM::run(int split_num)
{
//	dasm.set_time(timeGetTime());
	event->drive(split_num);

}

double VM::get_frame_rate()
{
	return event->get_frame_rate();
}

void VM::change_dipswitch(int num)
{
	emu->out_infoc_x(CMsg::MODE_Switch_, (pConfig->dipswitch & 4) ? CMsg::ON : CMsg::OFF, NULL);
}

bool VM::now_skip()
{
	return false;
}

void VM::update_params()
{
	change_fdd_type(emu->get_parami(ParamFddType), true);
	change_sys_mode(emu->get_parami(ParamSysMode) & SYS_MODE_S1L3, true);
	pConfig->exram_size_num = emu->get_parami(ParamExMemNum);
	pConfig->type_of_fmopn = emu->get_parami(ParamChipTypeOnFmOpn);
	pConfig->type_of_expsg = emu->get_parami(ParamChipTypeOnExPsg);

	set_volume();
}

void VM::pause(int value)
{
	msm58321->pause(value);
}

// ----------------------------------------------------------------------------
// debugger
// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
int VM::get_cpus() const
{
	int nums = 1;
#if defined(USE_Z80B_CARD)
	nums++;
#elif defined(USE_MPC_68008)
	nums++;
#endif
	return nums;
}

DEVICE *VM::get_cpu(int index)
{
	if(index == 0) {
		return cpu;
	}
#if defined(USE_Z80B_CARD)
	else if (index == 1) {
		return z80;
	}
#elif defined(USE_MPC_68008)
	else if (index == 1) {
		return mc68k;
	}
#endif
	return NULL;
}

DEVICE *VM::get_memory(int index)
{
	if(index == 0) {
		return memory;
	}
#if defined(USE_Z80B_CARD)
	else if (index == 1) {
		return z80b_card;
	}
#elif defined(USE_MPC_68008)
	else if (index == 1) {
		return mpc_68008;
	}
#endif
	return NULL;
}

/// device names
const struct VM::st_device_name_map VM::c_device_names_map[] = {
	{ _T("PIA"), DNM_PIA },
	{ _T("ACIA"), DNM_ACIA },
	{ _T("CRTC"), DNM_CRTC },
	{ _T("KB"), DNM_KEYBOARD },
	{ _T("CMT"), DNM_CMT },
	{ _T("TIMER"), DNM_TIMER },
	{ _T("FDC3"), DNM_FDC3 },
	{ _T("FDC5"), DNM_FDC5 },
	{ _T("FDC5EX"), DNM_FDC5EX },
	{ _T("FDDUNIT"), DNM_FDDUNIT },
	{ _T("BOARD"), DNM_BOARD },
	{ _T("PSG0"), DNM_PSG0 },
	{ _T("PSG1"), DNM_PSG1 },
	{ _T("PSG60"), DNM_PSG60 },
	{ _T("PSG61"), DNM_PSG61 },
	{ _T("PSG90"), DNM_PSG90 },
	{ _T("PSG91"), DNM_PSG91 },
	{ _T("PSG92"), DNM_PSG92 },
	{ _T("FMOPN"), DNM_FMOPN },
	{ _T("PIAEX"), DNM_PIAEX },
	{ _T("ACIAEX"), DNM_ACIAEX },
	{ _T("VIA"), DNM_VIA },
	{ _T("RTC"), DNM_RTC },
	{ _T("MSM58321"), DNM_MSM58321 },
#if defined(USE_Z80B_CARD)
	{ _T("Z80BCARD"), DNM_Z80B_CARD },
#elif defined(USE_MPC_68008)
	{ _T("MPC68008"), DNM_MPC_68008 },
#endif
	{ _T("EVENT"), DNM_EVENT },
	{ NULL, 0 }
};

bool VM::get_debug_device_name(const _TCHAR *param, uint32_t *num, int *idx, const _TCHAR **name)
{
	int i = 0; 
	for(; c_device_names_map[i].name != NULL; i++) {
		if (_tcsicmp(param, c_device_names_map[i].name) == 0) {
			if (num) *num = c_device_names_map[i].num;
			if (idx) *idx = i;
			if (name) *name = c_device_names_map[i].name;
			return true;
		}
	}
	return false;
}

void VM::get_debug_device_names_str(_TCHAR *buffer, size_t buffer_len)
{
	int i = 0;
	int len = 2;
	UTILITY::tcscpy(buffer, buffer_len, _T("  "));
	for(; c_device_names_map[i].name != NULL; i++) {
		if (i > 0) {
			UTILITY::tcscat(buffer, buffer_len, _T(","));
			len++;
		}
		int siz = (int)_tcslen(c_device_names_map[i].name);
		if (len + siz >= 78) {
			UTILITY::tcscat(buffer, buffer_len, _T("\n  "));
			len = 2;
		}
		UTILITY::tcscat(buffer, buffer_len, c_device_names_map[i].name);
		len += siz;
	}
}

bool VM::debug_write_reg(uint32_t num, uint32_t reg_num, uint32_t data)
{
	bool valid = false;
	switch(num) {
	case DNM_KEYBOARD:
		// keyboard
		if (key) valid = key->debug_write_reg(reg_num, data);
		break;
	case DNM_CRTC:
		// crtc
		if (crtc) valid = crtc->debug_write_reg(reg_num, data);
		break;
	case DNM_PIA:
		// pia
		if (pia) valid = pia->debug_write_reg(reg_num, data);
		break;
	case DNM_ACIA:
		// acia
		if (acia) valid = acia->debug_write_reg(reg_num, data);
		break;
	case DNM_FDC3:
		// fdc3
		if (fdc3) valid = fdc3->debug_write_reg(reg_num, data);
		break;
	case DNM_FDC5:
		// fdc5
		if (fdc5) valid = fdc5->debug_write_reg(reg_num, data);
		break;
	case DNM_FDC5EX:
		// fdc5ex
		if (fdc5ex) valid = fdc5ex->debug_write_reg(reg_num, data);
		break;
	case DNM_CMT:
		// cmt
		if (cmt) valid = cmt->debug_write_reg(reg_num, data);
		break;
	case DNM_TIMER:
		// timer
		if (timer) valid = timer->debug_write_reg(reg_num, data);
		break;
	case DNM_FDDUNIT:
		// fddunit
		if (fdd) valid = fdd->debug_write_reg(reg_num, data);
		break;
	case DNM_BOARD:
		// board
		if (board) valid = board->debug_write_reg(reg_num, data);
		break;
	case DNM_PSG0:
		// psg
		if (psgst) valid = psgst->debug_write_reg(reg_num, data);
		break;
	case DNM_PSG1:
		// psgex
		if (psgex) valid = psgex->debug_write_reg(reg_num, data);
		break;
	case DNM_PSG60:
	case DNM_PSG61:
		// psg6x
		if (psg3[num-DNM_PSG60]) valid = psg3[num-DNM_PSG60]->debug_write_reg(reg_num, data);
		break;
	case DNM_PSG90:
	case DNM_PSG91:
	case DNM_PSG92:
		// psg9x
		if (psg9[num-DNM_PSG90]) valid = psg9[num-DNM_PSG90]->debug_write_reg(reg_num, data);
		break;
	case DNM_PIAEX:
		// pia ex
		if (pia_ex) valid = pia_ex->debug_write_reg(reg_num, data);
		break;
	case DNM_ACIAEX:
		// acia ex
		if (acia_ex) valid = acia_ex->debug_write_reg(reg_num, data);
		break;
	case DNM_VIA:
		// via
		if (psg9_via) valid = psg9_via->debug_write_reg(reg_num, data);
		break;
	case DNM_FMOPN:
		// fm opn
		if (fmopn) valid = fmopn->debug_write_reg(reg_num, data);
		break;
	case DNM_RTC:
		// rtc
		if (rtc) valid = rtc->debug_write_reg(reg_num, data);
		break;
	case DNM_MSM58321:
		// msm58321
		if (msm58321) valid = msm58321->debug_write_reg(reg_num, data);
		break;
#if defined(USE_Z80B_CARD)
	case DNM_Z80B_CARD:
		// Z80B Card
		if (z80b_card) valid = z80b_card->debug_write_reg(reg_num, data);
		break;
#elif defined(USE_MPC_68008)
	case DNM_MPC_68008:
		// MPC-68008
		if (mpc_68008) valid = mpc_68008->debug_write_reg(reg_num, data);
		break;
#endif
	case DNM_EVENT:
		// event
		if (event) valid = event->debug_write_reg(reg_num, data);
		break;
	}
	return valid;
}

bool VM::debug_write_reg(uint32_t num, const _TCHAR *reg, uint32_t data)
{
	bool valid = false;
	switch(num) {
	case DNM_KEYBOARD:
		// keyboard
		if (key) valid = key->debug_write_reg(reg, data);
		break;
	case DNM_CRTC:
		// crtc
		if (crtc) valid = crtc->debug_write_reg(reg, data);
		break;
	case DNM_PIA:
		// pia
		if (pia) valid = pia->debug_write_reg(reg, data);
		break;
	case DNM_ACIA:
		// acia
		if (acia) valid = acia->debug_write_reg(reg, data);
		break;
	case DNM_FDC3:
		// fdc3
		if (fdc3) valid = fdc3->debug_write_reg(reg, data);
		break;
	case DNM_FDC5:
		// fdc5
		if (fdc5) valid = fdc5->debug_write_reg(reg, data);
		break;
	case DNM_FDC5EX:
		// fdc5ex
		if (fdc5ex) valid = fdc5ex->debug_write_reg(reg, data);
		break;
	case DNM_CMT:
		// cmt
		if (cmt) valid = cmt->debug_write_reg(reg, data);
		break;
	case DNM_TIMER:
		// timer
		if (timer) valid = timer->debug_write_reg(reg, data);
		break;
	case DNM_FDDUNIT:
		// fddunit
		if (fdd) valid = fdd->debug_write_reg(reg, data);
		break;
	case DNM_BOARD:
		// board
		if (board) valid = board->debug_write_reg(reg, data);
		break;
	case DNM_PSG0:
		// psg
		if (psgst) valid = psgst->debug_write_reg(reg, data);
		break;
	case DNM_PSG1:
		// psgex
		if (psgex) valid = psgex->debug_write_reg(reg, data);
		break;
	case DNM_PSG60:
	case DNM_PSG61:
		// psg6x
		if (psg3[num-DNM_PSG60]) valid = psg3[num-DNM_PSG60]->debug_write_reg(reg, data);
		break;
	case DNM_PSG90:
	case DNM_PSG91:
	case DNM_PSG92:
		// psg9x
		if (psg9[num-DNM_PSG90]) valid = psg9[num-DNM_PSG90]->debug_write_reg(reg, data);
		break;
	case DNM_PIAEX:
		// pia ex
		if (pia_ex) valid = pia_ex->debug_write_reg(reg, data);
		break;
	case DNM_ACIAEX:
		// acia ex
		if (acia_ex) valid = acia_ex->debug_write_reg(reg, data);
		break;
	case DNM_VIA:
		// via
		if (psg9_via) valid = psg9_via->debug_write_reg(reg, data);
		break;
	case DNM_FMOPN:
		// fm opn
		if (fmopn) valid = fmopn->debug_write_reg(reg, data);
		break;
	case DNM_RTC:
		// rtc
		if (rtc) valid = rtc->debug_write_reg(reg, data);
		break;
	case DNM_MSM58321:
		// msm58321
		if (msm58321) valid = msm58321->debug_write_reg(reg, data);
		break;
#if defined(USE_Z80B_CARD)
	case DNM_Z80B_CARD:
		// Z80B Card
		if (z80b_card) valid = z80b_card->debug_write_reg(reg, data);
		break;
#elif defined(USE_MPC_68008)
	case DNM_MPC_68008:
		// MPC-68008
		if (mpc_68008) valid = mpc_68008->debug_write_reg(reg, data);
		break;
#endif
	case DNM_EVENT:
		// event
		if (event) valid = event->debug_write_reg(reg, data);
		break;
	}
	return valid;
}

void VM::debug_regs_info(uint32_t num, _TCHAR *buffer, size_t buffer_len)
{
	switch(num) {
	case DNM_KEYBOARD:
		// keyboard
		if (key) key->debug_regs_info(buffer, buffer_len);
		break;
	case DNM_CRTC:
		// crtc
		if (crtc) crtc->debug_regs_info(buffer, buffer_len);
		break;
	case DNM_PIA:
		// pia
		if (pia) pia->debug_regs_info(buffer, buffer_len);
		break;
	case DNM_ACIA:
		// acia
		if (acia) acia->debug_regs_info(buffer, buffer_len);
		break;
	case DNM_FDC3:
		// fdc3
		if (fdc3) fdc3->debug_regs_info(buffer, buffer_len);
		break;
	case DNM_FDC5:
		// fdc5
		if (fdc5) fdc5->debug_regs_info(buffer, buffer_len);
		break;
	case DNM_FDC5EX:
		// fdc5ex
		if (fdc5ex) fdc5ex->debug_regs_info(buffer, buffer_len);
		break;
	case DNM_CMT:
		// cmt
		if (cmt) cmt->debug_regs_info(buffer, buffer_len);
		break;
	case DNM_TIMER:
		// timer
		if (timer) timer->debug_regs_info(buffer, buffer_len);
		break;
	case DNM_FDDUNIT:
		// fddunit
		if (fdd) fdd->debug_regs_info(buffer, buffer_len);
		break;
	case DNM_BOARD:
		// board
		if (board) board->debug_regs_info(buffer, buffer_len);
		break;
	case DNM_PSG0:
		// psg
		if (psgst) psgst->debug_regs_info(buffer, buffer_len);
		break;
	case DNM_PSG1:
		// psgex
		if (psgex) psgex->debug_regs_info(buffer, buffer_len);
		break;
	case DNM_PSG60:
	case DNM_PSG61:
		// psg6x
		if (psg3[num-DNM_PSG60]) psg3[num-DNM_PSG60]->debug_regs_info(buffer, buffer_len);
		break;
	case DNM_PSG90:
	case DNM_PSG91:
	case DNM_PSG92:
		// psg9x
		if (psg9[num-DNM_PSG90]) psg9[num-DNM_PSG90]->debug_regs_info(buffer, buffer_len);
		break;
	case DNM_PIAEX:
		// pia_ex
		if (pia_ex) pia_ex->debug_regs_info(buffer, buffer_len);
		break;
	case DNM_ACIAEX:
		// acia_ex
		if (acia_ex) acia_ex->debug_regs_info(buffer, buffer_len);
		break;
	case DNM_VIA:
		// via
		if (psg9_via) psg9_via->debug_regs_info(buffer, buffer_len);
		break;
	case DNM_FMOPN:
		// fm opn
		if (fmopn) fmopn->debug_regs_info(buffer, buffer_len);
		break;
	case DNM_RTC:
		// rtc
		if (rtc) rtc->debug_regs_info(buffer, buffer_len);
		break;
	case DNM_MSM58321:
		// msm58321
		if (msm58321) msm58321->debug_regs_info(buffer, buffer_len);
		break;
#if defined(USE_Z80B_CARD)
	case DNM_Z80B_CARD:
		// Z80B Card
		if (z80b_card) z80b_card->debug_regs_info(buffer, buffer_len);
		break;
#elif defined(USE_MPC_68008)
	case DNM_MPC_68008:
		// MPC-68008
		if (mpc_68008) mpc_68008->debug_regs_info(buffer, buffer_len);
		break;
#endif
	case DNM_EVENT:
		// event
		if (event) event->debug_regs_info(buffer, buffer_len);
		break;
	}
}

#endif	/* USE_DEBUGGER */

// ----------------------------------------------------------------------------
// event manager
// ----------------------------------------------------------------------------

void VM::register_event(DEVICE* dev, int event_id, int usec, bool loop, int* register_id)
{
	event->register_event(dev, event_id, usec, loop, register_id);
}

void VM::register_event_by_clock(DEVICE* dev, int event_id, int clock, bool loop, int* register_id)
{
	event->register_event_by_clock(dev, event_id, clock, loop, register_id);
}

void VM::cancel_event(DEVICE *dev, int register_id)
{
	event->cancel_event(dev, register_id);
}

void VM::register_frame_event(DEVICE* dev)
{
	event->register_frame_event(dev);
}

void VM::register_vline_event(DEVICE* dev)
{
	event->register_vline_event(dev);
}

uint64_t VM::get_current_clock()
{
	return event->get_current_clock();
}

uint64_t VM::get_passed_clock(uint64_t prev)
{
	return event->get_passed_clock(prev);
}

//uint32_t VM::get_pc()
//{
//	return cpu->get_pc();
//}

void VM::set_lines_per_frame(int lines) {
	event->set_lines_per_frame(lines);
}

// ----------------------------------------------------------------------------
// draw screen
// ----------------------------------------------------------------------------

void VM::set_display_size(int left, int top, int right, int bottom)
{
	display->set_display_size(left, top, right, bottom);
}

void VM::draw_screen()
{
//	display->set_vram_ptr(memory->get_vram());
	display->draw_screen();
}

// ----------------------------------------------------------------------------
uint64_t VM::update_led()
{
	uint64_t status = 0;

	// b0: power on/off
	status |= (pConfig->now_power_off ? 0 : 1);
	// b1: mode
	status |= ((1 - (emu->get_parami(ParamSysMode) & SYS_MODE_S1L3)) << 1);
#ifdef USE_FD1
	// b2-b3: fdd type
	status |= ((pConfig->fdd_type & MSK_FDD_TYPE) << 2);
#endif
	// b4-b6: kbd led
	status |= ((key->get_kb_mode()) << 4);
	// b7: reset signal
	status |= board->update_led();
	// b8-b11: cmt led  b12-b27:cmt cnt
	status |= (cmt->get_cmt_mode() << 8);
#ifdef USE_FD1
	// b28-b31: drive select, b32-b35: 0:green led, 1:red led b36-39:inserted?
	status |= ((uint64_t)fdd->get_drive_select() << 28);
#endif
	// b40-b43 system mode
	status |= ((uint64_t)REG_SYS_MODE << 40);
	return status;
}

// ----------------------------------------------------------------------------
// sound manager
// ----------------------------------------------------------------------------

/// Initialize sound device at starting application
/// @param [in] rate : sampling rate
/// @param [in] samples : sample number per second
void VM::initialize_sound(int rate, int samples)
{
	// init sound manager
	event->initialize_sound(rate, samples);

	// init sound gen
	sound->initialize_sound(rate, 0);
	psgst->initialize_sound(rate, CLOCKS_1MHZ, samples, 0, 3);
//	psgex->initialize_sound(rate, CLOCKS_1MHZ, samples, 0, 3);
	psgex->initialize_sound(rate, pConfig->c_type_of_sound[pConfig->type_of_expsg].clock, samples, 0, 0, pConfig->c_type_of_sound[pConfig->type_of_expsg].chip_type != Config::CHIP_AY38910 ? 0 : 3);
	psg3[0]->initialize_sound(rate, CLOCKS_1MHZ, samples, 0, 0);
	psg3[1]->initialize_sound(rate, CLOCKS_1MHZ, samples, 0, 0);
	psg9[0]->initialize_sound(rate, CLOCKS_1MHZ * 2, samples, 0, 0);
	psg9[1]->initialize_sound(rate, CLOCKS_1MHZ * 2, samples, 0, 0);
	psg9[2]->initialize_sound(rate, CLOCKS_1MHZ * 2, samples, 0, 0);
	cmt->initialize_sound(rate, 0);
#ifdef USE_FD1
	fdd->initialize_sound(rate, 0);
#endif
	fmopn->initialize_sound(rate, pConfig->c_type_of_sound[pConfig->type_of_fmopn].clock, samples, 0, 0, pConfig->c_type_of_sound[pConfig->type_of_fmopn].chip_type != Config::CHIP_AY38910 ? 0 : 3);

	//
	set_volume();
}

/// Re-initialize sound device under power-on operation
/// @param [in] rate : sampling rate
/// @param [in] samples : sample number per second
void VM::reset_sound(int rate, int samples)
{
	psgex->init_context_irq();
	fmopn->init_context_irq();
	psgex->set_chiptype(pConfig->c_type_of_sound[pConfig->type_of_expsg].chip_type == Config::CHIP_YM2608);
	fmopn->set_chiptype(pConfig->c_type_of_sound[pConfig->type_of_fmopn].chip_type == Config::CHIP_YM2608);
	psgex->initialize_sound(rate, pConfig->c_type_of_sound[pConfig->type_of_expsg].clock, samples, 0, 0, pConfig->c_type_of_sound[pConfig->type_of_expsg].chip_type != Config::CHIP_AY38910 ? 0 : 3);
	fmopn->initialize_sound(rate, pConfig->c_type_of_sound[pConfig->type_of_fmopn].clock, samples, 0, 0, pConfig->c_type_of_sound[pConfig->type_of_fmopn].chip_type != Config::CHIP_AY38910 ? 0 : 3);
	if (pConfig->opn_irq > 0) {
		int id = (pConfig->opn_irq == 2 ? SIG_CPU_FIRQ : SIG_CPU_IRQ);
		fmopn->set_context_irq(board, id, SIG_IRQ_FMOPN_MASK);
		psgex->set_context_irq(board, id, SIG_IRQ_FMOPNEX_MASK);
	}

	set_volume();
}

/// @attention called by another thread
audio_sample_t* VM::create_sound(int* extra_frames, int samples)
{
	return event->create_sound(extra_frames, samples);
}

void VM::set_volume()
{
	int vol = 0;
	bool is_psg;
	event->set_volume(pConfig->volume - 81, pConfig->mute);
	sound->set_volume(pConfig->beep_volume - 81, pConfig->beep_mute);
	vol = pConfig->psg_volume - 81;
	psgst->set_volume(vol, vol, pConfig->psg_mute, 3);
//	psgex->set_volume(vol, vol, pConfig->psg_mute || (IOPORT_USE_EXPSG == 0), 3);
	is_psg = (pConfig->c_type_of_sound[pConfig->type_of_expsg].chip_type == Config::CHIP_AY38910);
	vol = pConfig->psgexfm_volume - 81;
	psgex->set_volume(0, vol, vol, pConfig->psgexfm_mute || (IOPORT_USE_EXPSG == 0) || is_psg, 0);
	vol = pConfig->psgexssg_volume - 81;
	psgex->set_volume(1, vol, vol, pConfig->psgexssg_mute || (IOPORT_USE_EXPSG == 0), is_psg ? 3 : 0);
	vol = pConfig->psgexpcm_volume - 81;
	psgex->set_volume(2, vol, vol, pConfig->psgexpcm_mute || (IOPORT_USE_EXPSG == 0) || is_psg, 0);
	vol = pConfig->psgexrhy_volume - 81;
	psgex->set_volume(3, vol, vol, pConfig->psgexrhy_mute || (IOPORT_USE_EXPSG == 0) || is_psg, 0);
	vol = pConfig->psg6_volume - 81;
	psg3[0]->set_volume(vol, vol, pConfig->psg6_mute || (IOPORT_USE_PSG6 == 0), 0);
	psg3[1]->set_volume(vol, vol, pConfig->psg6_mute || (IOPORT_USE_PSG6 == 0), 0);
	vol = pConfig->psg9_volume - 81;
	psg9[0]->set_volume(vol, vol, pConfig->psg9_mute || (IOPORT_USE_PSG9 == 0), 0);
	psg9[1]->set_volume(vol, vol, pConfig->psg9_mute || (IOPORT_USE_PSG9 == 0), 0);
	psg9[2]->set_volume(vol, vol, pConfig->psg9_mute || (IOPORT_USE_PSG9 == 0), 0);
	cmt->set_volume(pConfig->relay_volume - 81, pConfig->cmt_volume - 81, pConfig->relay_mute, pConfig->cmt_mute);
#ifdef USE_FD1
	fdd->set_volume(pConfig->fdd_volume - 81, pConfig->fdd_mute);
#endif
	is_psg = (pConfig->c_type_of_sound[pConfig->type_of_fmopn].chip_type == Config::CHIP_AY38910);
	vol = pConfig->opnfm_volume - 81;
	fmopn->set_volume(0, vol, vol, pConfig->opnfm_mute || (IOPORT_USE_FMOPN == 0) || is_psg, 0);
	vol = pConfig->opnssg_volume - 81;
	fmopn->set_volume(1, vol, vol, pConfig->opnssg_mute || (IOPORT_USE_FMOPN == 0), is_psg ? 3 : 0);
	vol = pConfig->opnpcm_volume - 81;
	fmopn->set_volume(2, vol, vol, pConfig->opnpcm_mute || (IOPORT_USE_FMOPN == 0) || is_psg, 0);
	vol = pConfig->opnrhy_volume - 81;
	fmopn->set_volume(3, vol, vol, pConfig->opnrhy_mute || (IOPORT_USE_FMOPN == 0) || is_psg, 0);
}

// ----------------------------------------------------------------------------
// notify key
// ----------------------------------------------------------------------------

void VM::key_down(int code)
{
}

void VM::key_up(int code)
{
}

void VM::system_key_down(int code)
{
	key->system_key_down(code);
}

void VM::system_key_up(int code)
{
	key->system_key_up(code);
}

void VM::set_mouse_position(int px, int py)
{
	mouse->set_mouse_position(px, py);
}

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

bool VM::play_datarec(const _TCHAR* file_path)
{
	return cmt->play_datarec(file_path);
}

bool VM::rec_datarec(const _TCHAR* file_path)
{
	return cmt->rec_datarec(file_path);
}

void VM::close_datarec()
{
	cmt->close_datarec();
}

void VM::rewind_datarec()
{
	cmt->rewind_datarec();
}

void VM::fast_forward_datarec()
{
	cmt->fast_forward_datarec();
}

void VM::stop_datarec()
{
	cmt->stop_datarec();
}

void VM::realmode_datarec()
{
	cmt->realmode_datarec();
}

bool VM::datarec_opened(bool play_mode)
{
	return cmt->datarec_opened(play_mode);
}

// ----------------------------------------------------------------------------

#ifdef USE_FD1
bool VM::open_floppy_disk(int drv, const _TCHAR* file_path, int offset, uint32_t flags)
{
	bool rc = fdd->open_disk(drv, file_path, offset, flags);
	if (rc) {
		if (!(flags & OPEN_DISK_FLAGS_FORCELY)) {
			int sdrv = fdd->inserted_disk_another_drive(drv, file_path, offset);
			if (sdrv >= 0) {
				int drvmin = MIN(drv, sdrv);
				int drvmax = MAX(drv, sdrv);
				logging->out_logf_x(LOG_WARN, CMsg::There_is_the_same_floppy_disk_in_drive_VDIGIT_and_VDIGIT, drvmin, drvmax);
			}
		}
	}
	return rc;
}

bool VM::close_floppy_disk(int drv, uint32_t flags)
{
	return fdd->close_disk(drv, flags);
}

int VM::change_floppy_disk(int drv)
{
	switch(pConfig->fdd_type) {
		case FDD_TYPE_3FDD:
			return fdd->change_disk(drv);
			break;
	}
	return 0;
}

bool VM::floppy_disk_inserted(int drv)
{
	return fdd->disk_inserted(drv);
}

int VM::get_floppy_disk_side(int drv)
{
	return fdd->get_disk_side(drv);
}

void VM::toggle_floppy_disk_write_protect(int drv)
{
	fdd->toggle_disk_write_protect(drv);
}

bool VM::floppy_disk_write_protected(int drv)
{
	return fdd->disk_write_protected(drv);
}

bool VM::is_same_floppy_disk(int drv, const _TCHAR *file_path, int offset)
{
	return fdd->is_same_disk(drv, file_path, offset);
}
#endif
// ----------------------------------------------------------------------------
void VM::update_config()
{
	for(DEVICE* device = first_device; device; device = device->get_next_device()) {
		device->update_config();
	}
}

// ----------------------------------------------------------------------------
bool VM::save_printer(int dev, const _TCHAR* filename)
{
	return printer[dev]->save_printer(filename);
}

void VM::clear_printer(int dev)
{
	printer[dev]->reset();
}

int VM::get_printer_buffer_size(int dev)
{
	return printer[dev]->get_buffer_size();
}

uint8_t* VM::get_printer_buffer(int dev)
{
	return printer[dev]->get_buffer();
}

void VM::enable_printer_direct(int dev)
{
	printer[dev]->set_direct_mode();
}

bool VM::print_printer(int dev)
{
	return printer[dev]->print_printer();
}

void VM::toggle_printer_online(int dev)
{
	return printer[dev]->toggle_printer_online();
}

// ----------------------------------------------------------------------------
/// @note called by main thread
void VM::enable_comm_server(int dev)
{
	comm[dev]->enable_server();
}

/// @note called by main thread
void VM::enable_comm_connect(int dev, int num)
{
	comm[dev]->enable_connect(num);
}

/// @note called by main thread
bool VM::now_comm_connecting(int dev, int num)
{
	return comm[dev]->now_connecting(num);
}

/// @note called by main thread
void VM::send_comm_telnet_command(int dev, int num)
{
	comm[dev]->send_telnet_command(num);
}

// ----------------------------------------------------------------------------
void VM::modify_joytype()
{
}

void VM::save_keybind()
{
	key->save_keybind();
}

void VM::clear_joy2joyk_map()
{
}

void VM::set_joy2joyk_map(int num, int idx, uint32_t joy_code)
{
}

// ----------------------------------------------------------------------------
void VM::change_archtecture(int id, int num, bool reset)
{
	switch(id) {
	case ArchFddType:
		change_fdd_type(num, reset);
		break;
	case ArchSysMode:
		change_sys_mode(num, reset);
		break;
	default:
		break;
	}
}

void VM::change_fdd_type(int num, bool reset)
{
	const CMsg::Id list[] = {
		CMsg::Non_FDD,
		CMsg::FD3inch_compact_FDD,
		CMsg::FD5inch_mini_FDD_2D_Type,
		CMsg::FD5inch_mini_FDD_2HD_Type,
		CMsg::Unsupported_FDD,
		CMsg::End
	};

	if (num == -1) {
		num = emu->get_parami(ParamFddType);
		num = (num + 1) % 4;
	} else {
		num = num % 4;
	}
	emu->set_parami(ParamFddType, num);
	int io_port = emu->get_parami(ParamIOPort);
	switch(num) {
	case FDD_TYPE_NOFDD:
		io_port &= ~IOPORT_MSK_FDDALL;
		break;
	case FDD_TYPE_3FDD:
		io_port &= ~IOPORT_MSK_FDDALL;
		io_port |= IOPORT_MSK_3FDD;
		break;
	case FDD_TYPE_5FDD:
		io_port &= ~IOPORT_MSK_FDDALL;
		io_port |= IOPORT_MSK_5FDD;
		break;
	case FDD_TYPE_58FDD:
		io_port &= ~IOPORT_MSK_FDDALL;
		io_port |= IOPORT_MSK_5FDD;
		break;
	default:
		break;
	}
	CMsg::Id need = CMsg::Null;
	if (pConfig->fdd_type != num && !reset) {
		if(!pConfig->now_power_off) need = CMsg::LB_Need_PowerOn_RB;
	}
	emu->set_parami(ParamIOPort, io_port);
	if (reset) {
		pConfig->fdd_type = num;
		pConfig->io_port = io_port;
		if(!pConfig->now_power_off) logging->out_log_x(LOG_INFO, list[num]);
	} else {
		emu->out_infoc_x(list[num], need, 0);
	}
}

void VM::change_sys_mode(int num, bool reset)
{
	const CMsg::Id list[] = {
		CMsg::B_Mode_L3,
		CMsg::A_Mode_S1,
		CMsg::End
	};

	int sys_mode = emu->get_parami(ParamSysMode);

	if (num == -1) {
		num = ((sys_mode & SYS_MODE_S1L3) + 1) % 2;
	}
	sys_mode = (num != 0 ? (sys_mode | SYS_MODE_S1L3) : (sys_mode & ~SYS_MODE_S1L3));
	emu->set_parami(ParamSysMode, sys_mode);
	CMsg::Id need = CMsg::Null;
	if ((pConfig->sys_mode & SYS_MODE_S1L3) != (sys_mode & SYS_MODE_S1L3) && !reset) {
		if(!pConfig->now_power_off) need = CMsg::LB_Need_PowerOn_RB;
	}
	if (reset) {
		pConfig->sys_mode = sys_mode;
		if(!pConfig->now_power_off) logging->out_log_x(LOG_INFO, list[num]);
	} else {
		emu->out_infoc_x(list[num], need, 0);
	}
}

// ----------------------------------------------------------------------------

/// @brief load a data from file
///
/// @param[in]  file_path : directory
/// @param[in]  file_name : data file
/// @param[out] data      : loaded data
/// @param[in]  size      : buffer size of data
/// @param[in]  first_data      : (nullable) first pattern to compare to loaded data
/// @param[in]  first_data_size : size of first_data
/// @param[in]  first_data_pos  : comparing position in loaded data
/// @param[in]  last_data       : (nullable) last pattern to compare to loaded data
/// @param[in]  last_data_size  : size of last_data
/// @param[in]  last_data_pos   : comparing position in loaded data
/// @return 1:successfully loaded  2:data loaded but unmatch pattern or size  0:unloaded
int VM::load_data_from_file(const _TCHAR *file_path, const _TCHAR *file_name
	, uint8_t *data, size_t size
	, const uint8_t *first_data, size_t first_data_size, size_t first_data_pos
	, const uint8_t *last_data,  size_t last_data_size, size_t last_data_pos)
{
	return EMU::load_data_from_file(file_path, file_name, data, size
		, first_data, first_data_size, first_data_pos
		, last_data, last_data_size, last_data_pos);
}

/// @brief get VM specific parameter
///
/// @param[in] id
/// @return parameter
int VM::get_parami(enumParamiId id) const
{
	return emu->get_parami(id);
}

/// @brief set VM specific parameter
///
/// @param[in] id
/// @param[in] val : parameter
void VM::set_parami(enumParamiId id, int val)
{
	emu->set_parami(id, val);
}

/// @brief get VM specific object
///
/// @param[in] id
/// @return object
void *VM::get_paramv(enumParamvId id) const
{
	return emu->get_paramv(id);
}

/// @brief set VM specific object
///
/// @param[in] id
/// @param[in] val : object
void VM::set_paramv(enumParamvId id, void *val)
{
	emu->set_paramv(id, val);
}

const _TCHAR *VM::application_path() const
{
	return emu->application_path();
}

const _TCHAR *VM::initialize_path() const
{
	return emu->initialize_path();
}

bool VM::get_pause(int idx) const
{
	return emu->get_pause(idx);
}

void VM::set_pause(int idx, bool val)
{
	emu->set_pause(idx, val);
}

//
void VM::get_edition_string(char *buffer, size_t buffer_len) const
{
	*buffer = '\0';
#if defined(USE_Z80B_CARD)
	UTILITY::strcat(buffer, buffer_len, *buffer == '\0' ? " with " : ", ");
	UTILITY::strcat(buffer, buffer_len, "Z80B Card");
#elif defined(USE_MPC_68008)
	UTILITY::strcat(buffer, buffer_len, *buffer == '\0' ? " with " : ", ");
	UTILITY::strcat(buffer, buffer_len, "MPC-68008 Card");
#endif
#ifdef USE_DEBUGGER
	UTILITY::strcat(buffer, buffer_len, *buffer == '\0' ? " with " : ", ");
	UTILITY::strcat(buffer, buffer_len, "Debugger");
#endif
}

// ----------------------------------------------------------------------------

bool VM::save_state(const _TCHAR* filename)
{
	vm_state_header_t vm_state_h;

	// header
	memset(&vm_state_h, 0, sizeof(vm_state_h));
	UTILITY::strncpy(vm_state_h.header, sizeof(vm_state_h.header), RESUME_FILE_HEADER, 16);
	vm_state_h.version = Uint16_LE(RESUME_FILE_VERSION);
	vm_state_h.revision = Uint16_LE(RESUME_FILE_REVISION);
	vm_state_h.param = Uint32_LE(0);
	vm_state_h.emu_major = Uint16_LE(APP_VER_MAJOR);
	vm_state_h.emu_minor = Uint16_LE(APP_VER_MINOR);
	vm_state_h.emu_rev   = Uint16_LE(APP_VER_REV);

	FILEIO fio;
	bool rc = false;
	if(fio.Fopen(filename, FILEIO::WRITE_BINARY)) {
		// write header
		fio.Fwrite(&vm_state_h, sizeof(vm_state_h), 1);
		// write data
		for(DEVICE* device = first_device; device; device = device->get_next_device()) {
			device->save_state(&fio);
		}
		fio.Fclose();
		rc = true;
	}
	return rc;
}

bool VM::load_state(const _TCHAR* filename)
{
	vm_state_header_t vm_state_h;

	FILEIO fio;
	bool rc = false;
	do {
		if(!fio.Fopen(filename, FILEIO::READ_BINARY)) {
			logging->out_log_x(LOG_ERROR, CMsg::Load_State_Cannot_open);
			break;
		}
		// read header
		fio.Fread(&vm_state_h, sizeof(vm_state_h), 1);
		// check header
		if (strncmp(vm_state_h.header, RESUME_FILE_HEADER, 16) != 0) {
			if (strncmp(vm_state_h.header, RESUME_FILE_HEADER_L3, 16) != 0) {
				logging->out_log_x(LOG_ERROR, CMsg::Load_State_Unsupported_file);
				break;
			} else {
				logging->out_logf_x(LOG_WARN, CMsg::Load_State_No_longer_support_a_status_file_for_VSTR, _T("BML3MK5"));
			}
		}
		if (Uint16_LE(vm_state_h.version) != RESUME_FILE_VERSION) {
			logging->out_log_x(LOG_ERROR, CMsg::Load_State_Invalid_version);
			break;
		}
		// read data
		rc = true;
		for(DEVICE* device = first_device; rc && device != NULL; device = device->get_next_device()) {
			rc = device->load_state(&fio);
		}
		//
		if (!rc) {
			logging->out_log_x(LOG_ERROR, CMsg::Load_State_Invalid_version);
			break;
		}
		//
		set_volume();
	} while(0);

	fio.Fclose();


	return rc;
}

// ----------------------------------------------------------------------------

#ifdef USE_KEY_RECORD
bool VM::play_reckey(const _TCHAR* filename)
{
	return key->play_reckey(filename);
}

bool VM::record_reckey(const _TCHAR* filename)
{
	return key->record_reckey(filename);
}

void VM::stop_reckey(bool stop_play, bool stop_record)
{
	key->stop_reckey(stop_play, stop_record);
}
#endif

