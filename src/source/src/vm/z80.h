/** @file z80.h

	Skelton for retropc emulator

	@par Origin
	MAME 0.145
	@author Takeda.Toshiya
	@date   2012.02.15-

	@note Modify for MB-S1 By Sasaji on 2019.10.21 -

	@brief [ Z80 ]
*/

#ifndef _Z80_H_ 
#define _Z80_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"
#include "z80dasm.h"
#ifdef USE_DEBUGGER
#include "debugger.h"
#endif

#ifdef HAS_NSC800
#define SIG_NSC800_INT	0
#define SIG_NSC800_RSTA	1
#define SIG_NSC800_RSTB	2
#define SIG_NSC800_RSTC	3
#endif

/**
	@brief Z80 - CPU
*/
class Z80 : public DEVICE
{
private:
	/* ---------------------------------------------------------------------------
	contexts
	--------------------------------------------------------------------------- */

	DEVICE *d_mem, *d_io;
#ifdef Z80_NOTIFY_INTR
	DEVICE *d_pic;
#endif
#ifdef Z80_PSEUDO_BIOS
	DEVICE *d_bios;
#endif
#ifdef SINGLE_MODE_DMA
	DEVICE *d_dma;
#endif
#ifdef USE_DEBUGGER
	Z80DASM dasm;
	DEBUGGER *d_debugger;
	DEVICE *d_mem_stored, *d_io_stored;
#endif
	outputs_t outputs_busack;

	/* ---------------------------------------------------------------------------
	registers
	--------------------------------------------------------------------------- */

#ifdef USE_DEBUGGER
	uint64_t total_icount;
	uint16_t npc;
#endif
#ifdef USE_Z80_DEBUGGER
	uint64_t prev_total_icount;
#endif
	uint32_t cpu_clock;
	int icount;
	int extra_icount;
	uint16_t prevpc;
	pair32_t pc, sp, af, bc, de, hl, ix, iy, wz;
	pair32_t af2, bc2, de2, hl2;
	uint8_t I, R, R2;
	uint32_t ea;

	bool busreq, busack;
	bool after_halt;
	uint8_t im, iff1, iff2, icr;
	bool after_ei, after_ldair;
	uint32_t intr_req_bit, intr_pend_bit;

	uint32_t int_state;

	//for resume
#pragma pack(1)
	struct vm_state_st {
		uint64_t	total_icount;
		int			icount;
		int			extra_icount;

		uint32_t	prevpc;
		pair32_t	pc;
		pair32_t	sp;
		pair32_t	af;

		pair32_t	bc;
		pair32_t	de;
		pair32_t	hl;
		pair32_t	ix;

		pair32_t	iy;
		pair32_t	wz;
		pair32_t	af2;
		pair32_t	bc2;

		pair32_t	de2;
		pair32_t	hl2;
		uint8_t		I;
		uint8_t		R;
		uint8_t		R2;
		uint8_t		now_reset;
		uint32_t	ea;

		uint8_t		busreq;
		uint8_t		after_halt;
		uint8_t		im;
		uint8_t		iff1;
		uint8_t		iff2;
		uint8_t		icr;
		uint8_t		after_ei;
		uint8_t		after_ldair;
		uint32_t	intr_req_bit;
		uint32_t	intr_pend_bit;
	};
#pragma pack()

	inline uint8_t RM8(uint32_t addr);
	inline uint8_t RM8D(uint32_t addr);
	inline void WM8(uint32_t addr, uint8_t val);
	inline void WM8D(uint32_t addr, uint8_t val);
	inline void RM16(uint32_t addr, pair32_t *r);
	inline void WM16(uint32_t addr, pair32_t *r);
	inline uint8_t FETCHOP1();
	inline uint8_t FETCHOP2();
	inline uint8_t FETCH8();
	inline uint32_t FETCH16();
	inline uint8_t IN8(uint32_t addr);
	inline void OUT8(uint32_t addr, uint8_t val);

	inline uint8_t INC(uint8_t value);
	inline uint8_t DEC(uint8_t value);

	inline uint8_t RLC(uint8_t value);
	inline uint8_t RRC(uint8_t value);
	inline uint8_t RL(uint8_t value);
	inline uint8_t RR(uint8_t value);
	inline uint8_t SLA(uint8_t value);
	inline uint8_t SRA(uint8_t value);
	inline uint8_t SLL(uint8_t value);
	inline uint8_t SRL(uint8_t value);

	inline void RETI();

	inline uint8_t RES(uint8_t bit, uint8_t value);
	inline uint8_t SET(uint8_t bit, uint8_t value);

	void OP_CB(uint8_t code);
	void OP_XY(uint8_t code);
	void OP_DD(uint8_t code);
	void OP_FD(uint8_t code);
	void OP_ED(uint8_t code);
	void OP(uint8_t code);
	bool run_one_opecode();
	void check_interrupt();

	/* ---------------------------------------------------------------------------
	debug
	--------------------------------------------------------------------------- */

public:
	Z80(VM* parent_vm, EMU* parent_emu, const char* identifier);
	~Z80() {}

	// common functions
	void initialize();
	void reset();
	int run(int clock);
	void write_signal(int id, uint32_t data, uint32_t mask);
	uint32_t read_signal(int id);
	void set_intr_line(bool line, bool pending, uint32_t bit)
	{
		uint32_t mask = 1 << bit;
		intr_req_bit = line ? (intr_req_bit | mask) : (intr_req_bit & ~mask);
		intr_pend_bit = pending ? (intr_pend_bit | mask) : (intr_pend_bit & ~mask);
	}
	void set_extra_clock(int clock)
	{
		extra_icount += clock;
	}
	int get_extra_clock()
	{
		return extra_icount;
	}
	uint32_t get_pc()
	{
		return prevpc;
	}
	uint32_t get_next_pc()
	{
		return pc.w.l;
	}
	void set_cpu_clock(uint32_t clk)
	{
		cpu_clock = clk;
	}
	uint32_t get_cpu_clock() const
	{
		return cpu_clock;
	}
#ifdef USE_DEBUGGER
	bool is_cpu()
	{
		return true;
	}
	bool is_debugger_available()
	{
		return true;
	}
	DEVICE *get_debugger()
	{
		return d_debugger;
	}
	uint32_t debug_prog_addr_mask()
	{
		return 0xffff;
	}
	uint32_t debug_data_addr_mask()
	{
		return 0xffff;
	}
	uint32_t debug_io_addr_mask()
	{
		return 0xff;
	}
	uint32_t debug_data_mask()
	{
		return 0xff;
	}
	void debug_write_data8(int type, uint32_t addr, uint32_t data);
	uint32_t debug_read_data8(int type, uint32_t addr);
	void debug_write_io8(uint32_t addr, uint32_t data);
	uint32_t debug_read_io8(uint32_t addr);
	bool debug_write_reg(const _TCHAR *reg, uint32_t data);
//	bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len);
	void debug_regs_info(_TCHAR *buffer, size_t buffer_len);
	bool get_debug_reg_ptr(_TCHAR *reg, size_t regsiz, void * &regptr, int &reglen);
	int debug_dasm(int type, uint32_t pc, _TCHAR *buffer, size_t buffer_len, int flags);
	int debug_dasm_label(int type, uint32_t pc, _TCHAR *buffer, size_t buffer_len);
	int debug_trace_back_regs(int index, _TCHAR *buffer, size_t buffer_len);
	int debug_trace_back(int index, _TCHAR *buffer, size_t buffer_len);
	bool reach_break_point();
//	void go_suspend();
//	bool now_suspend();
	uint32_t get_debug_pc(int type);
	uint32_t get_debug_next_pc(int type);
	uint32_t get_debug_branch_pc(int type);
	uint64_t get_current_clock();

	bool get_debug_signal_name_index(const _TCHAR *param, uint32_t *num, uint32_t *mask, int *idx, const _TCHAR **name);
	bool get_debug_signal_name_index(uint32_t num, uint32_t *mask, int *idx, const _TCHAR **name);
	void get_debug_signal_names_str(_TCHAR *buffer, size_t buffer_len);
#endif
	bool load_state(FILEIO *fio);
	void save_state(FILEIO *fio);

	// unique functions
	void set_context_mem(DEVICE* device)
	{
		d_mem = device;
#ifdef USE_DEBUGGER
		dasm.set_context_mem(device);
#endif /* USE_DEBUGGER */
	}
	void set_context_io(DEVICE* device)
	{
		d_io = device;
	}
#ifdef Z80_NOTIFY_INTR
	void set_context_intr(DEVICE* device)
	{
		d_pic = device;
	}
	DEVICE *get_context_child()
	{
		return d_pic;
	}
#endif
#ifdef Z80_PSEUDO_BIOS
	void set_context_bios(DEVICE* device)
	{
		d_bios = device;
	}
#endif
#ifdef SINGLE_MODE_DMA
	void set_context_dma(DEVICE* device)
	{
		d_dma = device;
	}
#endif
#ifdef USE_DEBUGGER
	void set_context_debugger(DEBUGGER* device)
	{
		d_debugger = device;
	}
#endif
	void set_context_busack(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_busack, device, id, mask);
	}
	void set_pc(uint16_t value)
	{
		pc.w.l = value;
	}
	void set_sp(uint16_t value)
	{
		sp.w.l = value;
	}
};

#endif /* _Z80_H_ */

