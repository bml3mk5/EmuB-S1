/** @file z80b_card.h

	HITACHI MB-S1 Emulator 'EmuB-S1'
	Skelton for retropc emulator

	@author Sasaji
	@date   2019.10.19 -

	@brief [ z80b extention card ]
*/

#ifndef _Z80B_CARD_H_
#define _Z80B_CARD_H_

#include "../vm_defs.h"

#if defined(USE_Z80B_CARD)

#include "../device.h"

#define SIG_Z80BCARD_BUSACK	151

class EMU;

/**
	@brief z80b extention card
*/
class Z80B_CARD : public DEVICE
{
private:
	DEVICE *d_z80;
	DEVICE *d_board;

	uint8_t *memory;
	int memory_size;

	uint8_t membank;
	uint8_t *memory_a;
	uint8_t *memory_b;

	int  out_irq_id;
	bool busreq_on;
	bool z80_running;

	//for resume
#pragma pack(1)
	struct vm_state_st {
		uint8_t  membank;
		uint8_t  mbc;
		uint8_t  condition;
		uint8_t  out_irq;
		char     reserved[12];
	};
#pragma pack()

	void set_memory_from_membank();
	void set_busreq(bool onoff);
	void set_interrupt(bool onoff);
	void set_multibus(bool onoff);

public:
	Z80B_CARD(VM* parent_vm, EMU* parent_emu, const char* identifier);
	~Z80B_CARD();

	// common functions
	void initialize();
	void reset();

	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	void write_data8w(uint32_t addr, uint32_t data, int *wait);
	uint32_t read_data8w(uint32_t addr, int *wait);
	uint32_t fetch_op(uint32_t addr, int *wait);

	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void    write_memory_mapped_io8(uint32_t addr, uint32_t data);
	uint32_t read_memory_mapped_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
//	void event_frame();

	// unique function
	void set_context_cpu(DEVICE* device) {
		d_z80 = device;
	}
	void set_context_board(DEVICE* device) {
		d_board = device;
	}
	void set_memory(uint8_t *mem, int size);


	uint32_t address_mapping(uint32_t addr);

//	void event_callback(int event_id, int err);

	void save_state(FILEIO *fio);
	bool load_state(FILEIO *fio);

#ifdef USE_DEBUGGER
	uint32_t debug_latch_address(uint32_t addr);

	void debug_write_data8(int type, uint32_t addr, uint32_t data);
	uint32_t debug_read_data8(int type, uint32_t addr);
	void debug_write_data8w(int type, uint32_t addr, uint32_t data, int *wait);
	uint32_t debug_read_data8w(int type, uint32_t addr, int *wait);

	uint32_t debug_physical_addr_mask(int type);
	bool debug_physical_addr_type_name(int type, _TCHAR *buffer, size_t buffer_len);

	uint32_t debug_read_bank(uint32_t addr);
	void debug_memory_map_info(DebuggerConsole *dc);
	void print_memory_bank(uint32_t data, bool w, _TCHAR *buffer, size_t buffer_len);

	uint32_t debug_address_mapping_rev(uint32_t addr);

	void debug_write_io8(uint32_t addr, uint32_t data);
	uint32_t debug_read_io8(uint32_t addr);
	void debug_write_memory_mapped_io8(uint32_t addr, uint32_t data);
	uint32_t debug_read_memory_mapped_io8(uint32_t addr);
#endif
};

#endif /* USE_Z80B_CARD */

#endif /* _Z80B_CARD_H_ */

