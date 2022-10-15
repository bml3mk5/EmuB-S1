/** @file mpc_68008.h

	HITACHI MB-S1 Emulator 'EmuB-S1'
	Skelton for retropc emulator

	@author Sasaji
	@date   2022.05.05 -

	@brief [ MPC-68008 - mc68008 extention card ]
*/

#ifndef _MPC_68008_H_
#define _MPC_68008_H_

#include "../vm_defs.h"

#if defined(USE_MPC_68008)

#include "../device.h"

//#define SIG_Z80BCARD_BUSACK	151

class EMU;
class MC68000BASE;
class MEMORY;

/**
	@brief MPC-68008 - mc68008 extention card
*/
class MPC_68008 : public DEVICE
{
public:
public:
	/// @brief signals
	enum en_signals {
		SIG_BGACK = 1,
		SIG_HALTOFF = 2
	};

private:
	MC68000BASE *d_mc68k;
	DEVICE *d_board;
	MEMORY *d_memory;

	uint8_t *p_memory;	///< extended memory
	uint32_t m_memory_size;	///< size of p_memory (bytes)

	enum en_pro_reg_flags {
		PRO_REG_R68 = 0x01,
		PRO_REG_INT = 0x10,
		PRO_REG_RESET = 0x80,	///< reset on (original flag on emu)
		PRO_REG_NOBQ = 0x40,	///< no busreq (original flag on emu)
		PRO_REG_MASK = 0x11,
	};
	uint8_t m_pro_reg;
	enum en_acc_reg_flags {
		ACC_REG_BS = 0x01,
		ACC_REG_AS = 0x08,
		ACC_REG_MASK = 0x09,
	};
	uint8_t m_acc_reg;

	uint32_t m_now_fc;		///< function code

	uint32_t m_assert_intr;
	uint32_t m_6809_halt;

	int m_haltoff_register_id;

	//for resume
#pragma pack(1)
	struct vm_state_st {
		uint8_t  m_mbc;
		uint8_t  m_pro_reg;
		uint8_t  m_acc_reg;
		char     reserved1;
		uint32_t m_memory_size;
		uint32_t m_now_fc;
		uint32_t m_assert_intr;

		uint32_t m_6809_halt;
		int32_t  m_haltoff_register_id;
		char     reserved2[8];
	};
#pragma pack()

	inline void cancel_my_event(int &id);
	inline void register_my_event(uint32_t wait_clk, int &id);
	void warm_reset(bool por);
	inline void update_reset_signal();
	inline void update_busreq();
	inline void update_intr();
	inline void set_multibus(bool onoff);
	inline void assert_halt_to_6809();
	inline void degate_halt_to_6809();

public:
	MPC_68008(VM* parent_vm, EMU* parent_emu, const char* identifier);
	~MPC_68008();

	// common functions
	void initialize();
	void reset();

	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	void write_data8w(uint32_t addr, uint32_t data, int *wait);
	uint32_t read_data8w(uint32_t addr, int *wait);
//	uint32_t fetch_op(uint32_t addr, int *wait);

	void    write_memory_mapped_io8(uint32_t addr, uint32_t data);
	uint32_t read_memory_mapped_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
//	void event_frame();

	// unique function
	void set_context_cpu(MC68000BASE* device) {
		d_mc68k = device;
	}
	void set_context_board(DEVICE* device) {
		d_board = device;
	}
	void set_context_memory(MEMORY* device) {
		d_memory = device;
	}
	void set_memory(uint8_t *mem, int size);

	void update_intr_condition();

//	uint32_t address_mapping(uint32_t addr);

	void event_callback(int event_id, int err);

	void save_state(FILEIO *fio);
	bool load_state(FILEIO *fio);

#ifdef USE_DEBUGGER
	uint32_t debug_latch_address(uint32_t addr);

	void debug_write_data8(int type, uint32_t addr, uint32_t data);
	uint32_t debug_read_data8(int type, uint32_t addr);
	void debug_write_data8w(int type, uint32_t addr, uint32_t data, int *wait);
	uint32_t debug_read_data8w(int type, uint32_t addr, int *wait);

	void debug_write_data16(int type, uint32_t addr, uint32_t data);
	uint32_t debug_read_data16(int type, uint32_t addr);

	uint32_t debug_physical_addr_mask(int type);
	bool debug_physical_addr_type_name(int type, _TCHAR *buffer, size_t buffer_len);

	uint32_t debug_read_bank(uint32_t addr);
	void debug_memory_map_info(DebuggerConsole *dc);
	void print_memory_bank(uint32_t data, bool w, _TCHAR *buffer, size_t buffer_len);

	uint32_t debug_address_mapping_rev(uint32_t addr);

	void debug_write_memory_mapped_io8(uint32_t addr, uint32_t data);
	uint32_t debug_read_memory_mapped_io8(uint32_t addr);
#endif
};

#endif /* USE_MPC_68008 */

#endif /* _MPC_68008_H_ */

