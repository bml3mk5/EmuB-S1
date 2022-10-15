/** @file z80.cpp

	Skelton for retropc emulator

	@par Origin
	MAME 0.145
	@author Takeda.Toshiya
	@date   2012.02.15-

	@note Modify for MB-S1 By Sasaji on 2019.10.21 -

	@brief [ Z80 ]
*/

#include "z80.h"
#include "../utility.h"
#include "../fileio.h"
#include "z80_consts.h"
#include "../depend.h"
#ifdef USE_DEBUGGER
#include "debugger.h"
#endif

#ifndef CPU_START_ADDR
#define CPU_START_ADDR	0
#endif

#define NMI_REQ_BIT	0x80000000

#define PCD	pc.d
#define PC	pc.w.l

#define SPD 	sp.d
#define SP	sp.w.l

#define AFD 	af.d
#define AF	af.w.l
#define A	af.b.h
#define F	af.b.l

#define BCD 	bc.d
#define BC	bc.w.l
#define B	bc.b.h
#define C	bc.b.l

#define DED 	de.d
#define DE	de.w.l
#define D	de.b.h
#define E	de.b.l

#define HLD 	hl.d
#define HL	hl.w.l
#define H	hl.b.h
#define L	hl.b.l

#define IXD 	ix.d
#define IX	ix.w.l
#define HX	ix.b.h
#define LX	ix.b.l

#define IYD 	iy.d
#define IY	iy.w.l
#define HY	iy.b.h
#define LY	iy.b.l

#define AF2	af2.w.l
#define A2	af2.b.h
#define F2	af2.b.l

#define BC2	bc2.w.l
#define B2	bc2.b.h
#define C2	bc2.b.l

#define DE2	de2.w.l
#define D2	de2.b.h
#define E2	de2.b.l

#define HL2	hl2.w.l
#define H2	hl2.b.h
#define L2	hl2.b.l

#define WZD	wz.d
#define WZ	wz.w.l
#define WZ_H	wz.b.h
#define WZ_L	wz.b.l

static uint8_t SZ[256];		/* zero and sign flags */
static uint8_t SZ_BIT[256];	/* zero, sign and parity/overflow (=zero) flags for BIT opcode */
static uint8_t SZP[256];		/* zero, sign and parity flags */
static uint8_t SZHV_inc[256];	/* zero, sign, half carry and overflow flags INC r8 */
static uint8_t SZHV_dec[256];	/* zero, sign, half carry and overflow flags DEC r8 */

static uint8_t SZHVC_add[2 * 256 * 256];
static uint8_t SZHVC_sub[2 * 256 * 256];

static bool flags_initialized = false;

static const uint8_t cc_op[0x100] = {
	 4,10, 7, 6, 4, 4, 7, 4, 4,11, 7, 6, 4, 4, 7, 4,
	 8,10, 7, 6, 4, 4, 7, 4,12,11, 7, 6, 4, 4, 7, 4,
	 7,10,16, 6, 4, 4, 7, 4, 7,11,16, 6, 4, 4, 7, 4,
	 7,10,13, 6,11,11,10, 4, 7,11,13, 6, 4, 4, 7, 4,
	 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
	 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
	 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
	 7, 7, 7, 7, 7, 7, 4, 7, 4, 4, 4, 4, 4, 4, 7, 4,
	 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
	 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
	 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
	 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
	 5,10,10,10,10,11, 7,11, 5,10,10, 0,10,17, 7,11,
	 5,10,10,11,10,11, 7,11, 5, 4,10,11,10, 0, 7,11,
	 5,10,10,19,10,11, 7,11, 5, 4,10, 4,10, 0, 7,11,
	 5,10,10, 4,10,11, 7,11, 5, 6,10, 4,10, 0, 7,11
};

static const uint8_t cc_cb[0x100] = {
	 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
	 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
	 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
	 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
	 8, 8, 8, 8, 8, 8,12, 8, 8, 8, 8, 8, 8, 8,12, 8,
	 8, 8, 8, 8, 8, 8,12, 8, 8, 8, 8, 8, 8, 8,12, 8,
	 8, 8, 8, 8, 8, 8,12, 8, 8, 8, 8, 8, 8, 8,12, 8,
	 8, 8, 8, 8, 8, 8,12, 8, 8, 8, 8, 8, 8, 8,12, 8,
	 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
	 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
	 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
	 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
	 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
	 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
	 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
	 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8
};

static const uint8_t cc_ed[0x100] = {
	 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	12,12,15,20, 8,14, 8, 9,12,12,15,20, 8,14, 8, 9,
	12,12,15,20, 8,14, 8, 9,12,12,15,20, 8,14, 8, 9,
	12,12,15,20, 8,14, 8,18,12,12,15,20, 8,14, 8,18,
	12,12,15,20, 8,14, 8, 8,12,12,15,20, 8,14, 8, 8,
	 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	16,16,16,16, 8, 8, 8, 8,16,16,16,16, 8, 8, 8, 8,
	16,16,16,16, 8, 8, 8, 8,16,16,16,16, 8, 8, 8, 8,
	 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8
};

static const uint8_t cc_xy[0x100] = {
	 4, 4, 4, 4, 4, 4, 4, 4, 4,15, 4, 4, 4, 4, 4, 4,
	 4, 4, 4, 4, 4, 4, 4, 4, 4,15, 4, 4, 4, 4, 4, 4,
	 4,14,20,10, 9, 9,11, 4, 4,15,20,10, 9, 9,11, 4,
	 4, 4, 4, 4,23,23,19, 4, 4,15, 4, 4, 4, 4, 4, 4,
	 4, 4, 4, 4, 9, 9,19, 4, 4, 4, 4, 4, 9, 9,19, 4,
	 4, 4, 4, 4, 9, 9,19, 4, 4, 4, 4, 4, 9, 9,19, 4,
	 9, 9, 9, 9, 9, 9,19, 9, 9, 9, 9, 9, 9, 9,19, 9,
	19,19,19,19,19,19, 4,19, 4, 4, 4, 4, 9, 9,19, 4,
	 4, 4, 4, 4, 9, 9,19, 4, 4, 4, 4, 4, 9, 9,19, 4,
	 4, 4, 4, 4, 9, 9,19, 4, 4, 4, 4, 4, 9, 9,19, 4,
	 4, 4, 4, 4, 9, 9,19, 4, 4, 4, 4, 4, 9, 9,19, 4,
	 4, 4, 4, 4, 9, 9,19, 4, 4, 4, 4, 4, 9, 9,19, 4,
	 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 4, 4, 4, 4,
	 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	 4,14, 4,23, 4,15, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4,
	 4, 4, 4, 4, 4, 4, 4, 4, 4,10, 4, 4, 4, 4, 4, 4
};

static const uint8_t cc_xycb[0x100] = {
	23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
	23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
	23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
	23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
	20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,
	20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,
	20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,
	20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,
	23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
	23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
	23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
	23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
	23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
	23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
	23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
	23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23
};

static const uint8_t cc_ex[0x100] = {
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* DJNZ */
	 5, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0,	/* JR NZ/JR Z */
	 5, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0,	/* JR NC/JR C */
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 5, 5, 5, 5, 0, 0, 0, 0, 5, 5, 5, 5, 0, 0, 0, 0,	/* LDIR/CPIR/INIR/OTIR LDDR/CPDR/INDR/OTDR */
	 6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2,
	 6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2,
	 6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2,
	 6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2
};

#ifdef USE_DEBUGGER
#define SET_NPC npc = PC
#else
#define SET_NPC
#endif

// opecode definitions

#define ENTER_HALT() do { \
	PC--; \
	after_halt = true; \
	int_state |= Z80_STATE_HALT; \
	DASM_SET_SIGNAL(int_state); \
} while(0)

#define LEAVE_HALT() do { \
	if(after_halt) { \
		after_halt = false; \
		int_state &= ~Z80_STATE_HALT; \
		DASM_SET_SIGNAL(int_state); \
		PC++; \
	} \
} while(0)

inline uint8_t Z80::RM8(uint32_t addr)
{
	uint8_t val;
#ifdef Z80_MEMORY_WAIT
	int wait;
	val = d_mem->read_data8w(addr, &wait);
	icount -= wait;
#else
	val = d_mem->read_data8(addr);
#endif
	DASM_SET_MEM1(addr, val);
	return val;
}

inline uint8_t Z80::RM8D(uint32_t addr)
{
	uint8_t val;
#ifdef Z80_MEMORY_WAIT
	int wait;
	val = d_mem->read_data8w(addr, &wait);
	icount -= wait;
#else
	val = d_mem->read_data8(addr);
#endif
	return val;
}

inline void Z80::WM8(uint32_t addr, uint8_t val)
{
#ifdef Z80_MEMORY_WAIT
	int wait;
	d_mem->write_data8w(addr, val, &wait);
	icount -= wait;
#else
	d_mem->write_data8(addr, val);
#endif
	DASM_SET_MEM2(addr, val, true);
}

inline void Z80::WM8D(uint32_t addr, uint8_t val)
{
#ifdef Z80_MEMORY_WAIT
	int wait;
	d_mem->write_data8w(addr, val, &wait);
	icount -= wait;
#else
	d_mem->write_data8(addr, val);
#endif
}

inline void Z80::RM16(uint32_t addr, pair32_t *r)
{
	r->b.l = RM8D(addr);
	r->b.h = RM8D((addr + 1) & 0xffff);
	DASM_SET_MEM1(addr, r->w.l); 
}

inline void Z80::WM16(uint32_t addr, pair32_t *r)
{
	WM8D(addr, r->b.l);
	WM8D((addr + 1) & 0xffff, r->b.h);
	DASM_SET_MEM2(addr, r->w.l, true); 
}

inline uint8_t Z80::FETCHOP1()
{
	unsigned pctmp = PCD;
	PC++;
	R++;

	// consider m1 cycle wait
	int wait;
	uint8_t val = d_mem->fetch_op(pctmp, &wait);
#ifdef USE_DEBUGGER
	dasm.ini_pc(pctmp, val);
#endif
	icount -= wait;
	return val;
}

inline uint8_t Z80::FETCHOP2()
{
	unsigned pctmp = PCD;
	PC++;
	R++;

	// consider m1 cycle wait
	int wait;
	uint8_t val = d_mem->fetch_op(pctmp, &wait);
	DASM_SET_CODE8(val);
	icount -= wait;
	return val;
}

inline uint8_t Z80::FETCH8()
{
	unsigned pctmp = PCD;
	PC++;
	uint8_t val = RM8D(pctmp);
	DASM_SET_CODE8(val);
	return val;
}

inline uint32_t Z80::FETCH16()
{
	unsigned pctmp = PCD;
	PC += 2;
	uint32_t val = RM8D(pctmp) | ((uint32_t)RM8D((pctmp + 1) & 0xffff) << 8);
	DASM_SET_CODE16(val);
	return val;
}

inline uint8_t Z80::IN8(uint32_t addr)
{
#ifdef Z80_IO_WAIT
	int wait;
	uint8_t val = d_io->read_io8w(addr, &wait);
	icount -= wait;
	return val;
#else
	return d_io->read_io8(addr);
#endif
}

inline void Z80::OUT8(uint32_t addr, uint8_t val)
{
#ifdef HAS_NSC800
	if((addr & 0xff) == 0xbb) {
		icr = val;
		return;
	}
#endif
#ifdef Z80_IO_WAIT
	int wait;
	d_io->write_io8w(addr, val, &wait);
	icount -= wait;
#else
	d_io->write_io8(addr, val);
#endif
}

#define EAX() do { \
	ea = (uint32_t)(uint16_t)(IX + (int8_t)FETCH8()); \
	WZ = ea; \
} while(0)

#define EAY() do { \
	ea = (uint32_t)(uint16_t)(IY + (int8_t)FETCH8()); \
	WZ = ea; \
} while(0)

#define POP(DR) do { \
	RM16(SPD, &DR); \
	SP += 2; \
} while(0)

#define PUSH(SR) do { \
	SP -= 2; \
	WM16(SPD, &SR); \
} while(0)

#define JP() do { \
	PCD = FETCH16(); \
	WZ = PCD; \
} while(0)

#define JP_COND(cond) do { \
	if(cond) { \
		PCD = FETCH16(); \
		WZ = PCD; \
	} else { \
		WZ = FETCH16(); /* implicit do PC += 2 */ \
	} \
} while(0)

#define JR() do { \
	int8_t arg = (int8_t)FETCH8(); /* FETCH8() also increments PC */ \
	PC += arg; /* so don't do PC += FETCH8() */ \
	WZ = PC; \
} while(0)

#define JR_COND(cond, opcode) do { \
	if(cond) { \
		JR(); \
		icount -= cc_ex[opcode]; \
	} else FETCH8(); \
} while(0)

#define CALL() do { \
	ea = FETCH16(); \
	WZ = ea; \
	PUSH(pc); \
	SET_NPC; \
	PCD = ea; \
} while(0)

#define CALL_COND(cond, opcode) do { \
	if(cond) { \
		ea = FETCH16(); \
		WZ = ea; \
		PUSH(pc); \
		SET_NPC; \
		PCD = ea; \
		icount -= cc_ex[opcode]; \
	} else { \
		WZ = FETCH16(); /* implicit call PC+=2; */ \
	} \
} while(0)

#define RET_COND(cond, opcode) do { \
	if(cond) { \
		SET_NPC; \
		POP(pc); \
		WZ = PC; \
		icount -= cc_ex[opcode]; \
	} \
} while(0)

#define RETN() do { \
	SET_NPC; \
	POP(pc); \
	WZ = PC; \
	iff1 = iff2; \
} while(0)

void Z80::RETI()
{
	SET_NPC;
	POP(pc);
	WZ = PC;
	iff1 = iff2;
#ifdef Z80_NOTIFY_INTR
	if (d_pic) d_pic->notify_intr_reti();
#endif
}

#define LD_R_A() do { \
	R = A; \
	R2 = A & 0x80; /* keep bit 7 of r */ \
} while(0)

#define LD_A_R() do { \
	A = (R & 0x7f) | R2; \
	F = (F & CF) | SZ[A] | (iff2 << 2); \
	after_ldair = true; \
} while(0)

#define LD_I_A() do { \
	I = A; \
} while(0)

#define LD_A_I() do { \
	A = I; \
	F = (F & CF) | SZ[A] | (iff2 << 2); \
	after_ldair = true; \
} while(0)

#define RST(addr) do { \
	PUSH(pc); \
	SET_NPC; \
	PCD = addr; \
	WZ = PC; \
} while(0)

inline uint8_t Z80::INC(uint8_t value)
{
	uint8_t res = value + 1;
	F = (F & CF) | SZHV_inc[res];
	return (uint8_t)res;
}

inline uint8_t Z80::DEC(uint8_t value)
{
	uint8_t res = value - 1;
	F = (F & CF) | SZHV_dec[res];
	return res;
}

#define RLCA() do { \
	A = (A << 1) | (A >> 7); \
	F = (F & (SF | ZF | PF)) | (A & (YF | XF | CF)); \
} while(0)

#define RRCA() do { \
	F = (F & (SF | ZF | PF)) | (A & CF); \
	A = (A >> 1) | (A << 7); \
	F |= (A & (YF | XF)); \
} while(0)

#define RLA() do { \
	uint8_t res = (A << 1) | (F & CF); \
	uint8_t c = (A & 0x80) ? CF : 0; \
	F = (F & (SF | ZF | PF)) | c | (res & (YF | XF)); \
	A = res; \
} while(0)

#define RRA() do { \
	uint8_t res = (A >> 1) | (F << 7); \
	uint8_t c = (A & 0x01) ? CF : 0; \
	F = (F & (SF | ZF | PF)) | c | (res & (YF | XF)); \
	A = res; \
} while(0)

#define RRD() do { \
	uint8_t n = RM8(HL); \
	WZ = HL + 1; \
	WM8(HL, (n >> 4) | (A << 4)); \
	A = (A & 0xf0) | (n & 0x0f); \
	F = (F & CF) | SZP[A]; \
} while(0)

#define RLD() do { \
	uint8_t n = RM8(HL); \
	WZ = HL + 1; \
	WM8(HL, (n << 4) | (A & 0x0f)); \
	A = (A & 0xf0) | (n >> 4); \
	F = (F & CF) | SZP[A]; \
} while(0)

#define ADD(value) do { \
	uint32_t ah = AFD & 0xff00; \
	uint32_t res = (uint8_t)((ah >> 8) + value); \
	F = SZHVC_add[ah | res]; \
	A = res; \
} while(0)

#define ADC(value) do { \
	uint32_t ah = AFD & 0xff00, c = AFD & 1; \
	uint32_t res = (uint8_t)((ah >> 8) + value + c); \
	F = SZHVC_add[(c << 16) | ah | res]; \
	A = res; \
} while(0)

#define SUB(value) do { \
	uint32_t ah = AFD & 0xff00; \
	uint32_t res = (uint8_t)((ah >> 8) - value); \
	F = SZHVC_sub[ah | res]; \
	A = res; \
} while(0)

#define SBC(value) do { \
	uint32_t ah = AFD & 0xff00, c = AFD & 1; \
	uint32_t res = (uint8_t)((ah >> 8) - value - c); \
	F = SZHVC_sub[(c << 16) | ah | res]; \
	A = res; \
} while(0)

#define NEG() do { \
	uint8_t value = A; \
	A = 0; \
	SUB(value); \
} while(0)

#define DAA() do { \
	uint8_t a = A; \
	if(F & NF) { \
		if((F & HF) | ((A & 0xf) > 9)) a -= 6; \
		if((F & CF) | (A > 0x99)) a -= 0x60; \
	} else { \
		if((F & HF) | ((A & 0xf) > 9)) a += 6; \
		if((F & CF) | (A > 0x99)) a += 0x60; \
	} \
	F = (F & (CF | NF)) | (A > 0x99) | ((A ^ a) & HF) | SZP[a]; \
	A = a; \
} while(0)

#define AND(value) do { \
	A &= value; \
	F = SZP[A] | HF; \
} while(0)

#define OR(value) do { \
	A |= value; \
	F = SZP[A]; \
} while(0)

#define XOR(value) do { \
	A ^= value; \
	F = SZP[A]; \
} while(0)

#define CP(value) do { \
	unsigned val = value; \
	uint32_t ah = AFD & 0xff00; \
	uint32_t res = (uint8_t)((ah >> 8) - val); \
	F = (SZHVC_sub[ah | res] & ~(YF | XF)) | (val & (YF | XF)); \
} while(0)

#define EX_AF() do { \
	pair32_t tmp; \
	tmp = af; af = af2; af2 = tmp; \
} while(0)

#define EX_DE_HL() do { \
	pair32_t tmp; \
	tmp = de; de = hl; hl = tmp; \
} while(0)

#define EXX() do { \
	pair32_t tmp; \
	tmp = bc; bc = bc2; bc2 = tmp; \
	tmp = de; de = de2; de2 = tmp; \
	tmp = hl; hl = hl2; hl2 = tmp; \
} while(0)

#define EXSP(DR) do { \
	pair32_t tmp; \
	tmp.d = 0; \
	RM16(SPD, &tmp); \
	WM16(SPD, &DR); \
	DR = tmp; \
	WZ = DR.d; \
} while(0)

#define ADD16(DR, SR) do { \
	uint32_t res = DR.d + SR.d; \
	WZ = DR.d + 1; \
	F = (F & (SF | ZF | VF)) | (((DR.d ^ res ^ SR.d) >> 8) & HF) | ((res >> 16) & CF) | ((res >> 8) & (YF | XF)); \
	DR.w.l = (uint16_t)res; \
} while(0)

#define ADC16(Reg) do { \
	uint32_t res = HLD + Reg.d + (F & CF); \
	WZ = HL + 1; \
	F = (((HLD ^ res ^ Reg.d) >> 8) & HF) | ((res >> 16) & CF) | ((res >> 8) & (SF | YF | XF)) | ((res & 0xffff) ? 0 : ZF) | (((Reg.d ^ HLD ^ 0x8000) & (Reg.d ^ res) & 0x8000) >> 13); \
	HL = (uint16_t)res; \
} while(0)

#define SBC16(Reg) do { \
	uint32_t res = HLD - Reg.d - (F & CF); \
	WZ = HL + 1; \
	F = (((HLD ^ res ^ Reg.d) >> 8) & HF) | NF | ((res >> 16) & CF) | ((res >> 8) & (SF | YF | XF)) | ((res & 0xffff) ? 0 : ZF) | (((Reg.d ^ HLD) & (HLD ^ res) &0x8000) >> 13); \
	HL = (uint16_t)res; \
} while(0)

inline uint8_t Z80::RLC(uint8_t value)
{
	unsigned res = value;
	unsigned c = (res & 0x80) ? CF : 0;
	res = ((res << 1) | (res >> 7)) & 0xff;
	F = SZP[res] | c;
	return res;
}

inline uint8_t Z80::RRC(uint8_t value)
{
	unsigned res = value;
	unsigned c = (res & 0x01) ? CF : 0;
	res = ((res >> 1) | (res << 7)) & 0xff;
	F = SZP[res] | c;
	return res;
}

inline uint8_t Z80::RL(uint8_t value)
{
	unsigned res = value;
	unsigned c = (res & 0x80) ? CF : 0;
	res = ((res << 1) | (F & CF)) & 0xff;
	F = SZP[res] | c;
	return res;
}

inline uint8_t Z80::RR(uint8_t value)
{
	unsigned res = value;
	unsigned c = (res & 0x01) ? CF : 0;
	res = ((res >> 1) | (F << 7)) & 0xff;
	F = SZP[res] | c;
	return res;
}

inline uint8_t Z80::SLA(uint8_t value)
{
	unsigned res = value;
	unsigned c = (res & 0x80) ? CF : 0;
	res = (res << 1) & 0xff;
	F = SZP[res] | c;
	return res;
}

inline uint8_t Z80::SRA(uint8_t value)
{
	unsigned res = value;
	unsigned c = (res & 0x01) ? CF : 0;
	res = ((res >> 1) | (res & 0x80)) & 0xff;
	F = SZP[res] | c;
	return res;
}

inline uint8_t Z80::SLL(uint8_t value)
{
	unsigned res = value;
	unsigned c = (res & 0x80) ? CF : 0;
	res = ((res << 1) | 0x01) & 0xff;
	F = SZP[res] | c;
	return res;
}

inline uint8_t Z80::SRL(uint8_t value)
{
	unsigned res = value;
	unsigned c = (res & 0x01) ? CF : 0;
	res = (res >> 1) & 0xff;
	F = SZP[res] | c;
	return res;
}

#define BIT(bit, reg) do { \
	F = (F & CF) | HF | (SZ_BIT[reg & (1 << bit)] & ~(YF | XF)) | (reg & (YF | XF)); \
} while(0)

#define BIT_HL(bit, reg) do { \
	F = (F & CF) | HF | (SZ_BIT[reg & (1 << bit)] & ~(YF | XF)) | (WZ_H & (YF | XF)); \
} while(0)

#define BIT_XY(bit, reg) do { \
	F = (F & CF) | HF | (SZ_BIT[reg & (1 << bit)] & ~(YF | XF)) | ((ea >> 8) & (YF | XF)); \
} while(0)

inline uint8_t Z80::RES(uint8_t bit, uint8_t value)
{
	return value & ~(1 << bit);
}

inline uint8_t Z80::SET(uint8_t bit, uint8_t value)
{
	return value | (1 << bit);
}

#define LDI() do { \
	uint8_t io = RM8(HL); \
	WM8(DE, io); \
	F &= SF | ZF | CF; \
	if((A + io) & 0x02) F |= YF; /* bit 1 -> flag 5 */ \
	if((A + io) & 0x08) F |= XF; /* bit 3 -> flag 3 */ \
	HL++; DE++; BC--; \
	if(BC) F |= VF; \
} while(0)

#define CPI() do { \
	uint8_t val = RM8(HL); \
	uint8_t res = A - val; \
	WZ++; \
	HL++; BC--; \
	F = (F & CF) | (SZ[res] & ~(YF | XF)) | ((A ^ val ^ res) & HF) | NF; \
	if(F & HF) res -= 1; \
	if(res & 0x02) F |= YF; /* bit 1 -> flag 5 */ \
	if(res & 0x08) F |= XF; /* bit 3 -> flag 3 */ \
	if(BC) F |= VF; \
} while(0)

#define INI() do { \
	unsigned t; \
	uint8_t io = IN8(BC); \
	WZ = BC + 1; \
	B--; \
	WM8(HL, io); \
	HL++; \
	F = SZ[B]; \
	t = (unsigned)((C + 1) & 0xff) + (unsigned)io; \
	if(io & SF) F |= NF; \
	if(t & 0x100) F |= HF | CF; \
	F |= SZP[(uint8_t)(t & 0x07) ^ B] & PF; \
} while(0)

#define OUTI() do { \
	unsigned t; \
	uint8_t io = RM8(HL); \
	B--; \
	WZ = BC + 1; \
	OUT8(BC, io); \
	HL++; \
	F = SZ[B]; \
	t = (unsigned)L + (unsigned)io; \
	if(io & SF) F |= NF; \
	if(t & 0x100) F |= HF | CF; \
	F |= SZP[(uint8_t)(t & 0x07) ^ B] & PF; \
} while(0)

#define LDD() do { \
	uint8_t io = RM8(HL); \
	WM8(DE, io); \
	F &= SF | ZF | CF; \
	if((A + io) & 0x02) F |= YF; /* bit 1 -> flag 5 */ \
	if((A + io) & 0x08) F |= XF; /* bit 3 -> flag 3 */ \
	HL--; DE--; BC--; \
	if(BC) F |= VF; \
} while(0)

#define CPD() do { \
	uint8_t val = RM8(HL); \
	uint8_t res = A - val; \
	WZ--; \
	HL--; BC--; \
	F = (F & CF) | (SZ[res] & ~(YF | XF)) | ((A ^ val ^ res) & HF) | NF; \
	if(F & HF) res -= 1; \
	if(res & 0x02) F |= YF; /* bit 1 -> flag 5 */ \
	if(res & 0x08) F |= XF; /* bit 3 -> flag 3 */ \
	if(BC) F |= VF; \
} while(0)

#define IND() do { \
	unsigned t; \
	uint8_t io = IN8(BC); \
	WZ = BC - 1; \
	B--; \
	WM8(HL, io); \
	HL--; \
	F = SZ[B]; \
	t = ((unsigned)(C - 1) & 0xff) + (unsigned)io; \
	if(io & SF) F |= NF; \
	if(t & 0x100) F |= HF | CF; \
	F |= SZP[(uint8_t)(t & 0x07) ^ B] & PF; \
} while(0)

#define OUTD() do { \
	unsigned t; \
	uint8_t io = RM8(HL); \
	B--; \
	WZ = BC - 1; \
	OUT8(BC, io); \
	HL--; \
	F = SZ[B]; \
	t = (unsigned)L + (unsigned)io; \
	if(io & SF) F |= NF; \
	if(t & 0x100) F |= HF | CF; \
	F |= SZP[(uint8_t)(t & 0x07) ^ B] & PF; \
} while(0)

#define LDIR() do { \
	LDI(); \
	if(BC != 0) { \
		PC -= 2; \
		WZ = PC + 1; \
		icount -= cc_ex[0xb0]; \
	} \
} while(0)

#define CPIR() do { \
	CPI(); \
	if(BC != 0 && !(F & ZF)) { \
		PC -= 2; \
		WZ = PC + 1; \
		icount -= cc_ex[0xb1]; \
	} \
} while(0)

#define INIR() do { \
	INI(); \
	if(B != 0) { \
		PC -= 2; \
		icount -= cc_ex[0xb2]; \
	} \
} while(0)

#define OTIR() do { \
	OUTI(); \
	if(B != 0) { \
		PC -= 2; \
		icount -= cc_ex[0xb3]; \
	} \
} while(0)

#define LDDR() do { \
	LDD(); \
	if(BC != 0) { \
		PC -= 2; \
		WZ = PC + 1; \
		icount -= cc_ex[0xb8]; \
	} \
} while(0)

#define CPDR() do { \
	CPD(); \
	if(BC != 0 && !(F & ZF)) { \
		PC -= 2; \
		WZ = PC + 1; \
		icount -= cc_ex[0xb9]; \
	} \
} while(0)

#define INDR() do { \
	IND(); \
	if(B != 0) { \
		PC -= 2; \
		icount -= cc_ex[0xba]; \
	} \
} while(0)

#define OTDR() do { \
	OUTD(); \
	if(B != 0) { \
		PC -= 2; \
		icount -= cc_ex[0xbb]; \
	} \
} while(0)

#define EI() do { \
	iff1 = iff2 = 1; \
	after_ei = true; \
	int_state |= Z80_STATE_EI; \
	DASM_SET_SIGNAL(int_state); \
} while(0)

#define DI() do { \
	iff1 = iff2 = 0; \
	int_state &= ~Z80_STATE_EI; \
	DASM_SET_SIGNAL(int_state); \
} while(0)

void Z80::OP_CB(uint8_t code)
{
	icount -= cc_cb[code];

	switch(code) {
	case 0x00: B = RLC(B); break;			/* RLC  B           */
	case 0x01: C = RLC(C); break;			/* RLC  C           */
	case 0x02: D = RLC(D); break;			/* RLC  D           */
	case 0x03: E = RLC(E); break;			/* RLC  E           */
	case 0x04: H = RLC(H); break;			/* RLC  H           */
	case 0x05: L = RLC(L); break;			/* RLC  L           */
	case 0x06: WM8(HL, RLC(RM8(HL))); break;	/* RLC  (HL)        */
	case 0x07: A = RLC(A); break;			/* RLC  A           */
	case 0x08: B = RRC(B); break;			/* RRC  B           */
	case 0x09: C = RRC(C); break;			/* RRC  C           */
	case 0x0a: D = RRC(D); break;			/* RRC  D           */
	case 0x0b: E = RRC(E); break;			/* RRC  E           */
	case 0x0c: H = RRC(H); break;			/* RRC  H           */
	case 0x0d: L = RRC(L); break;			/* RRC  L           */
	case 0x0e: WM8(HL, RRC(RM8(HL))); break;	/* RRC  (HL)        */
	case 0x0f: A = RRC(A); break;			/* RRC  A           */
	case 0x10: B = RL(B); break;			/* RL   B           */
	case 0x11: C = RL(C); break;			/* RL   C           */
	case 0x12: D = RL(D); break;			/* RL   D           */
	case 0x13: E = RL(E); break;			/* RL   E           */
	case 0x14: H = RL(H); break;			/* RL   H           */
	case 0x15: L = RL(L); break;			/* RL   L           */
	case 0x16: WM8(HL, RL(RM8(HL))); break;		/* RL   (HL)        */
	case 0x17: A = RL(A); break;			/* RL   A           */
	case 0x18: B = RR(B); break;			/* RR   B           */
	case 0x19: C = RR(C); break;			/* RR   C           */
	case 0x1a: D = RR(D); break;			/* RR   D           */
	case 0x1b: E = RR(E); break;			/* RR   E           */
	case 0x1c: H = RR(H); break;			/* RR   H           */
	case 0x1d: L = RR(L); break;			/* RR   L           */
	case 0x1e: WM8(HL, RR(RM8(HL))); break;		/* RR   (HL)        */
	case 0x1f: A = RR(A); break;			/* RR   A           */
	case 0x20: B = SLA(B); break;			/* SLA  B           */
	case 0x21: C = SLA(C); break;			/* SLA  C           */
	case 0x22: D = SLA(D); break;			/* SLA  D           */
	case 0x23: E = SLA(E); break;			/* SLA  E           */
	case 0x24: H = SLA(H); break;			/* SLA  H           */
	case 0x25: L = SLA(L); break;			/* SLA  L           */
	case 0x26: WM8(HL, SLA(RM8(HL))); break;	/* SLA  (HL)        */
	case 0x27: A = SLA(A); break;			/* SLA  A           */
	case 0x28: B = SRA(B); break;			/* SRA  B           */
	case 0x29: C = SRA(C); break;			/* SRA  C           */
	case 0x2a: D = SRA(D); break;			/* SRA  D           */
	case 0x2b: E = SRA(E); break;			/* SRA  E           */
	case 0x2c: H = SRA(H); break;			/* SRA  H           */
	case 0x2d: L = SRA(L); break;			/* SRA  L           */
	case 0x2e: WM8(HL, SRA(RM8(HL))); break;	/* SRA  (HL)        */
	case 0x2f: A = SRA(A); break;			/* SRA  A           */
	case 0x30: B = SLL(B); break;			/* SLL  B           */
	case 0x31: C = SLL(C); break;			/* SLL  C           */
	case 0x32: D = SLL(D); break;			/* SLL  D           */
	case 0x33: E = SLL(E); break;			/* SLL  E           */
	case 0x34: H = SLL(H); break;			/* SLL  H           */
	case 0x35: L = SLL(L); break;			/* SLL  L           */
	case 0x36: WM8(HL, SLL(RM8(HL))); break;	/* SLL  (HL)        */
	case 0x37: A = SLL(A); break;			/* SLL  A           */
	case 0x38: B = SRL(B); break;			/* SRL  B           */
	case 0x39: C = SRL(C); break;			/* SRL  C           */
	case 0x3a: D = SRL(D); break;			/* SRL  D           */
	case 0x3b: E = SRL(E); break;			/* SRL  E           */
	case 0x3c: H = SRL(H); break;			/* SRL  H           */
	case 0x3d: L = SRL(L); break;			/* SRL  L           */
	case 0x3e: WM8(HL, SRL(RM8(HL))); break;	/* SRL  (HL)        */
	case 0x3f: A = SRL(A); break;			/* SRL  A           */
	case 0x40: BIT(0, B); break;			/* BIT  0,B         */
	case 0x41: BIT(0, C); break;			/* BIT  0,C         */
	case 0x42: BIT(0, D); break;			/* BIT  0,D         */
	case 0x43: BIT(0, E); break;			/* BIT  0,E         */
	case 0x44: BIT(0, H); break;			/* BIT  0,H         */
	case 0x45: BIT(0, L); break;			/* BIT  0,L         */
	case 0x46: BIT_HL(0, RM8(HL)); break;		/* BIT  0,(HL)      */
	case 0x47: BIT(0, A); break;			/* BIT  0,A         */
	case 0x48: BIT(1, B); break;			/* BIT  1,B         */
	case 0x49: BIT(1, C); break;			/* BIT  1,C         */
	case 0x4a: BIT(1, D); break;			/* BIT  1,D         */
	case 0x4b: BIT(1, E); break;			/* BIT  1,E         */
	case 0x4c: BIT(1, H); break;			/* BIT  1,H         */
	case 0x4d: BIT(1, L); break;			/* BIT  1,L         */
	case 0x4e: BIT_HL(1, RM8(HL)); break;		/* BIT  1,(HL)      */
	case 0x4f: BIT(1, A); break;			/* BIT  1,A         */
	case 0x50: BIT(2, B); break;			/* BIT  2,B         */
	case 0x51: BIT(2, C); break;			/* BIT  2,C         */
	case 0x52: BIT(2, D); break;			/* BIT  2,D         */
	case 0x53: BIT(2, E); break;			/* BIT  2,E         */
	case 0x54: BIT(2, H); break;			/* BIT  2,H         */
	case 0x55: BIT(2, L); break;			/* BIT  2,L         */
	case 0x56: BIT_HL(2, RM8(HL)); break;		/* BIT  2,(HL)      */
	case 0x57: BIT(2, A); break;			/* BIT  2,A         */
	case 0x58: BIT(3, B); break;			/* BIT  3,B         */
	case 0x59: BIT(3, C); break;			/* BIT  3,C         */
	case 0x5a: BIT(3, D); break;			/* BIT  3,D         */
	case 0x5b: BIT(3, E); break;			/* BIT  3,E         */
	case 0x5c: BIT(3, H); break;			/* BIT  3,H         */
	case 0x5d: BIT(3, L); break;			/* BIT  3,L         */
	case 0x5e: BIT_HL(3, RM8(HL)); break;		/* BIT  3,(HL)      */
	case 0x5f: BIT(3, A); break;			/* BIT  3,A         */
	case 0x60: BIT(4, B); break;			/* BIT  4,B         */
	case 0x61: BIT(4, C); break;			/* BIT  4,C         */
	case 0x62: BIT(4, D); break;			/* BIT  4,D         */
	case 0x63: BIT(4, E); break;			/* BIT  4,E         */
	case 0x64: BIT(4, H); break;			/* BIT  4,H         */
	case 0x65: BIT(4, L); break;			/* BIT  4,L         */
	case 0x66: BIT_HL(4, RM8(HL)); break;		/* BIT  4,(HL)      */
	case 0x67: BIT(4, A); break;			/* BIT  4,A         */
	case 0x68: BIT(5, B); break;			/* BIT  5,B         */
	case 0x69: BIT(5, C); break;			/* BIT  5,C         */
	case 0x6a: BIT(5, D); break;			/* BIT  5,D         */
	case 0x6b: BIT(5, E); break;			/* BIT  5,E         */
	case 0x6c: BIT(5, H); break;			/* BIT  5,H         */
	case 0x6d: BIT(5, L); break;			/* BIT  5,L         */
	case 0x6e: BIT_HL(5, RM8(HL)); break;		/* BIT  5,(HL)      */
	case 0x6f: BIT(5, A); break;			/* BIT  5,A         */
	case 0x70: BIT(6, B); break;			/* BIT  6,B         */
	case 0x71: BIT(6, C); break;			/* BIT  6,C         */
	case 0x72: BIT(6, D); break;			/* BIT  6,D         */
	case 0x73: BIT(6, E); break;			/* BIT  6,E         */
	case 0x74: BIT(6, H); break;			/* BIT  6,H         */
	case 0x75: BIT(6, L); break;			/* BIT  6,L         */
	case 0x76: BIT_HL(6, RM8(HL)); break;		/* BIT  6,(HL)      */
	case 0x77: BIT(6, A); break;			/* BIT  6,A         */
	case 0x78: BIT(7, B); break;			/* BIT  7,B         */
	case 0x79: BIT(7, C); break;			/* BIT  7,C         */
	case 0x7a: BIT(7, D); break;			/* BIT  7,D         */
	case 0x7b: BIT(7, E); break;			/* BIT  7,E         */
	case 0x7c: BIT(7, H); break;			/* BIT  7,H         */
	case 0x7d: BIT(7, L); break;			/* BIT  7,L         */
	case 0x7e: BIT_HL(7, RM8(HL)); break;		/* BIT  7,(HL)      */
	case 0x7f: BIT(7, A); break;			/* BIT  7,A         */
	case 0x80: B = RES(0, B); break;		/* RES  0,B         */
	case 0x81: C = RES(0, C); break;		/* RES  0,C         */
	case 0x82: D = RES(0, D); break;		/* RES  0,D         */
	case 0x83: E = RES(0, E); break;		/* RES  0,E         */
	case 0x84: H = RES(0, H); break;		/* RES  0,H         */
	case 0x85: L = RES(0, L); break;		/* RES  0,L         */
	case 0x86: WM8(HL, RES(0, RM8(HL))); break;	/* RES  0,(HL)      */
	case 0x87: A = RES(0, A); break;		/* RES  0,A         */
	case 0x88: B = RES(1, B); break;		/* RES  1,B         */
	case 0x89: C = RES(1, C); break;		/* RES  1,C         */
	case 0x8a: D = RES(1, D); break;		/* RES  1,D         */
	case 0x8b: E = RES(1, E); break;		/* RES  1,E         */
	case 0x8c: H = RES(1, H); break;		/* RES  1,H         */
	case 0x8d: L = RES(1, L); break;		/* RES  1,L         */
	case 0x8e: WM8(HL, RES(1, RM8(HL))); break;	/* RES  1,(HL)      */
	case 0x8f: A = RES(1, A); break;		/* RES  1,A         */
	case 0x90: B = RES(2, B); break;		/* RES  2,B         */
	case 0x91: C = RES(2, C); break;		/* RES  2,C         */
	case 0x92: D = RES(2, D); break;		/* RES  2,D         */
	case 0x93: E = RES(2, E); break;		/* RES  2,E         */
	case 0x94: H = RES(2, H); break;		/* RES  2,H         */
	case 0x95: L = RES(2, L); break;		/* RES  2,L         */
	case 0x96: WM8(HL, RES(2, RM8(HL))); break;	/* RES  2,(HL)      */
	case 0x97: A = RES(2, A); break;		/* RES  2,A         */
	case 0x98: B = RES(3, B); break;		/* RES  3,B         */
	case 0x99: C = RES(3, C); break;		/* RES  3,C         */
	case 0x9a: D = RES(3, D); break;		/* RES  3,D         */
	case 0x9b: E = RES(3, E); break;		/* RES  3,E         */
	case 0x9c: H = RES(3, H); break;		/* RES  3,H         */
	case 0x9d: L = RES(3, L); break;		/* RES  3,L         */
	case 0x9e: WM8(HL, RES(3, RM8(HL))); break;	/* RES  3,(HL)      */
	case 0x9f: A = RES(3, A); break;		/* RES  3,A         */
	case 0xa0: B = RES(4,	B); break;		/* RES  4,B         */
	case 0xa1: C = RES(4,	C); break;		/* RES  4,C         */
	case 0xa2: D = RES(4,	D); break;		/* RES  4,D         */
	case 0xa3: E = RES(4,	E); break;		/* RES  4,E         */
	case 0xa4: H = RES(4,	H); break;		/* RES  4,H         */
	case 0xa5: L = RES(4,	L); break;		/* RES  4,L         */
	case 0xa6: WM8(HL, RES(4, RM8(HL))); break;	/* RES  4,(HL)      */
	case 0xa7: A = RES(4,	A); break;		/* RES  4,A         */
	case 0xa8: B = RES(5, B); break;		/* RES  5,B         */
	case 0xa9: C = RES(5, C); break;		/* RES  5,C         */
	case 0xaa: D = RES(5, D); break;		/* RES  5,D         */
	case 0xab: E = RES(5, E); break;		/* RES  5,E         */
	case 0xac: H = RES(5, H); break;		/* RES  5,H         */
	case 0xad: L = RES(5, L); break;		/* RES  5,L         */
	case 0xae: WM8(HL, RES(5, RM8(HL))); break;	/* RES  5,(HL)      */
	case 0xaf: A = RES(5, A); break;		/* RES  5,A         */
	case 0xb0: B = RES(6, B); break;		/* RES  6,B         */
	case 0xb1: C = RES(6, C); break;		/* RES  6,C         */
	case 0xb2: D = RES(6, D); break;		/* RES  6,D         */
	case 0xb3: E = RES(6, E); break;		/* RES  6,E         */
	case 0xb4: H = RES(6, H); break;		/* RES  6,H         */
	case 0xb5: L = RES(6, L); break;		/* RES  6,L         */
	case 0xb6: WM8(HL, RES(6, RM8(HL))); break;	/* RES  6,(HL)      */
	case 0xb7: A = RES(6, A); break;		/* RES  6,A         */
	case 0xb8: B = RES(7, B); break;		/* RES  7,B         */
	case 0xb9: C = RES(7, C); break;		/* RES  7,C         */
	case 0xba: D = RES(7, D); break;		/* RES  7,D         */
	case 0xbb: E = RES(7, E); break;		/* RES  7,E         */
	case 0xbc: H = RES(7, H); break;		/* RES  7,H         */
	case 0xbd: L = RES(7, L); break;		/* RES  7,L         */
	case 0xbe: WM8(HL, RES(7, RM8(HL))); break;	/* RES  7,(HL)      */
	case 0xbf: A = RES(7, A); break;		/* RES  7,A         */
	case 0xc0: B = SET(0, B); break;		/* SET  0,B         */
	case 0xc1: C = SET(0, C); break;		/* SET  0,C         */
	case 0xc2: D = SET(0, D); break;		/* SET  0,D         */
	case 0xc3: E = SET(0, E); break;		/* SET  0,E         */
	case 0xc4: H = SET(0, H); break;		/* SET  0,H         */
	case 0xc5: L = SET(0, L); break;		/* SET  0,L         */
	case 0xc6: WM8(HL, SET(0, RM8(HL))); break;	/* SET  0,(HL)      */
	case 0xc7: A = SET(0, A); break;		/* SET  0,A         */
	case 0xc8: B = SET(1, B); break;		/* SET  1,B         */
	case 0xc9: C = SET(1, C); break;		/* SET  1,C         */
	case 0xca: D = SET(1, D); break;		/* SET  1,D         */
	case 0xcb: E = SET(1, E); break;		/* SET  1,E         */
	case 0xcc: H = SET(1, H); break;		/* SET  1,H         */
	case 0xcd: L = SET(1, L); break;		/* SET  1,L         */
	case 0xce: WM8(HL, SET(1, RM8(HL))); break;	/* SET  1,(HL)      */
	case 0xcf: A = SET(1, A); break;		/* SET  1,A         */
	case 0xd0: B = SET(2, B); break;		/* SET  2,B         */
	case 0xd1: C = SET(2, C); break;		/* SET  2,C         */
	case 0xd2: D = SET(2, D); break;		/* SET  2,D         */
	case 0xd3: E = SET(2, E); break;		/* SET  2,E         */
	case 0xd4: H = SET(2, H); break;		/* SET  2,H         */
	case 0xd5: L = SET(2, L); break;		/* SET  2,L         */
	case 0xd6: WM8(HL, SET(2, RM8(HL))); break;	/* SET  2,(HL)      */
	case 0xd7: A = SET(2, A); break;		/* SET  2,A         */
	case 0xd8: B = SET(3, B); break;		/* SET  3,B         */
	case 0xd9: C = SET(3, C); break;		/* SET  3,C         */
	case 0xda: D = SET(3, D); break;		/* SET  3,D         */
	case 0xdb: E = SET(3, E); break;		/* SET  3,E         */
	case 0xdc: H = SET(3, H); break;		/* SET  3,H         */
	case 0xdd: L = SET(3, L); break;		/* SET  3,L         */
	case 0xde: WM8(HL, SET(3, RM8(HL))); break;	/* SET  3,(HL)      */
	case 0xdf: A = SET(3, A); break;		/* SET  3,A         */
	case 0xe0: B = SET(4, B); break;		/* SET  4,B         */
	case 0xe1: C = SET(4, C); break;		/* SET  4,C         */
	case 0xe2: D = SET(4, D); break;		/* SET  4,D         */
	case 0xe3: E = SET(4, E); break;		/* SET  4,E         */
	case 0xe4: H = SET(4, H); break;		/* SET  4,H         */
	case 0xe5: L = SET(4, L); break;		/* SET  4,L         */
	case 0xe6: WM8(HL, SET(4, RM8(HL))); break;	/* SET  4,(HL)      */
	case 0xe7: A = SET(4, A); break;		/* SET  4,A         */
	case 0xe8: B = SET(5, B); break;		/* SET  5,B         */
	case 0xe9: C = SET(5, C); break;		/* SET  5,C         */
	case 0xea: D = SET(5, D); break;		/* SET  5,D         */
	case 0xeb: E = SET(5, E); break;		/* SET  5,E         */
	case 0xec: H = SET(5, H); break;		/* SET  5,H         */
	case 0xed: L = SET(5, L); break;		/* SET  5,L         */
	case 0xee: WM8(HL, SET(5, RM8(HL))); break;	/* SET  5,(HL)      */
	case 0xef: A = SET(5, A); break;		/* SET  5,A         */
	case 0xf0: B = SET(6, B); break;		/* SET  6,B         */
	case 0xf1: C = SET(6, C); break;		/* SET  6,C         */
	case 0xf2: D = SET(6, D); break;		/* SET  6,D         */
	case 0xf3: E = SET(6, E); break;		/* SET  6,E         */
	case 0xf4: H = SET(6, H); break;		/* SET  6,H         */
	case 0xf5: L = SET(6, L); break;		/* SET  6,L         */
	case 0xf6: WM8(HL, SET(6, RM8(HL))); break;	/* SET  6,(HL)      */
	case 0xf7: A = SET(6, A); break;		/* SET  6,A         */
	case 0xf8: B = SET(7, B); break;		/* SET  7,B         */
	case 0xf9: C = SET(7, C); break;		/* SET  7,C         */
	case 0xfa: D = SET(7, D); break;		/* SET  7,D         */
	case 0xfb: E = SET(7, E); break;		/* SET  7,E         */
	case 0xfc: H = SET(7, H); break;		/* SET  7,H         */
	case 0xfd: L = SET(7, L); break;		/* SET  7,L         */
	case 0xfe: WM8(HL, SET(7, RM8(HL))); break;	/* SET  7,(HL)      */
	case 0xff: A = SET(7, A); break;		/* SET  7,A         */
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
	default: __assume(0);
#endif
	}
}

void Z80::OP_XY(uint8_t code)
{
	icount -= cc_xycb[code];

	switch(code) {
	case 0x00: B = RLC(RM8(ea)); WM8(ea, B); break;		/* RLC  B=(XY+o)    */
	case 0x01: C = RLC(RM8(ea)); WM8(ea, C); break;		/* RLC  C=(XY+o)    */
	case 0x02: D = RLC(RM8(ea)); WM8(ea, D); break;		/* RLC  D=(XY+o)    */
	case 0x03: E = RLC(RM8(ea)); WM8(ea, E); break;		/* RLC  E=(XY+o)    */
	case 0x04: H = RLC(RM8(ea)); WM8(ea, H); break;		/* RLC  H=(XY+o)    */
	case 0x05: L = RLC(RM8(ea)); WM8(ea, L); break;		/* RLC  L=(XY+o)    */
	case 0x06: WM8(ea, RLC(RM8(ea))); break;		/* RLC  (XY+o)      */
	case 0x07: A = RLC(RM8(ea)); WM8(ea, A); break;		/* RLC  A=(XY+o)    */
	case 0x08: B = RRC(RM8(ea)); WM8(ea, B); break;		/* RRC  B=(XY+o)    */
	case 0x09: C = RRC(RM8(ea)); WM8(ea, C); break;		/* RRC  C=(XY+o)    */
	case 0x0a: D = RRC(RM8(ea)); WM8(ea, D); break;		/* RRC  D=(XY+o)    */
	case 0x0b: E = RRC(RM8(ea)); WM8(ea, E); break;		/* RRC  E=(XY+o)    */
	case 0x0c: H = RRC(RM8(ea)); WM8(ea, H); break;		/* RRC  H=(XY+o)    */
	case 0x0d: L = RRC(RM8(ea)); WM8(ea, L); break;		/* RRC  L=(XY+o)    */
	case 0x0e: WM8(ea, RRC(RM8(ea))); break;		/* RRC  (XY+o)      */
	case 0x0f: A = RRC(RM8(ea)); WM8(ea, A); break;		/* RRC  A=(XY+o)    */
	case 0x10: B = RL(RM8(ea)); WM8(ea, B); break;		/* RL   B=(XY+o)    */
	case 0x11: C = RL(RM8(ea)); WM8(ea, C); break;		/* RL   C=(XY+o)    */
	case 0x12: D = RL(RM8(ea)); WM8(ea, D); break;		/* RL   D=(XY+o)    */
	case 0x13: E = RL(RM8(ea)); WM8(ea, E); break;		/* RL   E=(XY+o)    */
	case 0x14: H = RL(RM8(ea)); WM8(ea, H); break;		/* RL   H=(XY+o)    */
	case 0x15: L = RL(RM8(ea)); WM8(ea, L); break;		/* RL   L=(XY+o)    */
	case 0x16: WM8(ea, RL(RM8(ea))); break;			/* RL   (XY+o)      */
	case 0x17: A = RL(RM8(ea)); WM8(ea, A); break;		/* RL   A=(XY+o)    */
	case 0x18: B = RR(RM8(ea)); WM8(ea, B); break;		/* RR   B=(XY+o)    */
	case 0x19: C = RR(RM8(ea)); WM8(ea, C); break;		/* RR   C=(XY+o)    */
	case 0x1a: D = RR(RM8(ea)); WM8(ea, D); break;		/* RR   D=(XY+o)    */
	case 0x1b: E = RR(RM8(ea)); WM8(ea, E); break;		/* RR   E=(XY+o)    */
	case 0x1c: H = RR(RM8(ea)); WM8(ea, H); break;		/* RR   H=(XY+o)    */
	case 0x1d: L = RR(RM8(ea)); WM8(ea, L); break;		/* RR   L=(XY+o)    */
	case 0x1e: WM8(ea, RR(RM8(ea))); break;			/* RR   (XY+o)      */
	case 0x1f: A = RR(RM8(ea)); WM8(ea, A); break;		/* RR   A=(XY+o)    */
	case 0x20: B = SLA(RM8(ea)); WM8(ea, B); break;		/* SLA  B=(XY+o)    */
	case 0x21: C = SLA(RM8(ea)); WM8(ea, C); break;		/* SLA  C=(XY+o)    */
	case 0x22: D = SLA(RM8(ea)); WM8(ea, D); break;		/* SLA  D=(XY+o)    */
	case 0x23: E = SLA(RM8(ea)); WM8(ea, E); break;		/* SLA  E=(XY+o)    */
	case 0x24: H = SLA(RM8(ea)); WM8(ea, H); break;		/* SLA  H=(XY+o)    */
	case 0x25: L = SLA(RM8(ea)); WM8(ea, L); break;		/* SLA  L=(XY+o)    */
	case 0x26: WM8(ea, SLA(RM8(ea))); break;		/* SLA  (XY+o)      */
	case 0x27: A = SLA(RM8(ea)); WM8(ea, A); break;		/* SLA  A=(XY+o)    */
	case 0x28: B = SRA(RM8(ea)); WM8(ea, B); break;		/* SRA  B=(XY+o)    */
	case 0x29: C = SRA(RM8(ea)); WM8(ea, C); break;		/* SRA  C=(XY+o)    */
	case 0x2a: D = SRA(RM8(ea)); WM8(ea, D); break;		/* SRA  D=(XY+o)    */
	case 0x2b: E = SRA(RM8(ea)); WM8(ea, E); break;		/* SRA  E=(XY+o)    */
	case 0x2c: H = SRA(RM8(ea)); WM8(ea, H); break;		/* SRA  H=(XY+o)    */
	case 0x2d: L = SRA(RM8(ea)); WM8(ea, L); break;		/* SRA  L=(XY+o)    */
	case 0x2e: WM8(ea, SRA(RM8(ea))); break;		/* SRA  (XY+o)      */
	case 0x2f: A = SRA(RM8(ea)); WM8(ea, A); break;		/* SRA  A=(XY+o)    */
	case 0x30: B = SLL(RM8(ea)); WM8(ea, B); break;		/* SLL  B=(XY+o)    */
	case 0x31: C = SLL(RM8(ea)); WM8(ea, C); break;		/* SLL  C=(XY+o)    */
	case 0x32: D = SLL(RM8(ea)); WM8(ea, D); break;		/* SLL  D=(XY+o)    */
	case 0x33: E = SLL(RM8(ea)); WM8(ea, E); break;		/* SLL  E=(XY+o)    */
	case 0x34: H = SLL(RM8(ea)); WM8(ea, H); break;		/* SLL  H=(XY+o)    */
	case 0x35: L = SLL(RM8(ea)); WM8(ea, L); break;		/* SLL  L=(XY+o)    */
	case 0x36: WM8(ea, SLL(RM8(ea))); break;		/* SLL  (XY+o)      */
	case 0x37: A = SLL(RM8(ea)); WM8(ea, A); break;		/* SLL  A=(XY+o)    */
	case 0x38: B = SRL(RM8(ea)); WM8(ea, B); break;		/* SRL  B=(XY+o)    */
	case 0x39: C = SRL(RM8(ea)); WM8(ea, C); break;		/* SRL  C=(XY+o)    */
	case 0x3a: D = SRL(RM8(ea)); WM8(ea, D); break;		/* SRL  D=(XY+o)    */
	case 0x3b: E = SRL(RM8(ea)); WM8(ea, E); break;		/* SRL  E=(XY+o)    */
	case 0x3c: H = SRL(RM8(ea)); WM8(ea, H); break;		/* SRL  H=(XY+o)    */
	case 0x3d: L = SRL(RM8(ea)); WM8(ea, L); break;		/* SRL  L=(XY+o)    */
	case 0x3e: WM8(ea, SRL(RM8(ea))); break;		/* SRL  (XY+o)      */
	case 0x3f: A = SRL(RM8(ea)); WM8(ea, A); break;		/* SRL  A=(XY+o)    */
	case 0x40: BIT_XY(0, RM8(ea)); break;			/* BIT  0,(XY+o)    */
	case 0x41: BIT_XY(0, RM8(ea)); break;			/* BIT  0,(XY+o)    */
	case 0x42: BIT_XY(0, RM8(ea)); break;			/* BIT  0,(XY+o)    */
	case 0x43: BIT_XY(0, RM8(ea)); break;			/* BIT  0,(XY+o)    */
	case 0x44: BIT_XY(0, RM8(ea)); break;			/* BIT  0,(XY+o)    */
	case 0x45: BIT_XY(0, RM8(ea)); break;			/* BIT  0,(XY+o)    */
	case 0x46: BIT_XY(0, RM8(ea)); break;			/* BIT  0,(XY+o)    */
	case 0x47: BIT_XY(0, RM8(ea)); break;			/* BIT  0,(XY+o)    */
	case 0x48: BIT_XY(1, RM8(ea)); break;			/* BIT  1,(XY+o)    */
	case 0x49: BIT_XY(1, RM8(ea)); break;			/* BIT  1,(XY+o)    */
	case 0x4a: BIT_XY(1, RM8(ea)); break;			/* BIT  1,(XY+o)    */
	case 0x4b: BIT_XY(1, RM8(ea)); break;			/* BIT  1,(XY+o)    */
	case 0x4c: BIT_XY(1, RM8(ea)); break;			/* BIT  1,(XY+o)    */
	case 0x4d: BIT_XY(1, RM8(ea)); break;			/* BIT  1,(XY+o)    */
	case 0x4e: BIT_XY(1, RM8(ea)); break;			/* BIT  1,(XY+o)    */
	case 0x4f: BIT_XY(1, RM8(ea)); break;			/* BIT  1,(XY+o)    */
	case 0x50: BIT_XY(2, RM8(ea)); break;			/* BIT  2,(XY+o)    */
	case 0x51: BIT_XY(2, RM8(ea)); break;			/* BIT  2,(XY+o)    */
	case 0x52: BIT_XY(2, RM8(ea)); break;			/* BIT  2,(XY+o)    */
	case 0x53: BIT_XY(2, RM8(ea)); break;			/* BIT  2,(XY+o)    */
	case 0x54: BIT_XY(2, RM8(ea)); break;			/* BIT  2,(XY+o)    */
	case 0x55: BIT_XY(2, RM8(ea)); break;			/* BIT  2,(XY+o)    */
	case 0x56: BIT_XY(2, RM8(ea)); break;			/* BIT  2,(XY+o)    */
	case 0x57: BIT_XY(2, RM8(ea)); break;			/* BIT  2,(XY+o)    */
	case 0x58: BIT_XY(3, RM8(ea)); break;			/* BIT  3,(XY+o)    */
	case 0x59: BIT_XY(3, RM8(ea)); break;			/* BIT  3,(XY+o)    */
	case 0x5a: BIT_XY(3, RM8(ea)); break;			/* BIT  3,(XY+o)    */
	case 0x5b: BIT_XY(3, RM8(ea)); break;			/* BIT  3,(XY+o)    */
	case 0x5c: BIT_XY(3, RM8(ea)); break;			/* BIT  3,(XY+o)    */
	case 0x5d: BIT_XY(3, RM8(ea)); break;			/* BIT  3,(XY+o)    */
	case 0x5e: BIT_XY(3, RM8(ea)); break;			/* BIT  3,(XY+o)    */
	case 0x5f: BIT_XY(3, RM8(ea)); break;			/* BIT  3,(XY+o)    */
	case 0x60: BIT_XY(4, RM8(ea)); break;			/* BIT  4,(XY+o)    */
	case 0x61: BIT_XY(4, RM8(ea)); break;			/* BIT  4,(XY+o)    */
	case 0x62: BIT_XY(4, RM8(ea)); break;			/* BIT  4,(XY+o)    */
	case 0x63: BIT_XY(4, RM8(ea)); break;			/* BIT  4,(XY+o)    */
	case 0x64: BIT_XY(4, RM8(ea)); break;			/* BIT  4,(XY+o)    */
	case 0x65: BIT_XY(4, RM8(ea)); break;			/* BIT  4,(XY+o)    */
	case 0x66: BIT_XY(4, RM8(ea)); break;			/* BIT  4,(XY+o)    */
	case 0x67: BIT_XY(4, RM8(ea)); break;			/* BIT  4,(XY+o)    */
	case 0x68: BIT_XY(5, RM8(ea)); break;			/* BIT  5,(XY+o)    */
	case 0x69: BIT_XY(5, RM8(ea)); break;			/* BIT  5,(XY+o)    */
	case 0x6a: BIT_XY(5, RM8(ea)); break;			/* BIT  5,(XY+o)    */
	case 0x6b: BIT_XY(5, RM8(ea)); break;			/* BIT  5,(XY+o)    */
	case 0x6c: BIT_XY(5, RM8(ea)); break;			/* BIT  5,(XY+o)    */
	case 0x6d: BIT_XY(5, RM8(ea)); break;			/* BIT  5,(XY+o)    */
	case 0x6e: BIT_XY(5, RM8(ea)); break;			/* BIT  5,(XY+o)    */
	case 0x6f: BIT_XY(5, RM8(ea)); break;			/* BIT  5,(XY+o)    */
	case 0x70: BIT_XY(6, RM8(ea)); break;			/* BIT  6,(XY+o)    */
	case 0x71: BIT_XY(6, RM8(ea)); break;			/* BIT  6,(XY+o)    */
	case 0x72: BIT_XY(6, RM8(ea)); break;			/* BIT  6,(XY+o)    */
	case 0x73: BIT_XY(6, RM8(ea)); break;			/* BIT  6,(XY+o)    */
	case 0x74: BIT_XY(6, RM8(ea)); break;			/* BIT  6,(XY+o)    */
	case 0x75: BIT_XY(6, RM8(ea)); break;			/* BIT  6,(XY+o)    */
	case 0x76: BIT_XY(6, RM8(ea)); break;			/* BIT  6,(XY+o)    */
	case 0x77: BIT_XY(6, RM8(ea)); break;			/* BIT  6,(XY+o)    */
	case 0x78: BIT_XY(7, RM8(ea)); break;			/* BIT  7,(XY+o)    */
	case 0x79: BIT_XY(7, RM8(ea)); break;			/* BIT  7,(XY+o)    */
	case 0x7a: BIT_XY(7, RM8(ea)); break;			/* BIT  7,(XY+o)    */
	case 0x7b: BIT_XY(7, RM8(ea)); break;			/* BIT  7,(XY+o)    */
	case 0x7c: BIT_XY(7, RM8(ea)); break;			/* BIT  7,(XY+o)    */
	case 0x7d: BIT_XY(7, RM8(ea)); break;			/* BIT  7,(XY+o)    */
	case 0x7e: BIT_XY(7, RM8(ea)); break;			/* BIT  7,(XY+o)    */
	case 0x7f: BIT_XY(7, RM8(ea)); break;			/* BIT  7,(XY+o)    */
	case 0x80: B = RES(0, RM8(ea)); WM8(ea, B); break;	/* RES  0,B=(XY+o)  */
	case 0x81: C = RES(0, RM8(ea)); WM8(ea, C); break;	/* RES  0,C=(XY+o)  */
	case 0x82: D = RES(0, RM8(ea)); WM8(ea, D); break;	/* RES  0,D=(XY+o)  */
	case 0x83: E = RES(0, RM8(ea)); WM8(ea, E); break;	/* RES  0,E=(XY+o)  */
	case 0x84: H = RES(0, RM8(ea)); WM8(ea, H); break;	/* RES  0,H=(XY+o)  */
	case 0x85: L = RES(0, RM8(ea)); WM8(ea, L); break;	/* RES  0,L=(XY+o)  */
	case 0x86: WM8(ea, RES(0, RM8(ea))); break;		/* RES  0,(XY+o)    */
	case 0x87: A = RES(0, RM8(ea)); WM8(ea, A); break;	/* RES  0,A=(XY+o)  */
	case 0x88: B = RES(1, RM8(ea)); WM8(ea, B); break;	/* RES  1,B=(XY+o)  */
	case 0x89: C = RES(1, RM8(ea)); WM8(ea, C); break;	/* RES  1,C=(XY+o)  */
	case 0x8a: D = RES(1, RM8(ea)); WM8(ea, D); break;	/* RES  1,D=(XY+o)  */
	case 0x8b: E = RES(1, RM8(ea)); WM8(ea, E); break;	/* RES  1,E=(XY+o)  */
	case 0x8c: H = RES(1, RM8(ea)); WM8(ea, H); break;	/* RES  1,H=(XY+o)  */
	case 0x8d: L = RES(1, RM8(ea)); WM8(ea, L); break;	/* RES  1,L=(XY+o)  */
	case 0x8e: WM8(ea, RES(1, RM8(ea))); break;		/* RES  1,(XY+o)    */
	case 0x8f: A = RES(1, RM8(ea)); WM8(ea, A); break;	/* RES  1,A=(XY+o)  */
	case 0x90: B = RES(2, RM8(ea)); WM8(ea, B); break;	/* RES  2,B=(XY+o)  */
	case 0x91: C = RES(2, RM8(ea)); WM8(ea, C); break;	/* RES  2,C=(XY+o)  */
	case 0x92: D = RES(2, RM8(ea)); WM8(ea, D); break;	/* RES  2,D=(XY+o)  */
	case 0x93: E = RES(2, RM8(ea)); WM8(ea, E); break;	/* RES  2,E=(XY+o)  */
	case 0x94: H = RES(2, RM8(ea)); WM8(ea, H); break;	/* RES  2,H=(XY+o)  */
	case 0x95: L = RES(2, RM8(ea)); WM8(ea, L); break;	/* RES  2,L=(XY+o)  */
	case 0x96: WM8(ea, RES(2, RM8(ea))); break;		/* RES  2,(XY+o)    */
	case 0x97: A = RES(2, RM8(ea)); WM8(ea, A); break;	/* RES  2,A=(XY+o)  */
	case 0x98: B = RES(3, RM8(ea)); WM8(ea, B); break;	/* RES  3,B=(XY+o)  */
	case 0x99: C = RES(3, RM8(ea)); WM8(ea, C); break;	/* RES  3,C=(XY+o)  */
	case 0x9a: D = RES(3, RM8(ea)); WM8(ea, D); break;	/* RES  3,D=(XY+o)  */
	case 0x9b: E = RES(3, RM8(ea)); WM8(ea, E); break;	/* RES  3,E=(XY+o)  */
	case 0x9c: H = RES(3, RM8(ea)); WM8(ea, H); break;	/* RES  3,H=(XY+o)  */
	case 0x9d: L = RES(3, RM8(ea)); WM8(ea, L); break;	/* RES  3,L=(XY+o)  */
	case 0x9e: WM8(ea, RES(3, RM8(ea))); break;		/* RES  3,(XY+o)    */
	case 0x9f: A = RES(3, RM8(ea)); WM8(ea, A); break;	/* RES  3,A=(XY+o)  */
	case 0xa0: B = RES(4, RM8(ea)); WM8(ea, B); break;	/* RES  4,B=(XY+o)  */
	case 0xa1: C = RES(4, RM8(ea)); WM8(ea, C); break;	/* RES  4,C=(XY+o)  */
	case 0xa2: D = RES(4, RM8(ea)); WM8(ea, D); break;	/* RES  4,D=(XY+o)  */
	case 0xa3: E = RES(4, RM8(ea)); WM8(ea, E); break;	/* RES  4,E=(XY+o)  */
	case 0xa4: H = RES(4, RM8(ea)); WM8(ea, H); break;	/* RES  4,H=(XY+o)  */
	case 0xa5: L = RES(4, RM8(ea)); WM8(ea, L); break;	/* RES  4,L=(XY+o)  */
	case 0xa6: WM8(ea, RES(4, RM8(ea))); break;		/* RES  4,(XY+o)    */
	case 0xa7: A = RES(4, RM8(ea)); WM8(ea, A); break;	/* RES  4,A=(XY+o)  */
	case 0xa8: B = RES(5, RM8(ea)); WM8(ea, B); break;	/* RES  5,B=(XY+o)  */
	case 0xa9: C = RES(5, RM8(ea)); WM8(ea, C); break;	/* RES  5,C=(XY+o)  */
	case 0xaa: D = RES(5, RM8(ea)); WM8(ea, D); break;	/* RES  5,D=(XY+o)  */
	case 0xab: E = RES(5, RM8(ea)); WM8(ea, E); break;	/* RES  5,E=(XY+o)  */
	case 0xac: H = RES(5, RM8(ea)); WM8(ea, H); break;	/* RES  5,H=(XY+o)  */
	case 0xad: L = RES(5, RM8(ea)); WM8(ea, L); break;	/* RES  5,L=(XY+o)  */
	case 0xae: WM8(ea, RES(5, RM8(ea))); break;		/* RES  5,(XY+o)    */
	case 0xaf: A = RES(5, RM8(ea)); WM8(ea, A); break;	/* RES  5,A=(XY+o)  */
	case 0xb0: B = RES(6, RM8(ea)); WM8(ea, B); break;	/* RES  6,B=(XY+o)  */
	case 0xb1: C = RES(6, RM8(ea)); WM8(ea, C); break;	/* RES  6,C=(XY+o)  */
	case 0xb2: D = RES(6, RM8(ea)); WM8(ea, D); break;	/* RES  6,D=(XY+o)  */
	case 0xb3: E = RES(6, RM8(ea)); WM8(ea, E); break;	/* RES  6,E=(XY+o)  */
	case 0xb4: H = RES(6, RM8(ea)); WM8(ea, H); break;	/* RES  6,H=(XY+o)  */
	case 0xb5: L = RES(6, RM8(ea)); WM8(ea, L); break;	/* RES  6,L=(XY+o)  */
	case 0xb6: WM8(ea, RES(6, RM8(ea))); break;		/* RES  6,(XY+o)    */
	case 0xb7: A = RES(6, RM8(ea)); WM8(ea, A); break;	/* RES  6,A=(XY+o)  */
	case 0xb8: B = RES(7, RM8(ea)); WM8(ea, B); break;	/* RES  7,B=(XY+o)  */
	case 0xb9: C = RES(7, RM8(ea)); WM8(ea, C); break;	/* RES  7,C=(XY+o)  */
	case 0xba: D = RES(7, RM8(ea)); WM8(ea, D); break;	/* RES  7,D=(XY+o)  */
	case 0xbb: E = RES(7, RM8(ea)); WM8(ea, E); break;	/* RES  7,E=(XY+o)  */
	case 0xbc: H = RES(7, RM8(ea)); WM8(ea, H); break;	/* RES  7,H=(XY+o)  */
	case 0xbd: L = RES(7, RM8(ea)); WM8(ea, L); break;	/* RES  7,L=(XY+o)  */
	case 0xbe: WM8(ea, RES(7, RM8(ea))); break;		/* RES  7,(XY+o)    */
	case 0xbf: A = RES(7, RM8(ea)); WM8(ea, A); break;	/* RES  7,A=(XY+o)  */
	case 0xc0: B = SET(0, RM8(ea)); WM8(ea, B); break;	/* SET  0,B=(XY+o)  */
	case 0xc1: C = SET(0, RM8(ea)); WM8(ea, C); break;	/* SET  0,C=(XY+o)  */
	case 0xc2: D = SET(0, RM8(ea)); WM8(ea, D); break;	/* SET  0,D=(XY+o)  */
	case 0xc3: E = SET(0, RM8(ea)); WM8(ea, E); break;	/* SET  0,E=(XY+o)  */
	case 0xc4: H = SET(0, RM8(ea)); WM8(ea, H); break;	/* SET  0,H=(XY+o)  */
	case 0xc5: L = SET(0, RM8(ea)); WM8(ea, L); break;	/* SET  0,L=(XY+o)  */
	case 0xc6: WM8(ea, SET(0, RM8(ea))); break;		/* SET  0,(XY+o)    */
	case 0xc7: A = SET(0, RM8(ea)); WM8(ea, A); break;	/* SET  0,A=(XY+o)  */
	case 0xc8: B = SET(1, RM8(ea)); WM8(ea, B); break;	/* SET  1,B=(XY+o)  */
	case 0xc9: C = SET(1, RM8(ea)); WM8(ea, C); break;	/* SET  1,C=(XY+o)  */
	case 0xca: D = SET(1, RM8(ea)); WM8(ea, D); break;	/* SET  1,D=(XY+o)  */
	case 0xcb: E = SET(1, RM8(ea)); WM8(ea, E); break;	/* SET  1,E=(XY+o)  */
	case 0xcc: H = SET(1, RM8(ea)); WM8(ea, H); break;	/* SET  1,H=(XY+o)  */
	case 0xcd: L = SET(1, RM8(ea)); WM8(ea, L); break;	/* SET  1,L=(XY+o)  */
	case 0xce: WM8(ea, SET(1, RM8(ea))); break;		/* SET  1,(XY+o)    */
	case 0xcf: A = SET(1, RM8(ea)); WM8(ea, A); break;	/* SET  1,A=(XY+o)  */
	case 0xd0: B = SET(2, RM8(ea)); WM8(ea, B); break;	/* SET  2,B=(XY+o)  */
	case 0xd1: C = SET(2, RM8(ea)); WM8(ea, C); break;	/* SET  2,C=(XY+o)  */
	case 0xd2: D = SET(2, RM8(ea)); WM8(ea, D); break;	/* SET  2,D=(XY+o)  */
	case 0xd3: E = SET(2, RM8(ea)); WM8(ea, E); break;	/* SET  2,E=(XY+o)  */
	case 0xd4: H = SET(2, RM8(ea)); WM8(ea, H); break;	/* SET  2,H=(XY+o)  */
	case 0xd5: L = SET(2, RM8(ea)); WM8(ea, L); break;	/* SET  2,L=(XY+o)  */
	case 0xd6: WM8(ea, SET(2, RM8(ea))); break;		/* SET  2,(XY+o)    */
	case 0xd7: A = SET(2, RM8(ea)); WM8(ea, A); break;	/* SET  2,A=(XY+o)  */
	case 0xd8: B = SET(3, RM8(ea)); WM8(ea, B); break;	/* SET  3,B=(XY+o)  */
	case 0xd9: C = SET(3, RM8(ea)); WM8(ea, C); break;	/* SET  3,C=(XY+o)  */
	case 0xda: D = SET(3, RM8(ea)); WM8(ea, D); break;	/* SET  3,D=(XY+o)  */
	case 0xdb: E = SET(3, RM8(ea)); WM8(ea, E); break;	/* SET  3,E=(XY+o)  */
	case 0xdc: H = SET(3, RM8(ea)); WM8(ea, H); break;	/* SET  3,H=(XY+o)  */
	case 0xdd: L = SET(3, RM8(ea)); WM8(ea, L); break;	/* SET  3,L=(XY+o)  */
	case 0xde: WM8(ea, SET(3, RM8(ea))); break;		/* SET  3,(XY+o)    */
	case 0xdf: A = SET(3, RM8(ea)); WM8(ea, A); break;	/* SET  3,A=(XY+o)  */
	case 0xe0: B = SET(4, RM8(ea)); WM8(ea, B); break;	/* SET  4,B=(XY+o)  */
	case 0xe1: C = SET(4, RM8(ea)); WM8(ea, C); break;	/* SET  4,C=(XY+o)  */
	case 0xe2: D = SET(4, RM8(ea)); WM8(ea, D); break;	/* SET  4,D=(XY+o)  */
	case 0xe3: E = SET(4, RM8(ea)); WM8(ea, E); break;	/* SET  4,E=(XY+o)  */
	case 0xe4: H = SET(4, RM8(ea)); WM8(ea, H); break;	/* SET  4,H=(XY+o)  */
	case 0xe5: L = SET(4, RM8(ea)); WM8(ea, L); break;	/* SET  4,L=(XY+o)  */
	case 0xe6: WM8(ea, SET(4, RM8(ea))); break;		/* SET  4,(XY+o)    */
	case 0xe7: A = SET(4, RM8(ea)); WM8(ea, A); break;	/* SET  4,A=(XY+o)  */
	case 0xe8: B = SET(5, RM8(ea)); WM8(ea, B); break;	/* SET  5,B=(XY+o)  */
	case 0xe9: C = SET(5, RM8(ea)); WM8(ea, C); break;	/* SET  5,C=(XY+o)  */
	case 0xea: D = SET(5, RM8(ea)); WM8(ea, D); break;	/* SET  5,D=(XY+o)  */
	case 0xeb: E = SET(5, RM8(ea)); WM8(ea, E); break;	/* SET  5,E=(XY+o)  */
	case 0xec: H = SET(5, RM8(ea)); WM8(ea, H); break;	/* SET  5,H=(XY+o)  */
	case 0xed: L = SET(5, RM8(ea)); WM8(ea, L); break;	/* SET  5,L=(XY+o)  */
	case 0xee: WM8(ea, SET(5, RM8(ea))); break;		/* SET  5,(XY+o)    */
	case 0xef: A = SET(5, RM8(ea)); WM8(ea, A); break;	/* SET  5,A=(XY+o)  */
	case 0xf0: B = SET(6, RM8(ea)); WM8(ea, B); break;	/* SET  6,B=(XY+o)  */
	case 0xf1: C = SET(6, RM8(ea)); WM8(ea, C); break;	/* SET  6,C=(XY+o)  */
	case 0xf2: D = SET(6, RM8(ea)); WM8(ea, D); break;	/* SET  6,D=(XY+o)  */
	case 0xf3: E = SET(6, RM8(ea)); WM8(ea, E); break;	/* SET  6,E=(XY+o)  */
	case 0xf4: H = SET(6, RM8(ea)); WM8(ea, H); break;	/* SET  6,H=(XY+o)  */
	case 0xf5: L = SET(6, RM8(ea)); WM8(ea, L); break;	/* SET  6,L=(XY+o)  */
	case 0xf6: WM8(ea, SET(6, RM8(ea))); break;		/* SET  6,(XY+o)    */
	case 0xf7: A = SET(6, RM8(ea)); WM8(ea, A); break;	/* SET  6,A=(XY+o)  */
	case 0xf8: B = SET(7, RM8(ea)); WM8(ea, B); break;	/* SET  7,B=(XY+o)  */
	case 0xf9: C = SET(7, RM8(ea)); WM8(ea, C); break;	/* SET  7,C=(XY+o)  */
	case 0xfa: D = SET(7, RM8(ea)); WM8(ea, D); break;	/* SET  7,D=(XY+o)  */
	case 0xfb: E = SET(7, RM8(ea)); WM8(ea, E); break;	/* SET  7,E=(XY+o)  */
	case 0xfc: H = SET(7, RM8(ea)); WM8(ea, H); break;	/* SET  7,H=(XY+o)  */
	case 0xfd: L = SET(7, RM8(ea)); WM8(ea, L); break;	/* SET  7,L=(XY+o)  */
	case 0xfe: WM8(ea, SET(7, RM8(ea))); break;		/* SET  7,(XY+o)    */
	case 0xff: A = SET(7, RM8(ea)); WM8(ea, A); break;	/* SET  7,A=(XY+o)  */
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
	default: __assume(0);
#endif
	}
}

void Z80::OP_DD(uint8_t code)
{
	icount -= cc_xy[code];

	switch(code) {
	case 0x09: ADD16(ix, bc); break;				/* ADD  IX,BC       */
	case 0x19: ADD16(ix, de); break;				/* ADD  IX,DE       */
	case 0x21: IX = FETCH16(); break;				/* LD   IX,w        */
	case 0x22: ea = FETCH16(); WM16(ea, &ix); WZ = ea + 1; break;	/* LD   (w),IX      */
	case 0x23: IX++; break;						/* INC  IX          */
	case 0x24: HX = INC(HX); break;					/* INC  HX          */
	case 0x25: HX = DEC(HX); break;					/* DEC  HX          */
	case 0x26: HX = FETCH8(); break;				/* LD   HX,n        */
	case 0x29: ADD16(ix, ix); break;				/* ADD  IX,IX       */
	case 0x2a: ea = FETCH16(); RM16(ea, &ix); WZ = ea + 1; break;	/* LD   IX,(w)      */
	case 0x2b: IX--; break;						/* DEC  IX          */
	case 0x2c: LX = INC(LX); break;					/* INC  LX          */
	case 0x2d: LX = DEC(LX); break;					/* DEC  LX          */
	case 0x2e: LX = FETCH8(); break;				/* LD   LX,n        */
	case 0x34: EAX(); WM8(ea, INC(RM8(ea))); break;			/* INC  (IX+o)      */
	case 0x35: EAX(); WM8(ea, DEC(RM8(ea))); break;			/* DEC  (IX+o)      */
	case 0x36: EAX(); WM8(ea, FETCH8()); break;			/* LD   (IX+o),n    */
	case 0x39: ADD16(ix, sp); break;				/* ADD  IX,SP       */
	case 0x44: B = HX; break;					/* LD   B,HX        */
	case 0x45: B = LX; break;					/* LD   B,LX        */
	case 0x46: EAX(); B = RM8(ea); break;				/* LD   B,(IX+o)    */
	case 0x4c: C = HX; break;					/* LD   C,HX        */
	case 0x4d: C = LX; break;					/* LD   C,LX        */
	case 0x4e: EAX(); C = RM8(ea); break;				/* LD   C,(IX+o)    */
	case 0x54: D = HX; break;					/* LD   D,HX        */
	case 0x55: D = LX; break;					/* LD   D,LX        */
	case 0x56: EAX(); D = RM8(ea); break;				/* LD   D,(IX+o)    */
	case 0x5c: E = HX; break;					/* LD   E,HX        */
	case 0x5d: E = LX; break;					/* LD   E,LX        */
	case 0x5e: EAX(); E = RM8(ea); break;				/* LD   E,(IX+o)    */
	case 0x60: HX = B; break;					/* LD   HX,B        */
	case 0x61: HX = C; break;					/* LD   HX,C        */
	case 0x62: HX = D; break;					/* LD   HX,D        */
	case 0x63: HX = E; break;					/* LD   HX,E        */
	case 0x64: break;						/* LD   HX,HX       */
	case 0x65: HX = LX; break;					/* LD   HX,LX       */
	case 0x66: EAX(); H = RM8(ea); break;				/* LD   H,(IX+o)    */
	case 0x67: HX = A; break;					/* LD   HX,A        */
	case 0x68: LX = B; break;					/* LD   LX,B        */
	case 0x69: LX = C; break;					/* LD   LX,C        */
	case 0x6a: LX = D; break;					/* LD   LX,D        */
	case 0x6b: LX = E; break;					/* LD   LX,E        */
	case 0x6c: LX = HX; break;					/* LD   LX,HX       */
	case 0x6d: break;						/* LD   LX,LX       */
	case 0x6e: EAX(); L = RM8(ea); break;				/* LD   L,(IX+o)    */
	case 0x6f: LX = A; break;					/* LD   LX,A        */
	case 0x70: EAX(); WM8(ea, B); break;				/* LD   (IX+o),B    */
	case 0x71: EAX(); WM8(ea, C); break;				/* LD   (IX+o),C    */
	case 0x72: EAX(); WM8(ea, D); break;				/* LD   (IX+o),D    */
	case 0x73: EAX(); WM8(ea, E); break;				/* LD   (IX+o),E    */
	case 0x74: EAX(); WM8(ea, H); break;				/* LD   (IX+o),H    */
	case 0x75: EAX(); WM8(ea, L); break;				/* LD   (IX+o),L    */
	case 0x77: EAX(); WM8(ea, A); break;				/* LD   (IX+o),A    */
	case 0x7c: A = HX; break;					/* LD   A,HX        */
	case 0x7d: A = LX; break;					/* LD   A,LX        */
	case 0x7e: EAX(); A = RM8(ea); break;				/* LD   A,(IX+o)    */
	case 0x84: ADD(HX); break;					/* ADD  A,HX        */
	case 0x85: ADD(LX); break;					/* ADD  A,LX        */
	case 0x86: EAX(); ADD(RM8(ea)); break;				/* ADD  A,(IX+o)    */
	case 0x8c: ADC(HX); break;					/* ADC  A,HX        */
	case 0x8d: ADC(LX); break;					/* ADC  A,LX        */
	case 0x8e: EAX(); ADC(RM8(ea)); break;				/* ADC  A,(IX+o)    */
	case 0x94: SUB(HX); break;					/* SUB  HX          */
	case 0x95: SUB(LX); break;					/* SUB  LX          */
	case 0x96: EAX(); SUB(RM8(ea)); break;				/* SUB  (IX+o)      */
	case 0x9c: SBC(HX); break;					/* SBC  A,HX        */
	case 0x9d: SBC(LX); break;					/* SBC  A,LX        */
	case 0x9e: EAX(); SBC(RM8(ea)); break;				/* SBC  A,(IX+o)    */
	case 0xa4: AND(HX); break;					/* AND  HX          */
	case 0xa5: AND(LX); break;					/* AND  LX          */
	case 0xa6: EAX(); AND(RM8(ea)); break;				/* AND  (IX+o)      */
	case 0xac: XOR(HX); break;					/* XOR  HX          */
	case 0xad: XOR(LX); break;					/* XOR  LX          */
	case 0xae: EAX(); XOR(RM8(ea)); break;				/* XOR  (IX+o)      */
	case 0xb4: OR(HX); break;					/* OR   HX          */
	case 0xb5: OR(LX); break;					/* OR   LX          */
	case 0xb6: EAX(); OR(RM8(ea)); break;				/* OR   (IX+o)      */
	case 0xbc: CP(HX); break;					/* CP   HX          */
	case 0xbd: CP(LX); break;					/* CP   LX          */
	case 0xbe: EAX(); CP(RM8(ea)); break;				/* CP   (IX+o)      */
	case 0xcb: EAX(); OP_XY(FETCH8()); break;			/* **   DD CB xx    */
	case 0xe1: POP(ix); break;					/* POP  IX          */
	case 0xe3: EXSP(ix); break;					/* EX   (SP),IX     */
	case 0xe5: PUSH(ix); break;					/* PUSH IX          */
	case 0xe9: PC = IX; break;					/* JP   (IX)        */
	case 0xf9: SP = IX; break;					/* LD   SP,IX       */
	default:   OP(code); break;
	}
}

void Z80::OP_FD(uint8_t code)
{
	icount -= cc_xy[code];

	switch(code) {
	case 0x09: ADD16(iy, bc); break;				/* ADD  IY,BC       */
	case 0x19: ADD16(iy, de); break;				/* ADD  IY,DE       */
	case 0x21: IY = FETCH16(); break;				/* LD   IY,w        */
	case 0x22: ea = FETCH16(); WM16(ea, &iy); WZ = ea + 1; break;	/* LD   (w),IY      */
	case 0x23: IY++; break;						/* INC  IY          */
	case 0x24: HY = INC(HY); break;					/* INC  HY          */
	case 0x25: HY = DEC(HY); break;					/* DEC  HY          */
	case 0x26: HY = FETCH8(); break;				/* LD   HY,n        */
	case 0x29: ADD16(iy, iy); break;				/* ADD  IY,IY       */
	case 0x2a: ea = FETCH16(); RM16(ea, &iy); WZ = ea + 1; break;	/* LD   IY,(w)      */
	case 0x2b: IY--; break;						/* DEC  IY          */
	case 0x2c: LY = INC(LY); break;					/* INC  LY          */
	case 0x2d: LY = DEC(LY); break;					/* DEC  LY          */
	case 0x2e: LY = FETCH8(); break;				/* LD   LY,n        */
	case 0x34: EAY(); WM8(ea, INC(RM8(ea))); break;			/* INC  (IY+o)      */
	case 0x35: EAY(); WM8(ea, DEC(RM8(ea))); break;			/* DEC  (IY+o)      */
	case 0x36: EAY(); WM8(ea, FETCH8()); break;			/* LD   (IY+o),n    */
	case 0x39: ADD16(iy, sp); break;				/* ADD  IY,SP       */
	case 0x44: B = HY; break;					/* LD   B,HY        */
	case 0x45: B = LY; break;					/* LD   B,LY        */
	case 0x46: EAY(); B = RM8(ea); break;				/* LD   B,(IY+o)    */
	case 0x4c: C = HY; break;					/* LD   C,HY        */
	case 0x4d: C = LY; break;					/* LD   C,LY        */
	case 0x4e: EAY(); C = RM8(ea); break;				/* LD   C,(IY+o)    */
	case 0x54: D = HY; break;					/* LD   D,HY        */
	case 0x55: D = LY; break;					/* LD   D,LY        */
	case 0x56: EAY(); D = RM8(ea); break;				/* LD   D,(IY+o)    */
	case 0x5c: E = HY; break;					/* LD   E,HY        */
	case 0x5d: E = LY; break;					/* LD   E,LY        */
	case 0x5e: EAY(); E = RM8(ea); break;				/* LD   E,(IY+o)    */
	case 0x60: HY = B; break;					/* LD   HY,B        */
	case 0x61: HY = C; break;					/* LD   HY,C        */
	case 0x62: HY = D; break;					/* LD   HY,D        */
	case 0x63: HY = E; break;					/* LD   HY,E        */
	case 0x64: break;						/* LD   HY,HY       */
	case 0x65: HY = LY; break;					/* LD   HY,LY       */
	case 0x66: EAY(); H = RM8(ea); break;				/* LD   H,(IY+o)    */
	case 0x67: HY = A; break;					/* LD   HY,A        */
	case 0x68: LY = B; break;					/* LD   LY,B        */
	case 0x69: LY = C; break;					/* LD   LY,C        */
	case 0x6a: LY = D; break;					/* LD   LY,D        */
	case 0x6b: LY = E; break;					/* LD   LY,E        */
	case 0x6c: LY = HY; break;					/* LD   LY,HY       */
	case 0x6d: break;						/* LD   LY,LY       */
	case 0x6e: EAY(); L = RM8(ea); break;				/* LD   L,(IY+o)    */
	case 0x6f: LY = A; break;					/* LD   LY,A        */
	case 0x70: EAY(); WM8(ea, B); break;				/* LD   (IY+o),B    */
	case 0x71: EAY(); WM8(ea, C); break;				/* LD   (IY+o),C    */
	case 0x72: EAY(); WM8(ea, D); break;				/* LD   (IY+o),D    */
	case 0x73: EAY(); WM8(ea, E); break;				/* LD   (IY+o),E    */
	case 0x74: EAY(); WM8(ea, H); break;				/* LD   (IY+o),H    */
	case 0x75: EAY(); WM8(ea, L); break;				/* LD   (IY+o),L    */
	case 0x77: EAY(); WM8(ea, A); break;				/* LD   (IY+o),A    */
	case 0x7c: A = HY; break;					/* LD   A,HY        */
	case 0x7d: A = LY; break;					/* LD   A,LY        */
	case 0x7e: EAY(); A = RM8(ea); break;				/* LD   A,(IY+o)    */
	case 0x84: ADD(HY); break;					/* ADD  A,HY        */
	case 0x85: ADD(LY); break;					/* ADD  A,LY        */
	case 0x86: EAY(); ADD(RM8(ea)); break;				/* ADD  A,(IY+o)    */
	case 0x8c: ADC(HY); break;					/* ADC  A,HY        */
	case 0x8d: ADC(LY); break;					/* ADC  A,LY        */
	case 0x8e: EAY(); ADC(RM8(ea)); break;				/* ADC  A,(IY+o)    */
	case 0x94: SUB(HY); break;					/* SUB  HY          */
	case 0x95: SUB(LY); break;					/* SUB  LY          */
	case 0x96: EAY(); SUB(RM8(ea)); break;				/* SUB  (IY+o)      */
	case 0x9c: SBC(HY); break;					/* SBC  A,HY        */
	case 0x9d: SBC(LY); break;					/* SBC  A,LY        */
	case 0x9e: EAY(); SBC(RM8(ea)); break;				/* SBC  A,(IY+o)    */
	case 0xa4: AND(HY); break;					/* AND  HY          */
	case 0xa5: AND(LY); break;					/* AND  LY          */
	case 0xa6: EAY(); AND(RM8(ea)); break;				/* AND  (IY+o)      */
	case 0xac: XOR(HY); break;					/* XOR  HY          */
	case 0xad: XOR(LY); break;					/* XOR  LY          */
	case 0xae: EAY(); XOR(RM8(ea)); break;				/* XOR  (IY+o)      */
	case 0xb4: OR(HY); break;					/* OR   HY          */
	case 0xb5: OR(LY); break;					/* OR   LY          */
	case 0xb6: EAY(); OR(RM8(ea)); break;				/* OR   (IY+o)      */
	case 0xbc: CP(HY); break;					/* CP   HY          */
	case 0xbd: CP(LY); break;					/* CP   LY          */
	case 0xbe: EAY(); CP(RM8(ea)); break;				/* CP   (IY+o)      */
	case 0xcb: EAY(); OP_XY(FETCH8()); break;			/* **   FD CB xx    */
	case 0xe1: POP(iy); break;					/* POP  IY          */
	case 0xe3: EXSP(iy); break;					/* EX   (SP),IY     */
	case 0xe5: PUSH(iy); break;					/* PUSH IY          */
	case 0xe9: PC = IY; break;					/* JP   (IY)        */
	case 0xf9: SP = IY; break;					/* LD   SP,IY       */
	default:   OP(code); break;
	}
}

void Z80::OP_ED(uint8_t code)
{
	icount -= cc_ed[code];

	switch(code) {
	case 0x40: B = IN8(BC); F = (F & CF) | SZP[B]; break;			/* IN   B,(C)       */
	case 0x41: OUT8(BC, B); break;						/* OUT  (C),B       */
	case 0x42: SBC16(bc); break;						/* SBC  HL,BC       */
	case 0x43: ea = FETCH16(); WM16(ea, &bc); WZ = ea + 1; break;		/* LD   (w),BC      */
	case 0x44: NEG(); break;						/* NEG              */
	case 0x45: RETN(); break;						/* RETN             */
	case 0x46: im = 0; break;						/* im   0           */
	case 0x47: LD_I_A(); break;						/* LD   i,A         */
	case 0x48: C = IN8(BC); F = (F & CF) | SZP[C]; break;			/* IN   C,(C)       */
	case 0x49: OUT8(BC, C); break;						/* OUT  (C),C       */
	case 0x4a: ADC16(bc); break;						/* ADC  HL,BC       */
	case 0x4b: ea = FETCH16(); RM16(ea, &bc); WZ = ea + 1; break;		/* LD   BC,(w)      */
	case 0x4c: NEG(); break;						/* NEG              */
	case 0x4d: RETI(); break;						/* RETI             */
	case 0x4e: im = 0; break;						/* im   0           */
	case 0x4f: LD_R_A(); break;						/* LD   r,A         */
	case 0x50: D = IN8(BC); F = (F & CF) | SZP[D]; break;			/* IN   D,(C)       */
	case 0x51: OUT8(BC, D); break;						/* OUT  (C),D       */
	case 0x52: SBC16(de); break;						/* SBC  HL,DE       */
	case 0x53: ea = FETCH16(); WM16(ea, &de); WZ = ea + 1; break;		/* LD   (w),DE      */
	case 0x54: NEG(); break;						/* NEG              */
	case 0x55: RETN(); break;						/* RETN             */
	case 0x56: im = 1; break;						/* im   1           */
	case 0x57: LD_A_I(); break;						/* LD   A,i         */
	case 0x58: E = IN8(BC); F = (F & CF) | SZP[E]; break;			/* IN   E,(C)       */
	case 0x59: OUT8(BC, E); break;						/* OUT  (C),E       */
	case 0x5a: ADC16(de); break;						/* ADC  HL,DE       */
	case 0x5b: ea = FETCH16(); RM16(ea, &de); WZ = ea + 1; break;		/* LD   DE,(w)      */
	case 0x5c: NEG(); break;						/* NEG              */
	case 0x5d: RETI(); break;						/* RETI             */
	case 0x5e: im = 2; break;						/* im   2           */
	case 0x5f: LD_A_R(); break;						/* LD   A,r         */
	case 0x60: H = IN8(BC); F = (F & CF) | SZP[H]; break;			/* IN   H,(C)       */
	case 0x61: OUT8(BC, H); break;						/* OUT  (C),H       */
	case 0x62: SBC16(hl); break;						/* SBC  HL,HL       */
	case 0x63: ea = FETCH16(); WM16(ea, &hl); WZ = ea + 1; break;		/* LD   (w),HL      */
	case 0x64: NEG(); break;						/* NEG              */
	case 0x65: RETN(); break;						/* RETN             */
	case 0x66: im = 0; break;						/* im   0           */
	case 0x67: RRD(); break;						/* RRD  (HL)        */
	case 0x68: L = IN8(BC); F = (F & CF) | SZP[L]; break;			/* IN   L,(C)       */
	case 0x69: OUT8(BC, L); break;						/* OUT  (C),L       */
	case 0x6a: ADC16(hl); break;						/* ADC  HL,HL       */
	case 0x6b: ea = FETCH16(); RM16(ea, &hl); WZ = ea + 1; break;		/* LD   HL,(w)      */
	case 0x6c: NEG(); break;						/* NEG              */
	case 0x6d: RETI(); break;						/* RETI             */
	case 0x6e: im = 0; break;						/* im   0           */
	case 0x6f: RLD(); break;						/* RLD  (HL)        */
	case 0x70: {uint8_t res = IN8(BC); F = (F & CF) | SZP[res];} break;	/* IN   F,(C)       */
	case 0x71: OUT8(BC, 0); break;						/* OUT  (C),0       */
	case 0x72: SBC16(sp); break;						/* SBC  HL,SP       */
	case 0x73: ea = FETCH16(); WM16(ea, &sp); WZ = ea + 1; break;		/* LD   (w),SP      */
	case 0x74: NEG(); break;						/* NEG              */
	case 0x75: RETN(); break;						/* RETN             */
	case 0x76: im = 1; break;						/* im   1           */
	case 0x78: A = IN8(BC); F = (F & CF) | SZP[A]; WZ = BC + 1; break;	/* IN   A,(C)       */
	case 0x79: OUT8(BC, A); WZ = BC + 1; break;				/* OUT  (C),A       */
	case 0x7a: ADC16(sp); break;						/* ADC  HL,SP       */
	case 0x7b: ea = FETCH16(); RM16(ea, &sp); WZ = ea + 1; break;		/* LD   SP,(w)      */
	case 0x7c: NEG(); break;						/* NEG              */
	case 0x7d: RETI(); break;						/* RETI             */
	case 0x7e: im = 2; break;						/* im   2           */
	case 0xa0: LDI(); break;						/* LDI              */
	case 0xa1: CPI(); break;						/* CPI              */
	case 0xa2: INI(); break;						/* INI              */
	case 0xa3: OUTI(); break;						/* OUTI             */
	case 0xa8: LDD(); break;						/* LDD              */
	case 0xa9: CPD(); break;						/* CPD              */
	case 0xaa: IND(); break;						/* IND              */
	case 0xab: OUTD(); break;						/* OUTD             */
	case 0xb0: LDIR(); break;						/* LDIR             */
	case 0xb1: CPIR(); break;						/* CPIR             */
	case 0xb2: INIR(); break;						/* INIR             */
	case 0xb3: OTIR(); break;						/* OTIR             */
	case 0xb8: LDDR(); break;						/* LDDR             */
	case 0xb9: CPDR(); break;						/* CPDR             */
	case 0xba: INDR(); break;						/* INDR             */
	case 0xbb: OTDR(); break;						/* OTDR             */
	default:   OP(code); break;
	}
}

void Z80::OP(uint8_t code)
{
	prevpc = PC - 1;
	icount -= cc_op[code];

	switch(code) {
	case 0x00: break;												/* NOP              */
	case 0x01: BC = FETCH16(); break;										/* LD   BC,w        */
	case 0x02: WM8(BC, A); WZ_L = (BC + 1) & 0xff; WZ_H = A; break;							/* LD (BC),A        */
	case 0x03: BC++; break;												/* INC  BC          */
	case 0x04: B = INC(B); break;											/* INC  B           */
	case 0x05: B = DEC(B); break;											/* DEC  B           */
	case 0x06: B = FETCH8(); break;											/* LD   B,n         */
	case 0x07: RLCA(); break;											/* RLCA             */
	case 0x08: EX_AF(); break;											/* EX   AF,AF'      */
	case 0x09: ADD16(hl, bc); break;										/* ADD  HL,BC       */
	case 0x0a: A = RM8(BC); WZ = BC+1; break;									/* LD   A,(BC)      */
	case 0x0b: BC--; break;												/* DEC  BC          */
	case 0x0c: C = INC(C); break;											/* INC  C           */
	case 0x0d: C = DEC(C); break;											/* DEC  C           */
	case 0x0e: C = FETCH8(); break;											/* LD   C,n         */
	case 0x0f: RRCA(); break;											/* RRCA             */
	case 0x10: B--; JR_COND(B, 0x10); break;									/* DJNZ o           */
	case 0x11: DE = FETCH16(); break;										/* LD   DE,w        */
	case 0x12: WM8(DE, A); WZ_L = (DE + 1) & 0xff; WZ_H = A; break;							/* LD (DE),A        */
	case 0x13: DE++; break;												/* INC  DE          */
	case 0x14: D = INC(D); break;											/* INC  D           */
	case 0x15: D = DEC(D); break;											/* DEC  D           */
	case 0x16: D = FETCH8(); break;											/* LD   D,n         */
	case 0x17: RLA(); break;											/* RLA              */
	case 0x18: JR(); break;												/* JR   o           */
	case 0x19: ADD16(hl, de); break;										/* ADD  HL,DE       */
	case 0x1a: A = RM8(DE); WZ = DE + 1; break;									/* LD   A,(DE)      */
	case 0x1b: DE--; break;												/* DEC  DE          */
	case 0x1c: E = INC(E); break;											/* INC  E           */
	case 0x1d: E = DEC(E); break;											/* DEC  E           */
	case 0x1e: E = FETCH8(); break;											/* LD   E,n         */
	case 0x1f: RRA(); break;											/* RRA              */
	case 0x20: JR_COND(!(F & ZF), 0x20); break;									/* JR   NZ,o        */
	case 0x21: HL = FETCH16(); break;										/* LD   HL,w        */
	case 0x22: ea = FETCH16(); WM16(ea, &hl); WZ = ea + 1; break;							/* LD   (w),HL      */
	case 0x23: HL++; break;												/* INC  HL          */
	case 0x24: H = INC(H); break;											/* INC  H           */
	case 0x25: H = DEC(H); break;											/* DEC  H           */
	case 0x26: H = FETCH8(); break;											/* LD   H,n         */
	case 0x27: DAA(); break;											/* DAA              */
	case 0x28: JR_COND(F & ZF, 0x28); break;									/* JR   Z,o         */
	case 0x29: ADD16(hl, hl); break;										/* ADD  HL,HL       */
	case 0x2a: ea = FETCH16(); RM16(ea, &hl); WZ = ea + 1; break;							/* LD   HL,(w)      */
	case 0x2b: HL--; break;												/* DEC  HL          */
	case 0x2c: L = INC(L); break;											/* INC  L           */
	case 0x2d: L = DEC(L); break;											/* DEC  L           */
	case 0x2e: L = FETCH8(); break;											/* LD   L,n         */
	case 0x2f: A ^= 0xff; F = (F & (SF | ZF | PF | CF)) | HF | NF | (A & (YF | XF)); break;				/* CPL              */
	case 0x30: JR_COND(!(F & CF), 0x30); break;									/* JR   NC,o        */
	case 0x31: SP = FETCH16(); break;										/* LD   SP,w        */
	case 0x32: ea = FETCH16(); WM8(ea, A); WZ_L = (ea + 1) & 0xff; WZ_H = A; break;					/* LD   (w),A       */
	case 0x33: SP++; break;												/* INC  SP          */
	case 0x34: WM8(HL, INC(RM8(HL))); break;									/* INC  (HL)        */
	case 0x35: WM8(HL, DEC(RM8(HL))); break;									/* DEC  (HL)        */
	case 0x36: WM8(HL, FETCH8()); break;										/* LD   (HL),n      */
	case 0x37: F = (F & (SF | ZF | YF | XF | PF)) | CF | (A & (YF | XF)); break;					/* SCF              */
	case 0x38: JR_COND(F & CF, 0x38); break;									/* JR   C,o         */
	case 0x39: ADD16(hl, sp); break;										/* ADD  HL,SP       */
	case 0x3a: ea = FETCH16(); A = RM8(ea); WZ = ea + 1; break;							/* LD   A,(w)       */
	case 0x3b: SP--; break;												/* DEC  SP          */
	case 0x3c: A = INC(A); break;											/* INC  A           */
	case 0x3d: A = DEC(A); break;											/* DEC  A           */
	case 0x3e: A = FETCH8(); break;											/* LD   A,n         */
	case 0x3f: F = ((F & (SF | ZF | YF | XF | PF | CF)) | ((F & CF) << 4) | (A & (YF | XF))) ^ CF; break;		/* CCF              */
	case 0x40: break;												/* LD   B,B         */
	case 0x41: B = C; break;											/* LD   B,C         */
	case 0x42: B = D; break;											/* LD   B,D         */
	case 0x43: B = E; break;											/* LD   B,E         */
	case 0x44: B = H; break;											/* LD   B,H         */
	case 0x45: B = L; break;											/* LD   B,L         */
	case 0x46: B = RM8(HL); break;											/* LD   B,(HL)      */
	case 0x47: B = A; break;											/* LD   B,A         */
	case 0x48: C = B; break;											/* LD   C,B         */
	case 0x49: break;												/* LD   C,C         */
	case 0x4a: C = D; break;											/* LD   C,D         */
	case 0x4b: C = E; break;											/* LD   C,E         */
	case 0x4c: C = H; break;											/* LD   C,H         */
	case 0x4d: C = L; break;											/* LD   C,L         */
	case 0x4e: C = RM8(HL); break;											/* LD   C,(HL)      */
	case 0x4f: C = A; break;											/* LD   C,A         */
	case 0x50: D = B; break;											/* LD   D,B         */
	case 0x51: D = C; break;											/* LD   D,C         */
	case 0x52: break;												/* LD   D,D         */
	case 0x53: D = E; break;											/* LD   D,E         */
	case 0x54: D = H; break;											/* LD   D,H         */
	case 0x55: D = L; break;											/* LD   D,L         */
	case 0x56: D = RM8(HL); break;											/* LD   D,(HL)      */
	case 0x57: D = A; break;											/* LD   D,A         */
	case 0x58: E = B; break;											/* LD   E,B         */
	case 0x59: E = C; break;											/* LD   E,C         */
	case 0x5a: E = D; break;											/* LD   E,D         */
	case 0x5b: break;												/* LD   E,E         */
	case 0x5c: E = H; break;											/* LD   E,H         */
	case 0x5d: E = L; break;											/* LD   E,L         */
	case 0x5e: E = RM8(HL); break;											/* LD   E,(HL)      */
	case 0x5f: E = A; break;											/* LD   E,A         */
	case 0x60: H = B; break;											/* LD   H,B         */
	case 0x61: H = C; break;											/* LD   H,C         */
	case 0x62: H = D; break;											/* LD   H,D         */
	case 0x63: H = E; break;											/* LD   H,E         */
	case 0x64: break;												/* LD   H,H         */
	case 0x65: H = L; break;											/* LD   H,L         */
	case 0x66: H = RM8(HL); break;											/* LD   H,(HL)      */
	case 0x67: H = A; break;											/* LD   H,A         */
	case 0x68: L = B; break;											/* LD   L,B         */
	case 0x69: L = C; break;											/* LD   L,C         */
	case 0x6a: L = D; break;											/* LD   L,D         */
	case 0x6b: L = E; break;											/* LD   L,E         */
	case 0x6c: L = H; break;											/* LD   L,H         */
	case 0x6d: break;												/* LD   L,L         */
	case 0x6e: L = RM8(HL); break;											/* LD   L,(HL)      */
	case 0x6f: L = A; break;											/* LD   L,A         */
	case 0x70: WM8(HL, B); break;											/* LD   (HL),B      */
	case 0x71: WM8(HL, C); break;											/* LD   (HL),C      */
	case 0x72: WM8(HL, D); break;											/* LD   (HL),D      */
	case 0x73: WM8(HL, E); break;											/* LD   (HL),E      */
	case 0x74: WM8(HL, H); break;											/* LD   (HL),H      */
	case 0x75: WM8(HL, L); break;											/* LD   (HL),L      */
	case 0x76: ENTER_HALT(); break;											/* halt             */
	case 0x77: WM8(HL, A); break;											/* LD   (HL),A      */
	case 0x78: A = B; break;											/* LD   A,B         */
	case 0x79: A = C; break;											/* LD   A,C         */
	case 0x7a: A = D; break;											/* LD   A,D         */
	case 0x7b: A = E; break;											/* LD   A,E         */
	case 0x7c: A = H; break;											/* LD   A,H         */
	case 0x7d: A = L; break;											/* LD   A,L         */
	case 0x7e: A = RM8(HL); break;											/* LD   A,(HL)      */
	case 0x7f: break;												/* LD   A,A         */
	case 0x80: ADD(B); break;											/* ADD  A,B         */
	case 0x81: ADD(C); break;											/* ADD  A,C         */
	case 0x82: ADD(D); break;											/* ADD  A,D         */
	case 0x83: ADD(E); break;											/* ADD  A,E         */
	case 0x84: ADD(H); break;											/* ADD  A,H         */
	case 0x85: ADD(L); break;											/* ADD  A,L         */
	case 0x86: ADD(RM8(HL)); break;											/* ADD  A,(HL)      */
	case 0x87: ADD(A); break;											/* ADD  A,A         */
	case 0x88: ADC(B); break;											/* ADC  A,B         */
	case 0x89: ADC(C); break;											/* ADC  A,C         */
	case 0x8a: ADC(D); break;											/* ADC  A,D         */
	case 0x8b: ADC(E); break;											/* ADC  A,E         */
	case 0x8c: ADC(H); break;											/* ADC  A,H         */
	case 0x8d: ADC(L); break;											/* ADC  A,L         */
	case 0x8e: ADC(RM8(HL)); break;											/* ADC  A,(HL)      */
	case 0x8f: ADC(A); break;											/* ADC  A,A         */
	case 0x90: SUB(B); break;											/* SUB  B           */
	case 0x91: SUB(C); break;											/* SUB  C           */
	case 0x92: SUB(D); break;											/* SUB  D           */
	case 0x93: SUB(E); break;											/* SUB  E           */
	case 0x94: SUB(H); break;											/* SUB  H           */
	case 0x95: SUB(L); break;											/* SUB  L           */
	case 0x96: SUB(RM8(HL)); break;											/* SUB  (HL)        */
	case 0x97: SUB(A); break;											/* SUB  A           */
	case 0x98: SBC(B); break;											/* SBC  A,B         */
	case 0x99: SBC(C); break;											/* SBC  A,C         */
	case 0x9a: SBC(D); break;											/* SBC  A,D         */
	case 0x9b: SBC(E); break;											/* SBC  A,E         */
	case 0x9c: SBC(H); break;											/* SBC  A,H         */
	case 0x9d: SBC(L); break;											/* SBC  A,L         */
	case 0x9e: SBC(RM8(HL)); break;											/* SBC  A,(HL)      */
	case 0x9f: SBC(A); break;											/* SBC  A,A         */
	case 0xa0: AND(B); break;											/* AND  B           */
	case 0xa1: AND(C); break;											/* AND  C           */
	case 0xa2: AND(D); break;											/* AND  D           */
	case 0xa3: AND(E); break;											/* AND  E           */
	case 0xa4: AND(H); break;											/* AND  H           */
	case 0xa5: AND(L); break;											/* AND  L           */
	case 0xa6: AND(RM8(HL)); break;											/* AND  (HL)        */
	case 0xa7: AND(A); break;											/* AND  A           */
	case 0xa8: XOR(B); break;											/* XOR  B           */
	case 0xa9: XOR(C); break;											/* XOR  C           */
	case 0xaa: XOR(D); break;											/* XOR  D           */
	case 0xab: XOR(E); break;											/* XOR  E           */
	case 0xac: XOR(H); break;											/* XOR  H           */
	case 0xad: XOR(L); break;											/* XOR  L           */
	case 0xae: XOR(RM8(HL)); break;											/* XOR  (HL)        */
	case 0xaf: XOR(A); break;											/* XOR  A           */
	case 0xb0: OR(B); break;											/* OR   B           */
	case 0xb1: OR(C); break;											/* OR   C           */
	case 0xb2: OR(D); break;											/* OR   D           */
	case 0xb3: OR(E); break;											/* OR   E           */
	case 0xb4: OR(H); break;											/* OR   H           */
	case 0xb5: OR(L); break;											/* OR   L           */
	case 0xb6: OR(RM8(HL)); break;											/* OR   (HL)        */
	case 0xb7: OR(A); break;											/* OR   A           */
	case 0xb8: CP(B); break;											/* CP   B           */
	case 0xb9: CP(C); break;											/* CP   C           */
	case 0xba: CP(D); break;											/* CP   D           */
	case 0xbb: CP(E); break;											/* CP   E           */
	case 0xbc: CP(H); break;											/* CP   H           */
	case 0xbd: CP(L); break;											/* CP   L           */
	case 0xbe: CP(RM8(HL)); break;											/* CP   (HL)        */
	case 0xbf: CP(A); break;											/* CP   A           */
	case 0xc0: RET_COND(!(F & ZF), 0xc0); break;									/* RET  NZ          */
	case 0xc1: POP(bc); break;											/* POP  BC          */
	case 0xc2: JP_COND(!(F & ZF)); break;										/* JP   NZ,a        */
	case 0xc3: JP(); break;												/* JP   a           */
	case 0xc4: CALL_COND(!(F & ZF), 0xc4); break;									/* CALL NZ,a        */
	case 0xc5: PUSH(bc); break;											/* PUSH BC          */
	case 0xc6: ADD(FETCH8()); break;										/* ADD  A,n         */
	case 0xc7: RST(0x00); break;											/* RST  0           */
	case 0xc8: RET_COND(F & ZF, 0xc8); break;									/* RET  Z           */
#ifdef Z80_PSEUDO_BIOS
	case 0xc9:
		if(d_bios != NULL) {
			d_bios->bios_ret_z80(prevpc, &af, &bc, &de, &hl, &ix, &iy, &iff1);
		}
		SET_NPC; POP(pc); WZ = PCD; break;										/* RET              */
#else
	case 0xc9: SET_NPC; POP(pc); WZ = PCD; break;										/* RET              */
#endif
	case 0xca: JP_COND(F & ZF); break;										/* JP   Z,a         */
	case 0xcb: OP_CB(FETCHOP2()); break;										/* **** CB xx       */
	case 0xcc: CALL_COND(F & ZF, 0xcc); break;									/* CALL Z,a         */
	case 0xcd: CALL(); break;											/* CALL a           */
	case 0xce: ADC(FETCH8()); break;										/* ADC  A,n         */
	case 0xcf: RST(0x08); break;											/* RST  1           */
	case 0xd0: RET_COND(!(F & CF), 0xd0); break;									/* RET  NC          */
	case 0xd1: POP(de); break;											/* POP  DE          */
	case 0xd2: JP_COND(!(F & CF)); break;										/* JP   NC,a        */
	case 0xd3: {unsigned n = FETCH8() | (A << 8); OUT8(n, A); WZ_L = ((n & 0xff) + 1) & 0xff; WZ_H = A;} break;	/* OUT  (n),A       */
	case 0xd4: CALL_COND(!(F & CF), 0xd4); break;									/* CALL NC,a        */
	case 0xd5: PUSH(de); break;											/* PUSH DE          */
	case 0xd6: SUB(FETCH8()); break;										/* SUB  n           */
	case 0xd7: RST(0x10); break;											/* RST  2           */
	case 0xd8: RET_COND(F & CF, 0xd8); break;									/* RET  C           */
	case 0xd9: EXX(); break;											/* EXX              */
	case 0xda: JP_COND(F & CF); break;										/* JP   C,a         */
	case 0xdb: {unsigned n = FETCH8() | (A << 8); A = IN8(n); WZ = n + 1;} break;					/* IN   A,(n)       */
	case 0xdc: CALL_COND(F & CF, 0xdc); break;									/* CALL C,a         */
	case 0xdd: OP_DD(FETCHOP2()); break;										/* **** DD xx       */
	case 0xde: SBC(FETCH8()); break;										/* SBC  A,n         */
	case 0xdf: RST(0x18); break;											/* RST  3           */
	case 0xe0: RET_COND(!(F & PF), 0xe0); break;									/* RET  PO          */
	case 0xe1: POP(hl); break;											/* POP  HL          */
	case 0xe2: JP_COND(!(F & PF)); break;										/* JP   PO,a        */
	case 0xe3: EXSP(hl); break;											/* EX   HL,(SP)     */
	case 0xe4: CALL_COND(!(F & PF), 0xe4); break;									/* CALL PO,a        */
	case 0xe5: PUSH(hl); break;											/* PUSH HL          */
	case 0xe6: AND(FETCH8()); break;										/* AND  n           */
	case 0xe7: RST(0x20); break;											/* RST  4           */
	case 0xe8: RET_COND(F & PF, 0xe8); break;									/* RET  PE          */
	case 0xe9: PC = HL; break;											/* JP   (HL)        */
	case 0xea: JP_COND(F & PF); break;										/* JP   PE,a        */
	case 0xeb: EX_DE_HL(); break;											/* EX   DE,HL       */
	case 0xec: CALL_COND(F & PF, 0xec); break;									/* CALL PE,a        */
	case 0xed: OP_ED(FETCHOP2()); break;										/* **** ED xx       */
	case 0xee: XOR(FETCH8()); break;										/* XOR  n           */
	case 0xef: RST(0x28); break;											/* RST  5           */
	case 0xf0: RET_COND(!(F & SF), 0xf0); break;									/* RET  P           */
	case 0xf1: POP(af); break;											/* POP  AF          */
	case 0xf2: JP_COND(!(F & SF)); break;										/* JP   P,a         */
	case 0xf3: DI(); break;													/* DI               */
	case 0xf4: CALL_COND(!(F & SF), 0xf4); break;									/* CALL P,a         */
	case 0xf5: PUSH(af); break;											/* PUSH AF          */
	case 0xf6: OR(FETCH8()); break;											/* OR   n           */
	case 0xf7: RST(0x30); break;											/* RST  6           */
	case 0xf8: RET_COND(F & SF, 0xf8); break;									/* RET  M           */
	case 0xf9: SP = HL; break;											/* LD   SP,HL       */
	case 0xfa: JP_COND(F & SF); break;										/* JP   M,a         */
	case 0xfb: EI(); break;												/* EI               */
	case 0xfc: CALL_COND(F & SF, 0xfc); break;									/* CALL M,a         */
	case 0xfd: OP_FD(FETCHOP2()); break;										/* **** FD xx       */
	case 0xfe: CP(FETCH8()); break;											/* CP   n           */
	case 0xff: RST(0x38); break;											/* RST  7           */
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
	default: __assume(0);
#endif
	}
}

// main

Z80::Z80(VM* parent_vm, EMU* parent_emu, const char* identifier)
	: DEVICE(parent_vm, parent_emu, identifier)
{
#ifdef USE_DEBUGGER
	total_icount = 0;
	npc = 0;
#endif
#ifdef USE_Z80_DEBUGGER
	prev_total_icount = 0;
#endif
	busreq = busack = false;
	int_state = 0;
	cpu_clock = 1;

#ifdef Z80_PSEUDO_BIOS
	d_bios = NULL;
#endif
#ifdef SINGLE_MODE_DMA
	d_dma = NULL;
#endif
#ifdef Z80_NOTIFY_INTR
	d_pic = NULL;
#endif
	init_output_signals(&outputs_busack);
	set_class_name("Z80CPU");
}

void Z80::initialize()
{
	if(!flags_initialized) {
		uint8_t *padd = SZHVC_add;
		uint8_t *padc = SZHVC_add + 256 * 256;
		uint8_t *psub = SZHVC_sub;
		uint8_t *psbc = SZHVC_sub + 256 * 256;

		for(int oldval = 0; oldval < 256; oldval++) {
			for(int newval = 0; newval < 256; newval++) {
				/* add or adc w/o carry set */
				int val = newval - oldval;
				*padd = (newval) ? ((newval & 0x80) ? SF : 0) : ZF;
				*padd |= (newval & (YF | XF));	/* undocumented flag bits 5+3 */
				if((newval & 0x0f) < (oldval & 0x0f)) *padd |= HF;
				if(newval < oldval) *padd |= CF;
				if((val ^ oldval ^ 0x80) & (val ^ newval) & 0x80) *padd |= VF;
				padd++;

				/* adc with carry set */
				val = newval - oldval - 1;
				*padc = (newval) ? ((newval & 0x80) ? SF : 0) : ZF;
				*padc |= (newval & (YF | XF));	/* undocumented flag bits 5+3 */
				if((newval & 0x0f) <= (oldval & 0x0f)) *padc |= HF;
				if(newval <= oldval) *padc |= CF;
				if((val ^ oldval ^ 0x80) & (val ^ newval) & 0x80) *padc |= VF;
				padc++;

				/* cp, sub or sbc w/o carry set */
				val = oldval - newval;
				*psub = NF | ((newval) ? ((newval & 0x80) ? SF : 0) : ZF);
				*psub |= (newval & (YF | XF));	/* undocumented flag bits 5+3 */
				if((newval & 0x0f) > (oldval & 0x0f)) *psub |= HF;
				if(newval > oldval) *psub |= CF;
				if((val ^ oldval) & (oldval ^ newval) & 0x80) *psub |= VF;
				psub++;

				/* sbc with carry set */
				val = oldval - newval - 1;
				*psbc = NF | ((newval) ? ((newval & 0x80) ? SF : 0) : ZF);
				*psbc |= (newval & (YF | XF));	/* undocumented flag bits 5+3 */
				if((newval & 0x0f) >= (oldval & 0x0f)) *psbc |= HF;
				if(newval >= oldval) *psbc |= CF;
				if((val ^ oldval) & (oldval ^ newval) & 0x80) *psbc |= VF;
				psbc++;
			}
		}
		for(int i = 0; i < 256; i++) {
			int p = 0;
			if(i & 0x01) ++p;
			if(i & 0x02) ++p;
			if(i & 0x04) ++p;
			if(i & 0x08) ++p;
			if(i & 0x10) ++p;
			if(i & 0x20) ++p;
			if(i & 0x40) ++p;
			if(i & 0x80) ++p;
			SZ[i] = i ? i & SF : ZF;
			SZ[i] |= (i & (YF | XF));	/* undocumented flag bits 5+3 */
			SZ_BIT[i] = i ? i & SF : ZF | PF;
			SZ_BIT[i] |= (i & (YF | XF));	/* undocumented flag bits 5+3 */
			SZP[i] = SZ[i] | ((p & 1) ? 0 : PF);
			SZHV_inc[i] = SZ[i];
			if(i == 0x80) SZHV_inc[i] |= VF;
			if((i & 0x0f) == 0x00) SZHV_inc[i] |= HF;
			SZHV_dec[i] = SZ[i] | NF;
			if(i == 0x7f) SZHV_dec[i] |= VF;
			if((i & 0x0f) == 0x0f) SZHV_dec[i] |= HF;
		}
		flags_initialized = true;
	}

#ifdef USE_DEBUGGER
	d_mem_stored = d_mem;
	d_io_stored = d_io;
	d_debugger->set_name(_T("Z80"));
	d_debugger->rerate_to_cpu(true);
	d_debugger->set_context_mem(d_mem);
	d_debugger->set_context_io(d_io);
	dasm.set_debugger(d_debugger);
#endif
}

void Z80::reset()
{
	PCD = CPU_START_ADDR;
	SPD = 0;
	AFD = BCD = DED = HLD = 0;
	IXD = IYD = 0xffff;	/* IX and IY are FFFF after a reset! */
	F = ZF;			/* Zero flag is set */
	I = R = R2 = 0;
	WZD = PCD;
	af2.d = bc2.d = de2.d = hl2.d = 0;
	ea = 0;

	im = iff1 = iff2 = icr = 0;
	after_halt = false;
	after_ei = after_ldair = false;
	intr_req_bit = intr_pend_bit = 0;

	icount = extra_icount = 0;
}

void Z80::write_signal(int id, uint32_t data, uint32_t mask)
{
#ifdef USE_DEBUGGER
	uint32_t int_state_prev = int_state;
#endif
	if(id == SIG_CPU_IRQ) {
		intr_req_bit = (intr_req_bit & ~mask) | (data & mask);
		// always pending (temporary)
		intr_pend_bit = (intr_pend_bit & ~mask) | (0xffffffff & mask);
		int_state = (data & mask) ? int_state | Z80_STATE_INTR : int_state & ~Z80_STATE_INTR;
	} else if(id == SIG_CPU_NMI) {
		intr_req_bit = (data & mask) ? (intr_req_bit | NMI_REQ_BIT) : (intr_req_bit & ~NMI_REQ_BIT);
		int_state = (data & mask) ? int_state | Z80_STATE_NMI : int_state & ~Z80_STATE_NMI;
	} else if(id == SIG_CPU_BUSREQ) {
		busreq = ((data & mask) != 0);
		int_state = busreq ? int_state | Z80_STATE_BUSREQ : int_state & ~Z80_STATE_BUSREQ;
//		write_signals(&outputs_busack, busreq ? 0xffffffff : 0);
#ifdef HAS_NSC800
	} else if(id == SIG_NSC800_INT) {
		intr_req_bit = (data & mask) ? (intr_req_bit | 1) : (intr_req_bit & ~1);
	} else if(id == SIG_NSC800_RSTA) {
		intr_req_bit = (data & mask) ? (intr_req_bit | 8) : (intr_req_bit & ~8);
	} else if(id == SIG_NSC800_RSTB) {
		intr_req_bit = (data & mask) ? (intr_req_bit | 4) : (intr_req_bit & ~4);
	} else if(id == SIG_NSC800_RSTC) {
		intr_req_bit = (data & mask) ? (intr_req_bit | 2) : (intr_req_bit & ~2);
#endif
	} else if (id == SIG_CPU_RESET) {
		if (data & mask) {
			now_reset = true;
		} else {
			if (now_reset) {
				reset();
			}
			now_reset = false;
		}
		int_state = now_reset ? int_state | Z80_STATE_RESET : int_state & ~Z80_STATE_RESET;
	}

#ifdef USE_DEBUGGER
	dasm.set_signal(int_state);

	if(d_debugger->now_debugging() && int_state_prev != int_state) {
		d_debugger->check_intr_break_points((uint32_t)id, (data & mask) ? 0 : 1);
	}
#endif
}

uint32_t Z80::read_signal(int id)
{
	if(id == SIG_CPU_IRQ) {
		return intr_req_bit;
	}
	return 0;
}

/// @param[in] clock : -1: process one code / >0: continue processing codes until spend given clock
/// @return : spent clock for processing code
int Z80::run(int clock)
{
	if(clock == -1) {
		if (busreq != busack) {
			busack = busreq;
			write_signals(&outputs_busack, busack ? 0xffffffff : 0);
			int_state = busack ? int_state | Z80_STATE_BUSACK : int_state & ~Z80_STATE_BUSACK;
#ifdef USE_DEBUGGER
			dasm.set_signal(int_state);
#endif
		}
		if(busreq || now_reset) {
			// run dma once
			#ifdef SINGLE_MODE_DMA
				if(d_dma) {
					d_dma->do_dma();
				}
			#endif
			// don't run cpu!
			int passed_icount = MAX(1, extra_icount);
			// this is main cpu, icount is not used
			/*icount = */extra_icount = 0;
			#ifdef USE_DEBUGGER
				total_icount += passed_icount;
			#endif
			return passed_icount;
		} else {
			// run only one opcode
			#ifdef USE_DEBUGGER
				total_icount += extra_icount;
			#endif
			icount = -extra_icount;
			extra_icount = 0;
			run_one_opecode();
			return -icount;
		}
	} else {
		icount += clock;
		int first_icount = icount;
		#ifdef USE_DEBUGGER
			total_icount += extra_icount;
		#endif
		icount -= extra_icount;
		extra_icount = 0;

		if (icount > 0 && busreq != busack) {
			busack = busreq;
			write_signals(&outputs_busack, busack ? 0xffffffff : 0);
			int_state = busack ? int_state | Z80_STATE_BUSACK : int_state & ~Z80_STATE_BUSACK;
#ifdef USE_DEBUGGER
			dasm.set_signal(int_state);
#endif
		}
		if(busreq || now_reset) {
			// run dma once
			#ifdef SINGLE_MODE_DMA
				if(d_dma) {
					d_dma->do_dma();
				}
			#endif
		} else {
			// run cpu while given clocks
			while(icount > 0) {
#ifndef USE_DEBUGGER
				run_one_opecode();
#else
				if (!run_one_opecode()) return 0;
#endif
			}
		}
		// if busreq is raised, spin cpu while remained clock
		if(icount > 0 && (busreq || now_reset)) {
			#ifdef USE_DEBUGGER
				total_icount += icount;
			#endif
			icount = 0;
		}
		return first_icount - icount;
	}
}

bool Z80::run_one_opecode()
{
	// rune one opecode
#if defined(USE_DEBUGGER)
	bool now_debugging = d_debugger->now_debugging();
	if(now_debugging) {
#if defined(USE_SUSPEND_IN_CPU)
		d_debugger->check_break_points(PC);
		if(d_debugger->now_suspended()) {
			d_debugger->now_waiting(true);
			emu->start_waiting_in_debugger();
			while(d_debugger->now_suspend()) {
				emu->process_waiting_in_debugger();
			}
			emu->finish_waiting_in_debugger();
			d_debugger->now_waiting(false);
		}
#endif
		if(d_debugger->now_debugging()) {
			d_mem = d_io = d_debugger;
			if (d_debugger->now_suspended()) {
				return false;
			}
		} else {
			now_debugging = false;
		}

		after_halt = after_ei = false;
#if HAS_LDAIR_QUIRK
		after_ldair = false;
#endif
#ifdef USE_Z80_DEBUGGER
		d_debugger->add_cpu_trace(PC);
#endif
		int first_icount = icount;
		OP(FETCHOP1());
		icount -= extra_icount;
		extra_icount = 0;
#ifdef USE_DEBUGGER
		total_icount += first_icount - icount;
#endif
#if HAS_LDAIR_QUIRK
		if(after_ldair) {
			F &= ~PF;	// reset parity flag after LD A,I or LD A,R
		}
#endif
#ifdef SINGLE_MODE_DMA
		if(d_dma) {
			d_dma->do_dma();
		}
#endif
		dasm.set_regs(0, first_icount - icount, af.d, bc.d, de.d, hl.d, af2.d, bc2.d, de2.d, hl2.d, ix.d, iy.d, sp.d, I, R);
		if(!after_ei) {
			check_interrupt();
		}
		if(now_debugging) {
//			if(!d_debugger->now_going()) {
//				d_debugger->now_suspended(true);
//			}
			d_mem = d_mem_stored;
			d_io = d_io_stored;
		}
	} else {
#endif /* USE_DEBUGGER */

		after_halt = after_ei = false;
#if HAS_LDAIR_QUIRK
		after_ldair = false;
#endif
#ifdef USE_Z80_DEBUGGER
		d_debugger->add_cpu_trace(PC);
#endif
#ifdef USE_DEBUGGER
		int first_icount = icount;
#endif
		OP(FETCHOP1());
#ifdef USE_DEBUGGER
		icount -= extra_icount;
		extra_icount = 0;
		total_icount += first_icount - icount;
#endif
#if HAS_LDAIR_QUIRK
		if(after_ldair) {
			F &= ~PF;	// reset parity flag after LD A,I or LD A,R
		}
#endif
#ifdef SINGLE_MODE_DMA
		if(d_dma) {
			d_dma->do_dma();
		}
#endif
#ifdef USE_DEBUGGER
		dasm.set_regs(0, first_icount - icount, af.d, bc.d, de.d, hl.d, af2.d, bc2.d, de2.d, hl2.d, ix.d, iy.d, sp.d, I, R);
#endif
		if(!after_ei) {
			check_interrupt();
		}
#ifdef USE_DEBUGGER
	}
#endif

	// ei: run next opecode
	if(after_ei) {
#if defined(USE_DEBUGGER)
		bool now_debugging = d_debugger->now_debugging();
		if(now_debugging) {
#if defined(USE_SUSPEND_IN_CPU)
			d_debugger->check_break_points(PC);
			if(d_debugger->now_suspended()) {
				d_debugger->now_waiting(true);
				emu->start_waiting_in_debugger();
				while(d_debugger->now_suspend()) {
					emu->process_waiting_in_debugger();
				}
				emu->finish_waiting_in_debugger();
				d_debugger->now_waiting(false);
			}
#endif
			if(d_debugger->now_debugging()) {
				d_mem = d_io = d_debugger;
			} else {
				now_debugging = false;
			}

			after_halt = false;
#if HAS_LDAIR_QUIRK
			after_ldair = false;
#endif
#ifdef USE_Z80_DEBUGGER
			d_debugger->add_cpu_trace(PC);
#endif
			int first_icount = icount;
			OP(FETCHOP1());
			icount -= extra_icount;
			extra_icount = 0;
#ifdef USE_DEBUGGER
			total_icount += first_icount - icount;
#endif
#if HAS_LDAIR_QUIRK
			if(after_ldair) {
				F &= ~PF;	// reset parity flag after LD A,I or LD A,R
			}
#endif
#ifdef SINGLE_MODE_DMA
			if(d_dma) {
				d_dma->do_dma();
			}
#endif
			dasm.set_regs(0, first_icount - icount, af.d, bc.d, de.d, hl.d, af2.d, bc2.d, de2.d, hl2.d, ix.d, iy.d, sp.d, I, R);
#ifdef Z80_NOTIFY_INTR
			if (d_pic) {
				d_pic->notify_intr_ei();
			}
#endif
			check_interrupt();

			if(now_debugging) {
//				if(!d_debugger->now_going()) {
//					d_debugger->now_suspended(true);
//				}
				d_mem = d_mem_stored;
				d_io = d_io_stored;
			}
		} else {
#endif /* USE_DEBUGGER */

			after_halt = false;
#if HAS_LDAIR_QUIRK
			after_ldair = false;
#endif
#ifdef USE_Z80_DEBUGGER
			d_debugger->add_cpu_trace(PC);
#endif
#ifdef USE_DEBUGGER
			int first_icount = icount;
#endif
			OP(FETCHOP1());
#ifdef USE_DEBUGGER
			icount -= extra_icount;
			extra_icount = 0;
			total_icount += first_icount - icount;
#endif
#if HAS_LDAIR_QUIRK
			if(after_ldair) {
				F &= ~PF;	// reset parity flag after LD A,I or LD A,R
			}
#endif
#ifdef SINGLE_MODE_DMA
			if(d_dma) {
				d_dma->do_dma();
			}
#endif
#ifdef USE_DEBUGGER
			dasm.set_regs(0, first_icount - icount, af.d, bc.d, de.d, hl.d, af2.d, bc2.d, de2.d, hl2.d, ix.d, iy.d, sp.d, I, R);
#endif
#ifdef Z80_NOTIFY_INTR
			if (d_pic) {
				d_pic->notify_intr_ei();
			}
#endif
			check_interrupt();
#ifdef USE_DEBUGGER
		}
#endif
	}
#ifndef USE_DEBUGGER
	icount -= extra_icount;
	extra_icount = 0;
#endif
	return true;
}

void Z80::check_interrupt()
{
#ifdef USE_DEBUGGER
	int first_icount = icount;
#endif
	// check interrupt
	if(intr_req_bit) {
		if(intr_req_bit & NMI_REQ_BIT) {
			// nmi
			LEAVE_HALT();
			PUSH(pc);
			SET_NPC;
			PCD = WZD = 0x0066;
			icount -= 11;
			iff1 = 0;
			intr_req_bit &= ~NMI_REQ_BIT;
			int_state &= ~Z80_STATE_NMI;
#ifdef HAS_NSC800
		} else if((intr_req_bit & 1) && (icr & 1)) {
			// INTR
			LEAVE_HALT();
			PUSH(pc);
			SET_NPC;
#ifdef Z80_NOTIFY_INTR
			PCD = WZ = (d_pic ? d_pic->get_intr_ack() & 0xffff : 0);
#else
			PCD = WZ = 0;
#endif
			icount -= cc_op[0xcd] + cc_ex[0xff];
			iff1 = iff2 = 0;
			intr_req_bit &= ~1;
		} else if((intr_req_bit & 8) && (icr & 8)) {
			// RSTA
			LEAVE_HALT();
			PUSH(pc);
			SET_NPC;
			PCD = WZ = 0x003c;
			icount -= cc_op[0xff] + cc_ex[0xff];
			iff1 = iff2 = 0;
			intr_req_bit &= ~8;
		} else if((intr_req_bit & 4) && (icr & 4)) {
			// RSTB
			LEAVE_HALT();
			PUSH(pc);
			SET_NPC;
			PCD = WZ = 0x0034;
			icount -= cc_op[0xff] + cc_ex[0xff];
			iff1 = iff2 = 0;
			intr_req_bit &= ~4;
		} else if((intr_req_bit & 2) && (icr & 2)) {
			// RSTC
			LEAVE_HALT();
			PUSH(pc);
			SET_NPC;
			PCD = WZ = 0x002c;
			icount -= cc_op[0xff] + cc_ex[0xff];
			iff1 = iff2 = 0;
			intr_req_bit &= ~2;
#else
		} else if(iff1) {
			// interrupt
			LEAVE_HALT();

#ifdef Z80_NOTIFY_INTR
			uint32_t vector = d_pic->get_intr_ack();
#else
			uint32_t vector = 0;
#endif
			if(im == 0) {
				// mode 0 (support NOP/JMP/CALL/RST only)
				switch(vector & 0xff) {
				case 0x00: break;				// NOP
				case 0xc3: PCD = vector >> 8; break;		// JMP
				case 0xcd: PUSH(pc); SET_NPC; PCD = vector >> 8; break;	// CALL
				case 0xc7: PUSH(pc); SET_NPC; PCD = 0x0000; break;	// RST 00H
				case 0xcf: PUSH(pc); SET_NPC; PCD = 0x0008; break;	// RST 08H
				case 0xd7: PUSH(pc); SET_NPC; PCD = 0x0010; break;	// RST 10H
				case 0xdf: PUSH(pc); SET_NPC; PCD = 0x0018; break;	// RST 18H
				case 0xe7: PUSH(pc); SET_NPC; PCD = 0x0020; break;	// RST 20H
				case 0xef: PUSH(pc); SET_NPC; PCD = 0x0028; break;	// RST 28H
				case 0xf7: PUSH(pc); SET_NPC; PCD = 0x0030; break;	// RST 30H
				case 0xff: PUSH(pc); SET_NPC; PCD = 0x0038; break;	// RST 38H
				}
				icount -= cc_op[vector & 0xff] + cc_ex[0xff];
			} else if(im == 1) {
				// mode 1
				PUSH(pc);
				SET_NPC;
				PCD = 0x0038;
				icount -= cc_op[0xff] + cc_ex[0xff];
			} else {
				// mode 2
				PUSH(pc);
				SET_NPC;
				RM16((vector & 0xff) | (I << 8), &pc);
				icount -= cc_op[0xcd] + cc_ex[0xff];
			}
			iff1 = iff2 = 0;
			intr_req_bit = 0;
			WZ = PCD;
		} else {
			intr_req_bit &= intr_pend_bit;
#endif
		}
	}
#ifdef USE_DEBUGGER
	total_icount += first_icount - icount;
#endif
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
void Z80::debug_write_data8(int type, uint32_t addr, uint32_t data)
{
	d_mem_stored->debug_write_data8(type, addr, data);
}

uint32_t Z80::debug_read_data8(int type, uint32_t addr)
{
	return d_mem_stored->debug_read_data8(type, addr);
}

void Z80::debug_write_io8(uint32_t addr, uint32_t data)
{
	d_io_stored->debug_write_io8(addr, data);
}

uint32_t Z80::debug_read_io8(uint32_t addr)
{
	return d_io_stored->debug_read_io8(addr);
}

#define DEBUG_WRITE_REG(reg_name, reg_variable) \
	if(_tcsicmp(reg, _T(reg_name)) == 0) { \
		reg_variable = data; \
		return true; \
	}

bool Z80::debug_write_reg(const _TCHAR *reg, uint32_t data)
{
	DEBUG_WRITE_REG("PC", PC)
	DEBUG_WRITE_REG("SP", SP)
	DEBUG_WRITE_REG("AF", AF)
	DEBUG_WRITE_REG("BC", BC)
	DEBUG_WRITE_REG("DE", DE)
	DEBUG_WRITE_REG("HL", HL)
	DEBUG_WRITE_REG("IX", IX)
	DEBUG_WRITE_REG("IY", IY)
	DEBUG_WRITE_REG("A", A)
	DEBUG_WRITE_REG("F", F)
	DEBUG_WRITE_REG("B", B)
	DEBUG_WRITE_REG("C", C)
	DEBUG_WRITE_REG("D", D)
	DEBUG_WRITE_REG("E", E)
	DEBUG_WRITE_REG("H", H)
	DEBUG_WRITE_REG("L", L)
	DEBUG_WRITE_REG("HX", HX)
	DEBUG_WRITE_REG("XH", HX)
	DEBUG_WRITE_REG("IXH", HX)
	DEBUG_WRITE_REG("LX", LX)
	DEBUG_WRITE_REG("XL", LX)
	DEBUG_WRITE_REG("IXL", LX)
	DEBUG_WRITE_REG("HY", HY)
	DEBUG_WRITE_REG("YH", HY)
	DEBUG_WRITE_REG("IYH", HY)
	DEBUG_WRITE_REG("LX", LY)
	DEBUG_WRITE_REG("YL", LY)
	DEBUG_WRITE_REG("IYL", LY)
	DEBUG_WRITE_REG("I", I)
	DEBUG_WRITE_REG("R", R)
	DEBUG_WRITE_REG("AF'", AF2)
	DEBUG_WRITE_REG("BC'", BC2)
	DEBUG_WRITE_REG("DE'", DE2)
	DEBUG_WRITE_REG("HL'", HL2)
	DEBUG_WRITE_REG("A'", A2)
	DEBUG_WRITE_REG("F'", F2)
	DEBUG_WRITE_REG("B'", B2)
	DEBUG_WRITE_REG("C'", C2)
	DEBUG_WRITE_REG("D'", D2)
	DEBUG_WRITE_REG("E'", E2)
	DEBUG_WRITE_REG("H'", H2)
	DEBUG_WRITE_REG("L'", L2)
	return false;
}

void Z80::debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	dasm.print_regs_current(pc.d, af.d, bc.d, de.d, hl.d, af2.d, bc2.d, de2.d, hl2.d, ix.d, iy.d, sp.d, I, R, int_state);
	UTILITY::tcscpy(buffer, buffer_len, dasm.get_line());
}

#define SET_DEBUG_REG_PTR(reg_name, reg_variable) \
	if(_tcsicmp(reg, _T(reg_name)) == 0) { \
		UTILITY::tcscpy(reg, regsiz, _T(reg_name)); \
		regptr = reinterpret_cast<void *>(&reg_variable); \
		reglen = static_cast<int>(sizeof(reg_variable)); \
		return true; \
	}

bool Z80::get_debug_reg_ptr(_TCHAR *reg, size_t regsiz, void * &regptr, int &reglen)
{
	SET_DEBUG_REG_PTR("PC", PC)
	SET_DEBUG_REG_PTR("SP", SP)
	SET_DEBUG_REG_PTR("AF", AF)
	SET_DEBUG_REG_PTR("BC", BC)
	SET_DEBUG_REG_PTR("DE", DE)
	SET_DEBUG_REG_PTR("HL", HL)
	SET_DEBUG_REG_PTR("IX", IX)
	SET_DEBUG_REG_PTR("IY", IY)
	SET_DEBUG_REG_PTR("A", A)
	SET_DEBUG_REG_PTR("F", F)
	SET_DEBUG_REG_PTR("B", B)
	SET_DEBUG_REG_PTR("C", C)
	SET_DEBUG_REG_PTR("D", D)
	SET_DEBUG_REG_PTR("E", E)
	SET_DEBUG_REG_PTR("H", H)
	SET_DEBUG_REG_PTR("L", L)
	SET_DEBUG_REG_PTR("I", I)
	SET_DEBUG_REG_PTR("R", R)
	SET_DEBUG_REG_PTR("HX", HX)
	SET_DEBUG_REG_PTR("LX", LX)
	SET_DEBUG_REG_PTR("XH", HX)
	SET_DEBUG_REG_PTR("XL", LX)
	SET_DEBUG_REG_PTR("IXH", HX)
	SET_DEBUG_REG_PTR("IXL", LX)
	SET_DEBUG_REG_PTR("AF'", AF2)
	SET_DEBUG_REG_PTR("BC'", BC2)
	SET_DEBUG_REG_PTR("DE'", DE2)
	SET_DEBUG_REG_PTR("HL'", HL2)
	SET_DEBUG_REG_PTR("A'", A2)
	SET_DEBUG_REG_PTR("F'", F2)
	SET_DEBUG_REG_PTR("B'", B2)
	SET_DEBUG_REG_PTR("C'", C2)
	SET_DEBUG_REG_PTR("D'", D2)
	SET_DEBUG_REG_PTR("E'", E2)
	SET_DEBUG_REG_PTR("H'", H2)
	SET_DEBUG_REG_PTR("L'", L2)
	return false;
}

int Z80::debug_dasm(int type, uint32_t pc, _TCHAR *buffer, size_t buffer_len, int flags)
{
	int opspos;
	if ((flags & 1) == 0) {
		// next opcode
		opspos = dasm.print_dasm_preprocess(type, pc, flags);
		UTILITY::tcscpy(buffer, buffer_len, dasm.get_line());
	} else {
		// current opcode
		opspos = dasm.print_dasm_processed(pc & 0xffff);
		UTILITY::tcscpy(buffer, buffer_len, dasm.get_line());
	}
	return opspos;
}

int Z80::debug_dasm_label(int type, uint32_t pc, _TCHAR *buffer, size_t buffer_len)
{
	int len = dasm.print_dasm_label(type, pc);
	if (len) {
		UTILITY::tcscpy(buffer, buffer_len, dasm.get_line());
	}
	return len;
}

int Z80::debug_trace_back_regs(int index, _TCHAR *buffer, size_t buffer_len)
{
	int next = dasm.print_regs_traceback(index);
	if (next >= -1) {
		UTILITY::tcscpy(buffer, buffer_len, dasm.get_line());
	}
	return next;
}

int Z80::debug_trace_back(int index, _TCHAR *buffer, size_t buffer_len)
{
	int next = dasm.print_dasm_traceback(index);
	if (next >= -1) {
		UTILITY::tcscpy(buffer, buffer_len, dasm.get_line());
	}
	return next;
}

bool Z80::reach_break_point()
{
	return d_debugger->reach_break_point_at(PC);
}

//void Z80::go_suspend()
//{
//	d_debugger->go_suspend();
//}

//bool Z80::now_suspend()
//{
//	return d_debugger->now_suspend();
//}

uint32_t Z80::get_debug_pc(int type)
{
#ifdef _MBS1
	return type ? d_mem_stored->debug_latch_address(prevpc) : prevpc;
#else
	return prevpc;
#endif
}

uint32_t Z80::get_debug_next_pc(int type)
{
#ifdef _MBS1
	return type ? d_mem_stored->debug_latch_address(pc.w.l) : pc.w.l;
#else
	return pc.w.l;
#endif
}

uint32_t Z80::get_debug_branch_pc(int type)
{
	uint16_t addr = npc != prevpc ? npc : pc.w.l;
#ifdef _MBS1
	return type ? d_mem_stored->debug_latch_address(addr) : addr;
#else
	return addr;
#endif
}

uint64_t Z80::get_current_clock()
{
	return total_icount;
}

/// signal names
static struct {
	const _TCHAR *name;
	uint32_t        num;
} signal_names_map[] = {
	{ _T("RESET"), SIG_CPU_RESET },
	{ _T("NMI"), SIG_CPU_NMI },
	{ _T("INT"), SIG_CPU_IRQ },
	{ _T("BUSREQ"), SIG_CPU_BUSREQ },
	{ NULL, 0 }
};

bool Z80::get_debug_signal_name_index(const _TCHAR *param, uint32_t *num, uint32_t *mask, int *idx, const _TCHAR **name)
{
	int i = 0; 
	for(; signal_names_map[i].name != NULL; i++) {
		if (_tcsicmp(param, signal_names_map[i].name) == 0) {
			if (num) *num = signal_names_map[i].num;
			if (mask) *mask = d_debugger->get_stored_mask();
			if (idx) *idx = i;
			if (name) *name = signal_names_map[i].name;
			return true;
		}
	}
	return false;
}

bool Z80::get_debug_signal_name_index(uint32_t num, uint32_t *mask, int *idx, const _TCHAR **name)
{
	int i = 0; 
	for(; signal_names_map[i].name != NULL; i++) {
		if (signal_names_map[i].num == num) {
			if (mask) *mask = d_debugger->get_stored_mask();
			if (idx) *idx = i;
			if (name) *name = signal_names_map[i].name;
			return true;
		}
	}
	return false;
}

void Z80::get_debug_signal_names_str(_TCHAR *buffer, size_t buffer_len)
{
	int i = 0;
	buffer[0] = _T('\0');
	for(; signal_names_map[i].name != NULL; i++) {
		if (i > 0) UTILITY::tcscat(buffer, buffer_len, _T(","));
		UTILITY::tcscat(buffer, buffer_len, signal_names_map[i].name);
	}
}
#endif

// ----------------------------------------------------------------------------

void Z80::save_state(FILEIO *fio)
{
	struct vm_state_st vm_state;

	vm_state_ident.version = Uint16_LE(1);
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(vm_state));

	memset(&vm_state, 0, sizeof(vm_state));
#ifdef USE_DEBUGGER
	vm_state.total_icount = Uint64_LE(total_icount);
#endif
	vm_state.icount = Int32_LE(icount);
	vm_state.extra_icount = Int32_LE(extra_icount);
	vm_state.prevpc = Uint16_LE(prevpc);
	vm_state.pc.d =	Uint32_LE(pc.d);
	vm_state.sp.d = Uint32_LE(sp.d);
	vm_state.af.d = Uint32_LE(af.d);
	vm_state.bc.d = Uint32_LE(bc.d);
	vm_state.de.d = Uint32_LE(de.d);
	vm_state.hl.d = Uint32_LE(hl.d);
	vm_state.ix.d = Uint32_LE(ix.d);
	vm_state.iy.d = Uint32_LE(iy.d);
	vm_state.wz.d = Uint32_LE(wz.d);
	vm_state.af2.d = Uint32_LE(af2.d);
	vm_state.bc2.d = Uint32_LE(bc2.d);
	vm_state.de2.d = Uint32_LE(de2.d);
	vm_state.hl2.d = Uint32_LE(hl2.d);
	vm_state.I = (I);
	vm_state.R = (R);
	vm_state.R2 = (R2);
	vm_state.now_reset = (now_reset ? 1 : 0) | (available ? 0 : 0x80);
	vm_state.ea = Uint32_LE(ea);
	vm_state.busreq = (busreq ? 1 : 0) | (busack ? 2 : 0);
	vm_state.after_halt = after_halt ? 1 : 0;
	vm_state.im = (im);
	vm_state.iff1 = (iff1);
	vm_state.iff2 = (iff2);
	vm_state.icr = (icr);
	vm_state.after_ei = after_ei ? 1 : 0;
	vm_state.after_ldair = after_ldair ? 1 : 0;
	vm_state.intr_req_bit = Uint32_LE(intr_req_bit);
	vm_state.intr_pend_bit = Uint32_LE(intr_pend_bit);

	fio->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);
	fio->Fwrite(&vm_state, sizeof(vm_state), 1);
}

bool Z80::load_state(FILEIO *fio)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	READ_STATE_CHUNK(fio, vm_state_i, vm_state);

#ifdef USE_DEBUGGER
	total_icount = Uint64_LE(vm_state.total_icount);
#endif
	icount = Int32_LE(vm_state.icount);
	extra_icount = Int32_LE(vm_state.extra_icount);
	prevpc = Uint32_LE(vm_state.prevpc);
	pc.d = Uint32_LE(vm_state.pc.d);
	sp.d = Uint32_LE(vm_state.sp.d);
	af.d = Uint32_LE(vm_state.af.d);
	bc.d = Uint32_LE(vm_state.bc.d);
	de.d = Uint32_LE(vm_state.de.d);
	hl.d = Uint32_LE(vm_state.hl.d);
	ix.d = Uint32_LE(vm_state.ix.d);
	iy.d = Uint32_LE(vm_state.iy.d);
	wz.d = Uint32_LE(vm_state.wz.d);
	af2.d = Uint32_LE(vm_state.af2.d);
	bc2.d = Uint32_LE(vm_state.bc2.d);
	de2.d = Uint32_LE(vm_state.de2.d);
	hl2.d = Uint32_LE(vm_state.hl2.d);
	I = vm_state.I;
	R = vm_state.R;
	R2 = vm_state.R2;
	now_reset = ((vm_state.now_reset & 1) != 0);
	available = ((vm_state.now_reset & 0x80) == 0);
	ea = Uint32_LE(vm_state.ea);
	busreq = ((vm_state.busreq & 1) != 0);
	busack = ((vm_state.busreq & 2) != 0);
	after_halt = (vm_state.after_halt != 0);
	im = vm_state.im;
	iff1 = vm_state.iff1;
	iff2 = vm_state.iff2;
	icr = vm_state.icr;
	after_ei = (vm_state.after_ei != 0);
	after_ldair = (vm_state.after_ldair != 0);
	intr_req_bit = Uint32_LE(vm_state.intr_req_bit);
	intr_pend_bit = Uint32_LE(vm_state.intr_pend_bit);

#ifdef USE_Z80_DEBUGGER
	// post process
	prev_total_icount = total_icount;
#endif

	return true;
}


