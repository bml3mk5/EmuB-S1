/** @file registers.h

	HITACHI MB-S1 Emulator 'EmuB-S1'
	Skelton for retropc emulator

	@author Sasaji
	@date   2015.09.01 -

	@brief [ registers on circit ]
*/

#ifndef REGISTERS_H
#define REGISTERS_H

#include "../../common.h"
#include "../../config.h"

/// @defgroup MACROS_REGISTERS Macros related to REGISTERS class
/// Macros related to @ref REGISTERS class

/// @ingroup MACROS_REGISTERS
///@{
//#define SIG_INTRRUPT	registers.sig_intr
//#define SIG_IRQ_BIT		1	/* IRQ line number  */
//#define SIG_FIRQ_BIT	2	/* FIRQ line number */
//#define SIG_NMI_BIT		4	/* NMI line number  */
//#define NOW_SIG_IRQ		(SIG_INTRRUPT & SIG_IRQ_BIT)
//#define NOW_SIG_FIRQ	(SIG_INTRRUPT & SIG_FIRQ_BIT)
//#define NOW_SIG_NMI		(SIG_INTRRUPT & SIG_NMI_BIT)

#define REG_FUSE		registers.reg_fuse
#define FUSE_TRACE_MASK	(REG_FUSE & 0x80)
#define FUSE_INTR_MASK	(REG_FUSE & 0x40)
#define FUSE_MODE_DELAY	(REG_FUSE & 0x0f)
#define FUSE_INTR_BIT	0x40

#define REG_ADDRSEG		registers.reg_addrseg

#define REG_TRAPF		registers.reg_trapf
#define TRAPF_SIGNAL	0x80

#define REG_BUSCTRL		registers.reg_busctrl
#define BUSCTRL_SIGNAL	0x80

#define REG_BMSK_COLOR		registers.reg_bmsk_color
#define BMSK_COLOR_BMSK		(REG_BMSK_COLOR & 0x80)
#define BMSK_COLOR_B		(REG_BMSK_COLOR & 0x01)
#define BMSK_COLOR_R		(REG_BMSK_COLOR & 0x02)
#define BMSK_COLOR_G		(REG_BMSK_COLOR & 0x04)

#define REG_ACTIVE_PLANE	registers.reg_active_plane
#define ACTIVE_PLANE_DW		(REG_ACTIVE_PLANE & 0x08)
#define ACTIVE_PLANE_B		(REG_ACTIVE_PLANE & 0x01)
#define ACTIVE_PLANE_R		(REG_ACTIVE_PLANE & 0x02)
#define ACTIVE_PLANE_G		(REG_ACTIVE_PLANE & 0x04)
#define ACTIVE_PLANE_BRG	(REG_ACTIVE_PLANE & 0x07)

#define SIG_BMSK_DW			registers.bmsk_dw

#define REG_DISP_PAGE	registers.reg_disp_page
#define DISP_PAGE_TEXT		(REG_DISP_PAGE & 0x01)
#define DISP_PAGE_GRAPHIC	(REG_DISP_PAGE & 0x02)
#define DISP_PAGE_GRAPHIC_B (REG_DISP_PAGE & 0x20)
#define DISP_PAGE_GRAPHIC_R (REG_DISP_PAGE & 0x40)
#define DISP_PAGE_GRAPHIC_G (REG_DISP_PAGE & 0x80)
#define DISP_PAGE_GRAPHIC_BRG (REG_DISP_PAGE & 0xe0)
#define DISP_PAGE_B_MASK	0x20
#define DISP_PAGE_R_MASK	0x40
#define DISP_PAGE_G_MASK	0x80
#define DISP_PAGE_BRG_MASK	0xe0

#define REG_SCRN_MODE	registers.reg_scrn_mode
#define SCRN_MODE_TEXT_WIDTH	(REG_SCRN_MODE & 0x04)
#define SCRN_MODE_GRAPHIC_WIDTH	(REG_SCRN_MODE & 0x02)
#define SCRN_MODE_T_AND_G_WIDTH	(REG_SCRN_MODE & 0x06)
#define SCRN_MODE_GRAPHIC		(REG_SCRN_MODE & 0x03)
#define SCRN_MODE_IS_64COLOR	((REG_SCRN_MODE & 0x03) == 0x01)
#define SCRN_MODE_IS_640x400	((REG_SCRN_MODE & 0x03) == 0x02)
#define SCRN_MODE_MASK			0x03
#define SCRN_MODE_64COLOR_MASK	0x01
#define SCRN_MODE_640x400_MASK	0x02

#define REG_GCOLOR		registers.reg_gcolor
#define REG_GCOLOR_B	REG_GCOLOR[0]
#define REG_GCOLOR_R	REG_GCOLOR[1]
#define REG_GCOLOR_G	REG_GCOLOR[2]

#define REG_PALETTE			registers.reg_palette

#define REG_MODE_SEL		registers.reg_mode_sel
#define REG_REMOTE			registers.reg_remote
#define REG_MUSIC_SEL		registers.reg_music_sel
#define REG_TIME_MASK		registers.reg_time_mask
#define REG_INTERLACE_SEL	registers.reg_interlace_sel
#define REG_BAUD_SEL		registers.reg_baud_sel

#define REG_SYS_MODE		config.sys_mode
#define SYS_MODE_S1L3		1
#define SYS_MODE_CLK		2
#define SYS_MODE_SU			4
#define NOW_S1_MODE			((REG_SYS_MODE & 1) != 0)
#define NOW_L3_MODE			((REG_SYS_MODE & 1) == 0)
#define NOW_2MHZ			((REG_SYS_MODE & 2) != 0)
#define NOW_1MHZ			((REG_SYS_MODE & 2) == 0)
#define NOW_SYSTEM_MODE		((REG_SYS_MODE & 4) != 0)
#define NOW_USER_MODE		((REG_SYS_MODE & 4) == 0)

#define REG_TVSUPER			config.tvsuper
// 0x10:analog 0:digital
#define TVSUPER_DIGITAL		((REG_TVSUPER & 0x10) == 0)
#define TVSUPER_ANALOG		((REG_TVSUPER & 0x10) != 0)

#define REG_IGENABLE		registers.reg_igenable

#define REG_IGMODE			registers.reg_igmode
#define DISPTMG_ON			(REG_IGMODE & 0x04)
#define HSYNC_ON			(REG_IGMODE & 0x02)
#define VSYNC_ON			(REG_IGMODE & 0x01)
#define HVSYNC_ON			(REG_IGMODE & 0x03)
#define PAE_MASK			0x80
#define DISPTMG_MASK		0x04
#define HSYNC_MASK			0x02
#define VSYNC_MASK			0x01
#define REG_IGMODE_UNUSE	~(PAE_MASK | DISPTMG_MASK | HSYNC_MASK | VSYNC_MASK)

#define SIG_MBC_INTREFKIL	registers.sig_mbc
#define SIG_MBC_MASK		0x01
#define SIG_INTREFKIL_MASK	0x02
#define SIG_MBC_ALL_MASK	0x03
#define SIG_MBC_IS_ON		(SIG_MBC_INTREFKIL & SIG_MBC_MASK)
#define SIG_INTREFKIL_IS_ON	(SIG_MBC_INTREFKIL & SIG_INTREFKIL_MASK)
///@}

/**
	@brief Common register on the board

	@sa MACROS_REGISTERS
*/
class REGISTERS
{
public:
	uint8_t reg_fuse;			///< 0xfe10 fuse force user mode
	uint8_t reg_addrseg;		///< 0xfe11 segment (for OS-9 board)

	uint8_t reg_trapf;		///< 0xfe18 trap flag
	uint8_t reg_busctrl;		///< 0xfe19 bus control

	uint8_t reg_bmsk_color;	///< 0xfe20 bmsk color
	uint8_t reg_active_plane;	///< 0xfe21 active plane
	uint8_t bmsk_dw;			///< bmsk color bit7 and active plane bit3
	uint8_t reg_disp_page;	///< 0xfe23 disp page
	uint8_t reg_scrn_mode;	///< 0xfe24 screen mode
	uint8_t reg_gcolor[3];	///< 0xfe25-0xfe27 graphic color (b,r,g) / scroll
	uint8_t reg_palette[8];	///< 0xfe28-0xfe2f color palette

	uint8_t reg_mode_sel;		///< 0xffd0 mode sel
	uint8_t reg_remote;		///< 0xffd2 remote for cmt
	uint8_t reg_music_sel;	///< 0xffd3 music sel
	uint8_t reg_time_mask;	///< 0xffd4 time mask
	uint8_t reg_interlace_sel;///< 0xffd6 interlace sel (shifted)  0:noninterlace char  1:interlace char
	uint8_t reg_baud_sel;		///< 0xffd7 baud select 0:600 1:1200 2:2400 3:300 4:4800(not support)

	uint8_t reg_igenable;		///< 0xffe9 write
	uint8_t reg_igmode;		///< 0xffe9 read

	uint8_t sig_mbc;		///< multi bus control & int ref kill (use in z80B card)
public:
	REGISTERS();
	virtual ~REGISTERS() {}

	virtual void initialize();
};

extern REGISTERS registers;

#endif /* REGISTERS_H */
