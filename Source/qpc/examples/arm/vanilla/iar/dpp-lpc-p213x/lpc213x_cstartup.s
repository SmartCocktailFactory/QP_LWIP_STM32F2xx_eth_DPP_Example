;-----------------------------------------------------------------------------
; Product: startup code for the NXP LPC213x CPU with IAR C/C++ compiler
; Date of the Last Update:  May 27, 2008
;
;                    Q u a n t u m     L e a P s
;                    ---------------------------
;                    innovating embedded systems
;
; Copyright (C) 2002-2008 Quantum Leaps, LLC. All rights reserved.
;
; This software may be distributed and modified under the terms of the GNU
; General Public License version 2 (GPL) as published by the Free Software
; Foundation and appearing in the file GPL.TXT included in the packaging of
; this file. Please note that GPL Section 2[b] requires that all works based
; on this software must also be made publicly available under the terms of
; the GPL ("Copyleft").
;
; Alternatively, this software may be distributed and modified under the
; terms of Quantum Leaps commercial licenses, which expressly supersede
; the GPL and are specifically designed for licensees interested in
; retaining the proprietary status of their code.
;
; Contact information:
; Quantum Leaps Web site:  http://www.quantum-leaps.com
; e-mail:                  info@quantum-leaps.com
;-----------------------------------------------------------------------------
;
; Naming covention of labels in this file:
;
;  ?xxx   - External labels only accessed from assembler.
;  __xxx  - External labels accessed from or defined in C.
;  xxx    - Labels local to one module (note: this file contains
;           several modules).
;  main   - The starting point of the user program.
;
;-----------------------------------------------------------------------------


;-----------------------------------------------------------------------------
; Macros and definitions for the whole file

I_BIT               DEFINE      0x80  ; CPSR[7]
F_BIT               DEFINE      0x40  ; CPSR[6]
USR_MODE            DEFINE      0x10  ; User mode
FIQ_MODE            DEFINE      0x11  ; Fast Interrupt Request mode
IRQ_MODE            DEFINE      0x12  ; Interrupt Request mode
SVC_MODE            DEFINE      0x13  ; Supervisor mode
ABT_MODE            DEFINE      0x17  ; Abort mode
UND_MODE            DEFINE      0x1B  ; Undefined Instruction mode
SYS_MODE            DEFINE      0x1F  ; System mode

; LPC213x Memory Mapping Control
LPC213X_RAM         DEFINE      0x40000000 ; RAM is always at this address
LPC231X_MEMMAP      DEFINE      0xE01FC040 ; The Memory Contoller Remap reg.

;-----------------------------------------------------------------------------
; Reset and other vectors
;
    SECTION .intvec:CODE:NOROOT(2)
    PUBLIC  __vector
    PUBLIC  __vector_0x14
    PUBLIC  __iar_program_start

; Execution starts here.
; After a reset, the mode is ARM, Supervisor, interrupts disabled.

    CODE32                  ; Always ARM mode after reset
__vector:
    LDR     pc,[pc,#24]     ; load the secondary vector
    LDR     pc,[pc,#24]     ; load the secondary vector
    LDR     pc,[pc,#24]     ; load the secondary vector
    LDR     pc,[pc,#24]     ; load the secondary vector
    LDR     pc,[pc,#24]     ; load the secondary vector
__vector_0x14:
    DC32    0               ; checksum (see "LPC213x User's Manual" 20.4.2)
    LDR     pc,[pc,#24]     ; load the secondary vector
    LDR     pc,[pc,#24]     ; load the secondary vector

    ; The secondary jump talbe starts below. It contains the
    ; addresses for the PC-relative loads from the primary vector table.
    ; The secondary vectors are to be overwritten by the application to hook
    ; up exception handlers at runtime. The default implementation provided
    ; below causes endless loops around the selected addresses.
    ; For example, a prefetch abort exception forces the PC to 0x0C and
    ; the address loaded from the secondary vector table will be again 0x0C.
    ;
    DC32    __iar_program_start  ; Reset
    DC32    0x04            ; Undefined Instruction
    DC32    0x08            ; Software Interrupt
    DC32    0x0C            ; Prefetch Abort
    DC32    0x10            ; Data Abort
    DC32    0x14            ; Reserved
    DC32    0x18            ; IRQ
    DC32    0x1C            ; FIQ


;-----------------------------------------------------------------------------
; Reset handler
;

    SECTION FIQ_STACK:DATA:NOROOT(3)
    SECTION IRQ_STACK:DATA:NOROOT(3)
    SECTION SVC_STACK:DATA:NOROOT(3)
    SECTION ABT_STACK:DATA:NOROOT(3)
    SECTION UND_STACK:DATA:NOROOT(3)
    SECTION CSTACK:DATA:NOROOT(3)

    SECTION .text:CODE:NOROOT(2)
    REQUIRE __vector
    EXTERN  ?main
    PUBLIC  __iar_program_start

    ; Embed a copyright notice prominently at the begining of
    ; the Flash image.
    ;
    DC8     'Copyright (c) 2008, Quantum Leaps, LLC. All Rights Reserved.'
    ALIGNROM 2              ; re-align after the string at 2^2 boundary

    CODE32                  ; reset always executes in ARM
__iar_program_start:
    ; Copy the exception vectors from ROM to RAM
    ;
    ; NOTE: the exception vectors must be in RAM *before* the remap
    ; in order to guarantee that the ARM core is provided with valid vectors
    ; during the remap operation.
    ;
    LDR     r8,=__vector    ; Source
    LDR     r9,=LPC213X_RAM ; Destination

    MOV     r0,#0
    STR     r0,[r9,#0]      ; write zero at the start of RAM
    LDR     r0,[r0,#0]      ; read the location zero
    TEQ     r0,#0           ; is the result equal to zero?

    LDMIA   r8!,{r0-r7}     ; Load Vectors
    STMIA   r9!,{r0-r7}     ; Store Vectors
    LDMIA   r8!,{r0-r7}     ; Load secondary vector table
    STMIA   r9!,{r0-r7}     ; Store secondary vector table

    ; Setup Memory Mapping Control to remap the RAM vector area
    LDR     r0,=LPC231X_MEMMAP
    MOV     r1,#2           ; MEMMAP to User RAM mode
    STRNE   r1,[r0,#0]      ; Remap Memory, if not remapped already

    ; Initialize the stack pointers...
    ; The stack segments must be defined in the linker command file.
    ;

    MSR     cpsr_c,#(IRQ_MODE | I_BIT | F_BIT) ; Change to IRQ mode
    LDR     sp,=SFE(IRQ_STACK)                 ; End of IRQ_STACK

    MSR     cpsr_c,#(FIQ_MODE | I_BIT | F_BIT) ; Change to FIQ mode
    LDR     sp,=SFE(FIQ_STACK)                 ; End of FIQ_STACK

    MSR     cpsr_c,#(SVC_MODE | I_BIT | F_BIT) ; Change to SVC mode
    LDR     sp,=SFE(SVC_STACK)                 ; End of SVC_STACK

    MSR     cpsr_c,#(ABT_MODE | I_BIT | F_BIT) ; Change to ABT mode
    LDR     sp,=SFE(ABT_STACK)                 ; End of ABT_STACK

    MSR     cpsr_c,#(UND_MODE | I_BIT | F_BIT) ; Change to UND mode
    LDR     sp,=SFE(UND_STACK)                 ; End of UND_STACK

    MSR     cpsr_c,#(SYS_MODE | I_BIT | F_BIT) ; Change to SYSTEM mode
    LDR     sp,=SFE(CSTACK)                    ; End of CSTACK

    ; Add more initialization here...


    ; Continue to ?main for more IAR specific system startup
    ;
    LDR     r12,=?main
    BX      r12

    ; The code should never return from ?main, but just in case, put
    ; an endless loop here to hang the CPU.
    ; Alternatively, you might try a reset...
    ;
    B       .

    END
