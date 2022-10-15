/** @file z80b_card.cpp

	HITACHI MB-S1 Emulator 'EmuB-S1'
	Skelton for retropc emulator

	@author Sasaji
	@date   2019.10.19 -

	@brief [ z80b extention card ]
*/


#include "z80b_card.h"

#if defined(USE_Z80B_CARD)

//#include "../../emu.h"
#include "../vm.h"
#include "registers.h"
#include "../../logging.h"
#include "../../fileio.h"
#include "../../config.h"
#include "../../utility.h"
#include "../../osd/debugger_console.h"


Z80B_CARD::Z80B_CARD(VM* parent_vm, EMU* parent_emu, const char* identifier) : DEVICE(parent_vm, parent_emu, identifier)
{
	set_class_name("Z80B_CARD");

	d_z80 = NULL;
	memory = NULL;
	memory_size = 0;
	membank = 0;
	memory_a = NULL;
	memory_b = NULL;

	out_irq_id = SIG_CPU_IRQ;
	busreq_on = true;
	z80_running = false;
}

Z80B_CARD::~Z80B_CARD()
{
}

void Z80B_CARD::initialize()
{
	set_number_of_cpu(IOPORT_USE_Z80BCARD ? 2 : 1);
	d_z80->enable(IOPORT_USE_Z80BCARD != 0);
}

/// power on reset
void Z80B_CARD::reset()
{
	SIG_MBC_INTREFKIL = 0;
	z80_running = false;
	out_irq_id = config.z80b_card_out_irq == 1 ? SIG_CPU_NMI : SIG_CPU_IRQ;

	set_number_of_cpu(IOPORT_USE_Z80BCARD ? 2 : 1);
	d_z80->enable(IOPORT_USE_Z80BCARD != 0);

	// z80 is halt at boot
	d_z80->write_signal(SIG_CPU_RESET, 1, 1);
	set_busreq(true);
}

/// set memory pointer and size
/// @param[in] mem  : memory
/// @param[in] size : memory size (in KB)
void Z80B_CARD::set_memory(uint8_t *mem, int size)
{
	memory = mem;
	memory_size = size;	// KB

	set_memory_from_membank();
}

/// assign memory position from bank setting
void Z80B_CARD::set_memory_from_membank()
{
	int msize;
	msize = ((int)membank & 7) << 6;
	memory_a = memory && msize < memory_size ? memory + (msize << 10) : NULL;
	msize = ((int)membank & 0x70) << 2;
	memory_b = memory && msize < memory_size ? memory + (msize << 10) : NULL;
}

/// write to memory from Z80
void Z80B_CARD::write_data8(uint32_t addr, uint32_t data)
{
	addr &= 0xffff;
	if ((addr & 0xe000) == 0xe000) {
		if (memory_b) memory_b[addr] = (data & 0xff);
	} else {
		if (memory_a) memory_a[addr] = (data & 0xff);
	}
}

/// read from memory to Z80
uint32_t Z80B_CARD::read_data8(uint32_t addr)
{
	uint32_t data;
	addr &= 0xffff;
	if ((addr & 0xe000) == 0xe000) {
		data = memory_b ? memory_b[addr] : 0xff;
	} else {
		data = memory_a ? memory_a[addr] : 0xff;
	}
	return data;
}

/// write to memory from Z80
void Z80B_CARD::write_data8w(uint32_t addr, uint32_t data, int *wait)
{
	addr &= 0xffff;
	if ((addr & 0xe000) == 0xe000) {
		if (memory_b) memory_b[addr] = (data & 0xff);
	} else {
		if (memory_a) memory_a[addr] = (data & 0xff);
	}
	if (wait) *wait = 0;
}

/// read from memory to Z80
uint32_t Z80B_CARD::read_data8w(uint32_t addr, int *wait)
{
	uint32_t data;
	addr &= 0xffff;
	if ((addr & 0xe000) == 0xe000) {
		data = memory_b ? memory_b[addr] : 0xff;
	} else {
		data = memory_a ? memory_a[addr] : 0xff;
	}
	if (wait) *wait = 0;
	return data;
}

/// fetch op code (M1 phase)
uint32_t Z80B_CARD::fetch_op(uint32_t addr, int *wait)
{
	uint32_t data;
	addr &= 0xffff;
	if ((addr & 0xe000) == 0xe000) {
		data = memory_b ? memory_b[addr] : 0xff;
	} else {
		data = memory_a ? memory_a[addr] : 0xff;
	}
	if (wait) *wait = 1;	// 1 clock wait
	return data;
}

/// signals from each CPUs
void Z80B_CARD::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
		case SIG_CPU_BUSREQ:
			// stop MBC
			set_multibus(false);
			// send bus request signal to Z80 from MB-S1 I/O
			set_busreq((data & mask) != 0);
			break;
		case SIG_Z80BCARD_BUSACK:
			// receive bus ack signal from Z80, and send to MB-S1 I/O
			REG_BUSCTRL = (data & mask) ? BUSCTRL_SIGNAL : 0;
			set_multibus((data & mask) == 0);
			break;
		case SIG_CPU_RESET:
			now_reset = (data & mask) ? true : false;
			// now on reset signal
			d_z80->write_signal(SIG_CPU_RESET, now_reset || !z80_running ? 1 : 0, 1);
			if (now_reset) {
				// mbc disable
				set_multibus(false);
				// busreq signal keep on even if reset turned off.
				set_busreq(true);
			}
			break;
	}
}

/// write to I/O from Z80 cpu
void Z80B_CARD::write_io8(uint32_t addr, uint32_t data)
{
	// set memory bank and fire interrupt or not
	membank = data & 0xf7;

	set_memory_from_membank();

	set_interrupt((membank & 0x80) != 0);
}

/// read I/O from Z80 cpu
uint32_t Z80B_CARD::read_io8(uint32_t addr)
{
	return 0xff;
}

/// write to I/O from 6809
void Z80B_CARD::write_memory_mapped_io8(uint32_t addr, uint32_t data)
{
	if ((addr & 0x1ffff) == 0x0ff7f) {
		if (!SIG_MBC_IS_ON) {
			z80_running = ((data & 0x80) != 0);
			d_z80->write_signal(SIG_CPU_RESET, now_reset || !z80_running ? 1 : 0, 1);
			if (!busreq_on) set_busreq(!z80_running);
		}
	}
}

/// read I/O from 6809 cpu
uint32_t Z80B_CARD::read_memory_mapped_io8(uint32_t addr)
{
	return 0xff;
}

/// memory mapping
uint32_t Z80B_CARD::address_mapping(uint32_t addr)
{
	if ((addr & 0xe000) == 0xe000) {
		addr |= (((uint32_t)membank & 0x70) << 12);
	} else {
		addr |= (((uint32_t)membank & 7) << 16);
	}
	return addr;
}

/// bus request to Z80
void Z80B_CARD::set_busreq(bool onoff)
{
	d_z80->write_signal(SIG_CPU_BUSREQ, onoff || !z80_running ? 1 : 0, 1);
	busreq_on = onoff;
}

/// send interruput to 6809
void Z80B_CARD::set_interrupt(bool onoff)
{
	d_board->write_signal(out_irq_id, (onoff && SIG_MBC_IS_ON) ? SIG_IRQ_Z80BCARD_MASK : 0, SIG_IRQ_Z80BCARD_MASK);
}

/// Multi Bus Control
void Z80B_CARD::set_multibus(bool onoff)
{
	SIG_MBC_INTREFKIL = onoff ? SIG_MBC_INTREFKIL | SIG_MBC_ALL_MASK : SIG_MBC_INTREFKIL & ~SIG_MBC_ALL_MASK;
	if (!SIG_MBC_IS_ON) {
		membank &= 0x7f;
		set_interrupt(false);
	}
}

// ----------------------------------------------------------------------------
// event handler
// ----------------------------------------------------------------------------

//void Z80B_CARD::event_frame()
//{
//}

//void Z80B_CARD::event_callback(int event_id, int err)
//{
//}

// ----------------------------------------------------------------------------

void Z80B_CARD::save_state(FILEIO *fio)
{
	struct vm_state_st vm_state;

	//
	vm_state_ident.version = Uint16_LE(1);
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(vm_state));

	// copy values
	memset(&vm_state, 0, sizeof(vm_state));
	vm_state.membank = membank;
	vm_state.mbc = SIG_MBC_INTREFKIL;
	vm_state.condition = (z80_running ? 1 : 0) | (busreq_on ? 2 : 0);
	vm_state.out_irq = (out_irq_id == SIG_CPU_NMI ? 1 : 0) | ((config.z80b_card_out_irq & 4) << 4);

	fio->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);
	fio->Fwrite(&vm_state, sizeof(vm_state), 1);
}

bool Z80B_CARD::load_state(FILEIO *fio)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	READ_STATE_CHUNK(fio, vm_state_i, vm_state);

	membank = vm_state.membank;
	SIG_MBC_INTREFKIL = vm_state.mbc;
	z80_running = ((vm_state.condition & 1) != 0);
	busreq_on = ((vm_state.condition & 2) != 0);
	out_irq_id = (vm_state.out_irq & 4) == 1 ? SIG_CPU_NMI : SIG_CPU_IRQ;
	config.z80b_card_out_irq = (vm_state.out_irq >> 4);

	set_memory_from_membank();

	return true;
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
uint32_t Z80B_CARD::debug_latch_address(uint32_t addr)
{
	return address_mapping(addr & 0xffff);
}

void Z80B_CARD::debug_write_data8(int type, uint32_t addr, uint32_t data)
{
	debug_write_data8w(type, addr, data, NULL);
}

uint32_t Z80B_CARD::debug_read_data8(int type, uint32_t addr)
{
	return debug_read_data8w(type, addr, NULL);
}

void Z80B_CARD::debug_write_data8w(int type, uint32_t addr, uint32_t data, int *wait)
{
	if (type >= 0) {
		if (addr < (uint32_t)(memory_size << 10)) {
			memory[addr] = (data & 0xff);
		}
	} else {
		write_data8w(addr, data, wait);
	}
}

uint32_t Z80B_CARD::debug_read_data8w(int type, uint32_t addr, int *wait)
{
	if (type >= 0) {
		// physical address
		return addr < (uint32_t)(memory_size << 10) ? memory[addr] : 0xff;
	} else {
		return read_data8w(addr, wait);
	}
}

/// use DP,EP command in debugger
uint32_t Z80B_CARD::debug_physical_addr_mask(int type)
{
	uint32_t data = 0;
	switch(type) {
	case 0:
		data = 0xfffff;
		break;
	}
	return data;
}

/// use DP,EP command in debugger
bool Z80B_CARD::debug_physical_addr_type_name(int type, _TCHAR *buffer, size_t buffer_len)
{
	bool exist = true;
	switch(type) {
	case 0:
		UTILITY::tcscpy(buffer, buffer_len, _T("using physical address."));
		break;
	default:
		exist = false;
		break;
	}
	return exist;
}

/// assined memory type (use M command in debugger)
uint32_t Z80B_CARD::debug_read_bank(uint32_t addr)
{
	uint32_t data = 0;
	if ((addr & 0xe000) == 0xe000) {
		addr = (((uint32_t)membank & 0x70) << 12);
	} else {
		addr = (((uint32_t)membank & 7) << 16);
	}
	if (addr < (uint32_t)(memory_size << 10)) {
		data = (addr >> 16) | (addr >> 12);
	} else {
		data = 0xff;
	}
	return data;
}

/// assined memory type (use M command in debugger)
void Z80B_CARD::debug_memory_map_info(DebuggerConsole *dc)
{
	uint32_t prev_addr = 0;
	uint8_t  prev_data = 0;
	uint32_t end_addr = 0x10000;
	uint32_t inc_addr = 16;

	for(uint32_t addr=0; addr <= end_addr; addr+=inc_addr) {
		uint8_t data = debug_read_bank(addr);
		if (addr == 0) {
			prev_data = data;
		}
		if (data != prev_data || addr == end_addr) {
			dc->Printf(_T("%04X - %04X : Read:"), prev_addr, addr-1);
			print_memory_bank(prev_data, false, dc->GetBuffer(true), dc->GetBufferSize());
			dc->Out(false);
			dc->Print(_T("  Write:"), false);
			print_memory_bank(prev_data, true, dc->GetBuffer(true), dc->GetBufferSize());
			dc->Out(false);
			dc->Cr();
			prev_addr = addr;
			prev_data = data;
		}
	}
}

/// assined memory type (use M command in debugger)
void Z80B_CARD::print_memory_bank(uint32_t data, bool w, _TCHAR *buffer, size_t buffer_len)
{
	_TCHAR str[32];
	if (w) {
		data >>= 4;
	} else {
		data &= 0xf;
	}
	if (data < 8) {
		UTILITY::stprintf(str, sizeof(str) / sizeof(str[0]), _T("Extended 0x%X0000"), data);
	} else {
		UTILITY::tcscpy(str, sizeof(str) / sizeof(str[0]), _T("(no assign)"));
	}
	UTILITY::stprintf(buffer, buffer_len, _T("%-18s"), str); 
}

/// memory mapping reverse
uint32_t Z80B_CARD::debug_address_mapping_rev(uint32_t addr)
{
	addr &= 0xffff;
	return addr;
}

void Z80B_CARD::debug_write_io8(uint32_t addr, uint32_t data)
{
	write_io8(addr, data);
}

uint32_t Z80B_CARD::debug_read_io8(uint32_t addr)
{
	return membank;
}

void Z80B_CARD::debug_write_memory_mapped_io8(uint32_t addr, uint32_t data)
{
	write_memory_mapped_io8(addr, data);
}

uint32_t Z80B_CARD::debug_read_memory_mapped_io8(uint32_t addr)
{
	return read_memory_mapped_io8(addr);
}
#endif

#endif /* USE_Z80B_CARD */
