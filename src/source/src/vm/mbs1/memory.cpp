/** @file memory.cpp

	HITACHI MB-S1 Emulator 'EmuB-S1'
	Skelton for retropc emulator

	@author Sasaji
	@date   2011.06.08 -

	@brief [ memory ]
*/

#include "memory.h"
#include <stdlib.h>
#include <time.h>
//#include "../../emu.h"
#include "../vm.h"
#include "../../config.h"
#include "../event.h"
#include "../hd46505.h"
#include "sound.h"
#include "display.h"
#include "comm.h"
#include "cmt.h"
#include "../pia.h"
#include "../via.h"
#include "../../fileio.h"
#include "../../logging.h"
#include "../../utility.h"
#if defined(USE_Z80B_CARD)
#include "z80b_card.h"
#elif defined(USE_MPC_68008)
#include "mpc_68008.h"
#endif
#ifdef USE_DEBUGGER
#include "l3basic.h"
#include "../../osd/debugger_console.h"
#endif

#ifdef _DEBUG
//#define _DEBUG_RAM
//#define _DEBUG_CRAM
//#define _DEBUG_IO_COMMON
//#define _DEBUG_TRACE_COUNTER
#endif

#define IGRAM_SIZE				0x800	// 2KB
#define IGRAM_SIZE_1			0x7ff	// 2KB-1
#define FONT_SIZE_1				0xfff	// 4KB-1

#define S1_ADDR_IGRAM_BANK1		0xbd000
#define S1_ADDR_IGRAM_BANK2		0xbe000
#define S1_ADDR_IGRAM_START		0xfa000
#define S1_ADDR_IGRAM_END		0xfa800

#define L3_ADDR_IGRAM_START		0xfa000
#define L3_ADDR_IGRAM_END		0xfa800

#define S1_ADDR_TRAP			0xbf000

#define VCRAM(addr) vcram[addr & S1_VTRAM_SIZE_1]
#define VGRAM_R(addr) ram[addr & S1_VGRAM_SIZE_ONE_1]
#define VGRAM_G(addr) vgram[addr & S1_VGRAM_SIZE_ONE_1]
#define VGRAM_B(addr) vgram[(addr & S1_VGRAM_SIZE_ONE_1)+S1_VGRAM_SIZE_ONE]
#define IGRAM(addr,brg) ig_ram[(addr & IGRAM_SIZE_1)+(IGRAM_SIZE*brg)]
#define FONTRAM(addr) font[addr & FONT_SIZE_1]

#define TRACE_COUNTER_PHASE_IDLE		0
#define TRACE_COUNTER_PHASE_WAITING		1
#define TRACE_COUNTER_PHASE_FIRED		2

MEMORY::MEMORY(VM* parent_vm, EMU* parent_emu, const char* identifier)
	: DEVICE(parent_vm, parent_emu, identifier)
{
	set_class_name("MEMORY");

//	num_of_cpus = 0;
//	for(int i=0; i<2; i++) {
//		d_cpu[i] = NULL;
//	}

#ifdef USE_DEBUGGER
	dc  = NULL;
	bas = NULL;
#endif
}

MEMORY::~MEMORY()
{
#ifdef USE_DEBUGGER
	delete bas;
#endif
}

void MEMORY::SET_BANK(int s, int e, uint8_t* w, uint8_t* r, uint8_t** wbank, uint8_t** rbank, int bank_size, uint8_t *waitbank, uint8_t waitval, uint8_t *banktype, uint8_t type) {
	int sb = (s) >> bank_size, eb = (e) >> bank_size;
	for(int i = sb; i <= eb; i++) {
		if((w) == wdmy) {
			wbank[i] = wdmy;
			if (banktype) banktype[i] &= 0x0f; 
		} else if ((w) != NULL) {
			wbank[i] = (w) + (1 << bank_size) * (i - sb);
			if (banktype) banktype[i] = (banktype[i] & 0x0f) | (type & 0xf0); 

		}
		if ((w) != NULL) {
			waitbank[i] = (waitval & 0xf0) | (waitbank[i] & 0x0f);
		}
		if((r) == rdmy) {
			rbank[i] = rdmy;
			if (banktype) banktype[i] &= 0xf0; 
		} else if ((r) != NULL) {
			rbank[i] = (r) + (1 << bank_size) * (i - sb);
			if (banktype) banktype[i] = (banktype[i] & 0xf0) | (type & 0x0f); 
		}
		if ((r) != NULL) {
			waitbank[i] = (waitval & 0x0f) | (waitbank[i] & 0xf0);
		}
	}
}
#define L3_SET_BANK(s, e, w, r, wv, ty) SET_BANK(s, e, w, r, l3wbank, l3rbank, L3_BANK_SIZE, l3wait, wv, l3banktype, ty)
#define S1_SET_BANK(s, e, w, r, wv, ty) SET_BANK(s, e, w, r, s1wbank, s1rbank, S1_BANK_SIZE, s1wait, wv, s1banktype, ty)

void MEMORY::initialize()
{
	l3rom_loaded = 0;
	s1rom1_loaded = 0;
	s1rom2_loaded = 0;
	s1dicrom_loaded = 0;
	rom1802_loaded = 0;
	rom1805_loaded = 0;
	rom_loaded_at_first = false;

	cm01rom_loaded = 0;
	cm01eeprom_loaded = 0;

	bfxxx_access = false;

	memset(l3rom, 0xff, sizeof(l3rom));
	memset(s1rom, 0xff, sizeof(s1rom));
	memset(s1dicrom, 0xff, sizeof(s1dicrom));
	memset(rdmy, 0xff, sizeof(rdmy));
	memset(rom1802, 0xff, sizeof(rom1802));
	memset(rom1805, 0xff, sizeof(rom1805));

	cm01rom = NULL;
	cm01eeprom = NULL;

	memset(addr_map, 0, sizeof(addr_map));

	exram_size = 0;
	exram = NULL;

	// read rom image from file
	load_rom_files();

	// clear trace counter
//	trace_counter_id = -1;

	// allocate physical address
	for(int i=0; i<(0x100000 >> S1_BANK_SIZE); i++) {
		s1wbank[i] = wdmy;
		s1rbank[i] = rdmy;
	}
	memset(s1wait, 0, sizeof(s1wait));
	memset(s1banktype, 0, sizeof(s1banktype));

	// init memory map
	reset_s1_membank();
	reset_cm01_membank();
	reset_l3_membank();

//	cpu_wait = 0;

	update_config();

#ifdef USE_DEBUGGER
	bas = new L3Basic(this);

	for(int i=0; i<64; i++) {
		prev_addr_map_enable[i] = false;
	}
	memset(prev_addr_map, 0, sizeof(prev_addr_map));
#endif
}

///
/// power on reset
///
void MEMORY::reset()
{
	// save eeprom
	save_rom_files();
	// read rom image from file
	load_rom_files();

	// reset address segment
	REG_ADDRSEG = 0;

	// s1 or l3 mode
	reset_s1_l3_mode();

	memset(ram , 0, sizeof(ram));

	memset(vgram , 0, sizeof(vgram));
	reset_sram(vtram , sizeof(vtram));
	reset_sram(vcram , sizeof(vcram));

	// clear color ram
	srand((unsigned int)time(NULL));
	int co = (rand() % 3) * 3 + 1;	// 1 4 7
//	int cg = (rand() % 5);
	color_reg = co + (co == 7 ? 24 : 0);

	// clear address map
	memset(addr_map, 0, sizeof(addr_map));

	// reallocate extended ram
	set_extended_ram();

	// clear ig ram
	reset_igram();

	// init memory map
	reset_s1_membank();
	reset_cm01_membank();
	reset_l3_membank();

	// stop trace counter
	reset_trace_counter();
}

void MEMORY::release()
{
	// save eeprom
	save_rom_files();

	delete [] exram;
	delete [] cm01eeprom;
	delete [] cm01rom;
}

void MEMORY::load_rom_files()
{
	// load rom
	const _TCHAR *app_path, *rom_path[2];

	rom_path[0] = pConfig->rom_path.Get();
	rom_path[1] = vm->application_path();

	for(int i=0; i<2; i++) {
		app_path = rom_path[i];

		if (!l3rom_loaded) {
			l3rom_loaded = vm->load_data_from_file(app_path, _T("L3BAS.ROM"), l3rom, 0x6000, (const uint8_t *)"\xc4\x5c", 2);
		}
		if (!l3rom_loaded) {
			// try to load another main rom.
			l3rom_loaded = vm->load_data_from_file(app_path, _T("ROM1.ROM"), l3rom, 0x5f00, (const uint8_t *)"\xc4\x5c", 2);
			if (l3rom_loaded) {
				l3rom_loaded = vm->load_data_from_file(app_path, _T("ROM2.ROM"), &l3rom[0x5ff0], 16, (const uint8_t *)"\x00\x00\x01\x00", 4);
			}
		}
		if (!s1rom1_loaded) {
			s1rom1_loaded = vm->load_data_from_file(app_path, _T("S1BAS1.ROM"), s1rom, 0x8000, (const uint8_t *)"\xee\xc5", 2);
		}
		if (!s1rom2_loaded) {
			s1rom2_loaded = vm->load_data_from_file(app_path, _T("S1BAS2.ROM"), &s1rom[0x8000], 0x8000, (const uint8_t *)"\x31\xc6", 2);
		}
		if (!s1rom2_loaded) {
			s1rom2_loaded = vm->load_data_from_file(app_path, _T("S1ROM2.ROM"), &s1rom[0x8000], 0x7e00, (const uint8_t *)"\x31\xc6", 2);
			if (s1rom2_loaded) {
				s1rom2_loaded = vm->load_data_from_file(app_path, _T("S1ROMI.ROM"), &s1rom[0xfff0], 16, (const uint8_t *)"\xff\xff\xee\xfd", 4);
			}
		}
		if (!s1dicrom_loaded) {
			s1dicrom_loaded = vm->load_data_from_file(app_path, _T("S1DIC.ROM"), s1dicrom, 0x8000, (const uint8_t *)"\xb1\x00", 2);
		}
		if (!rom1802_loaded) {
			rom1802_loaded = vm->load_data_from_file(app_path, _T("MP1802.ROM"), rom1802, 0x800);
		}
		if (!rom1805_loaded) {
			rom1805_loaded = vm->load_data_from_file(app_path, _T("MP1805.ROM"), rom1805, 0x800);
		}
		if (IOPORT_USE_CM01 && !cm01rom_loaded) {
			if (cm01rom == NULL) {
				cm01rom = new uint8_t[0x8000];	// 32KB
				memset(cm01rom, 0xff, 0x8000);
			}
			cm01rom_loaded = vm->load_data_from_file(app_path, _T("MPC-CM01.ROM"), cm01rom, 0x8000);
		}
	}

	app_path = vm->initialize_path();

	if (IOPORT_USE_CM01) {
		if (!cm01eeprom_loaded) {
			if (cm01eeprom == NULL) {
				cm01eeprom = new uint8_t[0x2000];	// 8KB
				memset(cm01eeprom, 0xff, 0x2000);
			}
			cm01eeprom_loaded = vm->load_data_from_file(app_path, _T("mpc-cm01.eeprom"), cm01eeprom, 0x2000);
		}
	} else {
		if (cm01eeprom != NULL) {
			delete [] cm01eeprom;
			cm01eeprom = NULL;
		}
		cm01eeprom_loaded = 0;
	}

	if (!rom_loaded_at_first) {
		if (!l3rom_loaded) {
			logging->out_logf_x(LOG_WARN, CMsg::VSTR_couldn_t_be_loaded, _T("L3BAS.ROM"));
			logging->out_logf_x(LOG_WARN, CMsg::VSTR_couldn_t_be_loaded, _T("ROM1.ROM"));
			logging->out_logf_x(LOG_WARN, CMsg::VSTR_couldn_t_be_loaded, _T("ROM2.ROM"));
		}
		if (!s1rom1_loaded) {
			logging->out_logf_x(LOG_WARN, CMsg::VSTR_couldn_t_be_loaded, _T("S1BAS1.ROM"));
		}
		if (!s1rom2_loaded) {
			logging->out_logf_x(LOG_WARN, CMsg::VSTR_couldn_t_be_loaded, _T("S1BAS2.ROM"));
			logging->out_logf_x(LOG_WARN, CMsg::VSTR_couldn_t_be_loaded, _T("S1ROM2.ROM"));
			logging->out_logf_x(LOG_WARN, CMsg::VSTR_couldn_t_be_loaded, _T("S1ROMI.ROM"));
		}
		if (!s1dicrom_loaded) {
			logging->out_logf_x(LOG_WARN, CMsg::VSTR_couldn_t_be_loaded, _T("S1DIC.ROM"));
		}
		if (!rom1802_loaded) {
			logging->out_logf_x(LOG_WARN, CMsg::VSTR_couldn_t_be_loaded, _T("MP1802.ROM"));
		}
		if (!rom1805_loaded) {
			logging->out_logf_x(LOG_WARN, CMsg::VSTR_couldn_t_be_loaded, _T("MP1805.ROM"));
		}
		if (!cm01rom_loaded) {
			logging->out_logf_x(LOG_WARN, CMsg::VSTR_couldn_t_be_loaded, _T("MPC-CM01.ROM"));
		}
		rom_loaded_at_first = true;
	}
}

void MEMORY::save_rom_files()
{
	// save eeprom
	_TCHAR file_path[_MAX_PATH];

	FILEIO* fio = new FILEIO();
	if (cm01eeprom != NULL) {
		UTILITY::stprintf(file_path, _MAX_PATH, _T("%smpc-cm01.eeprom"), vm->initialize_path());
		if(fio->Fopen(file_path, FILEIO::WRITE_BINARY)) {
			fio->Fwrite(cm01eeprom, 0x2000, 1);
			fio->Fclose();
			logging->out_logf_x(LOG_INFO, CMsg::VSTR_was_saved, _T("mpc-cm01.eeprom"));
		}
	}
	delete fio;
}

void MEMORY::reset_sram(uint8_t *sram, size_t size)
{
	for(size_t i=0; i<size; i+=2) {
		sram[i]=(rand() & 0xff);
		sram[i+1]=0xff;
	}
}

void MEMORY::reset_igram()
{
	// clear ig ram data
	for(int co=0; co<3; co++) {
		for(int ch=0; ch<256; ch++) {
			for(int i=0; i<8; i+=2) {
				ig_ram[(co*256+ch)*8+i]  =0x00;
				ig_ram[(co*256+ch)*8+i+1]=0xff;
				if ((ch % 3) == co) {
					switch(i) {
						case 0:
							if ((ch >> 4) < 8) ig_ram[(co*256+ch)*8+i]=(1 << (ch >> 4));
							break;
						case 2:
							if ((ch >> 4) < 16) ig_ram[(co*256+ch)*8+i]=(1 << ((ch >> 4) - 8));
							break;
						case 4:
							if ((ch & 15) < 8) ig_ram[(co*256+ch)*8+i]=(1 << (ch & 15));
							break;
						case 6:
							if ((ch & 15) < 16) ig_ram[(co*256+ch)*8+i]=(1 << ((ch & 15) - 8));
							break;
					}
				}
			}
		}
	}
//	ig_enable = 0;
	ig_enreg = 0;
}

void MEMORY::reset_s1_membank()
{
	// main ram 0x84000 - 0x8ffff
	S1_SET_BANK(0x84000, 0x8ffff, ram + 0x4000, ram + 0x4000, 0x77, 0x11);
	// graphic ram 0xb0000 - 0xb3fff
	S1_SET_BANK(0xb0000, 0xb3fff, ram, ram, 0x77, 0x44);
	// graphic ram 0xb4000 - 0xb7fff,0xb8000 - 0xbbfff (use in text only mode)
	S1_SET_BANK(0xb4000, 0xbbfff, vgram, vgram, 0x77, 0x55);
	// text ram 0xbc000 - 0xbc7ff
	S1_SET_BANK(0xbc000, 0xbc7ff, vtram, vtram, 0x55, 0x66);
	S1_SET_BANK(0xbc800, 0xbcfff, vtram, vtram, 0x55, 0x66);
//	// font rom 0xbd000 - 0xbdfff
//	S1_SET_BANK(0xbd000, 0xbdfff, wdmy, font);
	// dictionary rom 0xd0000 - 0xd7fff
	if (s1dicrom_loaded) S1_SET_BANK(0xd0000, 0xd7fff, wdmy, s1dicrom, 0, 0x0d);
	// basic rom 0xe0000 - 0xefdff
	// system io 0xefe00 - 0xeffff
	if (s1rom1_loaded) S1_SET_BANK(0xe0000, 0xe7fff, wdmy, s1rom,          0, 0x0b);
	if (s1rom2_loaded) S1_SET_BANK(0xe8000, 0xeffff, wdmy, s1rom + 0x8000, 0, 0x0b);
	// main ram (image) 0xf0000 - 0xfffff 
	S1_SET_BANK(0xf0000, 0xfffff, ram, ram, 0x77, 0x11);
	// l3 basic rom 0xfa000 - 0xfffff
	if (l3rom_loaded) S1_SET_BANK(0xfa000, 0xfffff, ram, l3rom, 0x70, 0x18);
}

void MEMORY::reset_cm01_membank()
{
	cm01basic_enable = (IOPORT_USE_CM01 && cm01rom_loaded);
	cm01rom_access_ok = false;
	cm01eeprom_bank = 0;
	cm01eeprom_writing = false;

	change_cm01_membank();
}

void MEMORY::change_cm01_membank()
{
	if (IOPORT_USE_CM01 && cm01rom_loaded && cm01rom_access_ok) {
		// communication rom
		S1_SET_BANK(0xc0000, 0xc6fff, wdmy, cm01rom, 0, 0x0e);
		S1_SET_BANK(0xc7000, 0xc7fff, cm01eeprom + cm01eeprom_bank * 0x1000, cm01eeprom + cm01eeprom_bank * 0x1000, 0, 0x77);
	} else {
		S1_SET_BANK(0xc0000, 0xc7fff, wdmy, rdmy, 0, 0);
	}

	if (IOPORT_USE_CM01 && cm01rom_loaded && cm01basic_enable) {
		// set communication rom instead of s1rom
		S1_SET_BANK(0xe2000, 0xe2fff, wdmy, cm01rom + 0x7000, 0, 0x0e);
	} else {
		S1_SET_BANK(0xe2000, 0xe2fff, wdmy, s1rom + 0x2000, 0, 0x0b);
	}
}

void MEMORY::reset_l3_membank()
{
	// init memory map
	mem_bank_reg1 = 0x0e;
	mem_bank_mask = true;
	mem_bank_reg2 = 0;
	mem_vram_sel = true;
//	vram_sel = 0;
	change_l3_memory_bank();
}

void MEMORY::reset_trace_counter()
{
	// stop trace counter
//	if (trace_counter_id != -1) {
//		cancel_event(trace_counter_id);
//	}
//	trace_counter_id = -1;
	d_board->write_signal(SIG_CPU_NMI, 0, SIG_NMI_TRACE_MASK);
//	pc = 0;
	trace_counter_phase = TRACE_COUNTER_PHASE_IDLE;
	trace_count_remain = 0;
	prev_trace_clock = 0;

	// reset fuse
	REG_FUSE = 0x8f;
}

void MEMORY::set_igmode(uint32_t data)
{
//	ig_enable = data & 0x01;
	d_disp->write_io8(0xffe9, data);
}

#define set_s1_cpu_wait(addr_1, addr_seg04_1, addr_bank_1, addr_comio_1, ref_mask, disp_mask, wait) \
{ \
	if (NOW_2MHZ) { \
		if (((*wait) % 30) == 0) { \
			/* dram refresh */ \
			if ( \
				(0xeff00 <= addr_comio_1 && addr_comio_1 <= 0xeffef) \
				|| (!pConfig->mem_nowait && ( \
					(addr_1 >= 0xefe25 && addr_1 <= 0xefe27) \
				 || (s1wait[addr_bank_1] & ref_mask) \
				 || (addr_seg04_1 == S1_ADDR_TRAP) \
				   )) \
			) { \
				(*wait)++; \
				if (trace_counter_phase == TRACE_COUNTER_PHASE_WAITING) trace_count_remain++; \
			} \
		} else if (((*wait) & 1) == 0) { \
			/* type A wait */ \
			if ( \
				(0xeff00 <= addr_comio_1 && addr_comio_1 <= 0xeffef) \
				|| (!pConfig->mem_nowait && ( \
					(addr_1 >= 0xefe25 && addr_1 <= 0xefe27) \
				 || (DISPTMG_ON && DISP_PAGE_GRAPHIC_BRG && (s1wait[addr_bank_1] & disp_mask)) \
				 || (addr_seg04_1 == S1_ADDR_TRAP) \
				   )) \
			) { \
				(*wait)++; \
				if (trace_counter_phase == TRACE_COUNTER_PHASE_WAITING) trace_count_remain++; \
			} \
		} else { \
			/* type B wait */ \
			if ( \
				(0xeff00 <= addr_comio_1 && addr_comio_1 <= 0xeffef) \
				|| (!pConfig->mem_nowait && ( \
					(addr_1 >= 0xefe25 && addr_1 <= 0xefe27) \
				   )) \
			) { \
				(*wait)+=2; \
				if (trace_counter_phase == TRACE_COUNTER_PHASE_WAITING) trace_count_remain+=2; \
			} \
		} \
	} \
}

#define set_s1_cpu_wait_68k(addr_1, addr_seg04_1, addr_bank_1, addr_comio_1, ref_mask, disp_mask, wait) \
{ \
	if (NOW_2MHZ) { \
		if (((*wait) % 30) == 0) { \
			/* dram refresh */ \
			if ( \
				(0xeff00 <= addr_comio_1 && addr_comio_1 <= 0xeffef) \
				|| (!pConfig->mem_nowait && ( \
					(addr_1 >= 0xefe25 && addr_1 <= 0xefe27) \
				 || (s1wait[addr_bank_1] & ref_mask) \
				 || (addr_seg04_1 == S1_ADDR_TRAP) \
				   )) \
			) { \
				(*wait)++; \
			} \
		} else if (((*wait) & 1) == 0) { \
			/* type A wait */ \
			if ( \
				(0xeff00 <= addr_comio_1 && addr_comio_1 <= 0xeffef) \
				|| (!pConfig->mem_nowait && ( \
					(addr_1 >= 0xefe25 && addr_1 <= 0xefe27) \
				 || (DISPTMG_ON && DISP_PAGE_GRAPHIC_BRG && (s1wait[addr_bank_1] & disp_mask)) \
				 || (addr_seg04_1 == S1_ADDR_TRAP) \
				   )) \
			) { \
				(*wait)++; \
			} \
		} else { \
			/* type B wait */ \
			if ( \
				(0xeff00 <= addr_comio_1 && addr_comio_1 <= 0xeffef) \
				|| (!pConfig->mem_nowait && ( \
					(addr_1 >= 0xefe25 && addr_1 <= 0xefe27) \
				   )) \
			) { \
				(*wait)+=2; \
			} \
		} \
	} \
}

#define set_l3_cpu_wait(addr_1, addr_bank_1, addr_comio_1, ref_mask, disp_mask, wait) \
{ \
	if (NOW_2MHZ) { \
		if (((*wait) % 30) == 0) { \
			/* dram refresh */ \
			if ( \
				(0xeff00 <= addr_comio_1 && addr_comio_1 <= 0xeffef) \
				|| (!pConfig->mem_nowait && ( \
					(l3wait[addr_bank_1] & ref_mask) \
				   )) \
			) { \
				(*wait)++; \
				if (trace_counter_phase == TRACE_COUNTER_PHASE_WAITING) trace_count_remain++; \
			} \
		} else if (((*wait) & 1) == 0) { \
			/* type A wait */ \
			if ( \
				(0xeff00 <= addr_comio_1 && addr_comio_1 <= 0xeffef) \
				|| (!pConfig->mem_nowait && ( \
					(DISPTMG_ON && (l3wait[addr_bank_1] & disp_mask)) \
				   )) \
			) { \
				(*wait)++; \
				if (trace_counter_phase == TRACE_COUNTER_PHASE_WAITING) trace_count_remain++; \
			} \
		} else { \
			/* type B wait */ \
			if ( \
				(0xeff00 <= addr_comio_1 && addr_comio_1 <= 0xeffef) \
			) { \
				(*wait)+=2; \
				if (trace_counter_phase == TRACE_COUNTER_PHASE_WAITING) trace_count_remain+=2; \
			} \
		} \
	} \
}

#define set_l3_cpu_wait_68k(addr_1, addr_bank_1, addr_comio_1, ref_mask, disp_mask, wait) \
{ \
	if (NOW_2MHZ) { \
		if (((*wait) % 30) == 0) { \
			/* dram refresh */ \
			if ( \
				(0xeff00 <= addr_comio_1 && addr_comio_1 <= 0xeffef) \
				|| (!pConfig->mem_nowait && ( \
					(l3wait[addr_bank_1] & ref_mask) \
				   )) \
			) { \
				(*wait)++; \
			} \
		} else if (((*wait) & 1) == 0) { \
			/* type A wait */ \
			if ( \
				(0xeff00 <= addr_comio_1 && addr_comio_1 <= 0xeffef) \
				|| (!pConfig->mem_nowait && ( \
					(DISPTMG_ON && (l3wait[addr_bank_1] & disp_mask)) \
				   )) \
			) { \
				(*wait)++; \
			} \
		} else { \
			/* type B wait */ \
			if ( \
				(0xeff00 <= addr_comio_1 && addr_comio_1 <= 0xeffef) \
			) { \
				(*wait)+=2; \
			} \
		} \
	} \
}

/// write to memory and I/O with address mapping
/// @param[in] addr : logical address
/// @param[in] data : a byte data
/// @param[in,out] wait : add spent cycles
void MEMORY::write_data8w(uint32_t addr, uint32_t data, int *wait)
{
	addr = address_mapping(addr & 0xffff);

	addr_seg04 = (addr & 0xff000); // 4KB  block
	addr_seg16 = (addr & 0xfc000); // 16KB block
	addr_seg32 = (addr & 0xf8000); // 32KB block
	addr_comio = (addr & 0xeffff);

	if (addr < 0xf0000) {
		// S1 mode
		addr_bank = (addr >> S1_BANK_SIZE);

		// if access io port, cpu speed go down to 1MHz
		set_s1_cpu_wait(addr, addr_seg04, addr_bank, addr_comio, 0x20, 0x40, wait);

		if (write_data_s1(addr, data)) {
			return;
		}

#if defined(USE_Z80B_CARD) || defined(USE_MPC_68008)
		if (!SIG_MBC_IS_ON || addr >= 0x80000)
#endif
		{
			s1wbank[addr_bank][addr & ((1 << S1_BANK_SIZE) - 1)] = data;
		}
	} else {
		// L3 mode or upper 0xf0000
		addr_64kb = (addr & 0xffff);
		addr_bank = (addr_64kb >> L3_BANK_SIZE);

		// if access io port, cpu speed go down to 1MHz
		set_l3_cpu_wait(addr, addr_bank, addr_comio, 0x20, 0x40, wait);

		// vram area
		if (mem_vram_sel && NOW_L3_MODE && L3_ADDR_VRAM_START <= addr_64kb && addr_64kb < L3_ADDR_VRAM_END) {
#ifdef _DEBUG_CRAM
			{
				uint8_t ch=data;
				ch = (ch < 0x20 || ch > 0x7f) ? '.' : ch;
				logging->out_debugf("mw %05x=%02x->%02x %c creg:%02x cram:%02x",addr,ram[addr],data,ch,color_reg,vgram[(addr_64kb - L3_ADDR_VRAM_START) & L3_VRAM_SIZE_1]);
			}
#endif
			vgram[(addr_64kb - L3_ADDR_VRAM_START) & L3_VRAM_SIZE_1] = (color_reg & 0x3f);
		}
		// ig_area
		if (REG_IGENABLE) {
			if (L3_ADDR_IGRAM_START <= addr && addr < L3_ADDR_IGRAM_END) {
				for(int i=0; i<3; i++) {
					if (ig_enreg & (1 << i)) {
						IGRAM(addr,i) = data & 0xff;
					}
				}
				return;
			}
		}
		l3wbank[addr_bank][addr & ((1 << L3_BANK_SIZE) - 1)] = data;
	}

#undef DEBUG_WRITE_OK
#define WRITE_IO8 write_io8
#define WRITE_MEMORY_MAPPED_IO8 write_memory_mapped_io8
#define WRITE_SIGNAL write_signal
#include "memory_writeio.h"

#ifdef USE_DEBUGGER
	bas->SetTraceBack(addr);
#endif

	// send trap signal when access the address 0xbfxxx 
	if (addr_seg04 == S1_ADDR_TRAP) {
		bfxxx_access = true;
		REG_TRAPF |= TRAPF_SIGNAL;
		d_board->write_signal(SIG_CPU_NMI, SIG_NMI_TRAP_MASK, SIG_NMI_TRAP_MASK);
	} else if (bfxxx_access) {
		d_board->write_signal(SIG_CPU_NMI, 0, SIG_NMI_TRAP_MASK);
		bfxxx_access = false;
	}

}

/// write to S1 memory bus
/// @param[in] addr : physical address
/// @param[in] data : a byte data
/// @return true if finish writing
bool MEMORY::write_data_s1(uint32_t addr, uint32_t data)
{
#ifdef _DEBUG_IO_COMMON
	if ((addr & 0xfff00) == 0xeff00) {
		uint8_t ch = (data < 0x20 || data > 0x7f) ? 0x20 : data;
		logging->out_debugf("mw %04x=%02x %c",addr,data,ch);
	}
#endif
	// text vram
	if (addr_seg04 == S1_ADDR_VTRAM_START) {
		// copy color register to color ram (7bit)
		VCRAM(addr) = (color_reg & 0x7f);
#ifdef _DEBUG_CVRAM
		if ((addr & 0x3ff) >= 0x0 && (addr & 0x3ff) < 0x28) {
			uint8_t ch = (data < 0x20 || data > 0x7f) ? 0x20 : data;
			logging->out_debugf(_T("text w: %05x %02x %c c:%02x"),addr,data,ch,color_reg);
		}
#endif
	}
	// graphic vram
	if (addr_seg16 == S1_ADDR_VGRAM_START) {
		if ((SIG_BMSK_DW & 3) == 0) {
			// direct write
			if (ACTIVE_PLANE_R) VGRAM_R(addr) = (data & 0xff);
			if (ACTIVE_PLANE_G) VGRAM_G(addr) = (data & 0xff);
			if (ACTIVE_PLANE_B) VGRAM_B(addr) = (data & 0xff);
		} else if ((SIG_BMSK_DW & 3) == 1) {
			// indirect write
			if (ACTIVE_PLANE_R) VGRAM_R(addr) = REG_GCOLOR_R;
			if (ACTIVE_PLANE_G) VGRAM_G(addr) = REG_GCOLOR_G;
			if (ACTIVE_PLANE_B) VGRAM_B(addr) = REG_GCOLOR_B;
		} else if ((SIG_BMSK_DW & 2) == 2) {
			// indirect write with bit mask
#if 1
			uint8_t data_rev = ((~data) & 0xff);
			if (ACTIVE_PLANE_R) VGRAM_R(addr) = ((BMSK_COLOR_R ? (data & 0xff) : 0) | (REG_GCOLOR_R & data_rev));
			if (ACTIVE_PLANE_G) VGRAM_G(addr) = ((BMSK_COLOR_G ? (data & 0xff) : 0) | (REG_GCOLOR_G & data_rev));
			if (ACTIVE_PLANE_B) VGRAM_B(addr) = ((BMSK_COLOR_B ? (data & 0xff) : 0) | (REG_GCOLOR_B & data_rev));
#else
			if (ACTIVE_PLANE_R) VGRAM_R(addr) = 0;
			if (ACTIVE_PLANE_G) VGRAM_G(addr) = 0;
			if (ACTIVE_PLANE_B) VGRAM_B(addr) = 0;
			for(uint8_t dot=0x80; dot>0; dot >>= 1) {
				if (ACTIVE_PLANE_R) {
					if (data & dot) VGRAM_R(addr) |= (BMSK_COLOR_R ? dot : 0);
					else 			VGRAM_R(addr) |= (REG_GCOLOR_R & dot);
				}
				if (ACTIVE_PLANE_G) {
					if (data & dot) VGRAM_G(addr) |= (BMSK_COLOR_G ? dot : 0);
					else			VGRAM_G(addr) |= (REG_GCOLOR_G & dot);
				}
				if (ACTIVE_PLANE_B) {
					if (data & dot) VGRAM_B(addr) |= (BMSK_COLOR_B ? dot : 0);
					else			VGRAM_B(addr) |= (REG_GCOLOR_B & dot);
				}
			}
#endif
		}
#if 0	// color ram is not modified.
#ifdef _DEBUG_CVRAM
		if ((addr & 0x3ff) >= 0x0 && (addr & 0x3ff) < 0x28) {
			uint8_t ch = (data < 0x20 || data > 0x7f) ? 0x20 : data;
			logging->out_debugf(_T("graphic w: %05x %02x %c c:%02x"),addr,data,ch,color_reg);
		}
#endif
		// copy color register to color ram (7bit)
		if (SCRN_MODE_T_AND_G_WIDTH == 0x06) {
			// gw.640 and w.80
			vcram[addr & 0x7ff] = (color_reg & 0x7f);
		} else if (SCRN_MODE_T_AND_G_WIDTH == 0x02) {
			// gw.640 and w.40
			vcram[(addr & 0x7ff) >> 1] = (color_reg & 0x7f);
		} else if (SCRN_MODE_T_AND_G_WIDTH == 0x04) {
			// gw.320 and w.80
			vcram[(addr & 0x3ff) >> 1] = (color_reg & 0x7f);
		} else {
			// gw.320 and w.40
			vcram[addr & 0x3ff] = (color_reg & 0x7f);
		}
#endif
		return true;
	}
	// ig ram
	if (addr_seg04 == S1_ADDR_IGRAM_BANK1 || addr_seg04 == S1_ADDR_IGRAM_BANK2) {
		for(int i=0; i<3; i++) {
			if (ig_enreg & (1 << i)) {
				IGRAM(addr,i) = (data & 0xff);
			}
		}
		return true;
	}
	// communication rom
	if (IOPORT_USE_CM01 && cm01rom_loaded) {
		if (addr_seg16 == 0xc0000) {
			switch(addr & 0xff000) {
			case 0xc0000:
				cm01eeprom_bank = 0;
				change_cm01_membank();
				break;
			case 0xc1000:
				cm01eeprom_bank = 1;
				change_cm01_membank();
				break;
			case 0xc2000:
				cm01basic_enable = true;
				change_cm01_membank();
				break;
			case 0xc3000:
				cm01basic_enable = false;
				change_cm01_membank();
				break;
			}
		}
		if (cm01rom_access_ok) { 
			if ((addr & 0xfff00) == 0xe2100) {
				cm01rom_access_ok = false;
				change_cm01_membank();
			}
			if ((addr & 0xff000) == 0xc7000) {
				cm01eeprom_writing = true;
			}
		}
	}
	return false;
}

#if defined(USE_MPC_68008)
/// write to memory and I/O from MC68008
/// @param[in] addr : physical address
/// @param[in] data : a byte data
/// @param[in,out] wait : add spent cycles
void MEMORY::write_data8_68kw(uint32_t addr, uint32_t data, int *wait)
{
	addr_seg04 = (addr & 0xff000); // 4KB  block
	addr_seg16 = (addr & 0xfc000); // 16KB block
	addr_seg32 = (addr & 0xf8000); // 32KB block
	addr_comio = (addr & 0xeffff);

	int wait_count = (*wait) >> MAIN_SUB_CLOCK_RATIO;
	if (((*wait) & ((1 << MAIN_SUB_CLOCK_RATIO) - 1)) != 0) wait_count++;
	wait_count++;

	if (addr < 0xf0000) {
		// S1 mode
		addr_bank = (addr >> S1_BANK_SIZE);

		// if access io port, cpu speed go down to 1MHz
		set_s1_cpu_wait_68k(addr, addr_seg04, addr_bank, addr_comio, 0x20, 0x40, &wait_count);

		*wait = wait_count << MAIN_SUB_CLOCK_RATIO;

		if (write_data_s1(addr, data)) {
			return;
		}

#if defined(USE_Z80B_CARD) || defined(USE_MPC_68008)
		if (!SIG_MBC_IS_ON || addr >= 0x80000)
#endif
		{
			s1wbank[addr_bank][addr & ((1 << S1_BANK_SIZE) - 1)] = data;
		}
	} else {
		// L3 mode or upper 0xf0000
		addr_64kb = (addr & 0xffff);
		addr_bank = (addr_64kb >> L3_BANK_SIZE);

		// if access io port, cpu speed go down to 1MHz
		set_l3_cpu_wait_68k(addr, addr_bank, addr_comio, 0x20, 0x40, &wait_count);

		*wait = wait_count << MAIN_SUB_CLOCK_RATIO;

		// vram area
		if (mem_vram_sel && NOW_L3_MODE && L3_ADDR_VRAM_START <= addr_64kb && addr_64kb < L3_ADDR_VRAM_END) {
#ifdef _DEBUG_CRAM
			{
				uint8_t ch=data;
				ch = (ch < 0x20 || ch > 0x7f) ? '.' : ch;
				logging->out_debugf("mw %05x=%02x->%02x %c creg:%02x cram:%02x",addr,ram[addr],data,ch,color_reg,vgram[(addr_64kb - L3_ADDR_VRAM_START) & L3_VRAM_SIZE_1]);
			}
#endif
			vgram[(addr_64kb - L3_ADDR_VRAM_START) & L3_VRAM_SIZE_1] = (color_reg & 0x3f);
		}
		// ig_area
		if (REG_IGENABLE) {
			if (L3_ADDR_IGRAM_START <= addr && addr < L3_ADDR_IGRAM_END) {
				for(int i=0; i<3; i++) {
					if (ig_enreg & (1 << i)) {
						IGRAM(addr,i) = data & 0xff;
					}
				}
				return;
			}
		}
		l3wbank[addr_bank][addr & ((1 << L3_BANK_SIZE) - 1)] = data;
	}

#undef DEBUG_WRITE_OK
#define WRITE_IO8 write_io8
#define WRITE_MEMORY_MAPPED_IO8 write_memory_mapped_io8
#define WRITE_SIGNAL write_signal
#include "memory_writeio.h"

#ifdef USE_DEBUGGER
	bas->SetTraceBack(addr);
#endif

	// send trap signal when access the address 0xbfxxx 
	if (addr_seg04 == S1_ADDR_TRAP) {
		bfxxx_access = true;
		REG_TRAPF |= TRAPF_SIGNAL;
		d_board->write_signal(SIG_CPU_NMI, SIG_NMI_TRAP_MASK, SIG_NMI_TRAP_MASK);
	} else if (bfxxx_access) {
		d_board->write_signal(SIG_CPU_NMI, 0, SIG_NMI_TRAP_MASK);
		bfxxx_access = false;
	}
}
#endif

/// read from memory and I/O with address mapping
/// @param[in] addr : logical address
/// @param[in,out] wait : add spent cycles
/// @return : a byte data
uint32_t MEMORY::read_data8w(uint32_t addr, int *wait)
{
	uint32_t data;

	addr = address_mapping(addr & 0xffff);

	addr_seg04 = (addr & 0xff000); // 4KB  block
	addr_seg16 = (addr & 0xfc000); // 16KB block
	addr_seg32 = (addr & 0xf8000); // 32KB block
	addr_comio = (addr & 0xeffff);

	if (addr < 0xf0000) {
		// S1 mode
		addr_bank = (addr >> S1_BANK_SIZE);

		// if access io port, cpu speed go down to 1MHz
		set_s1_cpu_wait(addr, addr_seg04, addr_bank, addr_comio, 0x02, 0x04, wait);

		// trace counter
		process_trace_counter(*wait);

#if defined(USE_Z80B_CARD) || defined(USE_MPC_68008)
		if (SIG_MBC_IS_ON && addr < 0x80000) {
			data = 0xff;
		} else
#endif
		{
			data = s1rbank[addr_bank][addr & ((1 << S1_BANK_SIZE) - 1)];
		}

		if (read_data_s1(addr, data)) {
			return data;
		}

	} else {
		// L3 mode or upper 0xf0000
		addr_64kb = (addr & 0xffff);
		addr_bank = (addr_64kb >> L3_BANK_SIZE);

		// if access io port, cpu speed go down to 1MHz
		set_l3_cpu_wait(addr, addr_bank, addr_comio, 0x02, 0x04, wait);

		// trace counter
		process_trace_counter(*wait);

		// vram area
		if (mem_vram_sel && NOW_L3_MODE && L3_ADDR_VRAM_START <= addr_64kb && addr_64kb < L3_ADDR_VRAM_END) {
#ifdef _DEBUG_CRAM
			{
				uint8_t ch=ram[addr];
				ch = (ch < 0x20 || ch > 0x7f) ? '.' : ch;
				logging->out_debugf("mr %05x=%02x %c cram:%02x creg:%02x",addr,ram[addr],ch,vgram[addr],color_reg);
			}
#endif
			// 6bit
			// if MK bit is set, do not read from color ram.
			if ((color_reg & 0x80) == 0) {
				color_reg = (vgram[(addr_64kb - L3_ADDR_VRAM_START) & L3_VRAM_SIZE_1] & 0x3f);
			}
		}
		data = l3rbank[addr_bank][addr & ((1 << L3_BANK_SIZE) - 1)];
	}

	// memory mapped i/o
#undef DEBUG_READ_OK
#define READ_IO8 read_io8
#define READ_MEMORY_MAPPED_IO8 read_memory_mapped_io8
#undef WRITE_SIGNAL
#define WRITE_SIGNAL write_signal
#include "memory_readio.h"

#ifdef _DEBUG_IO_COMMON
		if ((addr & 0xfff00) == 0xeff00) {
			uint8_t ch = (data < 0x20 || data > 0x7f) ? 0x20 : data;
			logging->out_debugf("mr %04x=%02x %c",addr,data,ch);
		}
#endif

	// send trap signal when access the address 0xbfxxx 
	if (addr_seg04 == S1_ADDR_TRAP) {
		bfxxx_access = true;
		REG_TRAPF |= TRAPF_SIGNAL;
		d_board->write_signal(SIG_CPU_NMI, SIG_NMI_TRAP_MASK, SIG_NMI_TRAP_MASK);
	} else if (bfxxx_access) {
		d_board->write_signal(SIG_CPU_NMI, 0, SIG_NMI_TRAP_MASK);
		bfxxx_access = false;
	}

	return data;
}

/// read from S1 memory bus
/// @param[in] addr : physical address
/// @param[in,out] data : a byte data
/// @return true if finish reading 
bool MEMORY::read_data_s1(uint32_t addr, uint32_t &data)
{
	// text vram
	if (addr_seg04 == S1_ADDR_VTRAM_START) {
		// 7bit
		// if MK bit is set, do not read from color ram.
		if ((color_reg & 0x80) == 0) {
			color_reg = (VCRAM(addr) & 0x7f);
#ifdef _DEBUG_CRAM
			logging->out_debugf("text r: c%02x",color_reg);
#endif
		}
#ifdef _DEBUG_CVRAM
		if ((addr & 0x3ff) >= 0x0 && (addr & 0x3ff) < 0x28) {
			uint8_t ch = (data < 0x20 || data > 0x7f) ? 0x20 : data;
			logging->out_debugf(_T("text r: %05x %02x %c c:%02x"),addr,data,ch,color_reg);
		}
#endif
		return true;
	}
	// graphic vram
	if (addr_seg16 == S1_ADDR_VGRAM_START) {
		if ((SIG_BMSK_DW & 1) == 0) {
			// direct read
			data = 0;
			if (ACTIVE_PLANE_G) {
				data = VGRAM_G(addr);
			} else if (ACTIVE_PLANE_R) {
				data = VGRAM_R(addr);
			} else if (ACTIVE_PLANE_B) {
				data = VGRAM_B(addr);
			}
		} else if ((SIG_BMSK_DW & 1) == 1) {
			// indirect read
			data = 0;
			if (ACTIVE_PLANE_G) REG_GCOLOR_G = VGRAM_G(addr);
			if (ACTIVE_PLANE_R) REG_GCOLOR_R = VGRAM_R(addr);
			if (ACTIVE_PLANE_B) REG_GCOLOR_B = VGRAM_B(addr);
			if (ACTIVE_PLANE_G) {
				data = REG_GCOLOR_G;
			} else if (ACTIVE_PLANE_R) {
				data = REG_GCOLOR_R;
			} else if (ACTIVE_PLANE_B) {
				data = REG_GCOLOR_B;
			}
		}
#if 0
		// 7bit
		// if MK bit is set, do not read from color ram.
		if ((color_reg & 0x80) == 0) {
			if (SCRN_MODE_T_AND_G_WIDTH == 0x06) {
				// gw.640 and w.80
				color_reg = (vcram[addr & 0x7ff] & 0x7f);
			} else if (SCRN_MODE_T_AND_G_WIDTH == 0x02) {
				// gw.640 and w.40
				color_reg = (vcram[(addr & 0x7ff) >> 1] & 0x7f);
			} else if (SCRN_MODE_T_AND_G_WIDTH == 0x04) {
				// gw.320 and w.80
				color_reg = (vcram[(addr & 0x3ff) >> 1] & 0x7f);
			} else {
				// gw.320 and w.40
				color_reg = (vcram[addr & 0x3ff] & 0x7f);
			}
		}
#ifdef _DEBUG_CVRAM
		if ((addr & 0x3ff) >= 0x0 && (addr & 0x3ff) < 0x28) {
			uint8_t ch = (data < 0x20 || data > 0x7f) ? 0x20 : data;
			logging->out_debugf(_T("graphic r: %05x %02x %c c:%02x"),addr,data,ch,color_reg);
		}
#endif
#endif
		return true;
	}
	// ig ram (or font rom)
	if (addr_seg04 == S1_ADDR_IGRAM_BANK1 || addr_seg04 == S1_ADDR_IGRAM_BANK2) {
		if ((ig_enreg & 0x07) == 0x07) {
			// readable font rom
			data = FONTRAM(addr);
		} else if (!IOPORT_USE_DISIG) {
			// read from ig ram
			data = 0;
			for(int i=2; i>=0; i--) {
				// G > R > B
				if (ig_enreg & (1 << i)) {
					data = IGRAM(addr,i);
					break;
				}
			}
		} else {
			// disable ig
			data = 0xff;
		}
		return true;
	}
	// communication rom
	if (IOPORT_USE_CM01 && cm01rom_loaded) {
		if (cm01rom_access_ok && (addr & 0xfff00) == 0xe2100) {
			cm01rom_access_ok = false;
			change_cm01_membank();
		}
	}
	return false;
}

#if defined(USE_MPC_68008)
/// read from memory and I/O to MC68008
/// @param[in] addr : logical address
/// @param[in,out] wait : add spent cycles
/// @return : a byte data
uint32_t MEMORY::read_data8_68kw(uint32_t addr, int *wait)
{
	uint32_t data;

	addr_seg04 = (addr & 0xff000); // 4KB  block
	addr_seg16 = (addr & 0xfc000); // 16KB block
	addr_seg32 = (addr & 0xf8000); // 32KB block
	addr_comio = (addr & 0xeffff);

	int wait_count = (*wait) >> MAIN_SUB_CLOCK_RATIO;
	if (((*wait) & ((1 << MAIN_SUB_CLOCK_RATIO) - 1)) != 0) wait_count++;
	wait_count++;

	if (addr < 0xf0000) {
		// S1 mode
		addr_bank = (addr >> S1_BANK_SIZE);

		// if access io port, cpu speed go down to 1MHz
		set_s1_cpu_wait_68k(addr, addr_seg04, addr_bank, addr_comio, 0x02, 0x04, &wait_count);

//		// trace counter
//		process_trace_counter(wait_count);

		*wait = wait_count << MAIN_SUB_CLOCK_RATIO;

#if defined(USE_Z80B_CARD) || defined(USE_MPC_68008)
		if (SIG_MBC_IS_ON && addr < 0x80000) {
			data = 0xff;
		} else
#endif
		{
			data = s1rbank[addr_bank][addr & ((1 << S1_BANK_SIZE) - 1)];
		}

		if (read_data_s1(addr, data)) {
			return data;
		}

	} else {
		// L3 mode or upper 0xf0000
		addr_64kb = (addr & 0xffff);
		addr_bank = (addr_64kb >> L3_BANK_SIZE);

		// if access io port, cpu speed go down to 1MHz
		set_l3_cpu_wait_68k(addr, addr_bank, addr_comio, 0x02, 0x04, &wait_count);

//		// trace counter
//		process_trace_counter(wait_count);

		*wait = wait_count << MAIN_SUB_CLOCK_RATIO;

		// vram area
		if (mem_vram_sel && NOW_L3_MODE && L3_ADDR_VRAM_START <= addr_64kb && addr_64kb < L3_ADDR_VRAM_END) {
#ifdef _DEBUG_CRAM
			{
				uint8_t ch=ram[addr];
				ch = (ch < 0x20 || ch > 0x7f) ? '.' : ch;
				logging->out_debugf("mr %05x=%02x %c cram:%02x creg:%02x",addr,ram[addr],ch,vgram[(addr_64kb - L3_ADDR_VRAM_START) & L3_VRAM_SIZE_1],color_reg);
			}
#endif
			// 6bit
			// if MK bit is set, do not read from color ram.
			if ((color_reg & 0x80) == 0) {
				color_reg = (vgram[(addr_64kb - L3_ADDR_VRAM_START) & L3_VRAM_SIZE_1] & 0x3f);
			}
		}
		data = l3rbank[addr_bank][addr & ((1 << L3_BANK_SIZE) - 1)];
	}

	// memory mapped i/o
#undef DEBUG_READ_OK
#define READ_IO8 read_io8
#define READ_MEMORY_MAPPED_IO8 read_memory_mapped_io8
#undef WRITE_SIGNAL
#define WRITE_SIGNAL write_signal
#include "memory_readio.h"

#ifdef _DEBUG_IO_COMMON
		if ((addr & 0xfff00) == 0xeff00) {
			uint8_t ch = (data < 0x20 || data > 0x7f) ? 0x20 : data;
			logging->out_debugf("mr %04x=%02x %c",addr,data,ch);
		}
#endif

	// send trap signal when access the address 0xbfxxx 
	if (addr_seg04 == S1_ADDR_TRAP) {
		bfxxx_access = true;
		REG_TRAPF |= TRAPF_SIGNAL;
		d_board->write_signal(SIG_CPU_NMI, SIG_NMI_TRAP_MASK, SIG_NMI_TRAP_MASK);
	} else if (bfxxx_access) {
		d_board->write_signal(SIG_CPU_NMI, 0, SIG_NMI_TRAP_MASK);
		bfxxx_access = false;
	}

	return data;
}
#endif

void MEMORY::latch_address(uint32_t addr, int *wait)
{
	addr = address_mapping(addr & 0xffff);
	addr_seg04 = (addr & 0xff000); // 4KB  block
	addr_comio = (addr & 0xeffff);

	// if access io port, cpu speed go down to 1MHz
	if (addr < 0xf0000) {
		// S1 mode
		addr_bank = (addr >> S1_BANK_SIZE);
		set_s1_cpu_wait(addr, addr_seg04, addr_bank, addr_comio, 0x02, 0x04, wait);
	} else {
		// L3 mode or upper 0xf0000
		addr_64kb = (addr & 0xffff);
		addr_bank = (addr_64kb >> L3_BANK_SIZE);
		set_l3_cpu_wait(addr, addr_bank, addr_comio, 0x02, 0x04, wait);

		// vram area
		if (mem_vram_sel && NOW_L3_MODE && L3_ADDR_VRAM_START <= addr_64kb && addr_64kb < L3_ADDR_VRAM_END) {
#ifdef _DEBUG_CRAM
			{
				logging->out_debugf("la %05x cram:%02x creg:%02x",addr,vgram[(addr_64kb - L3_ADDR_VRAM_START) & L3_VRAM_SIZE_1],color_reg);
			}
#endif
			// 6bit
			// if MK bit is set, do not read from color ram.
			if ((color_reg & 0x80) == 0) {
				color_reg = (vgram[(addr_64kb - L3_ADDR_VRAM_START) & L3_VRAM_SIZE_1] & 0x3f);
			}
		}
	}
}

void MEMORY::write_io8(uint32_t addr, uint32_t data)
{

}

void MEMORY::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
//		case SIG_MEMORY_PIA_PA:
//			// from PIA portA (L3)
//			mem_bank_reg1 = data & 0xce;
//			change_l3_memory_bank();
//			break;
		case SIG_CPU_BABS:
			if ((data & mask) == 0x01) {
				// interrupt
				// go to system mode
				if (NOW_USER_MODE) change_system_mode(false);
			}
			break;
		case SIG_CPU_RESET:
			now_reset = (data & mask) ? true : false;
			if (now_reset) {
				// now on reset signal

				// set s1 or l3 mode
				REG_SYS_MODE = vm->get_parami(VM::ParamSysMode);
				reset_s1_l3_mode();
				// init memory map
				reset_l3_membank();
				// stop trace counter
				reset_trace_counter();
				// reset igmode
				set_igmode(0);
				ig_enreg = 0;
			} else {
				// turn off reset signal
			}
			break;
	}
}

uint32_t MEMORY::read_signal(int id)
{
	uint32_t data = 0xff;

	return data;
}

void MEMORY::change_l3_memory_bank()
{
//	mem_bank_reg1;	// from 0xffc0
//	mem_bank_reg2;	// from 0xffe8
	uint8_t *wexram00 = wdmy;
	uint8_t *rexram00 = rdmy;
	uint8_t *wexram04 = wdmy;
	uint8_t *rexram04 = rdmy;
	uint8_t *wexram80 = wdmy;
	uint8_t *rexram80 = rdmy;

	// have extended ram
	if (exram_size >= 64 && exram != NULL) {
		wexram00 = exram + 0x0000;
		rexram00 = exram + 0x0000;
		wexram04 = exram + 0x0400;
		rexram04 = exram + 0x0400;
		wexram80 = exram + 0x8000;
		rexram80 = exram + 0x8000;
	}

	memset(l3wait, 0, sizeof(l3wait));
	memset(l3banktype, 0, sizeof(l3banktype));

	// 0x0000 - 0x7fff
	if (mem_bank_reg2 & 0x08) {
		if (mem_bank_reg2 & 0x20) {
			// change to the extended ram both r/w
			L3_SET_BANK(0x0000, 0x7fff, wexram00, rexram00, 0x33, 0x22);
		} else {
			// change to the extended ram only write
			L3_SET_BANK(0x0000, 0x7fff, wexram00, ram + 0x0000, 0x37, 0x21);
		}
	} else {
		// change to standard
		L3_SET_BANK(0x0000, 0x7fff, ram + 0x0000, ram + 0x0000, 0x77, 0x11);
	}

	// 0x0400 - 0x43ff (vram area)
	// exchange vram area to/from extended ram
	if ((mem_bank_reg2 & 0x0c) == 0x04 || (mem_bank_reg2 & 0x0c) == 0x08) {
		// select extend ram
		mem_vram_sel = false;
		if (mem_bank_reg2 & 0x20) {
			// read/write
			L3_SET_BANK(0x0400, 0x43ff, wexram04, rexram04, 0x33, 0x22);
		} else {
			// write only
			L3_SET_BANK(0x0400, 0x43ff, wexram04, ram + 0x0400, 0x37, 0x21);
		}
	} else {
		// select standard ram
		mem_vram_sel = true;
		L3_SET_BANK(0x0400, 0x43ff, ram + 0x0400, ram + 0x0400, 0x77, 0x11);
	}

	// 0x8000 - 0xffff
	if ((mem_bank_reg2 & 0x50) == 0x50) {
		// change to the extended ram both r/w
		L3_SET_BANK(0x8000, 0xffff, wexram80, rexram80, 0x33, 0x22);
	} else {
		//
		// decide the area to write
		if (mem_bank_reg2 & 0x10) {
			// change to the extended ram only write
			L3_SET_BANK(0x8000, 0xffff, wexram80, NULL, 0x30, 0x20);
		} else {
			// change to the standard
			L3_SET_BANK(0x8000, 0xffff, ram + 0x8000, NULL, 0x70, 0x10);
		}

		//
		// decide the area to read
		L3_SET_BANK(0x8000, 0x9fff, NULL, ram + 0x8000, 0x07, 0x01);
		L3_SET_BANK(0xa000, 0xffff, NULL, rdmy, 0x00, 0x00);
		if (l3rom_loaded) {
			L3_SET_BANK(0xa000, 0xffff, NULL, l3rom + 0x0000, 0x00, 0x08);
		}
		if (IOPORT_USE_3FDD && rom1805_loaded) {
			// for 3inch compact floppy
			L3_SET_BANK(0xf800, 0xffff, NULL, rom1805, 0x00, 0x09);
		}
		if (IOPORT_USE_5FDD && rom1802_loaded) {
			// for 5.25inch mini floppy
			L3_SET_BANK(0xf800, 0xffff, NULL, rom1802, 0x00, 0x0a);
		}

		if (!(mem_bank_reg1 & 0x02)) {
			if (mem_bank_reg1 & 0x40) {
				// r/w to ram (this means cannot read rom data !)
				L3_SET_BANK(0xa000, 0xbfff, NULL, ram + 0xa000, 0x07, 0x01);
			}
		}
		if (!(mem_bank_reg1 & 0x04)) {
			if (mem_bank_reg1 & 0x80) {
				// r/w to ram (this means cannot read rom data !)
				L3_SET_BANK(0xc000, 0xdfff, NULL, ram + 0xc000, 0x07, 0x01);
			}
		}
		if (!(mem_bank_reg1 & 0x08)) {
			if (mem_bank_reg1 & 0x80) {
				// r/w to ram (this means cannot read rom data !)
				L3_SET_BANK(0xe000, 0xefff, NULL, ram + 0xe000, 0x07, 0x01);
			}
		}

		if (mem_bank_reg2 & 0x01) {
			// read from ram (this means cannot read rom data !)
			L3_SET_BANK(0xf000, 0xfeff, NULL, ram + 0xf000, 0x07, 0x01);
		} else {
			// read from rom , write to ram
			L3_SET_BANK(0xf000, 0xfeff, NULL, rdmy, 0x00, 0x00);
			if (l3rom_loaded) {
				L3_SET_BANK(0xf000, 0xfeff, NULL, l3rom + 0x5000, 0x00, 0x08);
			}
			if (IOPORT_USE_3FDD && rom1805_loaded) {
				// for 3inch compact floppy
				L3_SET_BANK(0xf800, 0xfeff, NULL, rom1805, 0x00, 0x09);
			}
			if (IOPORT_USE_5FDD && rom1802_loaded) {
				// for 5.25inch mini floppy
				L3_SET_BANK(0xf800, 0xfeff, NULL, rom1802, 0x00, 0x0a);
			}
		}
		if (mem_bank_reg2 & 0x02) {
			// r/w to ram (this means cannot read rom data !)
			L3_SET_BANK(0xfff0, 0xffff, NULL, ram + 0xfff0, 0x07, 0x01);
		} else {
			// read from rom , write to ram
			L3_SET_BANK(0xfff0, 0xffff, NULL, rdmy, 0x00, 0x00);
			if (l3rom_loaded) {
				L3_SET_BANK(0xfff0, 0xffff, NULL, l3rom + 0x5ff0, 0x00, 0x08);
			}
		}
	}
}

/// get L3 vram area 
uint8_t* MEMORY::get_l3vram() {
// vram is always the standard ram
//	return ((mem_bank_reg2 & 0x04) ? ram2 : ram );
	return ram;
}

/// get text vram
uint8_t* MEMORY::get_s1vtram() {
	return vtram;
}

/// get text color ram
uint8_t* MEMORY::get_s1vcram() {
	return vcram;
}

/// get graphic vram red (is main ram 0x0000 - 0x3fff)
uint8_t* MEMORY::get_s1vgrram() {
	return ram;
}

/// get graphic vram green
uint8_t* MEMORY::get_s1vggram() {
	return vgram;
}

/// get graphic vram blue
uint8_t* MEMORY::get_s1vgbram() {
	return &vgram[0x4000];
}

/// process trace counter if need
void MEMORY::process_trace_counter(int clk)
{
	if (trace_counter_phase != TRACE_COUNTER_PHASE_IDLE) {
#ifdef _DEBUG_TRACE_COUNTER
		logging->out_debugf(_T("process_trace_counter: clk:%d"), clk);
#endif
		if (trace_counter_phase == TRACE_COUNTER_PHASE_WAITING) {
			// declease trace counter
			set_trace_counter(clk);
		} else if (trace_counter_phase == TRACE_COUNTER_PHASE_FIRED) {
			// trace counter reset
			clear_trace_counter(clk);
		}
	}
}

void MEMORY::fetch_trace_counter(int clk)
{
#ifdef _DEBUG_TRACE_COUNTER
	logging->out_debugf(_T("fetch_trace_counter: clk:%d"), clk);
#endif
	// no start trace counter if delay time is 0.
	if (FUSE_MODE_DELAY == 0) return;
//	pc = 0;
	trace_counter_phase = TRACE_COUNTER_PHASE_WAITING;
	// set trace count ($fe10)
	trace_count_remain = ((FUSE_MODE_DELAY + 1) << (NOW_2MHZ ? 0 : 1));
	prev_trace_clock = clk;
#ifdef _DEBUG_TRACE_COUNTER
	logging->out_debugf(_T("trace_fetch: clk:%d remain:%d"), clk, trace_count_remain);
#endif
}

void MEMORY::set_trace_counter(int clk)
{
	int sum = clk - prev_trace_clock;
	if (sum < 0) sum += CLOCKS_CYCLE;
	trace_count_remain -= sum;
	prev_trace_clock = clk;
#ifdef _DEBUG_TRACE_COUNTER
	logging->out_debugf(_T("trace_set: clk:%d sum:%d remain:%d"), clk, sum, trace_count_remain);
#endif
	if (trace_count_remain <= 0) {
		fire_trace_counter(clk);
	}
}

void MEMORY::fire_trace_counter(int clk)
{
	if (FUSE_TRACE_MASK) {
		// send NMI interrupt when trace NMI is not masked
		d_board->write_signal(SIG_CPU_NMI, SIG_NMI_TRACE_MASK, SIG_NMI_TRACE_MASK);
#ifdef _DEBUG_TRACE_COUNTER
		logging->out_debugf(_T("trace_fire: clk:%d remain:%d NMI on"), clk, trace_count_remain);
#endif
	} else {
		// go to user mode when trace NMI is masked
		if (NOW_SYSTEM_MODE) change_system_mode(true);
#ifdef _DEBUG_TRACE_COUNTER
		logging->out_debugf(_T("trace_fire: clk:%d remain:%d NMI masked"), clk, trace_count_remain);
#endif
	}
	trace_counter_phase = TRACE_COUNTER_PHASE_FIRED;
}

void MEMORY::clear_trace_counter(int clk)
{
	// wait until next opcode read
	int curclk = (get_current_clock() % CLOCKS_CYCLE);
	if (curclk < clk) return;

	int sum = clk - prev_trace_clock;
	if (sum < 0) sum += CLOCKS_CYCLE;
	if (sum > 0) {
		if (FUSE_TRACE_MASK) {
			// clear NMI
			d_board->write_signal(SIG_CPU_NMI, 0, SIG_NMI_TRACE_MASK);
#ifdef _DEBUG_TRACE_COUNTER
			logging->out_debugf(_T("trace NMI off: clk:%d sum:%d"), clk, sum);
#endif
		}
		trace_counter_phase = TRACE_COUNTER_PHASE_IDLE;
		trace_count_remain = 0;
		prev_trace_clock = 0;
#ifdef _DEBUG_TRACE_COUNTER
		logging->out_debugf(_T("trace_clear: clk:%d curclk:%d sum:%d"), clk, curclk, sum);
#endif
	}
}

/// memory mapping
uint32_t MEMORY::address_mapping(uint32_t addr)
{
	if (NOW_S1_MODE) {
		// S1 mode
		if (NOW_SYSTEM_MODE && (addr & 0xe000) == 0xe000) {
			// system mode
			addr = ((addr & 0x0fff) | ((addr & 0xf000) == 0xf000 ? 0xef000 : 0x84000));
		} else {
			// mapping
			addr = ((addr & 0x0fff) | (addr_map[REG_ADDRSEG][(addr & 0xf000) >> 12] << 12));
		}
	} else {
		// L3 mode
		addr |= 0xf0000;
	}
	return addr;
}

/// system <-> user mode
void MEMORY::change_system_mode(bool usermode)
{
	if (usermode && FUSE_MODE_DELAY) {
		// go to user mode
		REG_SYS_MODE &= ~SYS_MODE_SU;
		// all interrupt ok
		d_board->write_signal(SIG_FUSE_INTR_MASK, 0, 1);
	} else if (!usermode) {
		// go to system mode
		REG_SYS_MODE |= SYS_MODE_SU;
#ifdef _DEBUG
//		dasm.trace_on(50);
#endif
		// all interrupt mask except trace processing.
		if (trace_counter_phase == TRACE_COUNTER_PHASE_IDLE) {
			d_board->write_signal(SIG_FUSE_INTR_MASK, 1, 1);
		}
	}
#ifdef _DEBUG_TRACE_COUNTER
	logging->out_debugf(_T("change_system_mode: %s trace_phase:%d"), (NOW_SYSTEM_MODE ? _T("S") : _T("U")), trace_counter_phase);
#endif
}

/// set S1/L3 mode
void MEMORY::reset_s1_l3_mode()
{
	REG_SYS_MODE |= SYS_MODE_SU;
	if (NOW_S1_MODE) {
		REG_SYS_MODE |= SYS_MODE_CLK;
	} else {
		REG_SYS_MODE &= ~SYS_MODE_CLK;
	}
	set_cpu_speed(REG_SYS_MODE);

	// initialize crtc clock 
	d_crtc->write_signal(HD46505::SIG_HD46505_CHR_CLOCKS, CLOCKS_1MHZ * (NOW_S1_MODE || (REG_MODE_SEL & 0x80) ? 2 : 1), 0xffffffff);
	d_disp->write_signal(DISPLAY::SIG_DISPLAY_CHR_CLOCKS, CLOCKS_1MHZ * (NOW_S1_MODE || (REG_MODE_SEL & 0x80) ? 2 : 1), 0xffffffff);

#ifdef USE_DEBUGGER
	if (bas) bas->Reset(NOW_S1_MODE ? 1 : 0);
#endif
}

/// set cpu clock (bit1)
void MEMORY::set_cpu_speed(uint8_t data)
{
	REG_SYS_MODE = ((data & SYS_MODE_CLK) ? (REG_SYS_MODE | SYS_MODE_CLK) : (REG_SYS_MODE & ~SYS_MODE_CLK));
	d_event->write_signal(SIG_EVENT_CPU_HALF_SPEED, NOW_2MHZ ? 0 : 1, 1);
}

/// (re)allocate extended ram 
void MEMORY::set_extended_ram()
{
	// 0:0KB 1:64KB 2:128KB 3:256KB 4:512KB
	int size = (pConfig->exram_size_num ? ((1 << pConfig->exram_size_num) * 32) : 0);
	if (exram != NULL) {
		delete [] exram;
		exram_size = 0;
		exram = NULL;

		// reset ext ram 0x00000 - 0x7ffff
		S1_SET_BANK(0x00000, 0x7ffff, wdmy, rdmy, 0, 0);
	}
	if (size > 0) {
		exram = new uint8_t[size * 1024];
		exram_size = size;

		memset(exram, 0, sizeof(uint8_t)* size * 1024);

		// ext ram 0x00000 - 0x0ffff - 0x7ffff
		S1_SET_BANK(0x00000, size*1024-1, exram, exram, 0x33, 0x22);
	}
#if defined(USE_Z80B_CARD)
	d_z80b_card->set_memory(exram, exram_size);
#elif defined(USE_MPC_68008)
	d_mpc_68008->set_memory(exram, exram_size);
#endif
}

// ----------------------------------------------------------------------------
// event handler
// ----------------------------------------------------------------------------

#if 0
void MEMORY::event_callback(int event_id, int err)
{
	if (event_id == SIG_TRACE_COUNTER) {
		// event stop
//		cancel_event(trace_counter_id);
		trace_counter_id = -1;

		pc = d_cpu->get_pc();

#if 0
		if (pc == pc_prev || pc_prev == TRACE_COUNTER_PHASE_WAITING) {
			// inhibit the NMI interrupt until next opcode is executed.
			pc_prev = pc;
			// next event
			register_event(this, SIG_TRACE_COUNTER, 2, false, &trace_counter_id);
			logging->out_debugf(_T("trace_fire: clk:%lld pc:%04x next"), current_clock() % CLOCKS_CYCLE, pc);
		} else
#endif
		{
			// go to user mode
			if (NOW_SYSTEM_MODE) change_system_mode(true);
			// send NMI interrupt
			if (FUSE_TRACE_MASK) {
				d_board->write_signal(SIG_CPU_NMI, SIG_NMI_TRACE_MASK, SIG_NMI_TRACE_MASK);
//				logging->out_debugf(_T("trace_fire: clk:%lld pc:%04x NMI on"), current_clock() % CLOCKS_CYCLE, pc);
//			} else {
//				logging->out_debugf(_T("trace_fire: clk:%lld pc:%04x NMI masked"), current_clock() % CLOCKS_CYCLE, pc);
			}
			pc_prev = TRACE_COUNTER_PHASE_FIRED;
		}
	}
}
#endif

// ----------------------------------------------------------------------------

void MEMORY::update_config()
{
}

// ----------------------------------------------------------------------------

void MEMORY::save_state(FILEIO *fio)
{
	struct vm_state_st_s1 *vm_state = NULL;

	//
	vm_state_ident.version = Uint16_LE(0x45);
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(struct vm_state_st_s1) + (exram_size * 1024));
	fio->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);

	// save values
	fio->Fwrite(&ram,  sizeof(ram), 1);
	fio->Fwrite(&vgram, sizeof(vgram), 1);
	fio->Fwrite(&vtram, sizeof(vtram), 1);
	fio->Fwrite(&vcram, sizeof(vcram), 1);
	fio->Fwrite(&ig_ram, sizeof(ig_ram), 1);

//	vm_state.trace_counter_id = trace_counter_id;
//	vm_state.pc = pc;
//	vm_state.pc_prev = pc_prev;
	fio->FputInt32_LE(trace_counter_phase);
	fio->FputInt32_LE(trace_count_remain);
	fio->FputInt32_LE(prev_trace_clock);

	fio->FputUint8(mem_bank_reg1);
	fio->FputUint8(mem_bank_reg2);
	fio->FputUint8(mem_bank_mask ? 1 : 0);
	fio->Fsets(0, sizeof(vm_state->reserved1));

	// save config
	fio->FputUint8(pConfig->dipswitch);
	fio->FputUint8(REG_SYS_MODE);
	fio->FputInt32_LE(pConfig->fdd_type);
	fio->FputInt32_LE(pConfig->io_port);
	fio->FputUint8(REG_MODE_SEL);
	fio->FputUint8(color_reg);
	fio->FputUint8(ig_enreg);
	fio->FputUint8(REG_FUSE);
	fio->FputUint8(REG_TRAPF);
	fio->FputUint8(REG_BUSCTRL);

	// version 0x43
	fio->Fwrite(&addr_map, sizeof(addr_map), 1);
	fio->FputUint8(REG_ADDRSEG);
	fio->Fsets(0, sizeof(vm_state->v43.reserved3));
	fio->Fsets(0, sizeof(vm_state->v43.exram_ident));

	fio->FputUint16_LE(exram_size);

	// save extended ram
	if (exram_size > 0) {
		fio->Fwrite(exram, exram_size * 1024, 1);
	}
}

bool MEMORY::load_state(FILEIO *fio)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st_l3 *vm_state_l3 = NULL;
	struct vm_state_st_s1 *vm_state_s1 = NULL;

	if (find_state_chunk(fio, &vm_state_i) != true) {
		return true;
	}

	if (Uint16_LE(vm_state_i.version) < 0x41) {
		// L3

		// set B mode
		memset(addr_map, 0, sizeof(addr_map));
		REG_SYS_MODE = SYS_MODE_SU;

		// load values
		fio->Fread(&ram,  sizeof(ram), 1);
		// load extended memory
		exram_size = 64;	// 64KB constant
		pConfig->exram_size_num = 1;
		set_extended_ram();
		fio->Fread(exram, sizeof(vm_state_l3->ram2), 1);

		fio->Fread(&vgram, sizeof(vm_state_l3->color_ram), 1);
		fio->Fread(&ig_ram, sizeof(ig_ram), 1);

		mem_bank_reg1 = fio->FgetUint8();
		mem_bank_reg2 = fio->FgetUint8();
		mem_bank_mask = false;

//		trace_counter_id = vm_state_l3.trace_counter_id;
//		pc = vm_state_l3.pc;
//		pc_prev = vm_state_l3.pc_prev;
		fio->Fseek(sizeof(vm_state_l3->trace_counter_id), FILEIO::SEEKCUR);
		fio->Fseek(sizeof(vm_state_l3->pc), FILEIO::SEEKCUR);
		fio->Fseek(sizeof(vm_state_l3->pc_prev), FILEIO::SEEKCUR);
		trace_counter_phase = TRACE_COUNTER_PHASE_IDLE;
		trace_count_remain = 0;
		prev_trace_clock = 0;

		// load config
		pConfig->dipswitch = fio->FgetUint8();
		pConfig->fdd_type = fio->FgetInt32_LE();
		pConfig->io_port = fio->FgetInt32_LE();

		fio->Fseek(sizeof(vm_state_l3->reserved), FILEIO::SEEKCUR);

		// calc values
		// rs bit on mode_sel is set in COMM device. 
		REG_MODE_SEL = ram[0xffd0];
		d_disp->write_io8(0xffd0, REG_MODE_SEL);
		reset_s1_l3_mode();

		d_disp->write_io8(0xffd6, ram[0xffd6] & 0x08);	// interace sel
		color_reg = ram[0xffd8];	// color reg

//		write_data8(0xffea, ram[0xffea]);	// IG en register
		ig_enreg = ram[0xffea] & 0x07;

	} else {
		// S1

		// load values
		fio->Fread(&ram,  sizeof(ram), 1);
		fio->Fread(&vgram, sizeof(vgram), 1);
		fio->Fread(&vtram, sizeof(vtram), 1);
		fio->Fread(&vcram, sizeof(vcram), 1);
		fio->Fread(&ig_ram, sizeof(ig_ram), 1);

//		trace_counter_id = vm_state_s1.trace_counter_id;
//		pc = vm_state_s1.pc;
//		pc_prev = vm_state_s1.pc_prev;
		if (vm_state_i.version >= 0x42) {
			trace_counter_phase = fio->FgetInt32_LE();
			trace_count_remain = fio->FgetInt32_LE();
			prev_trace_clock = fio->FgetInt32_LE();
		} else {
			fio->Fseek(sizeof(vm_state_s1->trace_counter_phase), FILEIO::SEEKCUR);
			fio->Fseek(sizeof(vm_state_s1->trace_count_remain), FILEIO::SEEKCUR);
			fio->Fseek(sizeof(vm_state_s1->prev_trace_clock), FILEIO::SEEKCUR);
			trace_counter_phase = TRACE_COUNTER_PHASE_IDLE;
			trace_count_remain = 0;
			prev_trace_clock = 0;
		}

		mem_bank_reg1 = fio->FgetUint8();
		mem_bank_reg2 = fio->FgetUint8();
		mem_bank_mask = (fio->FgetUint8() != 0);
		fio->Fseek(sizeof(vm_state_s1->reserved1), FILEIO::SEEKCUR);

		pConfig->dipswitch = fio->FgetUint8();
		REG_SYS_MODE = fio->FgetUint8();
		set_cpu_speed(REG_SYS_MODE);

		pConfig->fdd_type = fio->FgetInt32_LE();
		pConfig->io_port = fio->FgetInt32_LE();

		REG_MODE_SEL = fio->FgetUint8();	// mode sel
		color_reg = fio->FgetUint8();	// color reg
		ig_enreg = fio->FgetUint8();	// IG en register
		REG_FUSE = fio->FgetUint8();
		REG_TRAPF = fio->FgetUint8();
		REG_BUSCTRL = fio->FgetUint8();

		if (Uint16_LE(vm_state_i.version) >= 0x43) {
			// version 0x43
			fio->Fread(&addr_map, sizeof(addr_map), 1);
			REG_ADDRSEG = fio->FgetUint8();
			fio->Fseek(sizeof(vm_state_s1->v43.reserved3), FILEIO::SEEKCUR);
			fio->Fseek(sizeof(vm_state_s1->v43.exram_ident), FILEIO::SEEKCUR);
			// load extended memory
			exram_size = fio->FgetUint16_LE();
		} else {
			// version ..0x42
			fio->Fread(addr_map[0], sizeof(addr_map[0]), 1);
			fio->Fseek(sizeof(vm_state_s1->v42.reserved2), FILEIO::SEEKCUR);
			fio->Fseek(sizeof(vm_state_s1->v42.exram_ident), FILEIO::SEEKCUR);
			// load extended memory
			exram_size = fio->FgetUint16_LE();
			// shift color ram if L3 mode
			if (NOW_L3_MODE) {
				for(int i=0; i<L3_VRAM_SIZE; i++) {
					vgram[i] = vgram[i + L3_ADDR_VRAM_START];
				}
				for(int i=L3_VRAM_SIZE; i<L3_ADDR_VRAM_END; i++) {
					vgram[i] = 0;
				}
			}
		}

		if (exram_size > 512) exram_size = 64;
		pConfig->exram_size_num = 0;
		for(int i=1; i<=4; i++) {
			if (exram_size == (1 << i) * 32) {
				pConfig->exram_size_num = i;
				break;
			}
		}
		set_extended_ram();

		if (exram_size > 0) {
			fio->Fread(exram, exram_size * 1024, 1);
		}

		d_disp->write_io8(0xffd0, REG_MODE_SEL);	// mode sel
	}

	if (Uint16_LE(vm_state_i.version) < 0x44) {
		// enable keyboard and mouse
		pConfig->io_port |= (IOPORT_MSK_KEYBD | IOPORT_MSK_MOUSE);
	}

	// now trap ?
	bfxxx_access = (d_board->read_signal(SIG_CPU_NMI) & SIG_NMI_TRAP_MASK) ? true : false;

	change_l3_memory_bank();

	return true;
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
void MEMORY::set_debugger_console(DebuggerConsole *dc)
{
	this->dc = dc;
	if (bas) bas->SetDebuggerConsole(dc);
}

uint32_t MEMORY::debug_latch_address(uint32_t addr)
{
	return address_mapping(addr & 0xffff);
}

void MEMORY::debug_write_data8(int type, uint32_t addr, uint32_t data)
{
	uint32_t addr_bank;
	uint32_t addr_seg04, addr_seg16, addr_seg32;
	uint32_t addr_comio;
	int wait;
	switch(type) {
	case 0:
		addr_seg04 = (addr & 0xff000); // 4KB  block
		addr_seg16 = (addr & 0xfc000); // 16KB block
		addr_seg32 = (addr & 0xf8000); // 32KB block
		addr_comio = (addr & 0xeffff);

		if (addr < 0xf0000) {
			// S1 mode
			addr_bank = (addr >> S1_BANK_SIZE);

			// ig ram
			if ((addr_seg04 == S1_ADDR_IGRAM_BANK1 || addr_seg04 == S1_ADDR_IGRAM_BANK2)) {
				for(int i=0; i<3; i++) {
					if (ig_enreg & (1 << i)) {
						IGRAM(addr,i) = (data & 0xff);
					}
				}
				return;
			}

			s1wbank[addr_bank][addr & ((1 << S1_BANK_SIZE) - 1)] = data;

		} else {
			// L3 mode
			addr_bank = ((addr & 0xffff) >> L3_BANK_SIZE);

			// ig_area
			if (REG_IGENABLE) {
				if (L3_ADDR_IGRAM_START <= addr && addr < L3_ADDR_IGRAM_END) {
					for(int i=0; i<3; i++) {
						if (ig_enreg & (1 << i)) {
							IGRAM(addr,i) = data & 0xff;
						}
					}
					return;
				}
			}

			l3wbank[addr_bank][addr & ((1 << L3_BANK_SIZE) - 1)] = data;

		}

	// memory mapped i/o
#undef WRITE_IO8
#undef WRITE_MEMORY_MAPPED_IO8
#undef WRITE_SIGNAL
#define DEBUG_WRITE_OK
#define WRITE_IO8 debug_write_io8
#define WRITE_MEMORY_MAPPED_IO8 debug_write_memory_mapped_io8
#include "memory_writeio.h"
		break;

	case 1:
		 vcram[addr & S1_VTRAM_SIZE_1] = data;
		break;

	case 2:
		break;

	case 3:
		ig_ram[addr & 0x7ff] = data;
		break;
	case 4:
		ig_ram[(addr & 0x7ff) + 0x800] = data;
		break;
	case 5:
		ig_ram[(addr & 0x7ff) + 0x1000] = data;
		break;
	case 6:
		// adpcm memory on fmopn
		d_fmopn->debug_write_data8(0, addr, data);
		break;
	case 7:
		// adpcm memory on expsg
		d_psgex->debug_write_data8(0, addr, data);
		break;

	default:
#ifdef USE_CPU_REAL_MACHINE_CYCLE
		write_data8w(addr, data, &wait);
#else
		write_data8(addr, data);
#endif
		break;
	}
}

uint32_t MEMORY::debug_read_data8(int type, uint32_t addr)
{
	uint32_t addr_bank;
	uint32_t addr_seg04, addr_seg16, addr_seg32;
	uint32_t addr_comio;
	uint32_t data;

	switch(type) {
	case 0:
		addr_seg04 = (addr & 0xff000); // 4KB  block
		addr_seg16 = (addr & 0xfc000); // 16KB block
		addr_seg32 = (addr & 0xf8000); // 32KB block
		addr_comio = (addr & 0xeffff);

		if (addr < 0xf0000) {
			// S1 mode
			addr_bank = (addr >> S1_BANK_SIZE);

			data = s1rbank[addr_bank][addr & ((1 << S1_BANK_SIZE) - 1)];

			// ig ram (or font rom)
			if (addr_seg04 == S1_ADDR_IGRAM_BANK1 || addr_seg04 == S1_ADDR_IGRAM_BANK2) {
				if ((ig_enreg & 0x07) == 0x07) {
					// readable font rom
					data = FONTRAM(addr);
				} else {
					data = 0;
					for(int i=2; i>=0; i--) {
						// G > R > B
						if (ig_enreg & (1 << i)) {
							data = IGRAM(addr,i);
							break;
						}
					}
				}
				return data;
			}

		} else {
			// L3 mode
			addr_bank = ((addr & 0xffff) >> L3_BANK_SIZE);

			data = l3rbank[addr_bank][addr & ((1 << L3_BANK_SIZE) - 1)];

		}

	// memory mapped i/o
#undef READ_IO8
#undef READ_MEMORY_MAPPED_IO8
#undef WRITE_SIGNAL
#define DEBUG_READ_OK
#define READ_IO8 debug_read_io8
#define READ_MEMORY_MAPPED_IO8 debug_read_memory_mapped_io8
#include "memory_readio.h"
		break;

	case 1:
		data = vcram[addr & S1_VTRAM_SIZE_1];
		break;

	case 2:
		data = 0;
		break;

	case 3:
		data = ig_ram[addr & 0x7ff];
		break;
	case 4:
		data = ig_ram[(addr & 0x7ff) + 0x800];
		break;
	case 5:
		data = ig_ram[(addr & 0x7ff) + 0x1000];
		break;
	case 6:
		// adpcm memory on fmopn
		data = d_fmopn->debug_read_data8(0, addr);
		break;
	case 7:
		// adpcm memory on expsg
		data = d_psgex->debug_read_data8(0, addr);
		break;

	default:
		addr = address_mapping(addr & 0xffff);
		addr_seg04 = (addr & 0xff000); // 4KB  block
		addr_seg16 = (addr & 0xfc000); // 16KB block
		addr_seg32 = (addr & 0xf8000); // 32KB block
		addr_comio = (addr & 0xeffff);

		if (addr < 0xf0000) {
			// S1 mode
			addr_bank = (addr >> S1_BANK_SIZE);

			data = s1rbank[addr_bank][addr & ((1 << S1_BANK_SIZE) - 1)];

			// ig ram (or font rom)
			if (addr_seg04 == S1_ADDR_IGRAM_BANK1 || addr_seg04 == S1_ADDR_IGRAM_BANK2) {
				if ((ig_enreg & 0x07) == 0x07) {
					// readable font rom
					data = FONTRAM(addr);
				} else {
					data = 0;
					for(int i=2; i>=0; i--) {
						// G > R > B
						if (ig_enreg & (1 << i)) {
							data = IGRAM(addr,i);
							break;
						}
					}
				}
				return data;
			}

		} else {
			// L3 mode
			addr_bank = ((addr & 0xffff) >> L3_BANK_SIZE);

			data = l3rbank[addr_bank][addr & ((1 << L3_BANK_SIZE) - 1)];

		}

		// memory mapped i/o
#undef READ_IO8
#undef READ_MEMORY_MAPPED_IO8
#undef WRITE_SIGNAL
#undef DEBUG_READ_OK
#define READ_IO8 debug_read_io8
#define READ_MEMORY_MAPPED_IO8 debug_read_memory_mapped_io8
#include "memory_readio.h"
		break;
	}
	return data;
}

void MEMORY::debug_write_data16(int type, uint32_t addr, uint32_t data)
{
	// big endien
	debug_write_data8(type, addr, (data >> 8) & 0xff);
	debug_write_data8(type, addr + 1, data & 0xff);
}

uint32_t MEMORY::debug_read_data16(int type, uint32_t addr)
{
	// big endien
	uint32_t val = debug_read_data8(type, addr) << 8;
	val |= debug_read_data8(type, addr + 1);
	return val;
}

void MEMORY::debug_write_data32(int type, uint32_t addr, uint32_t data)
{
	// big endien
	debug_write_data16(type, addr, (data >> 16) & 0xffff);
	debug_write_data16(type, addr + 2, data & 0xffff);
}

uint32_t MEMORY::debug_read_data32(int type, uint32_t addr)
{
	// big endien
	uint32_t val = debug_read_data16(type,addr) << 16;
	val |= debug_read_data16(type,addr + 2);
	return val;
}

uint32_t MEMORY::debug_physical_addr_mask(int type)
{
	uint32_t data = 0;
	switch(type) {
	case 0:
		data = 0xfffff;
		break;
	case 1:
		data = S1_VTRAM_SIZE_1;
		break;
	case 3:
	case 4:
	case 5:
		data = 0x7ff;
		break;
	case 6:
		// adpcm memory on fmopn
		data = d_fmopn->debug_physical_addr_mask(0);
		break;
	case 7:
		// adpcm memory on expsg
		data = d_psgex->debug_physical_addr_mask(0);
		break;
	}
	return data;
}

bool MEMORY::debug_physical_addr_type_name(int type, _TCHAR *buffer, size_t buffer_len)
{
	bool exist = true;
	switch(type) {
	case 0:
		UTILITY::tcscpy(buffer, buffer_len, _T("using physical address."));
		break;
	case 1:
		UTILITY::tcscpy(buffer, buffer_len, _T("attribute RAM"));
		break;
	case 2:
		buffer[0] = _T('\0');
		break;
	case 3:
		UTILITY::tcscpy(buffer, buffer_len, _T("IG RAM (blue)"));
		break;
	case 4:
		UTILITY::tcscpy(buffer, buffer_len, _T("IG RAM (red)"));
		break;
	case 5:
		UTILITY::tcscpy(buffer, buffer_len, _T("IG RAM (green)"));
		break;
	case 6:
		UTILITY::tcscpy(buffer, buffer_len, _T("ADPCM memory on FM synth card"));
		break;
	case 7:
		UTILITY::tcscpy(buffer, buffer_len, _T("ADPCM memory on extended PSG port"));
		break;
	default:
		exist = false;
		break;
	}
	return exist;
}

uint32_t MEMORY::debug_read_bank(uint32_t addr)
{
	uint32_t data = 0;
	if (addr < 0xf0000) {
		if (S1_ADDR_IGRAM_BANK1 <= addr && addr < S1_ADDR_TRAP) {
			if ((ig_enreg & 7) == 7) {
				data = 0x3c;	// IG / Font
			} else {
				data = 0x33;	// IG
			}
		} else if (0xefe00 <= addr && addr < 0xefff0) {
			data = 0xff;	// io
		} else {
			data = s1banktype[addr >> S1_BANK_SIZE];
		}
	} else {
		if (REG_IGENABLE && L3_ADDR_IGRAM_START <= addr && addr < L3_ADDR_IGRAM_END) {
			data = l3banktype[(addr & 0xffff) >> L3_BANK_SIZE];
			data = (data & 0x0f) | 0x30;	// IG

		} else if (0xfff00 <= addr && addr <= 0xfffef) {
			data = 0xff;	// io
		} else {
			data = l3banktype[(addr & 0xffff) >> L3_BANK_SIZE];
		}
	}
	return data;
}

void MEMORY::debug_memory_map_info(DebuggerConsole *dc)
{
	uint32_t prev_addr = 0;
	uint8_t  prev_data = 0;
	uint32_t end_addr = 0x100000;
	uint32_t inc_addr = 16;

	for(uint32_t addr=0; addr <= end_addr; addr+=inc_addr) {
		uint8_t data = debug_read_bank(addr);
		if (addr == 0) {
			prev_data = data;
		}
		if (data != prev_data || addr == end_addr) {
			dc->Printf(_T("%05X - %05X : Read:"), prev_addr, addr-1);
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

void MEMORY::print_memory_bank(uint32_t data, bool w, _TCHAR *buffer, size_t buffer_len)
{
	_TCHAR str[32];
	if (w) {
		data >>= 4;
	}
	switch(data & 0x0f) {
	case 1:
		UTILITY::tcscpy(str, sizeof(str) / sizeof(str[0]), _T("Main RAM"));
		break;
	case 2:
		UTILITY::tcscpy(str, sizeof(str) / sizeof(str[0]), _T("Extend RAM"));
		break;
	case 3:
		UTILITY::tcscpy(str, sizeof(str) / sizeof(str[0]), _T("IG RAM"));
		break;
	case 4:
		UTILITY::tcscpy(str, sizeof(str) / sizeof(str[0]), _T("Main RAM (Red)"));
		break;
	case 5:
		UTILITY::tcscpy(str, sizeof(str) / sizeof(str[0]), _T("Graphic RAM"));
		break;
	case 6:
		UTILITY::tcscpy(str, sizeof(str) / sizeof(str[0]), _T("Text RAM"));
		break;
	case 7:
		UTILITY::tcscpy(str, sizeof(str) / sizeof(str[0]), _T("Comm EEPROM"));
		break;
	case 8:
		UTILITY::tcscpy(str, sizeof(str) / sizeof(str[0]), _T("L3 Basic ROM"));
		break;
	case 9:
		UTILITY::tcscpy(str, sizeof(str) / sizeof(str[0]), _T("MP-1805 ROM"));
		break;
	case 10:
		UTILITY::tcscpy(str, sizeof(str) / sizeof(str[0]), _T("MP-1802 ROM"));
		break;
	case 11:
		UTILITY::tcscpy(str, sizeof(str) / sizeof(str[0]), _T("S1 Basic ROM"));
		break;
	case 12:
		UTILITY::tcscpy(str, sizeof(str) / sizeof(str[0]), _T("Font ROM"));
		break;
	case 13:
		UTILITY::tcscpy(str, sizeof(str) / sizeof(str[0]), _T("Dictionary ROM"));
		break;
	case 14:
		UTILITY::tcscpy(str, sizeof(str) / sizeof(str[0]), _T("Communication ROM"));
		break;
	case 15:
		UTILITY::tcscpy(str, sizeof(str) / sizeof(str[0]), _T("I/O Port"));
		break;
	default:
		UTILITY::tcscpy(str, sizeof(str) / sizeof(str[0]), _T("(no assign)"));
		break;
	}
	UTILITY::stprintf(buffer, buffer_len, _T("%-18s"), str); 
}

int MEMORY::debug_address_map_info(DebuggerConsole *dc, int index)
{
	if ((index & 0xff00) == 0x100) {
		index = (IOPORT_USE_OS9BD ? REG_ADDRSEG : 0);
	}
	if (!IOPORT_USE_OS9BD) {
		index = 0;
	}
	int tskst = (0 <= index && index < 64 ? index : 0);
	int tsked = (0 <= index && index < 64 ? index + 1 : (IOPORT_USE_OS9BD ? 64 : 1));
	if (IOPORT_USE_OS9BD) {
		dc->Printf(_T("Current segment is %02X."), REG_ADDRSEG);
		dc->Cr();
	}
	for(int tsk = tskst; tsk < tsked; tsk++) {
		dc->Printf(_T("Segment %02X:"), tsk);
		if (IOPORT_USE_OS9BD) {
			if (REG_ADDRSEG == tsk) {
				dc->Print(_T(" Current"), false);
			}
		}
		dc->Cr();
		for(int i = 0; i < 16; i++) {
			if (i > 0) dc->PutCh(_T(' '));
			dc->Printf(_T("%01X:%02X"),i,addr_map[tsk][i]);
		}
		dc->Cr();
	}
	return 0;
}

int MEMORY::debug_address_map_edit(DebuggerConsole *dc, int index, int *values, int count)
{
	if (index < 0 || 64 <= index || (!IOPORT_USE_OS9BD && index != 0)) {
		return 1;
	}
	if (count > 16) count = 16;

	dc->Printf(_T("Segment %02X:"), index);
	dc->Cr();
	dc->Print(_T("        "), false);
	for(int i = 0; i < 16; i++) {
		dc->Printf(_T(" %01X "),i);
	}
	dc->Cr();

	dc->Print(_T("before: "), false);
	for(int i = 0; i < 16; i++) {
		dc->Printf(_T("%02X "),addr_map[index][i]);
	}
	dc->Cr();

	// backup
	for(int i = 0; i < 16; i++) {
		prev_addr_map[index][i] = addr_map[index][i];
	}
	prev_addr_map_enable[index] = true;

	// update
	dc->Print(_T("after:  "), false);
	for(int i = 0; i < 16; i++) {
		if (i < count) {
			addr_map[index][i] = (values[i] & 0xff);
		}
		dc->Printf(_T("%02X "),addr_map[index][i]);
	}
	dc->Cr();

	return 0;
}

int MEMORY::debug_address_map_get_prev(DebuggerConsole *dc, int index, int *values, int &count)
{
	if (index < 0 || 64 <= index) {
		return 1;
	}
	if (!prev_addr_map_enable[index]) {
		return 2;
	}
	if (count > 16) count = 16;
	for(int i = 0; i < count; i++) {
		values[i] = prev_addr_map[index][i];
	}
	return 0;
}

/// memory mapping reverse
uint32_t MEMORY::debug_address_mapping_rev(uint32_t addr)
{
	if (NOW_S1_MODE) {
		// S1 mode
		if (NOW_SYSTEM_MODE && ((addr & 0xff000) == 0xef000 || (addr & 0xff000) == 0x84000)) {
			// system mode
			addr = ((addr & 0x0fff) | ((addr & 0xff000) == 0xef000 ? 0xf000 : 0xe000));
		} else {
			// mapping
			bool found = false;
			for(int i=0; i<16; i++) {
				if ((addr & 0xff000) == ((uint32_t)addr_map[REG_ADDRSEG][i] << 12)) {
					addr = ((addr & 0x0fff) | (i << 12));
					found = true;
					break;
				}
			}
			if (!found) {
				addr = ~0;
			}
		}
	} else {
		// L3 mode
		addr &= 0xffff;
	}
	return addr;
}

int MEMORY::debug_memory_space_map_info(DebuggerConsole *dc, int index)
{
	switch(index & 0xff00) {
	case 0:
		bas->PrintMMPTBL(index & 0xff);
		break;
	case 0x100:
		bas->PrintCurrentSpaceMap(0);
		break;
	case 0x200:
		bas->PrintMRASGN();
		break;
	case 0x400:
		bas->PrintMCHTBL(index & 0xff);
		break;
	case 0x500:
		bas->PrintCurrentSpaceMap(0x400);
		break;
	case 0x600:
		bas->PrintSysCall(index & 0xff);
		break;
	case 0x700:
		bas->PrintCurrentSpaceMap(0x600);
		break;
	case 0x800:
		bas->PrintMMDPG(index & 0xff);
		break;
	case 0x900:
		bas->PrintCurrentSpaceMap(0x800);
		break;
	}
	return 0;
}

int MEMORY::debug_memory_space_map_edit(DebuggerConsole *dc, int index, int *values, int count)
{
	int mem = (index & 0xff);
	if (mem < 0 || mem >= 16) {
		return 1;
	}
	switch(index & 0xff00) {
	case 0:
		bas->EditMMPTBL(mem, values, count);
		break;
	case 0x400:
		bas->EditMCHTBL(mem, values, count);
		break;
	case 0x800:
		bas->EditMMDPG(mem, values, count);
		break;
	}
	return 0;
}

int MEMORY::debug_memory_space_map_get(DebuggerConsole *dc, int index, int *values, int &count)
{
	int mem = (index & 0xff);
	if (mem < 0 || mem >= 16) {
		return 1;
	}
	switch(index & 0xff00) {
	case 0:
		bas->GetMMPTBL(mem, values, count);
		break;
	}
	return 0;
}

bool MEMORY::debug_write_reg(uint32_t reg_num, uint32_t data)
{
	return false;
}

void MEMORY::debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
}

int MEMORY::get_debug_graphic_memory_size(int num, int type, int *width, int *height)
{
	return d_disp->get_debug_graphic_memory_size(num, type, width, height);
}

bool MEMORY::debug_graphic_type_name(int type, _TCHAR *buffer, size_t buffer_len)
{
	return d_disp->debug_graphic_type_name(type, buffer, buffer_len);
}

bool MEMORY::debug_draw_graphic(int type, int width, int height, scrntype *buffer)
{
	return d_disp->debug_draw_graphic(type, width, height, buffer);
}

bool MEMORY::debug_dump_graphic(int type, int width, int height, uint16_t *buffer)
{
	return d_disp->debug_dump_graphic(type, width, height, buffer);
}

bool MEMORY::debug_basic_is_supported()
{
	return true;
}

uint32_t MEMORY::debug_basic_get_line_number_ptr()
{
	return bas->GetLineNumberPtr();
}

uint32_t MEMORY::debug_basic_get_line_number()
{
	return bas->GetLineNumber();
}

void MEMORY::debug_basic_variables(DebuggerConsole *dc, int name_cnt, const _TCHAR **names)
{
	bas->PrintVariable(name_cnt, names);
}

void MEMORY::debug_basic_list(DebuggerConsole *dc, int st_line, int ed_line)
{
	if (st_line == -1 || st_line == ed_line) {
		bas->PrintCurrentLine(st_line);
	} else {
		bas->PrintList(st_line, ed_line, 0);
	}
}

void MEMORY::debug_basic_trace_onoff(DebuggerConsole *dc, bool enable)
{
	bas->TraceOnOff(enable);
}

void MEMORY::debug_basic_trace_current()
{
	bas->PrintCurrentTrace();
}

void MEMORY::debug_basic_trace_back(DebuggerConsole *dc, int num)
{
	bas->PrintTraceBack(num);
}

void MEMORY::debug_basic_command(DebuggerConsole *dc)
{
	bas->PrintCommandList();
}

void MEMORY::debug_basic_error(DebuggerConsole *dc, int num)
{
	bas->PrintError(num);
}

bool MEMORY::debug_basic_check_break_point(uint32_t line, int len)
{
	return bas->IsCurrentLine(line, line + len);
}

void MEMORY::debug_basic_post_checked_break_point()
{
	bas->UpdateCurrentLine();
}

#endif
