/** @file timer.cpp

	HITACHI MB-S1 Emulator 'EmuB-S1'
	Skelton for retropc emulator

	@author Sasaji
	@date   2011.06.08 -

	@brief [ timer ]
*/

#include "timer.h"
//#include "../../emu.h"
#include "registers.h"
#include "../../fileio.h"
#include "../../utility.h"

void TIMER::initialize()
{
	timer_irq = 0;
	REG_TIME_MASK = 0;
	cnt_2msec = 0;
	register_id = -1;

#ifdef USE_TIMER_EVENT
	register_id_t1 = -1;
#else
	register_vline_event(this);	// 60Hz timer
#endif
}

void TIMER::reset()
{
	timer_irq = 0;
	REG_TIME_MASK = 0;
	cnt_2msec = 0;

#ifdef USE_TIMER_EVENT
	if (register_id_t1 >= 0) {
		cancel_event(this, register_id_t1);
		register_id_t1 = -1;
	}
	register_event_by_clock(this, SIG_TIMER_60HZ, 33600, true, &register_id_t1);
#endif
	if (register_id >= 0) {
		cancel_event(this, register_id);
		register_id = -1;
	}
	if (NOW_S1_MODE) {
		REG_TIME_MASK = 0x82;
//		register_event(this, SIG_TIMER_1MSEC, 1000, true, &register_id);
		register_event_by_clock(this, SIG_TIMER_1MSEC, 2048, true, &register_id);
	}
}

void TIMER::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch (id) {
//		case SIG_TIMER_VSYNC:
//			if (data & mask) update_timer_clock();
//			break;
		case SIG_CPU_RESET:
			now_reset = (data & mask) ? true : false;
			if (!now_reset) {
				reset();
			}
			break;
	}
}

void TIMER::write_io8(uint32_t addr, uint32_t data)
{
	switch (addr & 0xffff) {
		case ADDR_TIMER_IRQ:
			break;
		case ADDR_TIME_MASK:
			REG_TIME_MASK = data & 0x83;
			if (REG_TIME_MASK & 0x80) {
				timer_irq &= 0x7f;
				// Clear FIRQ
				d_board->write_signal(SIG_CPU_FIRQ, 0, SIG_FIRQ_TIMER1_MASK);
			}
			if (REG_TIME_MASK & 0x02) {
				timer_irq &= 0xbf;
				// Clear FIRQ
				d_board->write_signal(SIG_CPU_FIRQ, 0, SIG_FIRQ_TIMER2_MASK);
			}
//			logging->out_debugf(_T("timer w %04x %02x"), addr, data);
			break;
	}
}

uint32_t TIMER::read_io8(uint32_t addr)
{
	uint32_t data = 0;
	switch (addr & 0xffff) {
		case ADDR_TIMER_IRQ:
			data = timer_irq;
			timer_irq = 0;
			// Clear FIRQ
			d_board->write_signal(SIG_CPU_FIRQ, 0, SIG_FIRQ_TIMER_MASK);
//			logging->out_debugf(_T("timer r %04x %02x"), addr, data);
			break;
		case ADDR_TIME_MASK:
			break;
	}
	return data;
}

/// 60Hz timer
void TIMER::update_timer_clock()
{
	if (!(REG_TIME_MASK & 0x80) && !now_reset) {
		timer_irq |= 0x80;		// timer irq set
		// FIRQ interrupt
		d_board->write_signal(SIG_CPU_FIRQ, SIG_FIRQ_TIMER1_MASK, SIG_FIRQ_TIMER1_MASK);
//		logging->out_debugf("timer normal set irq %02x", timer_irq);
	}
}

/// 1msec fast timer
void TIMER::update_fast_timer_clock()
{
	if (((REG_TIME_MASK & 0x03) == 1 || ((REG_TIME_MASK & 0x03) == 0 && cnt_2msec != 0)) && !now_reset) {
		timer_irq |= 0x40;		// timer irq set
		// FIRQ interrupt
		d_board->write_signal(SIG_CPU_FIRQ, SIG_FIRQ_TIMER2_MASK, SIG_FIRQ_TIMER2_MASK);
//		logging->out_debugf("timer fast   set irq %02x", timer_irq);
	}
	cnt_2msec = (1-cnt_2msec);
}

// ----------------------------------------------------------------------------
// event handler
// ----------------------------------------------------------------------------

void TIMER::event_vline(int v, int clock)
{
	if (v == LINES_PER_FRAME - 2) update_timer_clock();
}

void TIMER::event_callback(int event_id, int err)
{
#ifdef USE_TIMER_EVENT
	if (event_id == SIG_TIMER_60HZ) update_timer_clock();
	else
#endif
	update_fast_timer_clock();
}

// ----------------------------------------------------------------------------

void TIMER::save_state(FILEIO *fio)
{
	struct vm_state_st vm_state;

	//
	vm_state_ident.version = Uint16_LE(0x41);
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(vm_state));

	// copy values
	memset(&vm_state, 0, sizeof(vm_state));
	vm_state.timer_irq = timer_irq;
	vm_state.time_mask = REG_TIME_MASK;
	vm_state.cnt_2msec = cnt_2msec;
	vm_state.register_id = Int32_LE(register_id);

	fio->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);
	fio->Fwrite(&vm_state, sizeof(vm_state), 1);
}

bool TIMER::load_state(FILEIO *fio)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	READ_STATE_CHUNK(fio, vm_state_i, vm_state);

	timer_irq = vm_state.timer_irq;
	REG_TIME_MASK = vm_state.time_mask;

	if (Uint16_LE(vm_state_i.version) >= 0x41) {
		cnt_2msec = vm_state.cnt_2msec;
		register_id = Int32_LE(vm_state.register_id);
	} else {
		cnt_2msec = 0;
		register_id = -1;
	}

	return true;
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
uint32_t TIMER::debug_read_io8(uint32_t addr)
{
	uint32_t data = 0;
	switch (addr & 0xffff) {
		case ADDR_TIMER_IRQ:
			data = timer_irq;
			break;
		case ADDR_TIME_MASK:
			break;
	}
	return data;
}

bool TIMER::debug_write_reg(uint32_t reg_num, uint32_t data)
{
	switch(reg_num) {
	case 0:
		timer_irq = (data & 0xc0);
		// FIRQ interrupt
		d_board->write_signal(SIG_CPU_FIRQ, (timer_irq & 0x80) && (REG_TIME_MASK & 0x80) == 0 ? SIG_FIRQ_TIMER1_MASK : 0, SIG_FIRQ_TIMER1_MASK);
		d_board->write_signal(SIG_CPU_FIRQ, (timer_irq & 0x40) && (REG_TIME_MASK & 0x80) == 0 ? SIG_FIRQ_TIMER2_MASK : 0, SIG_FIRQ_TIMER2_MASK);
		return true;
	case 1:
		write_io8(ADDR_TIME_MASK, data);
		return true;
	}
	return false;
}

bool TIMER::debug_write_reg(const _TCHAR *reg, uint32_t data)
{
	return false;
}

void TIMER::debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	buffer[0] = _T('\0');
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 0, _T("TIMER"), timer_irq);
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 1, _T("TIME_MASK"), REG_TIME_MASK);
}
#endif

