/** @file z80_consts.h

	Skelton for retropc emulator

	@par Origin
	MAME 0.145
	@author Takeda.Toshiya
	@date   2012.02.15-

	@note Modify for MB-S1 By Sasaji on 2019.10.21 -

	@brief [ Z80 constant values ]
*/

#ifndef _Z80_CONSTS_H_ 
#define _Z80_CONSTS_H_

/// @brief flag bit in the F register on Z80
enum Z80_FLAG_BIT {
	 CF	= 0x01,	/**< Carry Flag */
	 NF	= 0x02,	/**< Add/Subtract */
	 PF	= 0x04,	/**< Parity Flag */
	 VF	= PF,	/**< Overflow Flag */
	 XF	= 0x08,	/**< not used */
	 HF	= 0x10,	/**< Half Carry Flag */
	 YF	= 0x20,	/**< not used */
	 ZF	= 0x40,	/**< Zero Flag */
	 SF	= 0x80,	/**< Sign Flag */
};

/* @brief int_state on Z80 */
enum Z80_INT_STATE_BIT {
	Z80_STATE_EI		= 0x0001,
	Z80_STATE_HALT		= 0x0004,
	Z80_STATE_RESET		= 0x0008,
	Z80_STATE_NMI		= 0x0010,
	Z80_STATE_INTR		= 0x0020,
	Z80_STATE_BUSREQ	= 0x0040,
	Z80_STATE_BUSACK	= 0x0080,

	Z80_STATE_BUSREQ_REL	= 0x400000,	/**< for debug */
	Z80_STATE_BUSACK_REL	= 0x800000,	/**< for debug */
};

#endif /* _Z80_CONSTS_H_ */

