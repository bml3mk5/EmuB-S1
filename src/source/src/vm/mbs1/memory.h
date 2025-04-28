/** @file memory.h

	HITACHI MB-S1 Emulator 'EmuB-S1'
	Skelton for retropc emulator

	@author Sasaji
	@date   2011.06.08 -

	@brief [ memory ]
*/

#ifndef MEMORY_H
#define MEMORY_H

#include "../vm_defs.h"
#include "../device.h"

class EMU;
#if defined(USE_Z80B_CARD)
class Z80B_CARD;
#elif defined(USE_MPC_68008)
class MPC_68008;
#endif
#ifdef USE_DEBUGGER
class L3Basic;
#endif

#define S1_BANK_SIZE	11	///< 2Kbytes
#define L3_BANK_SIZE	4	///< 16bytes

/**
	@brief Memory access
*/
class MEMORY : public DEVICE
{
public:
	/// @brief signals on MEMORY
	enum SIG_MEMORY_IDS {
		SIG_MEMORY_DISP		= 0,
		SIG_MEMORY_VSYNC	= 1,
//		SIG_MEMORY_PIA_PA	= 2,
		SIG_TRACE_COUNTER	= 4,
		SIG_TRACE_COUNTER_STOP	= 5
	};

private:
//	DEVICE *d_cpu[2];
	DEVICE *d_pia, *d_crtc, *d_disp, *d_acia;
	DEVICE *d_key, *d_mouse, *d_sound, *d_comm, *d_cmt, *d_timer;
	DEVICE *d_kanji, *d_fdc3, *d_fdc5, *d_fdc5_ex, *d_fdd;
	DEVICE *d_psgst, *d_psgex, *d_psg3, *d_psg9, *d_fmopn;
	DEVICE *d_pia_ex, *d_acia_ex, *d_comm1;
	DEVICE *d_rtc, *d_board;
#if defined(USE_Z80B_CARD)
	Z80B_CARD *d_z80b_card;
#elif defined(USE_MPC_68008)
	MPC_68008 *d_mpc_68008;
#endif
	DEVICE *d_event;

//	int num_of_cpus;

	uint8_t l3rom[0x6000];		///< 24KB
	uint8_t s1rom[0x10000];		///< 64KB
	uint8_t s1dicrom[0x8000];		///< 32KB
	uint8_t rom1802[0x800];
	uint8_t rom1805[0x800];
	uint8_t *cm01rom;				///< 32KB
	uint8_t *cm01eeprom;			///< 8KB

	uint8_t ram[0x10000];			///< main ram (S1 is also used as graphic vram red)
//	uint8_t color_ram[0x4000];	///< 0x0400 - 0x43ff -> use vgram
	uint8_t ig_ram[0x0800 * 3];	///< 0xa000 - 0xa7ff * 3 (b,r,g)
//	uint8_t ram2[0x10000];		///< extended ram
	int   exram_size;			///< extended ram size (KB)
	uint8_t *exram;				///< extended ram (variable)

	uint8_t vgram[0x8000];		///< S1 graphic ram (green,blue)(32KB)
	uint8_t vtram[0x800];			///< S1 text ram (2KB)
	uint8_t vcram[0x800];			///< S1 color ram (2KB)
	uint8_t *font;				///< font rom (4KB)

	uint8_t addr_map[64][16];		///< address mapping table

	uint8_t wdmy[1 << S1_BANK_SIZE];
	uint8_t rdmy[1 << S1_BANK_SIZE];
	uint8_t* l3wbank[0x10000 >> L3_BANK_SIZE];
	uint8_t* l3rbank[0x10000 >> L3_BANK_SIZE];
	uint8_t l3banktype[0x10000 >> L3_BANK_SIZE];
	uint8_t* s1wbank[0x100000 >> S1_BANK_SIZE];
	uint8_t* s1rbank[0x100000 >> S1_BANK_SIZE];
	uint8_t s1banktype[0x100000 >> S1_BANK_SIZE];

	/// bit0:read wait bit4:write wait
	/// bit1,5: refresh memory wait
	/// bit2,6: disptmg wait
	uint8_t  l3wait[0x10000 >> L3_BANK_SIZE];
	uint8_t  s1wait[0x100000 >> S1_BANK_SIZE];

	uint8_t color_reg;		///< 0xffd8

//	uint8_t ig_enable;		///< 0xffe9
	uint8_t ig_enreg;			///< 0xffea

	uint8_t mem_bank_reg1;	///< 0xffc0
	uint8_t mem_bank_reg2;	///< 0xffe8
	bool    mem_vram_sel;	///< select vram (L3 mode)
	bool    mem_bank_mask;	///< 0xffc0 mask

//	uint8_t vram_sel;
//	uint8_t cpu_wait;			///< io access (bit1) 0:1MHz 2:2MHz

//	int   trace_counter_id;
//	uint32_t pc, pc_prev;
	int   trace_counter_phase;
	int   trace_count_remain;
	int   prev_trace_clock;

	int   l3rom_loaded;
	int   s1rom1_loaded;
	int   s1rom2_loaded;
	int   s1dicrom_loaded;
	int   rom1802_loaded;
	int   rom1805_loaded;
	bool  rom_loaded_at_first;

	int   cm01rom_loaded;
	int   cm01eeprom_loaded;

	bool  cm01rom_access_ok;
	bool  cm01basic_enable;
	uint8_t cm01eeprom_bank;
	bool  cm01eeprom_writing;

	uint32_t addr_bank;
	uint32_t addr_seg04, addr_seg16, addr_seg32;
	uint32_t addr_comio, addr_64kb;

	bool  bfxxx_access;

	//for resume
#pragma pack(1)
	struct vm_state_st_l3 {
		uint8_t ram[0x10000];
		uint8_t ram2[0x10000];		// extended ram
		uint8_t color_ram[0x4000];	// 0x0400 - 0x43ff
		uint8_t ig_ram[0x0800 * 3];	// 0xa000 - 0xa7ff * 3 (b,r,g)

		uint8_t mem_bank_reg1;	// from PIA port A
		uint8_t mem_bank_reg2;	// from 0xffe8

		int   trace_counter_id;
		uint32_t pc;
		uint32_t pc_prev;

		// from config
		uint8_t dipswitch;
		int fdd_type;
		int io_port;

		char  reserved[9];
	};

	struct vm_state_st_s1 {
		uint8_t ram[0x10000];
		uint8_t vgram[0x8000];		// graphic ram
		uint8_t vtram[0x800];			// text chr
		uint8_t vcram[0x800];			// text color
		uint8_t ig_ram[0x0800 * 3];	// ig (b,r,g)

//		int   trace_counter_id;
//		uint32_t pc;
//		uint32_t pc_prev;
		int   trace_counter_phase;	// version 0x42
		int   trace_count_remain;	// version 0x42
		int   prev_trace_clock;		// version 0x42

		uint8_t mem_bank_reg1;	// from 0xffc0
		uint8_t mem_bank_reg2;	// from 0xffe8
		uint8_t mem_bank_mask;	// from 0xffc0 mask
		char  reserved1;

		// from config
		uint8_t dipswitch;
		uint8_t sys_mode;
		int fdd_type;
		int io_port;

		uint8_t mode_sel;
		uint8_t color_reg;
		uint8_t ig_enreg;
		uint8_t reg_fuse;
		uint8_t reg_trapf;
		uint8_t reg_busctrl;

		union {
			struct {
				uint8_t addr_map[16];
				char  reserved2[256];	// for OS-9 exboard

				char  exram_ident[14];
				uint16_t exram_size;
				// exram (variable)
				// uint8_t exram[0 - 512KB]
			} v42;
			struct {
				uint8_t addr_map[64][16];
				uint8_t reg_addrseg;

				char reserved3[7];
				char exram_ident[6];
				uint16_t exram_size;
				// exram (variable)
				// uint8_t exram[0 - 512KB]
			} v43;
		};
	};
#pragma pack()

	void SET_BANK(int s, int e, uint8_t* w, uint8_t* r, uint8_t** wbank, uint8_t** rbank, int bank_size, uint8_t *waitbank, uint8_t waitval, uint8_t *banktype, uint8_t type);

	void load_rom_files();
	void save_rom_files();
	void reset_sram(uint8_t *sram, size_t size);
	void reset_igram();
	void reset_s1_membank();
	void reset_cm01_membank();
	void reset_l3_membank();
	void reset_trace_counter();
	void set_igmode(uint32_t data);
	inline void process_trace_counter(int clk);
	inline void fetch_trace_counter(int clk);
	inline void set_trace_counter(int clk);
	inline void fire_trace_counter(int clk);
	inline void clear_trace_counter(int clk);

	inline bool write_data_s1(uint32_t addr, uint32_t data);
	inline bool read_data_s1(uint32_t addr, uint32_t &data);

	void change_cm01_membank();
	void change_l3_memory_bank();
	void change_system_mode(bool usermode);
	void reset_s1_l3_mode();
	inline void set_cpu_speed(uint8_t data);

	void set_extended_ram();

#ifdef USE_DEBUGGER
	DebuggerConsole *dc;
	L3Basic *bas;

	bool prev_addr_map_enable[64];
	uint8_t prev_addr_map[64][16];	///< previous values of address mapping table
#endif

public:
	MEMORY(VM* parent_vm, EMU* parent_emu, const char* identifier);
	~MEMORY();

	// common functions
	void initialize();
	void reset();
	void release();
	void write_data8w(uint32_t addr, uint32_t data, int *wait);
	uint32_t read_data8w(uint32_t addr, int *wait);
#if defined(USE_MPC_68008)
	void write_data8_68kw(uint32_t addr, uint32_t data, int *wait);
	uint32_t read_data8_68kw(uint32_t addr, int *wait);
#endif
	void latch_address(uint32_t addr, int *wait);

//	void write_data16(uint32_t addr, uint32_t data) {
//		write_data8(addr, data & 0xff); write_data8(addr + 1, data >> 8);
//	}
//	uint32_t read_data16(uint32_t addr) {
//		return read_data8(addr) | (read_data8(addr + 1) << 8);
//	}
	void write_io8(uint32_t addr, uint32_t data);
	void write_signal(int id, uint32_t data, uint32_t mask);
	uint32_t read_signal(int id);

	// unique function
//	void set_context_cpu(DEVICE* device) {
//		if (num_of_cpus < 2) d_cpu[num_of_cpus++] = device;
//	}
	void set_context_pia(DEVICE* device) {
		d_pia = device;
	}
	void set_context_crtc(DEVICE* device) {
		d_crtc = device;
	}
	void set_context_display(DEVICE* device) {
		d_disp = device;
	}
	void set_context_acia(DEVICE* device) {
		d_acia = device;
	}
	void set_context_key(DEVICE* device) {
		d_key = device;
	}
	void set_context_mouse(DEVICE* device) {
		d_mouse = device;
	}
	void set_context_sound(DEVICE* device) {
		d_sound = device;
	}
	void set_context_psg(DEVICE* device0, DEVICE* device1) {
		d_psgst = device0;
		d_psgex = device1;
	}
	void set_context_psg3(DEVICE* device) {
		d_psg3 = device;
	}
	void set_context_psg9(DEVICE* device) {
		d_psg9 = device;
	}
	void set_context_fmopn(DEVICE* device) {
		d_fmopn = device;
	}
	void set_context_comm(DEVICE* device) {
		d_comm = device;
	}
	void set_context_cmt(DEVICE* device) {
		d_cmt = device;
	}
	void set_context_timer(DEVICE* device) {
		d_timer = device;
	}
	void set_context_kanji(DEVICE* device) {
		d_kanji = device;
	}
	void set_context_fdc(DEVICE* device3, DEVICE* device5, DEVICE* device5e) {
		d_fdc3 = device3;
		d_fdc5 = device5;
		d_fdc5_ex = device5e;
	}
	void set_context_fdd(DEVICE* device) {
		d_fdd = device;
	}
	void set_context_pia_ex(DEVICE* device) {
		d_pia_ex = device;
	}
	void set_context_acia_ex(DEVICE* device) {
		d_acia_ex = device;
	}
	void set_context_comm1(DEVICE* device) {
		d_comm1 = device;
	}
	void set_context_rtc(DEVICE* device) {
		d_rtc = device;
	}
	void set_context_board(DEVICE* device) {
		d_board = device;
	}
#if defined(USE_Z80B_CARD)
	void set_context_z80bcard(Z80B_CARD* device) {
		d_z80b_card = device;
	}
	DEVICE *d_z80b;
#elif defined(USE_MPC_68008)
	void set_context_mpc_68008(MPC_68008* device) {
		d_mpc_68008 = device;
	}
	DEVICE *d_mc68k;
#endif
	void set_context_event(DEVICE* device) {
		d_event = device;
	}
	uint8_t* get_l3vram();
	uint8_t* get_s1vtram();
	uint8_t* get_s1vcram();
	uint8_t* get_s1vgrram();
	uint8_t* get_s1vggram();
	uint8_t* get_s1vgbram();

	uint8_t* get_color_ram() {
		return vgram;
	}
	uint8_t* get_ig_ram() {
		return ig_ram;
	}
	void set_font(uint8_t *val) {
		font = val;
	}

	uint32_t address_mapping(uint32_t addr);

//	void event_callback(int event_id, int err);
	void update_config();

	void save_state(FILEIO *fio);
	bool load_state(FILEIO *fio);


#ifdef USE_DEBUGGER
	void set_debugger_console(DebuggerConsole *dc);

	uint32_t debug_latch_address(uint32_t addr);

	void debug_write_data8(int type, uint32_t addr, uint32_t data);
	uint32_t debug_read_data8(int type, uint32_t addr);
	void debug_write_data16(int type, uint32_t addr, uint32_t data);
	uint32_t debug_read_data16(int type, uint32_t addr);
	void debug_write_data32(int type, uint32_t addr, uint32_t data);
	uint32_t debug_read_data32(int type, uint32_t addr);

	uint32_t debug_physical_addr_mask(int type);
	bool debug_physical_addr_type_name(int type, _TCHAR *buffer, size_t buffer_len);

	uint32_t debug_read_bank(uint32_t addr);
	void debug_memory_map_info(DebuggerConsole *dc);
	void print_memory_bank(uint32_t data, bool w, _TCHAR *buffer, size_t buffer_len);
	int debug_address_map_info(DebuggerConsole *dc, int index);
	int debug_address_map_edit(DebuggerConsole *dc, int index, int *values, int count);
	int debug_address_map_get_prev(DebuggerConsole *dc, int index, int *values, int &count);
	uint32_t debug_address_mapping_rev(uint32_t addr);

	int debug_memory_space_map_info(DebuggerConsole *dc, int index);
	int debug_memory_space_map_edit(DebuggerConsole *dc, int index, int *values, int count);
	int debug_memory_space_map_get(DebuggerConsole *dc, int index, int *values, int &count);

	bool debug_write_reg(uint32_t reg_num, uint32_t data);
	void debug_regs_info(_TCHAR *buffer, size_t buffer_len);

	int  get_debug_graphic_memory_size(int num, int type, int *width, int *height);
	bool debug_graphic_type_name(int type, _TCHAR *buffer, size_t buffer_len);
	bool debug_draw_graphic(int type, int width, int height, scrntype *buffer);
	bool debug_dump_graphic(int type, int width, int height, uint16_t *buffer);

	uint32_t debug_basic_get_line_number_ptr();
	uint32_t debug_basic_get_line_number();

	bool debug_basic_is_supported();
	void debug_basic_variables(DebuggerConsole *dc, int name_cnt, const _TCHAR **names);
	void debug_basic_list(DebuggerConsole *dc, int st_line, int ed_line);
	void debug_basic_trace_onoff(DebuggerConsole *dc, bool enable);
	void debug_basic_trace_current();
	void debug_basic_trace_back(DebuggerConsole *dc, int num);
	void debug_basic_command(DebuggerConsole *dc);
	void debug_basic_error(DebuggerConsole *dc, int num);

	bool debug_basic_check_break_point(uint32_t line, int len);
	void debug_basic_post_checked_break_point();

#endif

};

#endif /* MEMORY_H */
