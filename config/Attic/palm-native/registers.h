/*
    Copyright � 1995-2001, The AROS Development Team. All rights reserved.
    $Id$
*/

#ifndef REGISTERS_H
#define REGISTERS_h

#ifdef __PALM_CODE__
#define WREG_L(addr)	*(Long *)addr
#define RREG_L(addr)	*(Long *)addr
#define WREG_W(addr)	*(Word *)addr
#define RREG_W(addr)	*(Word *)addr
#define WREG_B(addr)	*(Byte *)addr
#define RREG_B(addr)	*(Byte *)addr
#else
#include <exec/types.h>
#define WREG_L(addr)	*(ULONG *)addr
#define RREG_L(addr)	*(ULONG *)addr
#define WREG_W(addr)	*(UWORD *)addr
#define RREG_W(addr)	*(UWORD *)addr
#define WREG_B(addr)	*(UBYTE *)addr
#define RREG_B(addr)	*(UBYTE *)addr
#endif

#define IRQ_LEVEL1	0x064
#define IRQ_LEVEL2	0x068
#define IRQ_LEVEL3	0x06c
#define IRQ_LEVEL4	0x070
#define IRQ_LEVEL5	0x074
#define IRQ_LEVEL6	0x078
#define TRAP_0		0x080
#define TRAP_1		0x084
#define TRAP_2		0x088
#define TRAP_3		0x08c
#define TRAP_4		0x090

#define SCR             0xfffff000
#define PCR             0xfffff003

/*
 * Whatever is in chpater 6 in the downloadable 
 * dragonball documentation seems to be wrong.
 * Found the correct register description in the 
 * xcopilot source.
 */
#if 0

#define CSGBA		0xfffff100
#define CSGBB		0xfffff102
#define CSGBC		0xfffff104
#define CSGBD		0xfffff106
#define CSUGBA		0xfffff108
#define CSCR		0xfffff10A
#define CSA		0xfffff110
#define CSB		0xfffff112
#define CSC		0xfffff114
#define CSD		0xfffff116

#else

#define GRPBASEA	0xfffff100
#define GRPBASEB	0xfffff102
#define GRPBASEC	0xfffff104
#define GRPBASED	0xfffff100

#define GRPMASKA	0xfffff108
#define GRPMASKB	0xfffff10A
#define GRPMASKC	0xfffff10C
#define GRPMASKD	0xfffff10E

#define CSA0		0xfffff110
#define CSA1		0xfffff114
#define CSA2		0xfffff118
#define CSA3		0xfffff11c

#define CSC0		0xfffff130
#define CSC1		0xfffff134
#define CSC2		0xfffff138
#define CSC3		0xfffff13c

/*
 * CSA0-3, CSC0-3:
 *
 * Bit 0-2  : Wait states
 * Bit 3    : Read only (if '1')
 * Bit 4-7  : reserved
 * Bit 8-15 : AM (???)
 * Bit 16   : bus width
 * Bit 17-23: reserved
 * Bit 24-31: AC (???)
 */
#endif

#define PCTLR		0xfffff207
#define IVR		0xfffff300
#define ICR             0xfffff302
#define IMR             0xfffff304
#define ISR             0xfffff30c
#define IPR             0xfffff310
#define ILCR            0xfffff314

#define TCTL1		0xfffff600
#define TPRER1          0xfffff602
#define TCMP1           0xfffff604
#define TCR1            0xfffff606
#define TCN1            0xfffff608
#define TSTAT1          0xfffff60a

#define TCTL2		0xfffff610
#define TPRER2          0xfffff612
#define TCMP2           0xfffff614
#define TCR2            0xfffff616
#define TCN2            0xfffff618
#define TSTAT2          0xfffff61a

#define USTCNT1		0xfffff900
#define UBAUD1          0xfffff902
#define URX1            0xfffff904
#define UTX1            0xfffff906
#define UMISC1          0xfffff908
#define NIPR1           0xfffff90a

#define USTCNT2		0xfffff910
#define UBAUD2          0xfffff912
#define URX2            0xfffff914
#define UTX2            0xfffff916
#define UMISC2          0xfffff918
#define NIPR2           0xfffff91a

#define LSSA		0xfffffa00
#define LVPW            0xfffffa05
#define LXMAX           0xfffffa08
#define LYMAX           0xfffffa0a
#define LCXP            0xfffffa18
#define LCYP            0xfffffa1a

#define LCWCH           0xfffffa1c
#define LBLKC           0xfffffa1f
#define LPICF           0xfffffa20
#define LPOLCF          0xfffffa21
#define LACDRC          0xfffffa23
#define LPXCD           0xfffffa25
#define LCKCON          0xfffffa27
#define LRRA            0xfffffa28

#define LPOSR           0xfffffa2d
#define LFRCM           0xfffffa31
#define LGPMR           0xfffffa33
#define PWMR            0xfffffa36
#define RMCR            0xfffffa38

#define RTCTIME         0xfffffb00
#define RTCAKRM         0xfffffb04
#define WATCHDOG        0xfffffb0a
#define RTCCTL          0xfffffb0c
#define RTCISR          0xfffffb0e
#define RTCIENR         0xfffffb10
#define STPWCH          0xfffffb12
#define DAYR            0xfffffb1a
#define DAYALARM        0xfffffb1c

#define DRAMMC		0xfffffc00
#define DRAMC		0xfffffc00
#define SDCTRL		0xfffffc00
#define SDPWDN		0xfffffc00

#endif
