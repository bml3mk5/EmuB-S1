/** @file z80dasm.h

	Skelton for retropc emulator

	@par Origin
	MAME 0.145
	@author Takeda.Toshiya
	@date   2012.02.15-

	@note Modify for MB-S1 By Sasaji on 2019.10.21 -

	@brief [ Z80 disassembler ]
*/

#ifndef _Z80_DASM_H_ 
#define _Z80_DASM_H_

#ifndef USE_DEBUGGER
#define DASM_SET_CODE8(a1)
#define DASM_SET_CODE16(a1)
#define DASM_SET_MEM1(a1,a2)
#define DASM_SET_MEM2(a1,a2,a3)
#define DASM_SET_SIGNAL(x)

#else /* USE_DEBUGGER*/
#define DASM_SET_CODE8(a1)			dasm.set_code8(a1)
#define DASM_SET_CODE16(a1)			dasm.set_code16(a1)
#define DASM_SET_MEM1(a1,a2)		dasm.set_mem(a1, a2)
#define DASM_SET_MEM2(a1,a2,a3)		dasm.set_mem(a1, a2, a3)
#define DASM_SET_SIGNAL(x)			dasm.set_signal(x)

#include "vm.h"
#include "../emu.h"
#include "device.h"

class DEBUGGER;

/// store operating status on Z80
typedef struct z80dasm_regs_st {
	uint32_t phyaddr;
	uint16_t pc;

	uint16_t af;
	uint16_t bc;
	uint16_t de;
	uint16_t hl;

	uint16_t af2;
	uint16_t bc2;
	uint16_t de2;
	uint16_t hl2;

	uint16_t ix;
	uint16_t iy;
	uint16_t sp;

	uint8_t  i;
	uint8_t  r;

	uint8_t  code[4];
	uint8_t  flags;	// bit0:valid data  bit1:data store=1  bit2:write data=1  bit3:2bytes data=1 

	uint32_t state;

	uint32_t rw_phyaddr;
	uint16_t rw_addr;

	uint16_t rw_data;
	uint16_t cycles;
} z80dasm_regs_t;

#define Z80DASM_PCSTACK_NUM 1000
#define Z80DASM_CMDLINE_LEN	128

/**
	@brief Z80 disassembler
*/
class Z80DASM
{
public:
	enum en_operate {
		NONE = 0,
		FETCH8,		// operand 1 byte
		FETCHID8,	// operand 1 byte indirect
		FETCHREL8,	// operand 1 byte relative
		FETCHREL88, // operand 2 bytes
		FETCH16,	// operand 2 bytes
		FETCHID16,	// operand 2 bytes indirect
		JMPREL8,	// jump relative 1 byte
		CODE_CB,	// code CBh
		CODE_DD,	// code DDh
		CODE_ED,	// code EDh
		CODE_FD,	// code FDh
	};

	typedef struct opcode_st {
		uint8_t       c;
		const _TCHAR *s;
		en_operate    m;
	} opcode_t;

private:
	DEVICE *d_mem;
	DEBUGGER *debugger;

	_TCHAR line[_MAX_PATH];

	z80dasm_regs_t regs[Z80DASM_PCSTACK_NUM];
	z80dasm_regs_t *current_reg;
	int current_idx;
	int codelen;

	int codepos;

	_TCHAR cmd[Z80DASM_CMDLINE_LEN];

	static const opcode_t *find_opcode(const opcode_t *list, size_t start, size_t end, uint8_t code);

//	inline uint8_t  dasm_fetchop();
//	inline uint8_t  debug_fetch8();
//	inline int8_t   debug_fetch8_rel();
	inline uint16_t debug_fetch16(const uint8_t *ops);
	inline uint16_t debug_fetch8_relpc(uint32_t pc, uint8_t ops);

	const _TCHAR *format_digit(uint32_t val, int len);
	const _TCHAR *format_offset(int8_t val);
	int select_dasm_type(uint32_t pc, const uint8_t *ops, int opslen, en_operate type, const _TCHAR *mnemonic, _TCHAR *buffer, size_t buffer_len);

	int dasm_cb(uint32_t pc, const uint8_t *ops, int opslen, _TCHAR *buffer, size_t buffer_len);
	int dasm_dd(uint32_t pc, const uint8_t *ops, int opslen, _TCHAR *buffer, size_t buffer_len);
	int dasm_ed(uint32_t pc, const uint8_t *ops, int opslen, _TCHAR *buffer, size_t buffer_len);
	int dasm_fd(uint32_t pc, const uint8_t *ops, int opslen, _TCHAR *buffer, size_t buffer_len);
	int dasm_ddcb(uint32_t pc, const uint8_t *ops, int opslen, _TCHAR *buffer, size_t buffer_len);
	int dasm_fdcb(uint32_t pc, const uint8_t *ops, int opslen, _TCHAR *buffer, size_t buffer_len);

	void push_stack_pc(uint32_t phyaddr, uint16_t pc, uint8_t code);

public:
	Z80DASM();
	~Z80DASM();

	void ini_pc(uint16_t pc, uint8_t code);
	void set_code8(uint8_t code);
	void set_code16(uint16_t code);
	void set_mem(uint16_t addr, uint8_t data, bool write = false);
	void set_mem(uint16_t addr, uint16_t data, bool write = false);
	void set_phymem(uint32_t addr, uint8_t data, bool write = false);
	void set_signal(uint32_t state);

	void set_regs(int accum, int cycles, uint16_t af, uint16_t bc, uint16_t de, uint16_t hl, uint16_t af2, uint16_t bc2, uint16_t de2, uint16_t hl2, uint16_t ix, uint16_t iy, uint16_t sp, uint8_t i, uint8_t r);

	int print_dasm(uint32_t phy_addr, uint16_t pc, const uint8_t *ops, int opslen, int flags);
	int print_dasm_label(int type, uint32_t pc);
	int print_dasm_preprocess(int type, uint32_t pc, int flags);
	int print_dasm_processed(uint16_t pc);
	int print_dasm_traceback(int index);

	void print_cycles(int cycles);
	void print_regs(const z80dasm_regs_t &regs);
	void print_regs_current();
	void print_regs_current(uint16_t pc, uint16_t af, uint16_t bc, uint16_t de, uint16_t hl, uint16_t af2, uint16_t bc2, uint16_t de2, uint16_t hl2, uint16_t ix, uint16_t iy, uint16_t sp, uint8_t i, uint8_t r, uint32_t state);
	int print_regs_traceback(int index);

	void print_memory_data(z80dasm_regs_t &stack);

	int get_stack_pc(int index, z80dasm_regs_t *stack);

	int dasm_first(uint32_t phy_addr, uint32_t pc, const uint8_t *ops, int opslen);

	void set_context_mem(DEVICE* device) {
		d_mem = device;
	}
	void set_debugger(DEBUGGER* device) {
		debugger = device;
	}
	const _TCHAR *get_line() const {
		return line;
	}
	size_t get_line_length() const {
		return _tcslen(line);
	}
};

#endif /* USE_DEBUGGER */

#endif /* _Z80_DASM_H_ */

