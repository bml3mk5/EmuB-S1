/** @file mouse.h

	HITACHI MB-S1 Emulator 'EmuB-S1'
	Skelton for retropc emulator

	@author Sasaji
	@date   2015.09.11

	@brief [ mouse ]
*/

#ifndef MOUSE_H
#define MOUSE_H

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"
#include "../../config.h"
#include "keyrecord.h"

/**
	@brief Mouse emulate
*/
class MOUSE : public DEVICE
{
private:
	DEVICE *d_board;
#ifdef USE_KEY_RECORD
	KEYRECORD *reckey;
#endif

	int *mouse_stat;

	t_mouse_stat mst[2];

	uint8_t reg_mouse[2];
	int cntdir[2];	///< counter direction 1 or -1

		//for resume
#pragma pack(1)
	struct vm_state_st {
		struct {
			int pos;
			uint8_t dir;
			uint8_t btn;
			uint8_t prev_btn;
			char reserved1;
		} st[2];
		uint8_t reg0;
		char  reserved[15];
	};
#pragma pack()

	void set_reg0(uint8_t data);
	void set_reg1(uint8_t data);
	void update_mouse();

public:
	MOUSE(VM* parent_vm, EMU* parent_emu, const char* identifier) : DEVICE(parent_vm, parent_emu, identifier)
	{
		set_class_name("MOUSE");
	}
	~MOUSE() {}

	void initialize();
	void reset();

	void write_signal(int id, uint32_t data, uint32_t mask);
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);

	void event_frame();

	void set_context_board(DEVICE *device) {
		d_board = device;
	}
#ifdef USE_KEY_RECORD
	void set_keyrecord(KEYRECORD *value) {
		reckey = value;
	}
#endif
	void set_mouse_position(int px, int py);

	void save_state(FILEIO *fio);
	bool load_state(FILEIO *fio);

#ifdef USE_DEBUGGER
	uint32_t debug_read_io8(uint32_t addr);
#endif
};

#endif /* MOUSE_H */
