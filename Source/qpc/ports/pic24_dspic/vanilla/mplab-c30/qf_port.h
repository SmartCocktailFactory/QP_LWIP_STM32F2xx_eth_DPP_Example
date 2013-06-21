/*****************************************************************************
* Product:  QF/C, PIC24/dsPIC, vanilla kernel, MPLAB-C30 compiler
* Last Updated for Version: 4.4.00
* Date of the Last Update:  Feb 03, 2012
*
*                    Q u a n t u m     L e a P s
*                    ---------------------------
*                    innovating embedded systems
*
* Copyright (C) 2002-2012 Quantum Leaps, LLC. All rights reserved.
*
* This software may be distributed and modified under the terms of the GNU
* General Public License version 2 (GPL) as published by the Free Software
* Foundation and appearing in the file GPL.TXT included in the packaging of
* this file. Please note that GPL Section 2[b] requires that all works based
* on this software must also be made publicly available under the terms of
* the GPL ("Copyleft").
*
* Alternatively, this software may be distributed and modified under the
* terms of Quantum Leaps commercial licenses, which expressly supersede
* the GPL and are specifically designed for licensees interested in
* retaining the proprietary status of their code.
*
* Contact information:
* Quantum Leaps Web site:  http://www.quantum-leaps.com
* e-mail:                  info@quantum-leaps.com
*****************************************************************************/
#ifndef qf_port_h
#define qf_port_h

     /* The maximum number of active objects in the application, see NOTE01 */
#define QF_MAX_ACTIVE           8

#define QF_EVENT_SIZ_SIZE       1
#define QF_EQUEUE_CTR_SIZE      1
#define QF_MPOOL_SIZ_SIZE       1
#define QF_MPOOL_CTR_SIZE       1
#define QF_TIMEEVT_CTR_SIZE     2

                                 /* QF interrupt disable/enable, see NOTE02 */
#define QF_INT_DISABLE()            __asm__ volatile("disi #0x3FFF")
#define QF_INT_ENABLE()             __asm__ volatile("disi #0x0000")

                              /* QF critical section entry/exit, see NOTE02 */
/* QF_CRIT_STAT_TYPE not defined: unconditional interrupt unlocking" policy */
#define QF_CRIT_ENTRY(dummy)        __asm__ volatile("disi #0x3FFF")
#define QF_CRIT_EXIT(dummy)         __asm__ volatile("disi #0x0000")


#include "qep_port.h"                                           /* QEP port */
#include "qvanilla.h"                       /* "Vanilla" cooperative kernel */
#include "qf.h"                 /* QF platform-independent public interface */


/*****************************************************************************
* NOTE01:
* The maximum number of active objects QF_MAX_ACTIVE can be increased up to
* 63, inclusive. The lower limit of 8 is used here to save RAM and also
* because the internal scheduler algorithms perform slighlty better for this
* lower limit.
*
* NOTE02:
* The DISI (disable interrupts) instruction is used for very fast,
* unconditional locking and unlocking of interrupts. The DISI #0x3FFF
* instruction disables interrupts for 16383 instruction cycles, which is
* much longer than any critical section in QP. The DISI #0 instruction
* is then used to unconditionally unlock the interrupts at the end of the
* critical section. The DISI instruction only disables interrupts with
* priority levels 1-6. Priority level 7 interrupts and all trap events still
* have the ability to interrupt the CPU when the DISI instruction is active.
* This means that from the perspective of QP, the level 7 interrupts are
* treated as non-maskable interrupts (NMIs). Such non-maskable interrupts
* _cannot_ call any QP services. In particular, they cannot post events.
*
* CAUTION: This QP port assumes that interrupt nesting is _enabled_,
* which is the default in the PIC24/dsPIC processors. Interrupt nesting should
* never be disabled by setting the NSTDIS control bit (INTCON1<15>). If you
* don't want interrupts to nest, you can always prioritize them at the same
* level. For example, the default priority level for all interrupts is 4 out
* of reset. If you don't change this level for any interrupt the nesting of
* interrupt will not occur.
*/

#endif                                                         /* qf_port_h */
