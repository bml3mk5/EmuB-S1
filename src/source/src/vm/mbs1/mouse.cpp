/** @file mouse.cpp

	HITACHI MB-S1 Emulator 'EmuB-S1'
	Skelton for retropc emulator

	@author Sasaji
	@date   2015.09.11

	@brief [ mouse ]
*/

#include "mouse.h"
#include "../../fileio.h"

#define MOUSE_BUTTON_ONOFF		0x40
#define MOUSE_BUTTON_TRIGGER	0x80

void MOUSE::initialize()
{
	mouse_stat = emu->mouse_buffer();

	register_frame_event(this);
}

void MOUSE::reset()
{
	for(int i=0; i<2; i++) {
		mst[i].pos = 0;
		mst[i].dir = 0;
		mst[i].btn = 0;
		mst[i].prev_btn = 0;

		reg_mouse[i] = 0;
		cntdir[i] = 1;
	}
}

void MOUSE::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch (id) {
		case SIG_CPU_RESET:
			now_reset = (data & mask) ? true : false;
			reset();
			break;
	}
}

void MOUSE::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 3) {
	case 0:
		set_reg0(data & 0xff);
		break;
	case 2:
		set_reg1(data & 0xff);
		break;
	}
}

void MOUSE::set_reg0(uint8_t data)
{
	reg_mouse[0] = data;
	if (reg_mouse[0] & 0x01) {
		mst[0].pos = 0;
	}
	if (reg_mouse[0] & 0x02) {
		mst[0].dir = 0;
	}
	if (reg_mouse[0] & 0x04) {
		mst[1].pos = 0;
	}
	if (reg_mouse[0] & 0x08) {
		mst[1].dir = 0;
	}
	cntdir[0] = (reg_mouse[0] & 0x20) ? -1 : 1;
	cntdir[1] = (reg_mouse[0] & 0x10) ? 1 : -1;

//	logging->out_debugf(_T("mousew reg0 %02X"),data);
}

void MOUSE::set_reg1(uint8_t data)
{
	reg_mouse[1] = data;
	if (reg_mouse[1] & 0x01) {
		mst[0].pos = 0x1fff;
	}
	if (reg_mouse[1] & 0x02) {
		mst[0].dir = 0x20;
	}
	if (reg_mouse[1] & 0x04) {
		mst[1].pos = 0x1fff;
	}
	if (reg_mouse[1] & 0x08) {
		mst[1].dir = 0x20;
	}

//	logging->out_debugf(_T("mousew reg1 %02X"),data);
}

uint32_t MOUSE::read_io8(uint32_t addr)
{
	uint32_t data = 0;
	switch(addr & 3) {
	case 0:
		data = ((mst[0].pos & 0x1f00) >> 8) | mst[0].dir | (mst[0].btn ^ 0x40);
		// release IRQ signal
		mst[0].btn &= ~MOUSE_BUTTON_TRIGGER;
		d_board->write_signal(SIG_CPU_IRQ, 0, SIG_IRQ_MOUSE_MASK);
		break;
	case 1:
		data = (mst[0].pos & 0xff);
		break;
	case 2:
		data = ((mst[1].pos & 0x1f00) >> 8) | mst[1].dir | (mst[1].btn ^ 0x40);
		// release IRQ signal
		mst[1].btn &= ~MOUSE_BUTTON_TRIGGER;
		d_board->write_signal(SIG_CPU_IRQ, 0, SIG_IRQ_MOUSE_MASK);
		break;
	case 3:
		data = (mst[1].pos & 0xff);
		break;
	}

//	logging->out_debugf(_T("mouser %x %02x  stat:%4d %4d %08X  mst 0dir:%d pos:%d 1dir:%d pos:%d")
//		,addr,data
//		,mouse_stat[0],mouse_stat[1],mouse_stat[2],mst[0].dir,mst[0].pos,mst[1].dir,mst[1].pos);

	return data;
}

void MOUSE::update_mouse()
{
	int i;
	bool now_irq = false;

#ifdef USE_KEY_RECORD
	reckey->processing_mouse_status(mouse_stat);
#endif

	if (FLG_USEMOUSE != 0 || config.reckey_playing) {
		for(i = 0; i < 2; i++) {
#ifndef USE_MOUSE_ABSOLUTE
			mst[i].pos += (mouse_stat[i] * cntdir[i]);

//			if (mst[i].pos > 0x1fff) mst[i].pos = 0x1fff;
//			if (mst[i].pos < 0) mst[i].pos = 0;
			mst[i].pos &= 0x1fff;
			mst[i].dir = (mouse_stat[i] >= 0 ? 0x20 : 0);
#else
			mst[i].pos = (mouse_stat[i] & 0x1fff);
			mst[i].dir = (mouse_stat[i] & 0x8000 ? 0 : 0x20);
#endif
		}

		for(i = 0; i < 2; i++) {
			mst[i].prev_btn = (mst[i].btn & MOUSE_BUTTON_ONOFF);
			mst[i].btn = ((mouse_stat[2] & (1 << i)) ? MOUSE_BUTTON_ONOFF : 0);
			if (mst[i].btn != mst[i].prev_btn) {
				if (((reg_mouse[0] & 0x40) ^ mst[i].btn) == 0 || FLG_ORIG_MOUSEEG) {
					// button pressed
					mst[i].btn |= MOUSE_BUTTON_TRIGGER;
					if ((reg_mouse[0] & 0x80) != 0) {
						// if irq enable
						now_irq = true;
					}
				}
			}
		}
	}

#if 0
	if (config.reckey_playing) {
		memcpy(mst, reckey->get_mouse_recp_stat(), sizeof(mst));
		for(i = 0; i < 2; i++) {
			mst[i].prev_btn = (mst[i].btn & MOUSE_BUTTON_ONOFF);
			if (mst[i].btn != mst[i].prev_btn) {
				if (((reg_mouse[0] & 0x40) ^ mst[i].btn) == 0 || FLG_ORIG_MOUSEEG) {
					// button pressed
					mst[i].btn |= MOUSE_BUTTON_TRIGGER;
					if ((reg_mouse[0] & 0x80) != 0) {
						// if irq enable
						now_irq = true;
					}
				}
			}
		}
	}
	if (config.reckey_recording) {
		reckey->recording_mouse_status(mst);
	}
#endif

	if (now_irq) {
		// send IRQ signal
		d_board->write_signal(SIG_CPU_IRQ, SIG_IRQ_MOUSE_MASK, SIG_IRQ_MOUSE_MASK);
	}

//	logging->out_debugf(_T("mouse_stat %4d %4d %08X  mst 0dir:%d pos:%d 1dir:%d pos:%d")
//		,mouse_stat[0],mouse_stat[1],mouse_stat[2],mst[0].dir,mst[0].pos,mst[1].dir,mst[1].pos);
}

void MOUSE::set_mouse_position(int px, int py)
{
	mst[0].pos = px;
	if (mst[0].pos > 0x1fff) mst[0].pos = 0x1fff;
	if (mst[0].pos < 0) mst[0].pos = 0;
	mst[1].pos = py;
	if (mst[1].pos > 0x1fff) mst[1].pos = 0x1fff;
	if (mst[1].pos < 0) mst[1].pos = 0;
}

// ----------------------------------------------------------------------------
// event handler
// ----------------------------------------------------------------------------

void MOUSE::event_frame()
{
	update_mouse();
}

// ----------------------------------------------------------------------------
void MOUSE::save_state(FILEIO *fio)
{
	struct vm_state_st vm_state;

	//
	vm_state_ident.version = Uint16_LE(0x41);
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(vm_state));

	// copy values
	memset(&vm_state, 0, sizeof(vm_state));
	for(int i=0; i<2; i++) {
		vm_state.st[i].pos = Int32_LE(mst[i].pos);
		vm_state.st[i].dir = mst[i].dir;
		vm_state.st[i].btn = mst[i].btn;
		vm_state.st[i].prev_btn = mst[i].prev_btn;
	}
	vm_state.reg0 = reg_mouse[0] & 0xf0;

	fio->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);
	fio->Fwrite(&vm_state, sizeof(vm_state), 1);
}

bool MOUSE::load_state(FILEIO *fio)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	READ_STATE_CHUNK(fio, vm_state_i, vm_state);
	for(int i=0; i<2; i++) {
		mst[i].pos = Int32_LE(vm_state.st[i].pos);
		mst[i].dir = vm_state.st[i].dir;
		mst[i].btn = vm_state.st[i].btn;
		mst[i].prev_btn = vm_state.st[i].prev_btn;
	}
	set_reg0(vm_state.reg0);
	set_reg1(0);

	return true;
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
uint32_t MOUSE::debug_read_io8(uint32_t addr)
{
	uint32_t data = 0;
	switch(addr & 3) {
	case 0:
		data = ((mst[0].pos & 0x1f00) >> 8) | mst[0].dir | mst[0].btn;
		break;
	case 1:
		data = (mst[0].pos & 0xff);
		break;
	case 2:
		data = ((mst[1].pos & 0x1f00) >> 8) | mst[1].dir | mst[1].btn;
		break;
	case 3:
		data = (mst[1].pos & 0xff);
		break;
	}

	return data;
}
#endif

