/** @file z80dasm.cpp

	Skelton for retropc emulator

	@par Origin
	MAME 0.145
	@author Takeda.Toshiya
	@date   2012.02.15-

	@note Modify for MB-S1 By Sasaji on 2019.10.21 -

	@brief [ Z80 disassembler ]
*/

#include "z80dasm.h"

#ifdef USE_DEBUGGER

#include "z80_consts.h"
#include "../utility.h"
#include "../fileio.h"
#include "debugger.h"


// FETCH8    : %02x : debug_fetch8()
// FETCHID8  : (%02x) : debug_fetch8()
// FETCHREL8 : (%d) : debug_fetch8_rel()
// FETCHREL88 : (%d), %02x : ofs = debug_fetch8_rel(); UTILITY::stprintf(buffer, buffer_len, _T("LD (IX+(%d)), %02x"), ofs, debug_fetch8()); break;
// FETCH16   : %s   : get_value_or_symbol(first_symbol, _T("%04x"), debug_fetch16())
// FETCHID16 : (%s) : get_value_or_symbol(first_symbol, _T("%04x"), debug_fetch16())
// JMPREL8   : %s   : get_value_or_symbol(first_symbol, _T("%04x"), debug_fetch8_relpc(pc))

// CODE_CB : dasm_cb(pc, buffer, buffer_len, first_symbol)
// CODE_DD : dasm_dd(pc, buffer, buffer_len, first_symbol)
// CODE_ED : dasm_ed(pc, buffer, buffer_len, first_symbol)
// CODE_FD : dasm_fd(pc, buffer, buffer_len, first_symbol)

static const Z80DASM::opcode_t opcodes0[] =
{
{	0x00, _T("NOP"), Z80DASM::NONE },
{	0x01, _T("LD   BC, %s"), Z80DASM::FETCH16 },
{	0x02, _T("LD  (BC), A"), Z80DASM::NONE },
{	0x03, _T("INC  BC"), Z80DASM::NONE },
{	0x04, _T("INC  B"), Z80DASM::NONE },
{	0x05, _T("DEC  B"), Z80DASM::NONE },
{	0x06, _T("LD   B, %s"), Z80DASM::FETCH8 },
{	0x07, _T("RLCA"), Z80DASM::NONE },
{	0x08, _T("EX   AF, AF'"), Z80DASM::NONE },
{	0x09, _T("ADD  HL, BC"), Z80DASM::NONE },
{	0x0a, _T("LD   A, (BC)"), Z80DASM::NONE },
{	0x0b, _T("DEC  BC"), Z80DASM::NONE },
{	0x0c, _T("INC  C"), Z80DASM::NONE },
{	0x0d, _T("DEC  C"), Z80DASM::NONE },
{	0x0e, _T("LD   C, %s"), Z80DASM::FETCH8 },
{	0x0f, _T("RRCA"), Z80DASM::NONE },

{	0x10, _T("DJNZ "), Z80DASM::JMPREL8 },
{	0x11, _T("LD   DE, %s"), Z80DASM::FETCH16 },
{	0x12, _T("LD  (DE), A"), Z80DASM::NONE },
{	0x13, _T("INC  DE"), Z80DASM::NONE },
{	0x14, _T("INC  D"), Z80DASM::NONE },
{	0x15, _T("DEC  D"), Z80DASM::NONE },
{	0x16, _T("LD   D, %s"), Z80DASM::FETCH8 },
{	0x17, _T("RLA"), Z80DASM::NONE },
{	0x18, _T("JR   "), Z80DASM::JMPREL8 },
{	0x19, _T("ADD  HL, DE"), Z80DASM::NONE },
{	0x1a, _T("LD   A, (DE)"), Z80DASM::NONE },
{	0x1b, _T("DEC  DE"), Z80DASM::NONE },
{	0x1c, _T("INC  E"), Z80DASM::NONE },
{	0x1d, _T("DEC  E"), Z80DASM::NONE },
{	0x1e, _T("LD   E, %s"), Z80DASM::FETCH8 },
{	0x1f, _T("RRA"), Z80DASM::NONE },

{	0x20, _T("JR   NZ, "), Z80DASM::JMPREL8 },
{	0x21, _T("LD   HL, %s"), Z80DASM::FETCH16 },
{	0x22, _T("LD  (%s), HL"), Z80DASM::FETCHID16 },
{	0x23, _T("INC  HL"), Z80DASM::NONE },
{	0x24, _T("INC  H"), Z80DASM::NONE },
{	0x25, _T("DEC  H"), Z80DASM::NONE },
{	0x26, _T("LD   H, %s"), Z80DASM::FETCH8 },
{	0x27, _T("DAA"), Z80DASM::NONE },
{	0x28, _T("JR   Z, "), Z80DASM::JMPREL8 },
{	0x29, _T("ADD  HL, HL"), Z80DASM::NONE },
{	0x2a, _T("LD   HL, (%s)"), Z80DASM::FETCHID16 },
{	0x2b, _T("DEC  HL"), Z80DASM::NONE },
{	0x2c, _T("INC  L"), Z80DASM::NONE },
{	0x2d, _T("DEC  L"), Z80DASM::NONE },
{	0x2e, _T("LD   L, %s"), Z80DASM::FETCH8 },
{	0x2f, _T("CPL"), Z80DASM::NONE },

{	0x30, _T("JR   NC, "), Z80DASM::JMPREL8 },
{	0x31, _T("LD   SP, %s"), Z80DASM::FETCH16 },
{	0x32, _T("LD  (%s), A"), Z80DASM::FETCHID16 },
{	0x33, _T("INC  SP"), Z80DASM::NONE },
{	0x34, _T("INC (HL)"), Z80DASM::NONE },
{	0x35, _T("DEC (HL)"), Z80DASM::NONE },
{	0x36, _T("LD  (HL), %s"), Z80DASM::FETCH8 },
{	0x37, _T("SCF"), Z80DASM::NONE },
{	0x38, _T("JR   C, "), Z80DASM::JMPREL8 },
{	0x39, _T("ADD  HL, SP"), Z80DASM::NONE },
{	0x3a, _T("LD   A, (%s)"), Z80DASM::FETCHID16 },
{	0x3b, _T("DEC  SP"), Z80DASM::NONE },
{	0x3c, _T("INC  A"), Z80DASM::NONE },
{	0x3d, _T("DEC  A"), Z80DASM::NONE },
{	0x3e, _T("LD   A, %s"), Z80DASM::FETCH8 },
{	0x3f, _T("CCF"), Z80DASM::NONE },

{	0x40, _T("LD   B, B"), Z80DASM::NONE },
{	0x41, _T("LD   B, C"), Z80DASM::NONE },
{	0x42, _T("LD   B, D"), Z80DASM::NONE },
{	0x43, _T("LD   B, E"), Z80DASM::NONE },
{	0x44, _T("LD   B, H"), Z80DASM::NONE },
{	0x45, _T("LD   B, L"), Z80DASM::NONE },
{	0x46, _T("LD   B, (HL)"), Z80DASM::NONE },
{	0x47, _T("LD   B, A"), Z80DASM::NONE },
{	0x48, _T("LD   C, B"), Z80DASM::NONE },
{	0x49, _T("LD   C, C"), Z80DASM::NONE },
{	0x4a, _T("LD   C, D"), Z80DASM::NONE },
{	0x4b, _T("LD   C, E"), Z80DASM::NONE },
{	0x4c, _T("LD   C, H"), Z80DASM::NONE },
{	0x4d, _T("LD   C, L"), Z80DASM::NONE },
{	0x4e, _T("LD   C, (HL)"), Z80DASM::NONE },
{	0x4f, _T("LD   C, A"), Z80DASM::NONE },

{	0x50, _T("LD   D, B"), Z80DASM::NONE },
{	0x51, _T("LD   D, C"), Z80DASM::NONE },
{	0x52, _T("LD   D, D"), Z80DASM::NONE },
{	0x53, _T("LD   D, E"), Z80DASM::NONE },
{	0x54, _T("LD   D, H"), Z80DASM::NONE },
{	0x55, _T("LD   D, L"), Z80DASM::NONE },
{	0x56, _T("LD   D, (HL)"), Z80DASM::NONE },
{	0x57, _T("LD   D, A"), Z80DASM::NONE },
{	0x58, _T("LD   E, B"), Z80DASM::NONE },
{	0x59, _T("LD   E, C"), Z80DASM::NONE },
{	0x5a, _T("LD   E, D"), Z80DASM::NONE },
{	0x5b, _T("LD   E, E"), Z80DASM::NONE },
{	0x5c, _T("LD   E, H"), Z80DASM::NONE },
{	0x5d, _T("LD   E, L"), Z80DASM::NONE },
{	0x5e, _T("LD   E, (HL)"), Z80DASM::NONE },
{	0x5f, _T("LD   E, A"), Z80DASM::NONE },

{	0x60, _T("LD   H, B"), Z80DASM::NONE },
{	0x61, _T("LD   H, C"), Z80DASM::NONE },
{	0x62, _T("LD   H, D"), Z80DASM::NONE },
{	0x63, _T("LD   H, E"), Z80DASM::NONE },
{	0x64, _T("LD   H, H"), Z80DASM::NONE },
{	0x65, _T("LD   H, L"), Z80DASM::NONE },
{	0x66, _T("LD   H, (HL)"), Z80DASM::NONE },
{	0x67, _T("LD   H, A"), Z80DASM::NONE },
{	0x68, _T("LD   L, B"), Z80DASM::NONE },
{	0x69, _T("LD   L, C"), Z80DASM::NONE },
{	0x6a, _T("LD   L, D"), Z80DASM::NONE },
{	0x6b, _T("LD   L, E"), Z80DASM::NONE },
{	0x6c, _T("LD   L, H"), Z80DASM::NONE },
{	0x6d, _T("LD   L, L"), Z80DASM::NONE },
{	0x6e, _T("LD   L, (HL)"), Z80DASM::NONE },
{	0x6f, _T("LD   L, A"), Z80DASM::NONE },

{	0x70, _T("LD  (HL), B"), Z80DASM::NONE },
{	0x71, _T("LD  (HL), C"), Z80DASM::NONE },
{	0x72, _T("LD  (HL), D"), Z80DASM::NONE },
{	0x73, _T("LD  (HL), E"), Z80DASM::NONE },
{	0x74, _T("LD  (HL), H"), Z80DASM::NONE },
{	0x75, _T("LD  (HL), L"), Z80DASM::NONE },
{	0x76, _T("HALT"), Z80DASM::NONE },
{	0x77, _T("LD  (HL), A"), Z80DASM::NONE },
{	0x78, _T("LD   A, B"), Z80DASM::NONE },
{	0x79, _T("LD   A, C"), Z80DASM::NONE },
{	0x7a, _T("LD   A, D"), Z80DASM::NONE },
{	0x7b, _T("LD   A, E"), Z80DASM::NONE },
{	0x7c, _T("LD   A, H"), Z80DASM::NONE },
{	0x7d, _T("LD   A, L"), Z80DASM::NONE },
{	0x7e, _T("LD   A, (HL)"), Z80DASM::NONE },
{	0x7f, _T("LD   A, A"), Z80DASM::NONE },

{	0x80, _T("ADD  A, B"), Z80DASM::NONE },
{	0x81, _T("ADD  A, C"), Z80DASM::NONE },
{	0x82, _T("ADD  A, D"), Z80DASM::NONE },
{	0x83, _T("ADD  A, E"), Z80DASM::NONE },
{	0x84, _T("ADD  A, H"), Z80DASM::NONE },
{	0x85, _T("ADD  A, L"), Z80DASM::NONE },
{	0x86, _T("ADD  A, (HL)"), Z80DASM::NONE },
{	0x87, _T("ADD  A, A"), Z80DASM::NONE },
{	0x88, _T("ADC  A, B"), Z80DASM::NONE },
{	0x89, _T("ADC  A, C"), Z80DASM::NONE },
{	0x8a, _T("ADC  A, D"), Z80DASM::NONE },
{	0x8b, _T("ADC  A, E"), Z80DASM::NONE },
{	0x8c, _T("ADC  A, H"), Z80DASM::NONE },
{	0x8d, _T("ADC  A, L"), Z80DASM::NONE },
{	0x8e, _T("ADC  A, (HL)"), Z80DASM::NONE },
{	0x8f, _T("ADC  A, A"), Z80DASM::NONE },

{	0x90, _T("SUB  B"), Z80DASM::NONE },
{	0x91, _T("SUB  C"), Z80DASM::NONE },
{	0x92, _T("SUB  D"), Z80DASM::NONE },
{	0x93, _T("SUB  E"), Z80DASM::NONE },
{	0x94, _T("SUB  H"), Z80DASM::NONE },
{	0x95, _T("SUB  L"), Z80DASM::NONE },
{	0x96, _T("SUB (HL)"), Z80DASM::NONE },
{	0x97, _T("SUB  A"), Z80DASM::NONE },
{	0x98, _T("SBC  A, B"), Z80DASM::NONE },
{	0x99, _T("SBC  A, C"), Z80DASM::NONE },
{	0x9a, _T("SBC  A, D"), Z80DASM::NONE },
{	0x9b, _T("SBC  A, E"), Z80DASM::NONE },
{	0x9c, _T("SBC  A, H"), Z80DASM::NONE },
{	0x9d, _T("SBC  A, L"), Z80DASM::NONE },
{	0x9e, _T("SBC  A, (HL)"), Z80DASM::NONE },
{	0x9f, _T("SBC  A, A"), Z80DASM::NONE },

{	0xa0, _T("AND  B"), Z80DASM::NONE },
{	0xa1, _T("AND  C"), Z80DASM::NONE },
{	0xa2, _T("AND  D"), Z80DASM::NONE },
{	0xa3, _T("AND  E"), Z80DASM::NONE },
{	0xa4, _T("AND  H"), Z80DASM::NONE },
{	0xa5, _T("AND  L"), Z80DASM::NONE },
{	0xa6, _T("AND (HL)"), Z80DASM::NONE },
{	0xa7, _T("AND  A"), Z80DASM::NONE },
{	0xa8, _T("XOR  B"), Z80DASM::NONE },
{	0xa9, _T("XOR  C"), Z80DASM::NONE },
{	0xaa, _T("XOR  D"), Z80DASM::NONE },
{	0xab, _T("XOR  E"), Z80DASM::NONE },
{	0xac, _T("XOR  H"), Z80DASM::NONE },
{	0xad, _T("XOR  L"), Z80DASM::NONE },
{	0xae, _T("XOR (HL)"), Z80DASM::NONE },
{	0xaf, _T("XOR  A"), Z80DASM::NONE },

{	0xb0, _T("OR   B"), Z80DASM::NONE },
{	0xb1, _T("OR   C"), Z80DASM::NONE },
{	0xb2, _T("OR   D"), Z80DASM::NONE },
{	0xb3, _T("OR   E"), Z80DASM::NONE },
{	0xb4, _T("OR   H"), Z80DASM::NONE },
{	0xb5, _T("OR   L"), Z80DASM::NONE },
{	0xb6, _T("OR  (HL)"), Z80DASM::NONE },
{	0xb7, _T("OR   A"), Z80DASM::NONE },
{	0xb8, _T("CP   B"), Z80DASM::NONE },
{	0xb9, _T("CP   C"), Z80DASM::NONE },
{	0xba, _T("CP   D"), Z80DASM::NONE },
{	0xbb, _T("CP   E"), Z80DASM::NONE },
{	0xbc, _T("CP   H"), Z80DASM::NONE },
{	0xbd, _T("CP   L"), Z80DASM::NONE },
{	0xbe, _T("CP  (HL)"), Z80DASM::NONE },
{	0xbf, _T("CP   A"), Z80DASM::NONE },

{	0xc0, _T("RET  NZ"), Z80DASM::NONE },
{	0xc1, _T("POP  BC"), Z80DASM::NONE },
{	0xc2, _T("JP   NZ, %s"), Z80DASM::FETCH16 },
{	0xc3, _T("JP   %s"), Z80DASM::FETCH16 },
{	0xc4, _T("CALL NZ, %s"), Z80DASM::FETCH16 },
{	0xc5, _T("PUSH BC"), Z80DASM::NONE },
{	0xc6, _T("ADD  A, %s"), Z80DASM::FETCH8 },
{	0xc7, _T("RST  00H"), Z80DASM::NONE },
{	0xc8, _T("RET  Z"), Z80DASM::NONE },
{	0xc9, _T("RET"), Z80DASM::NONE },
{	0xca, _T("JP   Z, %s"), Z80DASM::FETCH16 },
{	0xcb, _T("code CBh"), Z80DASM::CODE_CB },
{	0xcc, _T("CALL Z, %s"), Z80DASM::FETCH16 },
{	0xcd, _T("CALL %s"), Z80DASM::FETCH16 },
{	0xce, _T("ADC  A, %s"), Z80DASM::FETCH8 },
{	0xcf, _T("RST  08H"), Z80DASM::NONE },

{	0xd0, _T("RET  NC"), Z80DASM::NONE },
{	0xd1, _T("POP  DE"), Z80DASM::NONE },
{	0xd2, _T("JP   NC, %s"), Z80DASM::FETCH16 },
{	0xd3, _T("OUT (%s), A"), Z80DASM::FETCHID8 },
{	0xd4, _T("CALL NC, %s"), Z80DASM::FETCH16 },
{	0xd5, _T("PUSH DE"), Z80DASM::NONE },
{	0xd6, _T("SUB  %s"), Z80DASM::FETCH8 },
{	0xd7, _T("RST  10H"), Z80DASM::NONE },
{	0xd8, _T("RET  C"), Z80DASM::NONE },
{	0xd9, _T("EXX"), Z80DASM::NONE },
{	0xda, _T("JP   C, %s"), Z80DASM::FETCH16 },
{	0xdb, _T("IN   A, (%s)"), Z80DASM::FETCHID8 },
{	0xdc, _T("CALL C, %s"), Z80DASM::FETCH16 },
{	0xdd, _T("code DDh"), Z80DASM::CODE_DD },
{	0xde, _T("SBC  A, %s"), Z80DASM::FETCH8 },
{	0xdf, _T("RST  18H"), Z80DASM::NONE },

{	0xe0, _T("RET  PO"), Z80DASM::NONE },
{	0xe1, _T("POP  HL"), Z80DASM::NONE },
{	0xe2, _T("JP   PO, %s"), Z80DASM::FETCH16 },
{	0xe3, _T("EX   HL, (SP)"), Z80DASM::NONE },
{	0xe4, _T("CALL PO, %s"), Z80DASM::FETCH16 },
{	0xe5, _T("PUSH HL"), Z80DASM::NONE },
{	0xe6, _T("AND  %s"), Z80DASM::FETCH8 },
{	0xe7, _T("RST  20H"), Z80DASM::NONE },
{	0xe8, _T("RET  PE"), Z80DASM::NONE },
{	0xe9, _T("JP  (HL)"), Z80DASM::NONE },
{	0xea, _T("JP   PE, %s"), Z80DASM::FETCH16 },
{	0xeb, _T("EX   DE, HL"), Z80DASM::NONE },
{	0xec, _T("CALL PE, %s"), Z80DASM::FETCH16 },
{	0xed, _T("code EDh"), Z80DASM::CODE_ED },
{	0xee, _T("XOR  %s"), Z80DASM::FETCH8 },
{	0xef, _T("RST  28H"), Z80DASM::NONE },

{	0xf0, _T("RET  P"), Z80DASM::NONE },
{	0xf1, _T("POP  AF"), Z80DASM::NONE },
{	0xf2, _T("JP   P, %s"), Z80DASM::FETCH16 },
{	0xf3, _T("DI"), Z80DASM::NONE },
{	0xf4, _T("CALL P, %s"), Z80DASM::FETCH16 },
{	0xf5, _T("PUSH AF"), Z80DASM::NONE },
{	0xf6, _T("OR   %s"), Z80DASM::FETCH8 },
{	0xf7, _T("RST  30H"), Z80DASM::NONE },
{	0xf8, _T("RET  M"), Z80DASM::NONE },
{	0xf9, _T("LD   SP, HL"), Z80DASM::NONE },
{	0xfa, _T("JP   M, %s"), Z80DASM::FETCH16 },
{	0xfb, _T("EI"), Z80DASM::NONE },
{	0xfc, _T("CALL M, %s"), Z80DASM::FETCH16 },
{	0xfd, _T("code FDh"), Z80DASM::CODE_FD },
{	0xfe, _T("CP   %s"), Z80DASM::FETCH8 },
{	0xff, _T("RST  38H"), Z80DASM::NONE },

{	0x00, NULL, Z80DASM::NONE }
};

static const Z80DASM::opcode_t opcodes_cb[] = 
{
{	0x00, _T("RLC  B"), Z80DASM::NONE },
{	0x01, _T("RLC  C"), Z80DASM::NONE },
{	0x02, _T("RLC  D"), Z80DASM::NONE },
{	0x03, _T("RLC  E"), Z80DASM::NONE },
{	0x04, _T("RLC  H"), Z80DASM::NONE },
{	0x05, _T("RLC  L"), Z80DASM::NONE },
{	0x06, _T("RLC (HL)"), Z80DASM::NONE },
{	0x07, _T("RLC  A"), Z80DASM::NONE },
{	0x08, _T("RRC  B"), Z80DASM::NONE },
{	0x09, _T("RRC  C"), Z80DASM::NONE },
{	0x0a, _T("RRC  D"), Z80DASM::NONE },
{	0x0b, _T("RRC  E"), Z80DASM::NONE },
{	0x0c, _T("RRC  H"), Z80DASM::NONE },
{	0x0d, _T("RRC  L"), Z80DASM::NONE },
{	0x0e, _T("RRC (HL)"), Z80DASM::NONE },
{	0x0f, _T("RRC  A"), Z80DASM::NONE },
{	0x10, _T("RL   B"), Z80DASM::NONE },
{	0x11, _T("RL   C"), Z80DASM::NONE },
{	0x12, _T("RL   D"), Z80DASM::NONE },
{	0x13, _T("RL   E"), Z80DASM::NONE },
{	0x14, _T("RL   H"), Z80DASM::NONE },
{	0x15, _T("RL   L"), Z80DASM::NONE },
{	0x16, _T("RL  (HL)"), Z80DASM::NONE },
{	0x17, _T("RL   A"), Z80DASM::NONE },
{	0x18, _T("RR   B"), Z80DASM::NONE },
{	0x19, _T("RR   C"), Z80DASM::NONE },
{	0x1a, _T("RR   D"), Z80DASM::NONE },
{	0x1b, _T("RR   E"), Z80DASM::NONE },
{	0x1c, _T("RR   H"), Z80DASM::NONE },
{	0x1d, _T("RR   L"), Z80DASM::NONE },
{	0x1e, _T("RR  (HL)"), Z80DASM::NONE },
{	0x1f, _T("RR   A"), Z80DASM::NONE },
{	0x20, _T("SLA  B"), Z80DASM::NONE },
{	0x21, _T("SLA  C"), Z80DASM::NONE },
{	0x22, _T("SLA  D"), Z80DASM::NONE },
{	0x23, _T("SLA  E"), Z80DASM::NONE },
{	0x24, _T("SLA  H"), Z80DASM::NONE },
{	0x25, _T("SLA  L"), Z80DASM::NONE },
{	0x26, _T("SLA (HL)"), Z80DASM::NONE },
{	0x27, _T("SLA  A"), Z80DASM::NONE },
{	0x28, _T("SRA  B"), Z80DASM::NONE },
{	0x29, _T("SRA  C"), Z80DASM::NONE },
{	0x2a, _T("SRA  D"), Z80DASM::NONE },
{	0x2b, _T("SRA  E"), Z80DASM::NONE },
{	0x2c, _T("SRA  H"), Z80DASM::NONE },
{	0x2d, _T("SRA  L"), Z80DASM::NONE },
{	0x2e, _T("SRA (HL)"), Z80DASM::NONE },
{	0x2f, _T("SRA  A"), Z80DASM::NONE },
{	0x30, _T("SLL  B"), Z80DASM::NONE },
{	0x31, _T("SLL  C"), Z80DASM::NONE },
{	0x32, _T("SLL  D"), Z80DASM::NONE },
{	0x33, _T("SLL  E"), Z80DASM::NONE },
{	0x34, _T("SLL  H"), Z80DASM::NONE },
{	0x35, _T("SLL  L"), Z80DASM::NONE },
{	0x36, _T("SLL (HL)"), Z80DASM::NONE },
{	0x37, _T("SLL  A"), Z80DASM::NONE },
{	0x38, _T("SRL  B"), Z80DASM::NONE },
{	0x39, _T("SRL  C"), Z80DASM::NONE },
{	0x3a, _T("SRL  D"), Z80DASM::NONE },
{	0x3b, _T("SRL  E"), Z80DASM::NONE },
{	0x3c, _T("SRL  H"), Z80DASM::NONE },
{	0x3d, _T("SRL  L"), Z80DASM::NONE },
{	0x3e, _T("SRL (HL)"), Z80DASM::NONE },
{	0x3f, _T("SRL  A"), Z80DASM::NONE },
{	0x40, _T("BIT  0, B"), Z80DASM::NONE },
{	0x41, _T("BIT  0, C"), Z80DASM::NONE },
{	0x42, _T("BIT  0, D"), Z80DASM::NONE },
{	0x43, _T("BIT  0, E"), Z80DASM::NONE },
{	0x44, _T("BIT  0, H"), Z80DASM::NONE },
{	0x45, _T("BIT  0, L"), Z80DASM::NONE },
{	0x46, _T("BIT  0, (HL)"), Z80DASM::NONE },
{	0x47, _T("BIT  0, A"), Z80DASM::NONE },
{	0x48, _T("BIT  1, B"), Z80DASM::NONE },
{	0x49, _T("BIT  1, C"), Z80DASM::NONE },
{	0x4a, _T("BIT  1, D"), Z80DASM::NONE },
{	0x4b, _T("BIT  1, E"), Z80DASM::NONE },
{	0x4c, _T("BIT  1, H"), Z80DASM::NONE },
{	0x4d, _T("BIT  1, L"), Z80DASM::NONE },
{	0x4e, _T("BIT  1, (HL)"), Z80DASM::NONE },
{	0x4f, _T("BIT  1, A"), Z80DASM::NONE },
{	0x50, _T("BIT  2, B"), Z80DASM::NONE },
{	0x51, _T("BIT  2, C"), Z80DASM::NONE },
{	0x52, _T("BIT  2, D"), Z80DASM::NONE },
{	0x53, _T("BIT  2, E"), Z80DASM::NONE },
{	0x54, _T("BIT  2, H"), Z80DASM::NONE },
{	0x55, _T("BIT  2, L"), Z80DASM::NONE },
{	0x56, _T("BIT  2, (HL)"), Z80DASM::NONE },
{	0x57, _T("BIT  2, A"), Z80DASM::NONE },
{	0x58, _T("BIT  3, B"), Z80DASM::NONE },
{	0x59, _T("BIT  3, C"), Z80DASM::NONE },
{	0x5a, _T("BIT  3, D"), Z80DASM::NONE },
{	0x5b, _T("BIT  3, E"), Z80DASM::NONE },
{	0x5c, _T("BIT  3, H"), Z80DASM::NONE },
{	0x5d, _T("BIT  3, L"), Z80DASM::NONE },
{	0x5e, _T("BIT  3, (HL)"), Z80DASM::NONE },
{	0x5f, _T("BIT  3, A"), Z80DASM::NONE },
{	0x60, _T("BIT  4, B"), Z80DASM::NONE },
{	0x61, _T("BIT  4, C"), Z80DASM::NONE },
{	0x62, _T("BIT  4, D"), Z80DASM::NONE },
{	0x63, _T("BIT  4, E"), Z80DASM::NONE },
{	0x64, _T("BIT  4, H"), Z80DASM::NONE },
{	0x65, _T("BIT  4, L"), Z80DASM::NONE },
{	0x66, _T("BIT  4, (HL)"), Z80DASM::NONE },
{	0x67, _T("BIT  4, A"), Z80DASM::NONE },
{	0x68, _T("BIT  5, B"), Z80DASM::NONE },
{	0x69, _T("BIT  5, C"), Z80DASM::NONE },
{	0x6a, _T("BIT  5, D"), Z80DASM::NONE },
{	0x6b, _T("BIT  5, E"), Z80DASM::NONE },
{	0x6c, _T("BIT  5, H"), Z80DASM::NONE },
{	0x6d, _T("BIT  5, L"), Z80DASM::NONE },
{	0x6e, _T("BIT  5, (HL)"), Z80DASM::NONE },
{	0x6f, _T("BIT  5, A"), Z80DASM::NONE },
{	0x70, _T("BIT  6, B"), Z80DASM::NONE },
{	0x71, _T("BIT  6, C"), Z80DASM::NONE },
{	0x72, _T("BIT  6, D"), Z80DASM::NONE },
{	0x73, _T("BIT  6, E"), Z80DASM::NONE },
{	0x74, _T("BIT  6, H"), Z80DASM::NONE },
{	0x75, _T("BIT  6, L"), Z80DASM::NONE },
{	0x76, _T("BIT  6, (HL)"), Z80DASM::NONE },
{	0x77, _T("BIT  6, A"), Z80DASM::NONE },
{	0x78, _T("BIT  7, B"), Z80DASM::NONE },
{	0x79, _T("BIT  7, C"), Z80DASM::NONE },
{	0x7a, _T("BIT  7, D"), Z80DASM::NONE },
{	0x7b, _T("BIT  7, E"), Z80DASM::NONE },
{	0x7c, _T("BIT  7, H"), Z80DASM::NONE },
{	0x7d, _T("BIT  7, L"), Z80DASM::NONE },
{	0x7e, _T("BIT  7, (HL)"), Z80DASM::NONE },
{	0x7f, _T("BIT  7, A"), Z80DASM::NONE },
{	0x80, _T("RES  0, B"), Z80DASM::NONE },
{	0x81, _T("RES  0, C"), Z80DASM::NONE },
{	0x82, _T("RES  0, D"), Z80DASM::NONE },
{	0x83, _T("RES  0, E"), Z80DASM::NONE },
{	0x84, _T("RES  0, H"), Z80DASM::NONE },
{	0x85, _T("RES  0, L"), Z80DASM::NONE },
{	0x86, _T("RES  0, (HL)"), Z80DASM::NONE },
{	0x87, _T("RES  0, A"), Z80DASM::NONE },
{	0x88, _T("RES  1, B"), Z80DASM::NONE },
{	0x89, _T("RES  1, C"), Z80DASM::NONE },
{	0x8a, _T("RES  1, D"), Z80DASM::NONE },
{	0x8b, _T("RES  1, E"), Z80DASM::NONE },
{	0x8c, _T("RES  1, H"), Z80DASM::NONE },
{	0x8d, _T("RES  1, L"), Z80DASM::NONE },
{	0x8e, _T("RES  1, (HL)"), Z80DASM::NONE },
{	0x8f, _T("RES  1, A"), Z80DASM::NONE },
{	0x90, _T("RES  2, B"), Z80DASM::NONE },
{	0x91, _T("RES  2, C"), Z80DASM::NONE },
{	0x92, _T("RES  2, D"), Z80DASM::NONE },
{	0x93, _T("RES  2, E"), Z80DASM::NONE },
{	0x94, _T("RES  2, H"), Z80DASM::NONE },
{	0x95, _T("RES  2, L"), Z80DASM::NONE },
{	0x96, _T("RES  2, (HL)"), Z80DASM::NONE },
{	0x97, _T("RES  2, A"), Z80DASM::NONE },
{	0x98, _T("RES  3, B"), Z80DASM::NONE },
{	0x99, _T("RES  3, C"), Z80DASM::NONE },
{	0x9a, _T("RES  3, D"), Z80DASM::NONE },
{	0x9b, _T("RES  3, E"), Z80DASM::NONE },
{	0x9c, _T("RES  3, H"), Z80DASM::NONE },
{	0x9d, _T("RES  3, L"), Z80DASM::NONE },
{	0x9e, _T("RES  3, (HL)"), Z80DASM::NONE },
{	0x9f, _T("RES  3, A"), Z80DASM::NONE },
{	0xa0, _T("RES  4, B"), Z80DASM::NONE },
{	0xa1, _T("RES  4, C"), Z80DASM::NONE },
{	0xa2, _T("RES  4, D"), Z80DASM::NONE },
{	0xa3, _T("RES  4, E"), Z80DASM::NONE },
{	0xa4, _T("RES  4, H"), Z80DASM::NONE },
{	0xa5, _T("RES  4, L"), Z80DASM::NONE },
{	0xa6, _T("RES  4, (HL)"), Z80DASM::NONE },
{	0xa7, _T("RES  4, A"), Z80DASM::NONE },
{	0xa8, _T("RES  5, B"), Z80DASM::NONE },
{	0xa9, _T("RES  5, C"), Z80DASM::NONE },
{	0xaa, _T("RES  5, D"), Z80DASM::NONE },
{	0xab, _T("RES  5, E"), Z80DASM::NONE },
{	0xac, _T("RES  5, H"), Z80DASM::NONE },
{	0xad, _T("RES  5, L"), Z80DASM::NONE },
{	0xae, _T("RES  5, (HL)"), Z80DASM::NONE },
{	0xaf, _T("RES  5, A"), Z80DASM::NONE },
{	0xb0, _T("RES  6, B"), Z80DASM::NONE },
{	0xb1, _T("RES  6, C"), Z80DASM::NONE },
{	0xb2, _T("RES  6, D"), Z80DASM::NONE },
{	0xb3, _T("RES  6, E"), Z80DASM::NONE },
{	0xb4, _T("RES  6, H"), Z80DASM::NONE },
{	0xb5, _T("RES  6, L"), Z80DASM::NONE },
{	0xb6, _T("RES  6, (HL)"), Z80DASM::NONE },
{	0xb7, _T("RES  6, A"), Z80DASM::NONE },
{	0xb8, _T("RES  7, B"), Z80DASM::NONE },
{	0xb9, _T("RES  7, C"), Z80DASM::NONE },
{	0xba, _T("RES  7, D"), Z80DASM::NONE },
{	0xbb, _T("RES  7, E"), Z80DASM::NONE },
{	0xbc, _T("RES  7, H"), Z80DASM::NONE },
{	0xbd, _T("RES  7, L"), Z80DASM::NONE },
{	0xbe, _T("RES  7, (HL)"), Z80DASM::NONE },
{	0xbf, _T("RES  7, A"), Z80DASM::NONE },
{	0xc0, _T("SET  0, B"), Z80DASM::NONE },
{	0xc1, _T("SET  0, C"), Z80DASM::NONE },
{	0xc2, _T("SET  0, D"), Z80DASM::NONE },
{	0xc3, _T("SET  0, E"), Z80DASM::NONE },
{	0xc4, _T("SET  0, H"), Z80DASM::NONE },
{	0xc5, _T("SET  0, L"), Z80DASM::NONE },
{	0xc6, _T("SET  0, (HL)"), Z80DASM::NONE },
{	0xc7, _T("SET  0, A"), Z80DASM::NONE },
{	0xc8, _T("SET  1, B"), Z80DASM::NONE },
{	0xc9, _T("SET  1, C"), Z80DASM::NONE },
{	0xca, _T("SET  1, D"), Z80DASM::NONE },
{	0xcb, _T("SET  1, E"), Z80DASM::NONE },
{	0xcc, _T("SET  1, H"), Z80DASM::NONE },
{	0xcd, _T("SET  1, L"), Z80DASM::NONE },
{	0xce, _T("SET  1, (HL)"), Z80DASM::NONE },
{	0xcf, _T("SET  1, A"), Z80DASM::NONE },
{	0xd0, _T("SET  2, B"), Z80DASM::NONE },
{	0xd1, _T("SET  2, C"), Z80DASM::NONE },
{	0xd2, _T("SET  2, D"), Z80DASM::NONE },
{	0xd3, _T("SET  2, E"), Z80DASM::NONE },
{	0xd4, _T("SET  2, H"), Z80DASM::NONE },
{	0xd5, _T("SET  2, L"), Z80DASM::NONE },
{	0xd6, _T("SET  2, (HL)"), Z80DASM::NONE },
{	0xd7, _T("SET  2, A"), Z80DASM::NONE },
{	0xd8, _T("SET  3, B"), Z80DASM::NONE },
{	0xd9, _T("SET  3, C"), Z80DASM::NONE },
{	0xda, _T("SET  3, D"), Z80DASM::NONE },
{	0xdb, _T("SET  3, E"), Z80DASM::NONE },
{	0xdc, _T("SET  3, H"), Z80DASM::NONE },
{	0xdd, _T("SET  3, L"), Z80DASM::NONE },
{	0xde, _T("SET  3, (HL)"), Z80DASM::NONE },
{	0xdf, _T("SET  3, A"), Z80DASM::NONE },
{	0xe0, _T("SET  4, B"), Z80DASM::NONE },
{	0xe1, _T("SET  4, C"), Z80DASM::NONE },
{	0xe2, _T("SET  4, D"), Z80DASM::NONE },
{	0xe3, _T("SET  4, E"), Z80DASM::NONE },
{	0xe4, _T("SET  4, H"), Z80DASM::NONE },
{	0xe5, _T("SET  4, L"), Z80DASM::NONE },
{	0xe6, _T("SET  4, (HL)"), Z80DASM::NONE },
{	0xe7, _T("SET  4, A"), Z80DASM::NONE },
{	0xe8, _T("SET  5, B"), Z80DASM::NONE },
{	0xe9, _T("SET  5, C"), Z80DASM::NONE },
{	0xea, _T("SET  5, D"), Z80DASM::NONE },
{	0xeb, _T("SET  5, E"), Z80DASM::NONE },
{	0xec, _T("SET  5, H"), Z80DASM::NONE },
{	0xed, _T("SET  5, L"), Z80DASM::NONE },
{	0xee, _T("SET  5, (HL)"), Z80DASM::NONE },
{	0xef, _T("SET  5, A"), Z80DASM::NONE },
{	0xf0, _T("SET  6, B"), Z80DASM::NONE },
{	0xf1, _T("SET  6, C"), Z80DASM::NONE },
{	0xf2, _T("SET  6, D"), Z80DASM::NONE },
{	0xf3, _T("SET  6, E"), Z80DASM::NONE },
{	0xf4, _T("SET  6, H"), Z80DASM::NONE },
{	0xf5, _T("SET  6, L"), Z80DASM::NONE },
{	0xf6, _T("SET  6, (HL)"), Z80DASM::NONE },
{	0xf7, _T("SET  6, A"), Z80DASM::NONE },
{	0xf8, _T("SET  7, B"), Z80DASM::NONE },
{	0xf9, _T("SET  7, C"), Z80DASM::NONE },
{	0xfa, _T("SET  7, D"), Z80DASM::NONE },
{	0xfb, _T("SET  7, E"), Z80DASM::NONE },
{	0xfc, _T("SET  7, H"), Z80DASM::NONE },
{	0xfd, _T("SET  7, L"), Z80DASM::NONE },
{	0xfe, _T("SET  7, (HL)"), Z80DASM::NONE },
{	0xff, _T("SET  7, A"), Z80DASM::NONE },
{	0x00, NULL, Z80DASM::NONE }
};

static const Z80DASM::opcode_t opcodes_dd[] = 
{
{	0x09, _T("ADD  IX, BC"), Z80DASM::NONE },
{	0x19, _T("ADD  IX, DE"), Z80DASM::NONE },
{	0x21, _T("LD   IX, %s"), Z80DASM::FETCH16 },
{	0x22, _T("LD  (%s), IX"), Z80DASM::FETCHID16 },
{	0x23, _T("INC  IX"), Z80DASM::NONE },
{	0x24, _T("INC  HX"), Z80DASM::NONE },
{	0x25, _T("DEC  HX"), Z80DASM::NONE },
{	0x26, _T("LD   HX, %s"), Z80DASM::FETCH8 },
{	0x29, _T("ADD  IX, IX"), Z80DASM::NONE },
{	0x2a, _T("LD   IX, (%s)"), Z80DASM::FETCHID16 },
{	0x2b, _T("DEC  IX"), Z80DASM::NONE },
{	0x2c, _T("INC  LX"), Z80DASM::NONE },
{	0x2d, _T("DEC  LX"), Z80DASM::NONE },
{	0x2e, _T("LD   LX, %s"), Z80DASM::FETCH8 },
{	0x34, _T("INC (IX%s)"), Z80DASM::FETCHREL8 },
{	0x35, _T("DEC (IX%s)"), Z80DASM::FETCHREL8 },
{	0x36, _T("LD  (IX%s), %s"), Z80DASM::FETCHREL88 },
{	0x39, _T("ADD  IX, SP"), Z80DASM::NONE },
{	0x44, _T("LD   B, HX"), Z80DASM::NONE },
{	0x45, _T("LD   B, LX"), Z80DASM::NONE },
{	0x46, _T("LD   B, (IX%s)"), Z80DASM::FETCHREL8 },
{	0x4c, _T("LD   C, HX"), Z80DASM::NONE },
{	0x4d, _T("LD   C, LX"), Z80DASM::NONE },
{	0x4e, _T("LD   C, (IX%s)"), Z80DASM::FETCHREL8 },
{	0x54, _T("LD   D, HX"), Z80DASM::NONE },
{	0x55, _T("LD   D, LX"), Z80DASM::NONE },
{	0x56, _T("LD   D, (IX%s)"), Z80DASM::FETCHREL8 },
{	0x5c, _T("LD   E, HX"), Z80DASM::NONE },
{	0x5d, _T("LD   E, LX"), Z80DASM::NONE },
{	0x5e, _T("LD   E, (IX%s)"), Z80DASM::FETCHREL8 },
{	0x60, _T("LD   HX, B"), Z80DASM::NONE },
{	0x61, _T("LD   HX, C"), Z80DASM::NONE },
{	0x62, _T("LD   HX, D"), Z80DASM::NONE },
{	0x63, _T("LD   HX, E"), Z80DASM::NONE },
{	0x64, _T("LD   HX, HX"), Z80DASM::NONE },
{	0x65, _T("LD   HX, LX"), Z80DASM::NONE },
{	0x66, _T("LD   H, (IX%s)"), Z80DASM::FETCHREL8 },
{	0x67, _T("LD   HX, A"), Z80DASM::NONE },
{	0x68, _T("LD   LX, B"), Z80DASM::NONE },
{	0x69, _T("LD   LX, C"), Z80DASM::NONE },
{	0x6a, _T("LD   LX, D"), Z80DASM::NONE },
{	0x6b, _T("LD   LX, E"), Z80DASM::NONE },
{	0x6c, _T("LD   LX, HX"), Z80DASM::NONE },
{	0x6d, _T("LD   LX, LX"), Z80DASM::NONE },
{	0x6e, _T("LD   L, (IX%s)"), Z80DASM::FETCHREL8 },
{	0x6f, _T("LD   LX, A"), Z80DASM::NONE },
{	0x70, _T("LD  (IX%s), B"), Z80DASM::FETCHREL8 },
{	0x71, _T("LD  (IX%s), C"), Z80DASM::FETCHREL8 },
{	0x72, _T("LD  (IX%s), D"), Z80DASM::FETCHREL8 },
{	0x73, _T("LD  (IX%s), E"), Z80DASM::FETCHREL8 },
{	0x74, _T("LD  (IX%s), H"), Z80DASM::FETCHREL8 },
{	0x75, _T("LD  (IX%s), L"), Z80DASM::FETCHREL8 },
{	0x77, _T("LD  (IX%s), A"), Z80DASM::FETCHREL8 },
{	0x7c, _T("LD   A, HX"), Z80DASM::NONE },
{	0x7d, _T("LD   A, LX"), Z80DASM::NONE },
{	0x7e, _T("LD   A, (IX%s)"), Z80DASM::FETCHREL8 },
{	0x84, _T("ADD  A, HX"), Z80DASM::NONE },
{	0x85, _T("ADD  A, LX"), Z80DASM::NONE },
{	0x86, _T("ADD  A, (IX%s)"), Z80DASM::FETCHREL8 },
{	0x8c, _T("ADC  A, HX"), Z80DASM::NONE },
{	0x8d, _T("ADC  A, LX"), Z80DASM::NONE },
{	0x8e, _T("ADC  A, (IX%s)"), Z80DASM::FETCHREL8 },
{	0x94, _T("SUB  HX"), Z80DASM::NONE },
{	0x95, _T("SUB  LX"), Z80DASM::NONE },
{	0x96, _T("SUB (IX%s)"), Z80DASM::FETCHREL8 },
{	0x9c, _T("SBC  A, HX"), Z80DASM::NONE },
{	0x9d, _T("SBC  A, LX"), Z80DASM::NONE },
{	0x9e, _T("SBC  A, (IX%s)"), Z80DASM::FETCHREL8 },
{	0xa4, _T("AND  HX"), Z80DASM::NONE },
{	0xa5, _T("AND  LX"), Z80DASM::NONE },
{	0xa6, _T("AND (IX%s)"), Z80DASM::FETCHREL8 },
{	0xac, _T("XOR  HX"), Z80DASM::NONE },
{	0xad, _T("XOR  LX"), Z80DASM::NONE },
{	0xae, _T("XOR (IX%s)"), Z80DASM::FETCHREL8 },
{	0xb4, _T("OR   HX"), Z80DASM::NONE },
{	0xb5, _T("OR   LX"), Z80DASM::NONE },
{	0xb6, _T("OR  (IX%s)"), Z80DASM::FETCHREL8 },
{	0xbc, _T("CP   HX"), Z80DASM::NONE },
{	0xbd, _T("CP   LX"), Z80DASM::NONE },
{	0xbe, _T("CP  (IX%s)"), Z80DASM::FETCHREL8 },
{	0xcb, _T("code CBh"), Z80DASM::CODE_CB },	// dasm_ddcb(pc, buffer, buffer_len, first_symbol)
{	0xe1, _T("POP  IX"), Z80DASM::NONE },
{	0xe3, _T("EX  (SP), IX"), Z80DASM::NONE },
{	0xe5, _T("PUSH IX"), Z80DASM::NONE },
{	0xe9, _T("JP  (IX)"), Z80DASM::NONE },
{	0xf9, _T("LD   SP, IX"), Z80DASM::NONE },
{	0x00, NULL, Z80DASM::NONE }
};

static const Z80DASM::opcode_t opcodes_ed[] = 
{
{	0x40, _T("IN   B, (C)"), Z80DASM::NONE },
{	0x41, _T("OUT (C), B"), Z80DASM::NONE },
{	0x42, _T("SBC  HL, BC"), Z80DASM::NONE },
{	0x43, _T("LD  (%s), BC"), Z80DASM::FETCHID16 },
{	0x44, _T("NEG"), Z80DASM::NONE },
{	0x45, _T("RETN"), Z80DASM::NONE },
{	0x46, _T("IM   0"), Z80DASM::NONE },
{	0x47, _T("LD   I, A"), Z80DASM::NONE },
{	0x48, _T("IN   C, (C)"), Z80DASM::NONE },
{	0x49, _T("OUT (C), C"), Z80DASM::NONE },
{	0x4a, _T("ADC  HL, BC"), Z80DASM::NONE },
{	0x4b, _T("LD   BC, (%s)"), Z80DASM::FETCHID16 },
{	0x4c, _T("NEG"), Z80DASM::NONE },
{	0x4d, _T("RETI"), Z80DASM::NONE },
{	0x4e, _T("IM   0"), Z80DASM::NONE },
{	0x4f, _T("LD   R, A"), Z80DASM::NONE },
{	0x50, _T("IN   D, (C)"), Z80DASM::NONE },
{	0x51, _T("OUT (C), D"), Z80DASM::NONE },
{	0x52, _T("SBC  HL, DE"), Z80DASM::NONE },
{	0x53, _T("LD  (%s), DE"), Z80DASM::FETCHID16 },
{	0x54, _T("NEG"), Z80DASM::NONE },
{	0x55, _T("RETN"), Z80DASM::NONE },
{	0x56, _T("IM   1"), Z80DASM::NONE },
{	0x57, _T("LD   A, I"), Z80DASM::NONE },
{	0x58, _T("IN   E, (C)"), Z80DASM::NONE },
{	0x59, _T("OUT (C), E"), Z80DASM::NONE },
{	0x5a, _T("ADC  HL, DE"), Z80DASM::NONE },
{	0x5b, _T("LD   DE, (%s)"), Z80DASM::FETCHID16 },
{	0x5c, _T("NEG"), Z80DASM::NONE },
{	0x5d, _T("RETI"), Z80DASM::NONE },
{	0x5e, _T("IM   2"), Z80DASM::NONE },
{	0x5f, _T("LD   A, R"), Z80DASM::NONE },
{	0x60, _T("IN   H, (C)"), Z80DASM::NONE },
{	0x61, _T("OUT (C), H"), Z80DASM::NONE },
{	0x62, _T("SBC  HL, HL"), Z80DASM::NONE },
{	0x63, _T("LD  (%s), HL"), Z80DASM::FETCHID16 },
{	0x64, _T("NEG"), Z80DASM::NONE },
{	0x65, _T("RETN"), Z80DASM::NONE },
{	0x66, _T("IM   0"), Z80DASM::NONE },
{	0x67, _T("RRD (HL)"), Z80DASM::NONE },
{	0x68, _T("IN   L, (C)"), Z80DASM::NONE },
{	0x69, _T("OUT (C), L"), Z80DASM::NONE },
{	0x6a, _T("ADC  HL, HL"), Z80DASM::NONE },
{	0x6b, _T("LD   HL, (%s)"), Z80DASM::FETCHID16 },
{	0x6c, _T("NEG"), Z80DASM::NONE },
{	0x6d, _T("RETI"), Z80DASM::NONE },
{	0x6e, _T("IM   0"), Z80DASM::NONE },
{	0x6f, _T("RLD (HL)"), Z80DASM::NONE },
{	0x70, _T("IN   F, (C)"), Z80DASM::NONE },
{	0x71, _T("OUT (C), 0"), Z80DASM::NONE },
{	0x72, _T("SBC  HL, SP"), Z80DASM::NONE },
{	0x73, _T("LD  (%s), SP"), Z80DASM::FETCHID16 },
{	0x74, _T("NEG"), Z80DASM::NONE },
{	0x75, _T("RETN"), Z80DASM::NONE },
{	0x76, _T("IM   1"), Z80DASM::NONE },
{	0x78, _T("IN   A, (C)"), Z80DASM::NONE },
{	0x79, _T("OUT (C), A"), Z80DASM::NONE },
{	0x7a, _T("ADC  HL, SP"), Z80DASM::NONE },
{	0x7b, _T("LD   SP, (%s)"), Z80DASM::FETCHID16 },
{	0x7c, _T("NEG"), Z80DASM::NONE },
{	0x7d, _T("RETI"), Z80DASM::NONE },
{	0x7e, _T("IM   2"), Z80DASM::NONE },
{	0xa0, _T("LDI"), Z80DASM::NONE },
{	0xa1, _T("CPI"), Z80DASM::NONE },
{	0xa2, _T("INI"), Z80DASM::NONE },
{	0xa3, _T("OUTI"), Z80DASM::NONE },
{	0xa8, _T("LDD"), Z80DASM::NONE },
{	0xa9, _T("CPD"), Z80DASM::NONE },
{	0xaa, _T("IND"), Z80DASM::NONE },
{	0xab, _T("OUTD"), Z80DASM::NONE },
{	0xb0, _T("LDIR"), Z80DASM::NONE },
{	0xb1, _T("CPIR"), Z80DASM::NONE },
{	0xb2, _T("INIR"), Z80DASM::NONE },
{	0xb3, _T("OTIR"), Z80DASM::NONE },
{	0xb8, _T("LDDR"), Z80DASM::NONE },
{	0xb9, _T("CPDR"), Z80DASM::NONE },
{	0xba, _T("INDR"), Z80DASM::NONE },
{	0xbb, _T("OTDR"), Z80DASM::NONE },
{	0x00, NULL, Z80DASM::NONE }
};

static const Z80DASM::opcode_t opcodes_fd[] = 
{
{	0x09, _T("ADD  IY, BC"), Z80DASM::NONE },
{	0x19, _T("ADD  IY, DE"), Z80DASM::NONE },
{	0x21, _T("LD   IY, %s"), Z80DASM::FETCH16 },
{	0x22, _T("LD  (%s), IY"), Z80DASM::FETCHID16 },
{	0x23, _T("INC  IY"), Z80DASM::NONE },
{	0x24, _T("INC  HY"), Z80DASM::NONE },
{	0x25, _T("DEC  HY"), Z80DASM::NONE },
{	0x26, _T("LD   HY, %s"), Z80DASM::FETCH8 },
{	0x29, _T("ADD  IY, IY"), Z80DASM::NONE },
{	0x2a, _T("LD   IY, (%s)"), Z80DASM::FETCHID16 },
{	0x2b, _T("DEC  IY"), Z80DASM::NONE },
{	0x2c, _T("INC  LY"), Z80DASM::NONE },
{	0x2d, _T("DEC  LY"), Z80DASM::NONE },
{	0x2e, _T("LD   LY, %s"), Z80DASM::FETCH8 },
{	0x34, _T("INC (IY%s)"), Z80DASM::FETCHREL8 },
{	0x35, _T("DEC (IY%s)"), Z80DASM::FETCHREL8 },
{	0x36, _T("LD  (IY%s), %s"), Z80DASM::FETCHREL88 },
{	0x39, _T("ADD  IY, SP"), Z80DASM::NONE },
{	0x44, _T("LD   B, HY"), Z80DASM::NONE },
{	0x45, _T("LD   B, LY"), Z80DASM::NONE },
{	0x46, _T("LD   B, (IY%s)"), Z80DASM::FETCHREL8 },
{	0x4c, _T("LD   C, HY"), Z80DASM::NONE },
{	0x4d, _T("LD   C, LY"), Z80DASM::NONE },
{	0x4e, _T("LD   C, (IY%s)"), Z80DASM::FETCHREL8 },
{	0x54, _T("LD   D, HY"), Z80DASM::NONE },
{	0x55, _T("LD   D, LY"), Z80DASM::NONE },
{	0x56, _T("LD   D, (IY%s)"), Z80DASM::FETCHREL8 },
{	0x5c, _T("LD   E, HY"), Z80DASM::NONE },
{	0x5d, _T("LD   E, LY"), Z80DASM::NONE },
{	0x5e, _T("LD   E, (IY%s)"), Z80DASM::FETCHREL8 },
{	0x60, _T("LD   HY, B"), Z80DASM::NONE },
{	0x61, _T("LD   HY, C"), Z80DASM::NONE },
{	0x62, _T("LD   HY, D"), Z80DASM::NONE },
{	0x63, _T("LD   HY, E"), Z80DASM::NONE },
{	0x64, _T("LD   HY, HY"), Z80DASM::NONE },
{	0x65, _T("LD   HY, LY"), Z80DASM::NONE },
{	0x66, _T("LD   H, (IY%s)"), Z80DASM::FETCHREL8 },
{	0x67, _T("LD   HY, A"), Z80DASM::NONE },
{	0x68, _T("LD   LY, B"), Z80DASM::NONE },
{	0x69, _T("LD   LY, C"), Z80DASM::NONE },
{	0x6a, _T("LD   LY, D"), Z80DASM::NONE },
{	0x6b, _T("LD   LY, E"), Z80DASM::NONE },
{	0x6c, _T("LD   LY, HY"), Z80DASM::NONE },
{	0x6d, _T("LD   LY, LY"), Z80DASM::NONE },
{	0x6e, _T("LD   L, (IY%s)"), Z80DASM::FETCHREL8 },
{	0x6f, _T("LD   LY, A"), Z80DASM::NONE },
{	0x70, _T("LD  (IY%s), B"), Z80DASM::FETCHREL8 },
{	0x71, _T("LD  (IY%s), C"), Z80DASM::FETCHREL8 },
{	0x72, _T("LD  (IY%s), D"), Z80DASM::FETCHREL8 },
{	0x73, _T("LD  (IY%s), E"), Z80DASM::FETCHREL8 },
{	0x74, _T("LD  (IY%s), H"), Z80DASM::FETCHREL8 },
{	0x75, _T("LD  (IY%s), L"), Z80DASM::FETCHREL8 },
{	0x77, _T("LD  (IY%s), A"), Z80DASM::FETCHREL8 },
{	0x7c, _T("LD   A, HY"), Z80DASM::NONE },
{	0x7d, _T("LD   A, LY"), Z80DASM::NONE },
{	0x7e, _T("LD   A, (IY%s)"), Z80DASM::FETCHREL8 },
{	0x84, _T("ADD  A, HY"), Z80DASM::NONE },
{	0x85, _T("ADD  A, LY"), Z80DASM::NONE },
{	0x86, _T("ADD  A, (IY%s)"), Z80DASM::FETCHREL8 },
{	0x8c, _T("ADC  A, HY"), Z80DASM::NONE },
{	0x8d, _T("ADC  A, LY"), Z80DASM::NONE },
{	0x8e, _T("ADC  A, (IY%s)"), Z80DASM::FETCHREL8 },
{	0x94, _T("SUB  HY"), Z80DASM::NONE },
{	0x95, _T("SUB  LY"), Z80DASM::NONE },
{	0x96, _T("SUB (IY%s)"), Z80DASM::FETCHREL8 },
{	0x9c, _T("SBC  A, HY"), Z80DASM::NONE },
{	0x9d, _T("SBC  A, LY"), Z80DASM::NONE },
{	0x9e, _T("SBC  A, (IY%s)"), Z80DASM::FETCHREL8 },
{	0xa4, _T("AND  HY"), Z80DASM::NONE },
{	0xa5, _T("AND  LY"), Z80DASM::NONE },
{	0xa6, _T("AND (IY%s)"), Z80DASM::FETCHREL8 },
{	0xac, _T("XOR  HY"), Z80DASM::NONE },
{	0xad, _T("XOR  LY"), Z80DASM::NONE },
{	0xae, _T("XOR (IY%s)"), Z80DASM::FETCHREL8 },
{	0xb4, _T("OR   HY"), Z80DASM::NONE },
{	0xb5, _T("OR   LY"), Z80DASM::NONE },
{	0xb6, _T("OR  (IY%s)"), Z80DASM::FETCHREL8 },
{	0xbc, _T("CP   HY"), Z80DASM::NONE },
{	0xbd, _T("CP   LY"), Z80DASM::NONE },
{	0xbe, _T("CP  (IY%s)"), Z80DASM::FETCHREL8 },
{	0xcb, _T("code CBh"), Z80DASM::CODE_CB }, // dasm_fdcb(pc, buffer, buffer_len, first_symbol)
{	0xe1, _T("POP  IY"), Z80DASM::NONE },
{	0xe3, _T("EX  (SP), IY"), Z80DASM::NONE },
{	0xe5, _T("PUSH IY"), Z80DASM::NONE },
{	0xe9, _T("JP  (IY)"), Z80DASM::NONE },
{	0xf9, _T("LD   SP, IY"), Z80DASM::NONE },
{	0x00, NULL, Z80DASM::NONE }
};

static const Z80DASM::opcode_t opcodes_ddcb[] = 
{
{	0x00, _T("RLC  B=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x01, _T("RLC  C=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x02, _T("RLC  D=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x03, _T("RLC  E=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x04, _T("RLC  H=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x05, _T("RLC  L=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x06, _T("RLC (IX%s)"), Z80DASM::FETCHREL8 },
{	0x07, _T("RLC  A=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x08, _T("RRC  B=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x09, _T("RRC  C=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x0a, _T("RRC  D=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x0b, _T("RRC  E=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x0c, _T("RRC  H=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x0d, _T("RRC  L=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x0e, _T("RRC (IX%s)"), Z80DASM::FETCHREL8 },
{	0x0f, _T("RRC  A=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x10, _T("RL   B=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x11, _T("RL   C=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x12, _T("RL   D=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x13, _T("RL   E=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x14, _T("RL   H=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x15, _T("RL   L=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x16, _T("RL  (IX%s)"), Z80DASM::FETCHREL8 },
{	0x17, _T("RL   A=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x18, _T("RR   B=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x19, _T("RR   C=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x1a, _T("RR   D=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x1b, _T("RR   E=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x1c, _T("RR   H=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x1d, _T("RR   L=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x1e, _T("RR  (IX%s)"), Z80DASM::FETCHREL8 },
{	0x1f, _T("RR   A=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x20, _T("SLA  B=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x21, _T("SLA  C=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x22, _T("SLA  D=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x23, _T("SLA  E=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x24, _T("SLA  H=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x25, _T("SLA  L=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x26, _T("SLA (IX%s)"), Z80DASM::FETCHREL8 },
{	0x27, _T("SLA  A=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x28, _T("SRA  B=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x29, _T("SRA  C=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x2a, _T("SRA  D=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x2b, _T("SRA  E=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x2c, _T("SRA  H=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x2d, _T("SRA  L=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x2e, _T("SRA (IX%s)"), Z80DASM::FETCHREL8 },
{	0x2f, _T("SRA  A=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x30, _T("SLL  B=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x31, _T("SLL  C=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x32, _T("SLL  D=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x33, _T("SLL  E=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x34, _T("SLL  H=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x35, _T("SLL  L=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x36, _T("SLL (IX%s)"), Z80DASM::FETCHREL8 },
{	0x37, _T("SLL  A=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x38, _T("SRL  B=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x39, _T("SRL  C=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x3a, _T("SRL  D=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x3b, _T("SRL  E=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x3c, _T("SRL  H=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x3d, _T("SRL  L=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x3e, _T("SRL (IX%s)"), Z80DASM::FETCHREL8 },
{	0x3f, _T("SRL  A=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x40, _T("BIT  0, B=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x41, _T("BIT  0, C=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x42, _T("BIT  0, D=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x43, _T("BIT  0, E=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x44, _T("BIT  0, H=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x45, _T("BIT  0, L=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x46, _T("BIT  0, (IX%s)"), Z80DASM::FETCHREL8 },
{	0x47, _T("BIT  0, A=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x48, _T("BIT  1, B=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x49, _T("BIT  1, C=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x4a, _T("BIT  1, D=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x4b, _T("BIT  1, E=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x4c, _T("BIT  1, H=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x4d, _T("BIT  1, L=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x4e, _T("BIT  1, (IX%s)"), Z80DASM::FETCHREL8 },
{	0x4f, _T("BIT  1, A=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x50, _T("BIT  2, B=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x51, _T("BIT  2, C=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x52, _T("BIT  2, D=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x53, _T("BIT  2, E=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x54, _T("BIT  2, H=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x55, _T("BIT  2, L=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x56, _T("BIT  2, (IX%s)"), Z80DASM::FETCHREL8 },
{	0x57, _T("BIT  2, A=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x58, _T("BIT  3, B=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x59, _T("BIT  3, C=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x5a, _T("BIT  3, D=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x5b, _T("BIT  3, E=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x5c, _T("BIT  3, H=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x5d, _T("BIT  3, L=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x5e, _T("BIT  3, (IX%s)"), Z80DASM::FETCHREL8 },
{	0x5f, _T("BIT  3, A=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x60, _T("BIT  4, B=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x61, _T("BIT  4, C=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x62, _T("BIT  4, D=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x63, _T("BIT  4, E=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x64, _T("BIT  4, H=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x65, _T("BIT  4, L=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x66, _T("BIT  4, (IX%s)"), Z80DASM::FETCHREL8 },
{	0x67, _T("BIT  4, A=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x68, _T("BIT  5, B=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x69, _T("BIT  5, C=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x6a, _T("BIT  5, D=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x6b, _T("BIT  5, E=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x6c, _T("BIT  5, H=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x6d, _T("BIT  5, L=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x6e, _T("BIT  5, (IX%s)"), Z80DASM::FETCHREL8 },
{	0x6f, _T("BIT  5, A=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x70, _T("BIT  6, B=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x71, _T("BIT  6, C=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x72, _T("BIT  6, D=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x73, _T("BIT  6, E=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x74, _T("BIT  6, H=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x75, _T("BIT  6, L=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x76, _T("BIT  6, (IX%s)"), Z80DASM::FETCHREL8 },
{	0x77, _T("BIT  6, A=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x78, _T("BIT  7, B=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x79, _T("BIT  7, C=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x7a, _T("BIT  7, D=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x7b, _T("BIT  7, E=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x7c, _T("BIT  7, H=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x7d, _T("BIT  7, L=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x7e, _T("BIT  7, (IX%s)"), Z80DASM::FETCHREL8 },
{	0x7f, _T("BIT  7, A=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x80, _T("RES  0, B=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x81, _T("RES  0, C=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x82, _T("RES  0, D=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x83, _T("RES  0, E=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x84, _T("RES  0, H=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x85, _T("RES  0, L=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x86, _T("RES  0, (IX%s)"), Z80DASM::FETCHREL8 },
{	0x87, _T("RES  0, A=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x88, _T("RES  1, B=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x89, _T("RES  1, C=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x8a, _T("RES  1, D=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x8b, _T("RES  1, E=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x8c, _T("RES  1, H=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x8d, _T("RES  1, L=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x8e, _T("RES  1, (IX%s)"), Z80DASM::FETCHREL8 },
{	0x8f, _T("RES  1, A=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x90, _T("RES  2, B=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x91, _T("RES  2, C=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x92, _T("RES  2, D=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x93, _T("RES  2, E=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x94, _T("RES  2, H=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x95, _T("RES  2, L=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x96, _T("RES  2, (IX%s)"), Z80DASM::FETCHREL8 },
{	0x97, _T("RES  2, A=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x98, _T("RES  3, B=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x99, _T("RES  3, C=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x9a, _T("RES  3, D=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x9b, _T("RES  3, E=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x9c, _T("RES  3, H=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x9d, _T("RES  3, L=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x9e, _T("RES  3, (IX%s)"), Z80DASM::FETCHREL8 },
{	0x9f, _T("RES  3, A=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xa0, _T("RES  4, B=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xa1, _T("RES  4, C=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xa2, _T("RES  4, D=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xa3, _T("RES  4, E=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xa4, _T("RES  4, H=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xa5, _T("RES  4, L=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xa6, _T("RES  4, (IX%s)"), Z80DASM::FETCHREL8 },
{	0xa7, _T("RES  4, A=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xa8, _T("RES  5, B=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xa9, _T("RES  5, C=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xaa, _T("RES  5, D=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xab, _T("RES  5, E=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xac, _T("RES  5, H=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xad, _T("RES  5, L=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xae, _T("RES  5, (IX%s)"), Z80DASM::FETCHREL8 },
{	0xaf, _T("RES  5, A=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xb0, _T("RES  6, B=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xb1, _T("RES  6, C=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xb2, _T("RES  6, D=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xb3, _T("RES  6, E=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xb4, _T("RES  6, H=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xb5, _T("RES  6, L=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xb6, _T("RES  6, (IX%s)"), Z80DASM::FETCHREL8 },
{	0xb7, _T("RES  6, A=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xb8, _T("RES  7, B=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xb9, _T("RES  7, C=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xba, _T("RES  7, D=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xbb, _T("RES  7, E=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xbc, _T("RES  7, H=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xbd, _T("RES  7, L=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xbe, _T("RES  7, (IX%s)"), Z80DASM::FETCHREL8 },
{	0xbf, _T("RES  7, A=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xc0, _T("SET  0, B=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xc1, _T("SET  0, C=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xc2, _T("SET  0, D=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xc3, _T("SET  0, E=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xc4, _T("SET  0, H=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xc5, _T("SET  0, L=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xc6, _T("SET  0, (IX%s)"), Z80DASM::FETCHREL8 },
{	0xc7, _T("SET  0, A=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xc8, _T("SET  1, B=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xc9, _T("SET  1, C=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xca, _T("SET  1, D=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xcb, _T("SET  1, E=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xcc, _T("SET  1, H=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xcd, _T("SET  1, L=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xce, _T("SET  1, (IX%s)"), Z80DASM::FETCHREL8 },
{	0xcf, _T("SET  1, A=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xd0, _T("SET  2, B=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xd1, _T("SET  2, C=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xd2, _T("SET  2, D=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xd3, _T("SET  2, E=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xd4, _T("SET  2, H=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xd5, _T("SET  2, L=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xd6, _T("SET  2, (IX%s)"), Z80DASM::FETCHREL8 },
{	0xd7, _T("SET  2, A=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xd8, _T("SET  3, B=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xd9, _T("SET  3, C=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xda, _T("SET  3, D=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xdb, _T("SET  3, E=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xdc, _T("SET  3, H=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xdd, _T("SET  3, L=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xde, _T("SET  3, (IX%s)"), Z80DASM::FETCHREL8 },
{	0xdf, _T("SET  3, A=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xe0, _T("SET  4, B=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xe1, _T("SET  4, C=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xe2, _T("SET  4, D=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xe3, _T("SET  4, E=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xe4, _T("SET  4, H=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xe5, _T("SET  4, L=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xe6, _T("SET  4, (IX%s)"), Z80DASM::FETCHREL8 },
{	0xe7, _T("SET  4, A=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xe8, _T("SET  5, B=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xe9, _T("SET  5, C=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xea, _T("SET  5, D=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xeb, _T("SET  5, E=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xec, _T("SET  5, H=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xed, _T("SET  5, L=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xee, _T("SET  5, (IX%s)"), Z80DASM::FETCHREL8 },
{	0xef, _T("SET  5, A=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xf0, _T("SET  6, B=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xf1, _T("SET  6, C=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xf2, _T("SET  6, D=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xf3, _T("SET  6, E=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xf4, _T("SET  6, H=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xf5, _T("SET  6, L=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xf6, _T("SET  6, (IX%s)"), Z80DASM::FETCHREL8 },
{	0xf7, _T("SET  6, A=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xf8, _T("SET  7, B=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xf9, _T("SET  7, C=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xfa, _T("SET  7, D=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xfb, _T("SET  7, E=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xfc, _T("SET  7, H=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xfd, _T("SET  7, L=(IX%s)"), Z80DASM::FETCHREL8 },
{	0xfe, _T("SET  7, (IX%s)"), Z80DASM::FETCHREL8 },
{	0xff, _T("SET  7, A=(IX%s)"), Z80DASM::FETCHREL8 },
{	0x00, NULL, Z80DASM::NONE }
};

static const Z80DASM::opcode_t opcodes_fdcb[] = 
{
{	0x00, _T("RLC  B=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x01, _T("RLC  C=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x02, _T("RLC  D=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x03, _T("RLC  E=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x04, _T("RLC  H=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x05, _T("RLC  L=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x06, _T("RLC (IY%s)"), Z80DASM::FETCHREL8 },
{	0x07, _T("RLC  A=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x08, _T("RRC  B=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x09, _T("RRC  C=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x0a, _T("RRC  D=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x0b, _T("RRC  E=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x0c, _T("RRC  H=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x0d, _T("RRC  L=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x0e, _T("RRC (IY%s)"), Z80DASM::FETCHREL8 },
{	0x0f, _T("RRC  A=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x10, _T("RL   B=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x11, _T("RL   C=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x12, _T("RL   D=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x13, _T("RL   E=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x14, _T("RL   H=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x15, _T("RL   L=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x16, _T("RL  (IY%s)"), Z80DASM::FETCHREL8 },
{	0x17, _T("RL   A=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x18, _T("RR   B=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x19, _T("RR   C=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x1a, _T("RR   D=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x1b, _T("RR   E=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x1c, _T("RR   H=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x1d, _T("RR   L=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x1e, _T("RR  (IY%s)"), Z80DASM::FETCHREL8 },
{	0x1f, _T("RR   A=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x20, _T("SLA  B=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x21, _T("SLA  C=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x22, _T("SLA  D=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x23, _T("SLA  E=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x24, _T("SLA  H=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x25, _T("SLA  L=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x26, _T("SLA (IY%s)"), Z80DASM::FETCHREL8 },
{	0x27, _T("SLA  A=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x28, _T("SRA  B=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x29, _T("SRA  C=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x2a, _T("SRA  D=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x2b, _T("SRA  E=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x2c, _T("SRA  H=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x2d, _T("SRA  L=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x2e, _T("SRA (IY%s)"), Z80DASM::FETCHREL8 },
{	0x2f, _T("SRA  A=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x30, _T("SLL  B=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x31, _T("SLL  C=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x32, _T("SLL  D=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x33, _T("SLL  E=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x34, _T("SLL  H=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x35, _T("SLL  L=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x36, _T("SLL (IY%s)"), Z80DASM::FETCHREL8 },
{	0x37, _T("SLL  A=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x38, _T("SRL  B=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x39, _T("SRL  C=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x3a, _T("SRL  D=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x3b, _T("SRL  E=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x3c, _T("SRL  H=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x3d, _T("SRL  L=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x3e, _T("SRL (IY%s)"), Z80DASM::FETCHREL8 },
{	0x3f, _T("SRL A=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x40, _T("BIT  0, B=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x41, _T("BIT  0, C=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x42, _T("BIT  0, D=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x43, _T("BIT  0, E=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x44, _T("BIT  0, H=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x45, _T("BIT  0, L=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x46, _T("BIT  0, (IY%s)"), Z80DASM::FETCHREL8 },
{	0x47, _T("BIT  0, A=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x48, _T("BIT  1, B=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x49, _T("BIT  1, C=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x4a, _T("BIT  1, D=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x4b, _T("BIT  1, E=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x4c, _T("BIT  1, H=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x4d, _T("BIT  1, L=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x4e, _T("BIT  1, (IY%s)"), Z80DASM::FETCHREL8 },
{	0x4f, _T("BIT  1, A=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x50, _T("BIT  2, B=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x51, _T("BIT  2, C=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x52, _T("BIT  2, D=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x53, _T("BIT  2, E=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x54, _T("BIT  2, H=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x55, _T("BIT  2, L=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x56, _T("BIT  2, (IY%s)"), Z80DASM::FETCHREL8 },
{	0x57, _T("BIT  2, A=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x58, _T("BIT  3, B=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x59, _T("BIT  3, C=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x5a, _T("BIT  3, D=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x5b, _T("BIT  3, E=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x5c, _T("BIT  3, H=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x5d, _T("BIT  3, L=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x5e, _T("BIT  3, (IY%s)"), Z80DASM::FETCHREL8 },
{	0x5f, _T("BIT  3, A=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x60, _T("BIT  4, B=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x61, _T("BIT  4, C=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x62, _T("BIT  4, D=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x63, _T("BIT  4, E=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x64, _T("BIT  4, H=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x65, _T("BIT  4, L=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x66, _T("BIT  4, (IY%s)"), Z80DASM::FETCHREL8 },
{	0x67, _T("BIT  4, A=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x68, _T("BIT  5, B=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x69, _T("BIT  5, C=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x6a, _T("BIT  5, D=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x6b, _T("BIT  5, E=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x6c, _T("BIT  5, H=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x6d, _T("BIT  5, L=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x6e, _T("BIT  5, (IY%s)"), Z80DASM::FETCHREL8 },
{	0x6f, _T("BIT  5, A=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x70, _T("BIT  6, B=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x71, _T("BIT  6, C=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x72, _T("BIT  6, D=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x73, _T("BIT  6, E=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x74, _T("BIT  6, H=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x75, _T("BIT  6, L=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x76, _T("BIT  6, (IY%s)"), Z80DASM::FETCHREL8 },
{	0x77, _T("BIT  6, A=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x78, _T("BIT  7, B=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x79, _T("BIT  7, C=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x7a, _T("BIT  7, D=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x7b, _T("BIT  7, E=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x7c, _T("BIT  7, H=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x7d, _T("BIT  7, L=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x7e, _T("BIT  7, (IY%s)"), Z80DASM::FETCHREL8 },
{	0x7f, _T("BIT  7, A=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x80, _T("RES  0, B=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x81, _T("RES  0, C=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x82, _T("RES  0, D=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x83, _T("RES  0, E=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x84, _T("RES  0, H=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x85, _T("RES  0, L=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x86, _T("RES  0, (IY%s)"), Z80DASM::FETCHREL8 },
{	0x87, _T("RES  0, A=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x88, _T("RES  1, B=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x89, _T("RES  1, C=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x8a, _T("RES  1, D=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x8b, _T("RES  1, E=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x8c, _T("RES  1, H=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x8d, _T("RES  1, L=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x8e, _T("RES  1, (IY%s)"), Z80DASM::FETCHREL8 },
{	0x8f, _T("RES  1, A=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x90, _T("RES  2, B=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x91, _T("RES  2, C=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x92, _T("RES  2, D=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x93, _T("RES  2, E=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x94, _T("RES  2, H=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x95, _T("RES  2, L=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x96, _T("RES  2, (IY%s)"), Z80DASM::FETCHREL8 },
{	0x97, _T("RES  2, A=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x98, _T("RES  3, B=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x99, _T("RES  3, C=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x9a, _T("RES  3, D=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x9b, _T("RES  3, E=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x9c, _T("RES  3, H=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x9d, _T("RES  3, L=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x9e, _T("RES  3, (IY%s)"), Z80DASM::FETCHREL8 },
{	0x9f, _T("RES  3, A=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xa0, _T("RES  4, B=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xa1, _T("RES  4, C=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xa2, _T("RES  4, D=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xa3, _T("RES  4, E=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xa4, _T("RES  4, H=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xa5, _T("RES  4, L=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xa6, _T("RES  4, (IY%s)"), Z80DASM::FETCHREL8 },
{	0xa7, _T("RES  4, A=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xa8, _T("RES  5, B=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xa9, _T("RES  5, C=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xaa, _T("RES  5, D=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xab, _T("RES  5, E=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xac, _T("RES  5, H=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xad, _T("RES  5, L=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xae, _T("RES  5, (IY%s)"), Z80DASM::FETCHREL8 },
{	0xaf, _T("RES  5, A=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xb0, _T("RES  6, B=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xb1, _T("RES  6, C=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xb2, _T("RES  6, D=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xb3, _T("RES  6, E=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xb4, _T("RES  6, H=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xb5, _T("RES  6, L=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xb6, _T("RES  6, (IY%s)"), Z80DASM::FETCHREL8 },
{	0xb7, _T("RES  6, A=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xb8, _T("RES  7, B=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xb9, _T("RES  7, C=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xba, _T("RES  7, D=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xbb, _T("RES  7, E=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xbc, _T("RES  7, H=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xbd, _T("RES  7, L=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xbe, _T("RES  7, (IY%s)"), Z80DASM::FETCHREL8 },
{	0xbf, _T("RES  7, A=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xc0, _T("SET  0, B=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xc1, _T("SET  0, C=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xc2, _T("SET  0, D=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xc3, _T("SET  0, E=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xc4, _T("SET  0, H=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xc5, _T("SET  0, L=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xc6, _T("SET  0, (IY%s)"), Z80DASM::FETCHREL8 },
{	0xc7, _T("SET  0, A=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xc8, _T("SET  1, B=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xc9, _T("SET  1, C=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xca, _T("SET  1, D=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xcb, _T("SET  1, E=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xcc, _T("SET  1, H=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xcd, _T("SET  1, L=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xce, _T("SET  1, (IY%s)"), Z80DASM::FETCHREL8 },
{	0xcf, _T("SET  1, A=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xd0, _T("SET  2, B=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xd1, _T("SET  2, C=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xd2, _T("SET  2, D=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xd3, _T("SET  2, E=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xd4, _T("SET  2, H=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xd5, _T("SET  2, L=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xd6, _T("SET  2, (IY%s)"), Z80DASM::FETCHREL8 },
{	0xd7, _T("SET  2, A=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xd8, _T("SET  3, B=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xd9, _T("SET  3, C=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xda, _T("SET  3, D=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xdb, _T("SET  3, E=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xdc, _T("SET  3, H=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xdd, _T("SET  3, L=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xde, _T("SET  3, (IY%s)"), Z80DASM::FETCHREL8 },
{	0xdf, _T("SET  3, A=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xe0, _T("SET  4, B=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xe1, _T("SET  4, C=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xe2, _T("SET  4, D=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xe3, _T("SET  4, E=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xe4, _T("SET  4, H=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xe5, _T("SET  4, L=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xe6, _T("SET  4, (IY%s)"), Z80DASM::FETCHREL8 },
{	0xe7, _T("SET  4, A=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xe8, _T("SET  5, B=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xe9, _T("SET  5, C=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xea, _T("SET  5, D=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xeb, _T("SET  5, E=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xec, _T("SET  5, H=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xed, _T("SET  5, L=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xee, _T("SET  5, (IY%s)"), Z80DASM::FETCHREL8 },
{	0xef, _T("SET  5, A=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xf0, _T("SET  6, B=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xf1, _T("SET  6, C=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xf2, _T("SET  6, D=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xf3, _T("SET  6, E=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xf4, _T("SET  6, H=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xf5, _T("SET  6, L=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xf6, _T("SET  6, (IY%s)"), Z80DASM::FETCHREL8 },
{	0xf7, _T("SET  6, A=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xf8, _T("SET  7, B=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xf9, _T("SET  7, C=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xfa, _T("SET  7, D=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xfb, _T("SET  7, E=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xfc, _T("SET  7, H=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xfd, _T("SET  7, L=(IY%s)"), Z80DASM::FETCHREL8 },
{	0xfe, _T("SET  7, (IY%s)"), Z80DASM::FETCHREL8 },
{	0xff, _T("SET  7, A=(IY%s)"), Z80DASM::FETCHREL8 },
{	0x00, NULL, Z80DASM::NONE }
};

Z80DASM::Z80DASM()
{
	// register stack
	memset(regs, 0, sizeof(regs));
	current_reg = &regs[0];
	current_idx = 0;
	codelen = 0;
	codepos = 0;
}

Z80DASM::~Z80DASM()
{
}

void Z80DASM::ini_pc(uint16_t pc, uint8_t code)
{
	uint32_t inphyaddr = 0;
	if (d_mem) {
		inphyaddr = d_mem->debug_latch_address(pc);
	}
	push_stack_pc(inphyaddr, pc, code);
}

void Z80DASM::push_stack_pc(uint32_t phyaddr, uint16_t pc, uint8_t code)
{
	uint16_t prev_state = (uint16_t)current_reg->state;

	current_idx = (current_idx + 1) % Z80DASM_PCSTACK_NUM;

	current_reg = &regs[current_idx];

	current_reg->phyaddr = phyaddr;
	current_reg->pc      = pc;
	current_reg->code[0] = code;
	current_reg->state   = prev_state;

	codelen = 1;

	current_reg->flags = 1;
}

int Z80DASM::get_stack_pc(int index, z80dasm_regs_t *stack)
{
	if (index < 0) return -2;
	if (index >= Z80DASM_PCSTACK_NUM) index = Z80DASM_PCSTACK_NUM - 1;

	int reg_index = (current_idx - index + Z80DASM_PCSTACK_NUM) % Z80DASM_PCSTACK_NUM;
	bool exist = false;
	do {
		if (regs[reg_index].flags & 1) {
			exist = true;
			break;
		} 
		reg_index++;
		if (reg_index >= Z80DASM_PCSTACK_NUM) reg_index = 0;
	} while(reg_index != current_idx);
	if (!exist) return -2;

	if (stack) *stack = regs[reg_index];

	index--;
	return index;
}

void Z80DASM::set_mem(uint16_t addr, uint8_t data, bool write)
{
	if (d_mem) {
		current_reg->rw_phyaddr = d_mem->debug_latch_address(addr);
	}
	current_reg->rw_addr = addr;
	current_reg->rw_data = data;
	current_reg->flags &= 0xf1;
	current_reg->flags |= 2;
	if (write) current_reg->flags |= 4;
}

void Z80DASM::set_mem(uint16_t addr, uint16_t data, bool write)
{
	if (d_mem) {
		current_reg->rw_phyaddr = d_mem->debug_latch_address(addr);
	}
	current_reg->rw_addr = addr;
	current_reg->rw_data = data;
	current_reg->flags &= 0xf1;
	current_reg->flags |= 0xa;
	if (write) current_reg->flags |= 4;
}

void Z80DASM::set_phymem(uint32_t addr, uint8_t data, bool write)
{
	current_reg->rw_phyaddr = addr;
}

void Z80DASM::set_code8(uint8_t code)
{
	current_reg->code[codelen] = code;
	codelen++;
}

void Z80DASM::set_code16(uint16_t code)
{
	current_reg->code[codelen] = (code & 0xff);
	codelen++;
	current_reg->code[codelen] = (code >> 8);
	codelen++;
}

void Z80DASM::set_signal(uint32_t state)
{
	if ((current_reg->state & Z80_STATE_BUSREQ) != 0 && (state & Z80_STATE_BUSREQ) == 0) {
		// When release BUSREQ, set release flag.
		state |= ((current_reg->state & 0xffff0000) | Z80_STATE_BUSREQ_REL);
	}
	if ((current_reg->state & Z80_STATE_BUSACK) != 0 && (state & Z80_STATE_BUSACK) == 0) {
		// When release BUSACK, set release flag.
		state |= ((current_reg->state & 0xffff0000) | Z80_STATE_BUSACK_REL);
	}
	current_reg->state = state;
}

void Z80DASM::set_regs(int accum, int cycles, uint16_t af, uint16_t bc, uint16_t de, uint16_t hl, uint16_t af2, uint16_t bc2, uint16_t de2, uint16_t hl2, uint16_t ix, uint16_t iy, uint16_t sp, uint8_t i, uint8_t r)
{
//	start_accum = accum;
//	expended_cycles = cycles;
	current_reg->cycles = cycles;
	current_reg->af = af;
	current_reg->bc = bc;
	current_reg->de = de;
	current_reg->hl = hl;
	current_reg->af2 = af2;
	current_reg->bc2 = bc2;
	current_reg->de2 = de2;
	current_reg->hl2 = hl2;
	current_reg->ix = ix;
	current_reg->iy = iy;
	current_reg->sp = sp;
	current_reg->i = i;
	current_reg->r = r;
}

const Z80DASM::opcode_t *Z80DASM::find_opcode(const Z80DASM::opcode_t *list, size_t start, size_t end, uint8_t code)
{
	if (start > end) return NULL;
	size_t pos = (end + 1 - start) / 2 + start;
	const opcode_t *itm = &list[pos];
	if (itm->s && itm->c == code) {
		return itm;
	}
	if (start == end) return NULL;
	if (itm->c > code) {
		return find_opcode(list, start, pos-1, code);
	} else {
		return find_opcode(list, pos+1, end, code);
	}
}

//uint8_t Z80DASM::dasm_fetchop()
//{
//	return current_reg->code[codepos++];
//}

//uint8_t Z80DASM::debug_fetch8()
//{
//	return current_reg->code[codepos++];
//}

//int8_t Z80DASM::debug_fetch8_rel()
//{
//	return (int8_t)current_reg->code[codepos++];
//}

uint16_t Z80DASM::debug_fetch16(const uint8_t *ops)
{
	uint16_t val = ops[0] | ((uint16_t)ops[1] << 8);
	return val;
}

uint16_t Z80DASM::debug_fetch8_relpc(uint32_t pc, uint8_t ops)
{
	return pc + (int8_t)ops;
}

/// @param [in] phy_addr: physical address converted from pc
/// @param [in] pc      : program counter
/// @param [in] ops     : op code
/// @param [in] opslen  : op code length
/// @param [in] flags   : bit5: 1=calc rel addr using phy_addr
int Z80DASM::print_dasm(uint32_t phy_addr, uint16_t pc, const uint8_t *ops, int opslen, int flags)
{
	line[0] = _T('\0');

	int opspos = dasm_first(phy_addr, pc, ops, opslen);

#ifdef _MBS1
	// xxxxx xxxx op op op op    LD A, 0ABH        ; COMMENT
	UTILITY::sntprintf(line, sizeof(line) / sizeof(line[0]), _T("%05X %04X"), phy_addr, pc);
#else
	UTILITY::sntprintf(line, sizeof(line) / sizeof(line[0]), _T("%04X"), pc);
#endif
	for(int i=0; i < opslen; i++) {
		if (i < opspos) {
			UTILITY::sntprintf(line, sizeof(line) / sizeof(line[0]), _T(" %02X"), ops[i]);
		} else {
			UTILITY::tcscat(line, sizeof(line) / sizeof(line[0]), _T("   "));
		}
	}
	UTILITY::sntprintf(line, sizeof(line) / sizeof(line[0]), _T("    %-18s"), cmd);

	return opspos;
}

int Z80DASM::print_dasm_label(int type, uint32_t pc)
{
	const _TCHAR *label = debugger->find_symbol_name(pc);
	if (label) {
#ifdef _MBS1
		UTILITY::tcscpy(line, sizeof(line) / sizeof(line[0]), _T("                      "));
#else
		UTILITY::tcscpy(line, sizeof(line) / sizeof(line[0]), _T("                "));
#endif
		UTILITY::tcscat(line, sizeof(line) / sizeof(line[0]), label);
		UTILITY::tcscat(line, sizeof(line) / sizeof(line[0]), _T(":"));
		return 1;
	}
	return 0;
}

int Z80DASM::print_dasm_preprocess(int type, uint32_t pc, int flags)
{
	// next opcode
	uint8_t ops[4];
	uint32_t phy_addr = 0;
#ifdef _MBS1
	if (type >= 0) {
		phy_addr = pc;
		if (type == 0) {
			pc = d_mem->debug_address_mapping_rev(pc);
		}
		for(int i = 0; i < 4; i++) {
			ops[i] = d_mem->debug_read_data8(type, phy_addr + i);
		}
	} else {
		phy_addr = d_mem->debug_latch_address(pc);
		for(int i = 0; i < 4; i++) {
			ops[i] = d_mem->debug_read_data8(-1, pc + i);
		}
	}
#else
	phy_addr = pc;
	for(int i = 0; i < 4; i++) {
		ops[i] = d_mem->debug_read_data8(type, pc + i);
	}
#endif
	line[0]=0;
	int opspos = print_dasm(phy_addr, pc & 0xffff, ops, 4, flags);
	return opspos;
}

int Z80DASM::print_dasm_processed(uint16_t pc)
{
	line[0]=0;
	int opspos = print_dasm(current_reg->phyaddr, current_reg->pc, current_reg->code, 4, 0);
	print_cycles(current_reg->cycles);
	print_memory_data(*current_reg);
	return opspos;
}

int Z80DASM::print_dasm_traceback(int index)
{
	z80dasm_regs_t stack;
	int next = get_stack_pc(index, &stack);
	if (next >= -1) {
		line[0]=0;
		print_dasm(stack.phyaddr, stack.pc & 0xffff, stack.code, 4, 0);
		print_cycles(stack.cycles);
		print_memory_data(stack);
	}
	return next;
}

void Z80DASM::print_cycles(int cycles)
{
	// expended machine cycles 
	UTILITY::sntprintf(line, _MAX_PATH, _T(";(%2d)")
		,cycles
	);
}

void Z80DASM::print_regs(const z80dasm_regs_t &regs)
{
/*
F = [--------]  A = 00  BC = 0000  DE = 0000  HL = 0000  IX = 0000  IY = 0000
F'= [--------]  A'= 00  BC'= 0000  DE'= 0000  HL'= 0000  SP = 0000  PC = 0000
        I = 00  R = 00 <EI>
*/
//	int wait;
	UTILITY::stprintf(line, sizeof(line) / sizeof(line[0]),
	_T("F = [%c%c%c%c%c%c%c%c]  A = %02X  BC = %04X  DE = %04X  HL = %04X  IX = %04X  IY = %04X\n")
	_T("F'= [%c%c%c%c%c%c%c%c]  A'= %02X  BC'= %04X  DE'= %04X  HL'= %04X  SP = %04X  PC = %04X\n")
	_T("        I = %02X  R = %02X")
/*	_T("Clocks = %llu (%llu) Since Scanline = %d/%d (%d/%d)") */, 
	(regs.af & CF) ? _T('C') : _T('-'), (regs.af & NF) ? _T('N') : _T('-'), (regs.af & PF) ? _T('P') : _T('-'), (regs.af & XF) ? _T('X') : _T('-'),
	(regs.af & HF) ? _T('H') : _T('-'), (regs.af & YF) ? _T('Y') : _T('-'), (regs.af & ZF) ? _T('Z') : _T('-'), (regs.af & SF) ? _T('S') : _T('-'),
	(regs.af >> 8), regs.bc, regs.de, regs.hl, regs.ix, regs.iy,
	(regs.af2 & CF) ? _T('C') : _T('-'), (regs.af2 & NF) ? _T('N') : _T('-'), (regs.af2 & PF) ? _T('P') : _T('-'), (regs.af2 & XF) ? _T('X') : _T('-'),
	(regs.af2 & HF) ? _T('H') : _T('-'), (regs.af2 & YF) ? _T('Y') : _T('-'), (regs.af2 & ZF) ? _T('Z') : _T('-'), (regs.af2 & SF) ? _T('S') : _T('-'),
	(regs.af2 >> 8), regs.bc2, regs.de2, regs.hl2, regs.sp, regs.pc,
	regs.i, regs.r);
//	total_icount, total_icount - prev_total_icount);
//	get_passed_clock_since_vline(), get_cur_vline_clocks(), get_cur_vline(), get_lines_per_frame());
//	prev_total_icount = total_icount;
//	return true;

	UTILITY::tcscat(line, sizeof(line) / sizeof(line[0]), (regs.state & Z80_STATE_EI) ? _T(" <EI>") : _T(" <DI>"));
	if (regs.state & Z80_STATE_HALT)   UTILITY::tcscat(line, sizeof(line) / sizeof(line[0]), _T(" <HALT>"));
	if (regs.state & Z80_STATE_RESET)  UTILITY::tcscat(line, sizeof(line) / sizeof(line[0]), _T(" <RESET>"));
	if (regs.state & Z80_STATE_NMI)    UTILITY::tcscat(line, sizeof(line) / sizeof(line[0]), _T(" <NMI>"));
	if (regs.state & Z80_STATE_INTR)   UTILITY::tcscat(line, sizeof(line) / sizeof(line[0]), _T(" <INTR>"));
	if (regs.state & (Z80_STATE_BUSREQ | Z80_STATE_BUSREQ_REL)) UTILITY::tcscat(line, sizeof(line) / sizeof(line[0]), _T(" <BUSREQ>"));
	if (regs.state & (Z80_STATE_BUSACK | Z80_STATE_BUSACK_REL)) UTILITY::tcscat(line, sizeof(line) / sizeof(line[0]), _T(" <BUSACK>"));
}

int Z80DASM::print_regs_traceback(int index)
{
	z80dasm_regs_t stack;
	int next = get_stack_pc(index, &stack);
	if (next >= -1) {
		print_regs(stack);
	}
	return next;
}

void Z80DASM::print_memory_data(z80dasm_regs_t &stack)
{
	// memory data
	if ((stack.flags & 2) == 0) return;

	if ((stack.flags & 8) == 0) {
		UTILITY::sntprintf(line, _MAX_PATH
#if 0 // def _MBS1
			, _T(" ;(%c %05X %04X:%02X)")
			, (stack.flags & 4) ? _T('W') : _T('R')
			, stack.rw_phyaddr
#else
			, _T(" ;(%c %04X:%02X)")
			, (stack.flags & 4) ? _T('W') : _T('R')
#endif
			, stack.rw_addr, stack.rw_data & 0xff);
	} else {
		UTILITY::sntprintf(line, _MAX_PATH
#if 0 // def _MBS1
			, _T(" ;(%c %05X %04X:%04X)")
			, (stack.flags & 4) ? _T('W') : _T('R')
			, stack.rw_phyaddr
#else
			, _T(" ;(%c %04X:%04X)")
			, (stack.flags & 4) ? _T('W') : _T('R')
#endif
			, stack.rw_addr, stack.rw_data);
	}
}

const _TCHAR *Z80DASM::format_digit(uint32_t val, int len)
{
	static _TCHAR str[16];
	_TCHAR fmt[16];
	if (val >= ((uint32_t)0xa << ((len-1) << 2))) {
		len++;
	}
	UTILITY::stprintf(fmt, 16, _T("%%0%dXH"), len);
	UTILITY::stprintf(str, 16, fmt, val);
	return str;
}

const _TCHAR *Z80DASM::format_offset(int8_t val)
{
	static _TCHAR str[16];
	UTILITY::stprintf(str, 16, _T("%+d"), (int)val);
	return str;
}

int Z80DASM::select_dasm_type(uint32_t pc, const uint8_t *ops, int opslen, en_operate type, const _TCHAR *mnemonic, _TCHAR *buffer, size_t buffer_len)
{
	const _TCHAR *operand, *offset;
	uint32_t val;
	int opspos = 0;

	pc &= 0xffff;

	switch(type) {
	case FETCH8:
	case FETCHID8:
		operand = format_digit(ops[opspos++], 2);
		UTILITY::stprintf(buffer, buffer_len, mnemonic, operand);
		break;
	case FETCHREL8:
		offset = format_offset((int8_t)ops[opspos++]);
		UTILITY::stprintf(buffer, buffer_len, mnemonic, offset);
		break;
	case FETCHREL88:
		offset = format_offset((int8_t)ops[opspos++]);
		operand = format_digit(ops[opspos++], 2);
		UTILITY::stprintf(buffer, buffer_len, mnemonic, offset, operand);
		break;
	case FETCH16:
	case FETCHID16:
		val = debug_fetch16(&ops[opspos]);
		opspos += 2;
		operand = debugger->get_label_or_symbol(format_digit(val, 4), val);
		UTILITY::stprintf(buffer, buffer_len, mnemonic, operand);
		break;
	case JMPREL8:
		val = debug_fetch8_relpc(pc + 1, ops[opspos++]);
		debugger->cat_label_or_symbol(buffer, buffer_len, mnemonic, format_digit(val, 4), val);
//		operand = debugger->get_label_or_symbol(format_digit(val, 4), val);
//		UTILITY::stprintf(buffer, buffer_len, mnemonic, operand);
		break;
	default:
		// no operand
		UTILITY::tcscpy(buffer, buffer_len, mnemonic);
		break;
	}
	return opspos;
}

void Z80DASM::print_regs_current()
{
	print_regs(*current_reg);
}

void Z80DASM::print_regs_current(uint16_t pc, uint16_t af, uint16_t bc, uint16_t de, uint16_t hl, uint16_t af2, uint16_t bc2, uint16_t de2, uint16_t hl2, uint16_t ix, uint16_t iy, uint16_t sp, uint8_t i, uint8_t r, uint32_t state)
{
	z80dasm_regs_t reg;
	reg.pc = pc;
	reg.af = af;
	reg.bc = bc;
	reg.de = de;
	reg.hl = hl;
	reg.af2 = af2;
	reg.bc2 = bc2;
	reg.de2 = de2;
	reg.hl2 = hl2;
	reg.ix = ix;
	reg.iy = iy;
	reg.sp = sp;
	reg.i = i;
	reg.r = r;
	reg.state = (uint16_t)state;

	print_regs(reg);
}

int Z80DASM::dasm_first(uint32_t phy_addr, uint32_t pc, const uint8_t *ops, int opslen)
{
	cmd[0] = _T('\0');
	const opcode_t *item = NULL;

	int opspos = 0;
	uint8_t code = ops[opspos++];

	switch(code) {
	case 0xcb:
		opspos += dasm_cb(pc, &ops[opspos], opslen - opspos, cmd, sizeof(cmd) / sizeof(cmd[0]));
		break;
	case 0xdd:
		opspos += dasm_dd(pc, &ops[opspos], opslen - opspos, cmd, sizeof(cmd) / sizeof(cmd[0]));
		break;
	case 0xed:
		opspos += dasm_ed(pc, &ops[opspos], opslen - opspos, cmd, sizeof(cmd) / sizeof(cmd[0]));
		break;
	case 0xfd:
		opspos += dasm_fd(pc, &ops[opspos], opslen - opspos, cmd, sizeof(cmd) / sizeof(cmd[0]));
		break;
	default:
		item = &opcodes0[code];
		if (item->s) {
			opspos += select_dasm_type(pc + opspos, &ops[opspos], opslen - opspos, item->m, item->s, cmd, sizeof(cmd) / sizeof(cmd[0]));
		}
		break;
	}
	return opspos;
}

int Z80DASM::dasm_cb(uint32_t pc, const uint8_t *ops, int opslen, _TCHAR *buffer, size_t buffer_len)
{
	const opcode_t *item = NULL;

	int opspos = 0;
	uint8_t code = ops[opspos++];

	item = find_opcode(opcodes_cb, 0, sizeof(opcodes_cb) / sizeof(opcodes_cb[0]), code);
	if (item) {
		opspos += select_dasm_type(pc + opspos, &ops[opspos], opslen - opspos, item->m, item->s, buffer, buffer_len);
	}
	return opspos;
}

int Z80DASM::dasm_dd(uint32_t pc, const uint8_t *ops, int opslen, _TCHAR *buffer, size_t buffer_len)
{
	const opcode_t *item = NULL;

	int opspos = 0;
	uint8_t code = ops[opspos++];

	switch(code) {
	case 0xcb:
		opspos += dasm_ddcb(pc, &ops[opspos], opslen - opspos, buffer, buffer_len);
		break;
	default:
		item = find_opcode(opcodes_dd, 0, sizeof(opcodes_dd) / sizeof(opcodes_dd[0]), code);
		if (item) {
			opspos += select_dasm_type(pc + opspos, &ops[opspos], opslen - opspos, item->m, item->s, buffer, buffer_len);
		} else {
			// unknown opcode
			UTILITY::tcscpy(buffer, buffer_len, _T("DB dd"));
			opspos = 0;
		}
		break;
	}
	return opspos;
}

int Z80DASM::dasm_ed(uint32_t pc, const uint8_t *ops, int opslen, _TCHAR *buffer, size_t buffer_len)
{
	const opcode_t *item = NULL;

	int opspos = 0;
	uint8_t code = ops[opspos++];

	item = find_opcode(opcodes_ed, 0, sizeof(opcodes_ed) / sizeof(opcodes_ed[0]), code);
	if (item) {
		opspos += select_dasm_type(pc + opspos, &ops[opspos], opslen - opspos, item->m, item->s, buffer, buffer_len);
	} else {
		// unknown opcode
		UTILITY::tcscpy(buffer, buffer_len, _T("DB ed"));
		opspos = 0;
	}
	return opspos;
}

int Z80DASM::dasm_fd(uint32_t pc, const uint8_t *ops, int opslen, _TCHAR *buffer, size_t buffer_len)
{
	const opcode_t *item = NULL;

	int opspos = 0;
	uint8_t code = ops[opspos++];

	switch(code) {
	case 0xcb:
		opspos += dasm_fdcb(pc, &ops[opspos], opslen - opspos, buffer, buffer_len);
		break;
	default:
		item = find_opcode(opcodes_fd, 0, sizeof(opcodes_fd) / sizeof(opcodes_fd[0]), code);
		if (item) {
			opspos += select_dasm_type(pc + opspos, &ops[opspos], opslen - opspos, item->m, item->s, buffer, buffer_len);
		} else {
			// unknown opcode
			UTILITY::tcscpy(buffer, buffer_len, _T("DB fd"));
			opspos = 0;
		}
		break;
	}
	return opspos;
}

int Z80DASM::dasm_ddcb(uint32_t pc, const uint8_t *ops, int opslen, _TCHAR *buffer, size_t buffer_len)
{
	const opcode_t *item = NULL;

	int opspos = 0;
	uint8_t ofs = ops[opspos++];
	uint8_t code = ops[opspos++];

	item = find_opcode(opcodes_ddcb, 0, sizeof(opcodes_ddcb) / sizeof(opcodes_ddcb[0]), code);
	if (item) {
		select_dasm_type(pc + opspos, &ofs, 1, item->m, item->s, buffer, buffer_len);
	}
	return opspos;
}

int Z80DASM::dasm_fdcb(uint32_t pc, const uint8_t *ops, int opslen, _TCHAR *buffer, size_t buffer_len)
{
	const opcode_t *item = NULL;

	int opspos = 0;
	uint8_t ofs = ops[opspos++];
	uint8_t code = ops[opspos++];

	item = find_opcode(opcodes_fdcb, 0, sizeof(opcodes_fdcb) / sizeof(opcodes_fdcb[0]), code);
	if (item) {
		select_dasm_type(pc + opspos, &ofs, 1, item->m, item->s, buffer, buffer_len);
	}
	return opspos;
}

#endif /* USE_DEBUGGER */
