/** @file mpc_68008.cpp

	HITACHI MB-S1 Emulator 'EmuB-S1'
	Skelton for retropc emulator

	@author Sasaji
	@date   2022.05.05 -

	@brief [ MPC-68008 - mc68008 extention card ]
*/


#include "mpc_68008.h"

#if defined(USE_MPC_68008)

//#include "../../emu.h"
#include "../vm.h"
#include "registers.h"
#include "memory.h"
#include "../mc68000.h"
#include "../../logging.h"
#include "../../fileio.h"
#include "../../config.h"
#include "../../utility.h"
#include "../../osd/debugger_console.h"


MPC_68008::MPC_68008(VM* parent_vm, EMU* parent_emu, const char* identifier) : DEVICE(parent_vm, parent_emu, identifier)
{
	set_class_name("MPC_68008");

	d_mc68k = NULL;
	d_memory = NULL;
	p_memory = NULL;
	m_memory_size = 0;

	m_pro_reg = PRO_REG_NOBQ | PRO_REG_RESET;
	m_acc_reg = 0;

	m_now_fc = 0;
	m_assert_intr = 0;
	m_6809_halt = 0;

	m_haltoff_register_id = -1;
}

MPC_68008::~MPC_68008()
{
}

void MPC_68008::cancel_my_event(int &id)
{
	if(id != -1) {
		cancel_event(this, id);
		id = -1;
	}
}

void MPC_68008::register_my_event(uint32_t wait_clk, int &id)
{
	cancel_my_event(id);
	register_event_by_clock(this, SIG_HALTOFF, wait_clk, false, &id);
}

void MPC_68008::initialize()
{
	set_number_of_cpu(IOPORT_USE_MPC68008 ? 2 : 1);
	d_mc68k->enable(IOPORT_USE_MPC68008 != 0);
}

/// power on reset
void MPC_68008::reset()
{
	set_number_of_cpu(IOPORT_USE_MPC68008 ? 2 : 1);
	d_mc68k->enable(IOPORT_USE_MPC68008 != 0);

	warm_reset(true);
}

void MPC_68008::warm_reset(bool por)
{
	SIG_MBC_INTREFKIL = 0;
	m_pro_reg = PRO_REG_NOBQ | PRO_REG_RESET;
	m_acc_reg = 0;
	m_assert_intr = 0;
	m_6809_halt = 0;
	m_now_fc = 0;

	if (!por) {
		cancel_my_event(m_haltoff_register_id);
	}
	m_haltoff_register_id = -1;

	// degate HALT signal
	d_board->write_signal(SIG_CPU_HALT, 0, SIG_HALT_MPC68008_MASK);

	// mc68008 is reset and halt at boot
	update_reset_signal();
	update_busreq();
}

/// set memory pointer and size
/// @param[in] mem  : memory
/// @param[in] size : memory size (in KB)
void MPC_68008::set_memory(uint8_t *mem, int size)
{
	p_memory = mem;
	m_memory_size = (uint32_t)size * 1024;
}

/// write to memory from MC68008
void MPC_68008::write_data8(uint32_t addr, uint32_t data)
{
}

/// read from memory to MC68008
uint32_t MPC_68008::read_data8(uint32_t addr)
{
	return 0xff;
}

/// write to memory from MC68008
void MPC_68008::write_data8w(uint32_t addr, uint32_t data, int *wait)
{
	addr &= 0xfffff;
	if (addr < 0x80000) {
		// 6809 halt off
		degate_halt_to_6809();
		// extended memory
		if (addr < m_memory_size) {
			p_memory[addr] = data & 0xff;
		}
	} else {
		// 6809 halt on
		assert_halt_to_6809();
		// data bus in the main board
		d_memory->write_data8_68kw(addr, data, wait);
	}
}

/// read from memory to MC68008
uint32_t MPC_68008::read_data8w(uint32_t addr, int *wait)
{
	uint32_t data = 0xff;
	addr &= 0xfffff;

	update_intr_condition();
	if (m_now_fc != 7) {
		if (addr < 0x80000) {
			// 6809 halt off
			degate_halt_to_6809();
			// extended memory
			if (addr < m_memory_size) {
				data = p_memory[addr];
			}
		} else {
			// 6809 halt on
			assert_halt_to_6809();
			// data bus in the main board
			data = d_memory->read_data8_68kw(addr, wait);
		}
	}
	return data;
}

/// degate interrupt
void MPC_68008::update_intr_condition()
{
	if (m_assert_intr & 0x05) {
		// degate interrupt
		m_assert_intr &= ~0x05;
		update_intr();
		m_pro_reg &= ~PRO_REG_INT;
	}
}

/// signals from each CPUs
void MPC_68008::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
		case SIG_CPU_BUSREQ:
			// stop MBC
			set_multibus(false);
			// send bus request signal to MC68008 from MB-S1 I/O $FF19
			BIT_ONOFF(m_pro_reg, PRO_REG_NOBQ, (data & mask) == 0);
			update_busreq();
			break;
		case SIG_BGACK:
			// receive bus ack signal from MC68008, and send to MB-S1 I/O $FF19
			REG_BUSCTRL = (data & mask) ? 0 : BUSCTRL_SIGNAL;
			set_multibus((data & mask) == 0);
			break;
		case SIG_CPU_NMI:
			if ((data & mask) != 0 && (data & SIG_NMI_TRAP_MASK) == 0) {
				// assert interrupt level 2
				m_assert_intr |= 0x02;
				update_intr();
			} else if ((data & mask) == 0) {
				// degate interrupt level 2
				m_assert_intr &= ~0x02;
				update_intr();
			}
			break;
		case SIG_M68K_FC:
			// function code
			if (data == 7) {
				if (m_now_fc != 7) d_mc68k->write_signal(SIG_M68K_VPA_AVEC, 1, 1);
			} else {
				if (m_now_fc == 7) d_mc68k->write_signal(SIG_M68K_VPA_AVEC, 0, 1);
			}
			m_now_fc = data;
			break;
		case SIG_CPU_RESET:
			now_reset = ((data & mask) != 0);
			if (now_reset) {
				// mbc disable
				set_multibus(false);
				// now on reset signal
				warm_reset(false);
			} else {
				// now on reset signal
				warm_reset(false);
			}
			break;
	}
}

/// write to I/O from main bus
void MPC_68008::write_memory_mapped_io8(uint32_t addr, uint32_t data)
{
	addr &= 0xfffff;
	switch(addr) {
	case 0xefe1a:
		// PRO CONTROL
		if ((m_pro_reg & PRO_REG_INT) == 0 && (data & PRO_REG_INT) != 0) {
			// assert interrupt level 5
			m_assert_intr |= 0x05;
			update_intr();
		}
//		else if ((m_pro_reg & PRO_REG_INT) != 0 && (data & PRO_REG_INT) == 0) {
//			// degate interrupt level 5
//			m_assert_intr &= ~0x05;
//			update_intr();
//		}
		// update register with clearing reset flag
		m_pro_reg = (data & PRO_REG_MASK) | (m_pro_reg & ~(PRO_REG_MASK));
		if (data & PRO_REG_R68) {
			m_pro_reg &= ~PRO_REG_RESET;
		}
		update_reset_signal();
		update_busreq();
		break;
	case 0xefe1b:
		// ACC CONTROL
		if ((m_acc_reg & ACC_REG_BS) == 0 && (data & ACC_REG_BS) != 0) {
			// assert HALT signal to 6809
			m_acc_reg = (data & ACC_REG_MASK);
			assert_halt_to_6809();
		} else if ((m_acc_reg & ACC_REG_BS) != 0 && (data & ACC_REG_BS) == 0) { 
			m_acc_reg = (data & ACC_REG_MASK);
			if ((REG_BUSCTRL & BUSCTRL_SIGNAL) == 0) {
				// degate HALT signal to 6809 if MC68008 is stopping by busreq
				degate_halt_to_6809();
			}
		} else {
			m_acc_reg = (data & ACC_REG_MASK);
		}
		break;
	}
}

/// read I/O from I/O to main bus
uint32_t MPC_68008::read_memory_mapped_io8(uint32_t addr)
{
	uint32_t data = 0xff;
	addr &= 0xfffff;
	switch(addr) {
	case 0xefe1a:
		// PRO CONTROL
		if (IOPORT_USE_MPC68008) data &= 0xfe;
		break;
	}
	return data;
}

#if 0
/// memory mapping
uint32_t MPC_68008::address_mapping(uint32_t addr)
{
	if ((addr & 0xe000) == 0xe000) {
		addr |= (((uint32_t)m_membank & 0x70) << 12);
	} else {
		addr |= (((uint32_t)m_membank & 7) << 16);
	}
	return addr;
}
#endif

/// reset to MC68008
void MPC_68008::update_reset_signal()
{
	if (now_reset || (m_pro_reg & PRO_REG_RESET) != 0) {
		d_mc68k->write_signal(SIG_CPU_HALT, 1, 1);
		d_mc68k->write_signal(SIG_CPU_RESET, 1, 1);
	} else {
		d_mc68k->write_signal(SIG_CPU_RESET, 0, 1);
		d_mc68k->write_signal(SIG_CPU_HALT, 0, 1);
	}
}

/// bus request to MC68008
void MPC_68008::update_busreq()
{
	d_mc68k->write_signal(SIG_CPU_BUSREQ, (m_pro_reg & (PRO_REG_NOBQ | PRO_REG_R68)) == (PRO_REG_NOBQ | PRO_REG_R68) ? 0 : 1, 1);
}

/// send interruput to MC68008
void MPC_68008::update_intr()
{
	uint32_t intr = m_assert_intr;
	if ((intr & 0x05) == 0x05) intr = 0x05;
	d_mc68k->write_signal(SIG_CPU_IRQ, 1 << intr, 0xfe);
}

/// Multi Bus Control
void MPC_68008::set_multibus(bool onoff)
{
	BIT_ONOFF(SIG_MBC_INTREFKIL, SIG_MBC_ALL_MASK, onoff);
}

/// assert halt signal to stop 6809
void MPC_68008::assert_halt_to_6809()
{
	if (m_6809_halt == 0) {
		d_board->write_signal(SIG_CPU_HALT, SIG_HALT_MPC68008_MASK, SIG_HALT_MPC68008_MASK);
		set_multibus(false);
		m_6809_halt = 1;
	}
}

/// degate halt signal to resume 6809
void MPC_68008::degate_halt_to_6809()
{
	if (m_6809_halt != 0 && (m_acc_reg & ACC_REG_BS) == 0) {
		int mainclk = (int)(get_current_clock() % CLOCKS_CYCLE);
		int subclk = (int)(d_mc68k->get_current_clock() >> MAIN_SUB_CLOCK_RATIO);
		int sum = subclk - mainclk;
		if (sum < 0) sum = 1;
		register_my_event(sum, m_haltoff_register_id);
	}
}

// ----------------------------------------------------------------------------
// event handler
// ----------------------------------------------------------------------------

//void MPC_68008::event_frame()
//{
//}

void MPC_68008::event_callback(int event_id, int err)
{
	if (event_id == SIG_HALTOFF) {
		d_board->write_signal(SIG_CPU_HALT, 0, SIG_HALT_MPC68008_MASK);
		set_multibus((REG_BUSCTRL & BUSCTRL_SIGNAL) != 0);
		m_6809_halt = 0;
		m_haltoff_register_id = -1;
	}
}

// ----------------------------------------------------------------------------

void MPC_68008::save_state(FILEIO *fio)
{
	struct vm_state_st vm_state;

	//
	vm_state_ident.version = Uint16_LE(1);
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(vm_state));

	// copy values
	memset(&vm_state, 0, sizeof(vm_state));
	vm_state.m_mbc = SIG_MBC_INTREFKIL;
	vm_state.m_pro_reg = m_pro_reg;
	vm_state.m_acc_reg = m_acc_reg;
	vm_state.m_memory_size = Uint32_LE(m_memory_size);
	vm_state.m_now_fc = Uint32_LE(m_now_fc);
	vm_state.m_assert_intr = Uint32_LE(m_assert_intr);
	vm_state.m_6809_halt = Uint32_LE(m_6809_halt);
	vm_state.m_haltoff_register_id = Int32_LE(m_haltoff_register_id);

	fio->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);
	fio->Fwrite(&vm_state, sizeof(vm_state), 1);
}

bool MPC_68008::load_state(FILEIO *fio)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	READ_STATE_CHUNK(fio, vm_state_i, vm_state);

	SIG_MBC_INTREFKIL = vm_state.m_mbc;
	m_pro_reg = vm_state.m_pro_reg;
	m_acc_reg = vm_state.m_acc_reg;
	m_memory_size = Uint32_LE(vm_state.m_memory_size);
	m_now_fc = Uint32_LE(vm_state.m_now_fc);
	m_assert_intr = Uint32_LE(vm_state.m_assert_intr);
	m_6809_halt = Uint32_LE(vm_state.m_6809_halt);
	m_haltoff_register_id = Int32_LE(vm_state.m_haltoff_register_id);

	return true;
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
uint32_t MPC_68008::debug_latch_address(uint32_t addr)
{
	return addr;
}

void MPC_68008::debug_write_data8(int type, uint32_t addr, uint32_t data)
{
	d_memory->debug_write_data8(0, addr, data);
}

uint32_t MPC_68008::debug_read_data8(int type, uint32_t addr)
{
	return d_memory->debug_read_data8(0, addr);
}

void MPC_68008::debug_write_data8w(int type, uint32_t addr, uint32_t data, int *wait)
{
	d_memory->debug_write_data8(0, addr, data);
}

uint32_t MPC_68008::debug_read_data8w(int type, uint32_t addr, int *wait)
{
	return d_memory->debug_read_data8(0, addr);
}

void MPC_68008::debug_write_data16(int type, uint32_t addr, uint32_t data)
{
	// big endien
	debug_write_data8(type, addr, (data >> 8) & 0xff);
	debug_write_data8(type, addr + 1, data & 0xff);
}

uint32_t MPC_68008::debug_read_data16(int type, uint32_t addr)
{
	// big endien
	uint32_t val = debug_read_data8(type, addr);
	val <<= 8;
	val |= debug_read_data8(type, addr + 1);
	return val;
}

/// use DP,EP command in debugger
uint32_t MPC_68008::debug_physical_addr_mask(int type)
{
	return 0xfffff;
}

/// use DP,EP command in debugger
bool MPC_68008::debug_physical_addr_type_name(int type, _TCHAR *buffer, size_t buffer_len)
{
	return false;
}

/// assined memory type (use M command in debugger)
uint32_t MPC_68008::debug_read_bank(uint32_t addr)
{
	return d_memory->debug_read_bank(addr);
}

/// assined memory type (use M command in debugger)
void MPC_68008::debug_memory_map_info(DebuggerConsole *dc)
{
	d_memory->debug_memory_map_info(dc);
}

/// assined memory type (use M command in debugger)
void MPC_68008::print_memory_bank(uint32_t data, bool w, _TCHAR *buffer, size_t buffer_len)
{
	d_memory->print_memory_bank(data, w, buffer, buffer_len);
}

/// memory mapping reverse
uint32_t MPC_68008::debug_address_mapping_rev(uint32_t addr)
{
	return addr;
}

void MPC_68008::debug_write_memory_mapped_io8(uint32_t addr, uint32_t data)
{
	write_memory_mapped_io8(addr, data);
}

uint32_t MPC_68008::debug_read_memory_mapped_io8(uint32_t addr)
{
	return read_memory_mapped_io8(addr);
}

static const _TCHAR *c_reg_names[] = {
	_T("BUS_CTRL"),
	_T("PRO_CTRL"),
	_T("ACC_CTRL"),
	NULL
};

bool MPC_68008::debug_write_reg(uint32_t reg_num, uint32_t data)
{
	switch(reg_num) {
	case 0:
	case 0xfe19:
	case 0xefe19:
		write_signal(SIG_CPU_BUSREQ, data, 0x80);
		break;
	case 1:
	case 2:
		write_memory_mapped_io8(reg_num + 0xefe19, data);
		break;
	case 0xfe1a:
	case 0xfe1b:
		write_memory_mapped_io8(reg_num + 0xe0000, data);
		break;
	case 0xefe1a:
	case 0xefe1b:
		write_memory_mapped_io8(reg_num, data);
		break;
	default:
		return false;
	}
	return true;
}

bool MPC_68008::debug_write_reg(const _TCHAR *reg, uint32_t data)
{
	uint32_t num = find_debug_reg_name(c_reg_names, reg);
	return debug_write_reg(num, data);
}

void MPC_68008::debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	buffer[0] = _T('\0');

	UTILITY::sntprintf(buffer, buffer_len, _T(" %d(EFE19:%s):rdata:%02X\n"), 0, c_reg_names[0], REG_BUSCTRL);
	UTILITY::sntprintf(buffer, buffer_len, _T(" %d(EFE1A:%s):wdata:%02X\n"), 1, c_reg_names[1], m_pro_reg & PRO_REG_MASK);
	UTILITY::sntprintf(buffer, buffer_len, _T(" %d(EFE1B:%s):wdata:%02X\n"), 2, c_reg_names[2], m_acc_reg);
}

#endif

#endif /* USE_MPC_68008 */
