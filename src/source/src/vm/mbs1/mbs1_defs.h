/** @file mbs1_defs.h

	HITACHI MB-S1 Emulator
	Skelton for retropc emulator

	@author Sasaji
	@date   2015.06.08 -

	@brief [ virtual machine ]
*/

#ifndef MBS1_DEFS_H
#define MBS1_DEFS_H

#define FRAME_SPLIT_NUM	1

#define DEVICE_NAME		"HITACHI MB-S1 model05"
#define CONFIG_NAME		"mbs1"
#define CLASS_NAME      "MBS1"
#define CONFIG_VERSION		10

// device informations for virtual machine
#define USE_EMU_INHERENT_SPEC

#define FRAMES_PER_10SECS	600
#define FRAMES_PER_SEC		60
#define LINES_PER_FRAME 	262
#define CHARS_PER_LINE		64

#define CLOCKS_1MHZ			1008000
#define CPU_CLOCKS			2016000
#if defined(USE_Z80B_CARD)
#define Z80B_CLOCKS			6000000
#define NUMBER_OF_CPUS		2
#elif defined(USE_MPC_68008)
#define MC68K_CLOCKS		8064000
#define NUMBER_OF_CPUS		2
#define USE_MC68008
#define USE_MEM_REAL_MACHINE_CYCLE	1
#define USE_MC68000_IRQ_LEVEL
#define MAIN_SUB_CLOCK_RATIO	2
#else
#define NUMBER_OF_CPUS		1
#endif
#define USE_CPU_REAL_MACHINE_CYCLE	1
#define USE_CPU_HALF_SPEED	1
#define CLOCKS_CYCLE		120960000	// need divisible by 30

#define MAX_SOUND	12

//#define SCREEN_WIDTH		640
#define SCREEN_WIDTH		768
//#define SCREEN_HEIGHT		480
#define SCREEN_HEIGHT		512
#define LIMIT_MIN_WINDOW_WIDTH		640
#define LIMIT_MIN_WINDOW_HEIGHT		400
#define MIN_WINDOW_WIDTH		640
#define MIN_WINDOW_HEIGHT		480
#define MAX_WINDOW_WIDTH		768
#define MAX_WINDOW_HEIGHT		512

// max devices connected to the output port
#if defined(USE_Z80B_CARD) || defined(USE_MPC_68008)
#define MAX_OUTPUT	24
#else
#define MAX_OUTPUT	22
#endif

// device informations for win32
#define USE_SPECIAL_RESET
#define USE_DATAREC
// #define USE_ALT_F10_KEY
#define USE_AUTO_KEY		3
#define USE_AUTO_KEY_CAPS
#define USE_SCANLINE
#define USE_AFTERIMAGE
#define USE_DIPSWITCH
#define DIPSWITCH_DEFAULT 0x03
#define HAS_AY_3_8913
#define HAS_YM2608
#define USE_FMGEN_OPNA_STEREO
//#define USE_AUDIO_U8

#define USE_PRINTER
#define MAX_PRINTER		3
//#define USE_LIGHTPEN
#define USE_MOUSE
//#define USE_MOUSE_ABSOLUTE
#define USE_JOYSTICK
#define USE_PIAJOYSTICK


#define USE_FD1
#define USE_FD2
#define USE_FD3
#define USE_FD4
#define HAS_MB8876
#define MAX_DRIVE		4
#define USE_DRIVE		4

#define USE_SOCKET
#define USE_UART
#define MAX_COMM		2

#define USE_RTC

#define USE_STATE
#define USE_KEY_RECORD

#define USE_LEDBOX
#define USE_MESSAGE_BOARD
#define USE_VKEYBOARD

#ifdef USE_WIN
//#define USE_SCREEN_D3D_TEXTURE
#define USE_SCREEN_D3D_MIX_SURFACE

#define USE_DIRECTINPUT
#endif

//#define USE_PERFORMANCE_METER

#define RESUME_FILE_HEADER "RESUME_MBS1"
#define RESUME_FILE_HEADER_L3 "RESUME_BML3MK5"
#define RESUME_FILE_VERSION 1
#define RESUME_FILE_REVISION 1

#ifdef USE_PIAJOYSTICK
#define KEYBIND_MAX_NUM	3
#else
#define KEYBIND_MAX_NUM	2
#endif
#define KEYBIND_KEYS	130
#define KEYBIND_JOYS	24
#define KEYBIND_ASSIGN	2
#define KEYBIND_PRESETS	4

/// @ingroup Enums
/// @brief device masks of NMI signal
enum SIG_NMI_MASKS {
	SIG_NMI_TRACE_MASK		= 0x01,
	SIG_NMI_TRAP_MASK		= 0x02,
	SIG_NMI_FD_MASK			= 0x04,
	SIG_NMI_KEYBREAK_MASK	= 0x08,
#if defined(USE_Z80B_CARD)
	SIG_NMI_Z80BCARD_MASK	= 0x1000,	///< same as SIG_IRQ_Z80BCARD_MASK
#endif
};

/// @ingroup Enums
/// @brief device masks of IRQ signal
enum SIG_IRQ_MASKS {
	SIG_IRQ_KEYBOARD_MASK	= 0x001,
	SIG_IRQ_MOUSE_MASK		= 0x002,
	SIG_IRQ_PIAA_MASK		= 0x004,
	SIG_IRQ_PIAB_MASK		= 0x008,
	SIG_IRQ_EXPIAA_MASK		= 0x010,
	SIG_IRQ_EXPIAB_MASK		= 0x020,
	SIG_IRQ_ACIA_MASK		= 0x040,
	SIG_IRQ_EXACIA_MASK		= 0x080,
	SIG_IRQ_9PSG_MASK		= 0x100,
	SIG_IRQ_FMOPN_MASK		= 0x200,
	SIG_IRQ_FMOPNEX_MASK	= 0x400,
#if defined(USE_Z80B_CARD)
	SIG_IRQ_Z80BCARD_MASK	= 0x1000,
#endif
};

/// @ingroup Enums
/// @brief device masks of FIRQ signal
enum SIG_FIRQ_MASKS {
	SIG_FIRQ_TIMER1_MASK	= 0x001,
	SIG_FIRQ_TIMER2_MASK	= 0x002,
	SIG_FIRQ_TIMER_MASK		= 0x003,
	SIG_FIRQ_FMOPN_MASK		= 0x200,	///< same as SIG_IRQ_FMOPN_MASK
	SIG_FIRQ_FMOPNEX_MASK	= 0x400,	///< same as SIG_IRQ_FMOPNEX_MASK
};

/// @ingroup Enums
/// @brief device masks of HALT signal
enum SIG_HALT_MASKS {
	SIG_HALT_PALETTE_MASK	= 0x01,
	SIG_HALT_FD_MASK		= 0x02,
#if defined(USE_MPC_68008)
	SIG_HALT_MPC68008_MASK	= 0x2000,
#endif
};

/// @ingroup Enums
/// @brief device masks of FUSE register
enum SIG_FUSE_MASKS {
	SIG_FUSE_INTR_MASK		= 1,
};

//#define _FDC_DEBUG_LOG

#endif /* MBS1_DEFS_H */
