/** @file display.h

	HITACHI MB-S1 Emulator 'EmuB-S1'
	Skelton for retropc emulator

	@author Sasaji
	@date   2011.06.08 -

	@brief [ display ]
*/

#ifndef DISPLAY_H
#define DISPLAY_H

#include "../vm_defs.h"
//#include "../../emu.h"
#include "../device.h"

#define L3_ADDR_VRAM_START		0x0400
#define L3_ADDR_VRAM_END		0x4400
#define L3_VRAM_SIZE			0x4000
#define L3_VRAM_SIZE_1			0x3fff

#define IGRAM_SIZE				0x800

#define S1_ADDR_VTRAM_START		0xbc000
#define S1_ADDR_VTRAM_END		0xbd000
#define S1_VTRAM_SIZE			0x800
#define S1_VTRAM_SIZE_1			0x7ff

#define S1_ADDR_VGRAM_START		0xb0000
#define S1_ADDR_VGRAM_END		0xb4000
#define S1_VGRAM_SIZE_HALF		0x2000
#define S1_VGRAM_SIZE_HALF_1	0x1fff
#define S1_VGRAM_SIZE_ONE		0x4000
#define S1_VGRAM_SIZE_TWO		0x8000
#define S1_VGRAM_SIZE_TWO_1		0x7fff
#define S1_VGRAM_SIZE_ALL		0xc000
#define S1_VGRAM_SIZE_ONE_1		0x3fff

//#define VRAM_BUF_HEIGHT	524
//#define VRAM_BUF_HEIGHT	320
#define CRT_MON_HEIGHT 304

#define VRAM_BUF_HEIGHT	256
#define VRAM_BUF_WIDTH	128

class EMU;
class HD46505;

/**
	@brief Display Monitor
*/
class DISPLAY : public DEVICE
{
public:
	/// @brief signals on DISPLAY
	enum SIG_DISPLAY_IDS {
		SIG_DISPLAY_LPSTB	= 1,
		SIG_DISPLAY_CHR_CLOCKS	= 2,
		SIG_DISPLAY_VSYNC	= 3,
		SIG_DISPLAY_HSYNC	= 4,
		SIG_DISPLAY_DISPTMG	= 5,
		SIG_DISPLAY_WRITE_REGS	= 6
	};

private:
	HD46505* d_crtc;
	DEVICE*  d_board;

	uint8_t* crtc_regs;
	uint16_t cursor_addr;
	int *crtc_curdisp;

	uint8_t now_cpu_halt;				///< cpu halt when read palette while disptmg is on

	int   mode_sel;					///< ffd0
	bool  ig_mask;					///< compatible for MB-6890/6891
	uint8_t disable_ig;				///< disable ig ram (Limelight)

	uint8_t crtc_chr_clocks;		///< crtc clocks 0:1MHz 1:2MHz
	int   crtc_ppch;				///< crtc dot per char w.40:16 w.80:8

	// crtc reg8
	int  *videomode;				///< 1:videomode(interlace only)
	int  *disptmg_skew;				///< display timing skew
	int  *curdisp_skew;				///< cursor disp skew

	int   bg_color;					///< bg color (mode-sel bit0-bit2  S1 bit3:bright)
	int   hireso;					///< 0:normal 1:hireso (L3 only)
	int   width_sel;				///< 0:w.40 1:w.80 (S1:scrn-mode bit2  L3:mode-sel bit7)
	int   page_no;					///< page number on text (S1:40x25 only)

	int   gwidth_sel;				///< width on graphic 0:40 1:80 (S1:scrn-mode bit1)
	int   gpage_no;					///< page number on graphic (S1:320x200 8color only)

	// synchronizable vertical range
	int v_total_min[3];
	int v_total_max[3];

	int dws_left_diff;
	// synchronizable horizontal range
	int h_total_min[4];
	int h_total_max[4];

	int sw;

	uint32_t Rmask;
	uint32_t Gmask;
	uint32_t Bmask;
	uint32_t Amask;
	uint8_t  Rshift;
	uint8_t  Gshift;
	uint8_t  Bshift;
	uint8_t  Ashift;
	uint32_t Rloss[2];
	uint32_t Gloss[2];
	uint32_t Bloss[2];
	uint32_t Rboad;
	uint32_t Gboad;
	uint32_t Bboad;

	uint8_t font[0x2000];			///< font rom
	uint8_t chrfont[2][256][16];	///< converted font rom [interace][chr_code][line]
	uint8_t* l3vram;
	uint8_t* color_ram;
	uint8_t* ig_ram;

	uint8_t* s1vtram;		///< text vram
	uint8_t* s1vcram;		///< text color vram
	uint8_t* s1vgrram;	///< graphic vram red
	uint8_t* s1vggram;	///< graphic vram green
	uint8_t* s1vgbram;	///< graphic vram blue

	typedef union un_brg_color {
		struct {
#ifdef _BIG_ENDIAN
			uint8_t i,g,r,b;
#else
			uint8_t b,r,g,i;
#endif
		} b;
		uint32_t i;
	} t_brg_color;

#define BRG_COLOR_IMASK 0x00ffffff

	// CRT monitor
	typedef struct st_crt_mon {
		uint8_t disptmg;		///<   bit1: hsync start  bit0: disptmg area
		uint8_t text;			///< S1: text vram		L3: vram data
		uint8_t color;			///< S1: text color ram	L3: color ram
		uint8_t attr;			///< S1: graphic attribute L3: bg color
		t_brg_color c;			///< S1: text color
		t_brg_color g[2];		///< S1: graphic color
		t_brg_color brg_mask;	///< S1: graphic brg mask
	} t_crt_mon;
	t_crt_mon crt_mon[VRAM_BUF_HEIGHT][VRAM_BUF_WIDTH];
	int crt_mon_v_count;
	int crt_mon_v_start;
	int crt_mon_col;
	int crt_mon_row;
	int crt_mon_l3vaddr;
	int crt_mon_s1vtaddr;	///< text vram addr * 2
	int crt_mon_s1vgaddr;	///< graphic vram addr * 2
#ifdef USE_KEEPIMAGE
	int crt_mon_page;
#endif
	int crt_mon_stepcols;
//	int crt_mon_top;

	bool crt_mon_crtc_hsync;
	int crt_mon_bg_color;

	typedef struct st_crt_mon_vline {
		// horizontal
		int16_t left_col;				///< display timing area left side
#ifdef USE_LIGHTPEN
		int16_t right_col;				///< display timing area right side
#endif
		uint8_t pal[8];					///< palette
		uint8_t flags;					///< bit0:odd line  bit7:640x400 mode
	} t_crt_mon_vline;
	t_crt_mon_vline crt_mon_vline[VRAM_BUF_HEIGHT];

	typedef struct st_crt_mon_vinfo {
		int16_t row;
		uint8_t raster;					///< bit0-5:rastar line
		uint8_t cursor_st_col;			///< cursor display position
		uint8_t cursor_ed_col;			///< cursor display position
	} t_crt_mon_vinfo;
	t_crt_mon_vinfo crt_mon_vinfo[SCREEN_HEIGHT];

#ifdef USE_LIGHTPEN
	int crt_mon_disptmg_top;
	int crt_mon_disptmg_bottom;
#endif

	t_brg_color disp_page_brg_mask;	///< from $fe23 bit5-7
	t_brg_color disp_page_changed;	///< $fe23 bit5-7 changed

	int screen_left;
	int screen_right;
	int screen_top;
	int screen_bottom;
	int scrnline1_offset;
	scrntype scrnline1[SCREEN_HEIGHT][SCREEN_WIDTH];
	uint8_t scrnflag[SCREEN_HEIGHT][SCREEN_WIDTH];

	scrntype color16[16];		// B + R*2 + G*4 + I*8
	scrntype color64[16][16];	// B + R*2 + G*4 + I*8
	int  ppd[8];	// pixel per 1dot (L3)
	uint8_t dof[8];	// dot mask (L3)

	uint8_t scanline;
	int  afterimage;
	int  buf_stepcols;

	int skip_frame_count;
	int *crtc_vt_total;		///< crtc total rows (dot unit)
	int *crtc_vt_count;		///< crtc now line
	int *crtc_vt_disp;		///< crtc can display rows (disptmg)
	int *crtc_ma;			///< crtc memory address
	int *crtc_ra;			///< crtc raster address
	int *crtc_max_ra;		///< crtc max raster address
	int *crtc_odd_line;		///< crtc odd line (videomode only)
	int *crtc_vs_start;		///< crtc vsync start line
	int *crtc_vs_end;		///< crtc vsync end line

	int crtc_vs_mid;

	int *vm_pause;

	//for resume
#pragma pack(1)
	struct vm_state_st {
		int   crtc_addr;

		uint8_t ig_enable;
		uint8_t ig_modereg;
		uint8_t sys_mode;
		uint8_t mode_sel;

		uint8_t ig_mask;
		uint8_t chr_clocks;
		uint8_t now_cpu_halt;
		uint8_t interlace_sel;

		uint8_t tvsuper;

		char  reserved1[3];

		// screen register for S1 
		uint8_t bmsk_color;
		uint8_t active_plane;
		uint8_t dummy1;
		uint8_t disp_page;
		uint8_t scrn_mode;
		uint8_t gcolor[3];
		uint8_t palette[8];
	};
#pragma pack()

	int  font_rom_loaded;
	int  l3font_rom_loaded;
	bool font_rom_loaded_at_first;

	// for draw_screen and update_vram
	t_brg_color iwk1;
	uint8_t wk1u8, wk2u8;

	uint8_t dws_pattern;
	uint8_t dws_text_code;
	uint8_t dws_c_f;
//	uint8_t dws_c_b;
	uint8_t dws_text_cram;
	uint8_t dws_attr;
	t_brg_color dws_ct[2],dws_ct_m;	// text pattern (r,g,b,mask)
	t_brg_color dws_cg[2],dws_cg_m;	// grachic pattern (r,g,b,mask)
	t_brg_color dws_cd[2],dws_cd_m;	// draw pattern (r,g,b,mask)
	uint8_t dws_cdt2[2][8];		// dot pattern 0:LSB...7:MSB
	t_brg_color dws_b_p;			// bg palette num (r,g,b)
	t_brg_color dws_t_p;			// text palette num (r,g,b)
	t_brg_color dws_p_p;			// palette num (r,g,b)
	int dws_bt;
	int dws_bi;
	int dws_dt, dws_dtm;
	int dws_x,dws_x0,dws_x1,dws_xodd;
	int dws_y,dws_y0,dws_y1,dws_ys,dws_yd;
	int dws_raster,dws_raster2;
	int dws_int_vid;
	int dws_videomode_n;
	bool dws_disp_cursor_line;
	bool dws_disp_cursor_char;
	int dws_buf_x;
//	int dws_buf_y;
	int dws_buf_y_h;
	int dws_gline;
//	bool dws_l_flg;
	t_crt_mon *dws_p_crt_mon;
	t_crt_mon *dws_p_crt_mon_x;
	t_crt_mon_vline *dws_p_vline;
	t_crt_mon_vinfo *dws_p_vinfo;

	int dws_scrn_offset;
	scrntype *dws_scrn0;
	scrntype *dws_scrn;
	scrntype dws_dot;
	scrntype *dws_scrnline1p;
	uint8_t *dws_scrnf;
	scrntype dws_scrnline2m;
	uint32_t dws_r,dws_g,dws_b;

	int crt_mon_disptmg_left;

	void load_font_rom_file();
	uint8_t get_font_data(int, int, int);
	uint8_t get_l3font_data(int, int, int);
	void set_font_data();
	void set_l3font_data();
	void conv_to_l3font_data();

	void update_dws_params();
	void update_chr_clocks(int clk);

	void draw_screen_sub();
	void draw_screen_sub_afterimage1();
	void draw_screen_sub_afterimage2();

//	inline void set_palette_brg();
//	inline void set_palette_brg(int num);

	inline void set_disp_page();
	inline void set_s1_screen_mode();
	inline void set_l3_screen_mode();

	void change_palette();

public:
	DISPLAY(VM* parent_vm, EMU* parent_emu, const char* identifier) : DEVICE(parent_vm, parent_emu, identifier)
	{
		set_class_name("DISPLAY");
	}
	~DISPLAY() {}

	// common functions
	void initialize();
	void reset();
	void update_config();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_frame();
	void update_display(int v, int clock);

	// unique function
	void set_context_crtc(HD46505* device) {
		d_crtc = device;
	}
	void set_context_board(DEVICE* device) {
		d_board = device;
	}
	void set_l3vram_ptr(uint8_t* ptr) {
		// vram is main ram!
		l3vram = ptr;
	}
	void set_s1vtram_ptr(uint8_t* ptr) {
		// text vram
		s1vtram = ptr;
	}
	void set_s1vcram_ptr(uint8_t* ptr) {
		// text color ram
		s1vcram = ptr;
	}
	void set_s1vgrram_ptr(uint8_t* ptr) {
		// graphic vram red
		s1vgrram = ptr;
	}
	void set_s1vggram_ptr(uint8_t* ptr) {
		// graphic vram green
		s1vggram = ptr;
	}
	void set_s1vgbram_ptr(uint8_t* ptr) {
		// graphic vram blue
		s1vgbram = ptr;
	}
	void set_color_ram_ptr(uint8_t* ptr) {
		color_ram = ptr;
	}
	void set_ig_ram_ptr(uint8_t* ptr) {
		ig_ram = ptr;
	}
	void set_regs_ptr(uint8_t* ptr) {
		crtc_regs = ptr;
	}
	void set_crtc_vt_ptr(int* ptrt, int *ptrc, int *ptrd) {
		crtc_vt_total = ptrt;
		crtc_vt_count = ptrc;
		crtc_vt_disp  = ptrd;
	}
	void set_crtc_ma_ra_ptr(int *ptrma, int *ptrra) {
		crtc_ma = ptrma;
		crtc_ra = ptrra;
	}
	void set_crtc_max_ra_ptr(int *ptr) {
		crtc_max_ra = ptr;
	}
	void set_crtc_odd_line_ptr(int *ptr) {
		crtc_odd_line = ptr;
	}
	void set_crtc_reg8_ptr(int *ptrv, int *ptrds, int *ptrcs) {
		videomode = ptrv;
		disptmg_skew = ptrds;
		curdisp_skew = ptrcs;
	}
	void set_crtc_curdisp_ptr(int *ptr) {
		crtc_curdisp = ptr;
	}
	void set_crtc_vsync_ptr(int *ptrst, int *ptred) {
		crtc_vs_start = ptrst;
		crtc_vs_end = ptred;
	}
	void set_display_size(int left, int top, int right, int bottom);
	void draw_screen();
	uint8_t *get_font() {
		return font;
	}

	void save_state(FILEIO *fio);
	bool load_state(FILEIO *fio);

#ifdef USE_DEBUGGER
	uint32_t debug_read_io8(uint32_t addr);

	int  get_debug_graphic_memory_size(int num, int type, int *width, int *height);
	bool debug_graphic_type_name(int type, _TCHAR *buffer, size_t buffer_len);
	bool debug_draw_graphic(int type, int width, int height, scrntype *buffer);
	bool debug_dump_graphic(int type, int width, int height, uint16_t *buffer);
#endif
};

#endif /* DISPLAY_H */

