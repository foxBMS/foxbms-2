; @copyright &copy; 2010 - 2025, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
; All rights reserved.
;
; SPDX-License-Identifier: BSD-3-Clause
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions are met:
;
; 1. Redistributions of source code must retain the above copyright notice, this
;    list of conditions and the following disclaimer.
;
; 2. Redistributions in binary form must reproduce the above copyright notice,
;    this list of conditions and the following disclaimer in the documentation
;    and/or other materials provided with the distribution.
;
; 3. Neither the name of the copyright holder nor the names of its
;    contributors may be used to endorse or promote products derived from
;    this software without specific prior written permission.
;
; THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
; AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
; IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
; DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
; FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
; DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
; SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
; CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
; OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
; OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
;
; We kindly request you to use one or more of the following phrases to refer to
; foxBMS in your hardware, software, documentation or advertising materials:
;
; - "This product uses parts of foxBMS&reg;"
; - "This product includes parts of foxBMS&reg;"
; - "This product is derived from foxBMS&reg;"

        .sect ".text"
        .arm

;-------------------------------------------------------------------------------
; This Function implements the SVC handler to change the user mode to system mode
; More information can be found via following links:
; ARM Assembly Language Tools v20.2.0.LTS
; https://www.ti.com/lit/ug/spnu118z/spnu118z.pdf?ts=1718741253041
; The register set of ARM cortex-R5
; https://developer.arm.com/documentation/ddi0460/d/Programmers-Model/Registers/The-register-set?lang=en
; The program status register of ARM cortex-R5
; https://developer.arm.com/documentation/ddi0460/d/Programmers-Model/Program-status-registers?lang=en
; Interrupt and Exception Handling on Hercules(TM) ARM(R) Cortex(R)-R4/5-Based Microcontrollers
; https://www.ti.com/lit/an/spna218/spna218.pdf?ts=1718741961684
; Some instructions
; https://developer.arm.com/documentation/ddi0406/c/System-Level-Architecture/System-Instructions/Alphabetical-list-of-instructions/MSR--register-?lang=en
; MRS:https://developer.arm.com/documentation/dui0489/i/arm-and-thumb-instructions/mrs--psr-to-general-purpose-register-
; MSR:https://developer.arm.com/documentation/dui0489/i/arm-and-thumb-instructions/msr--general-purpose-register-to-psr-
; TST:https://developer.arm.com/documentation/dui0489/i/arm-and-thumb-instructions/tst
; ORR:https://developer.arm.com/documentation/dui0489/i/arm-and-thumb-instructions/orr
; MOV:https://developer.arm.com/documentation/dui0489/i/arm-and-thumb-instructions/mov

        .def     switch_operation_mode
        .asmfunc                        ; The .asmfunc and .endasmfunc directives
                                        ; mark function boundaries

        .align 4
switch_operation_mode:
        MRS     R3, SPSR                ; Get SPSR value.
        TST     R3, #0x20               ; Check if called in Thumb state by performing
                                        ; bitwise AND between R3 and 0x20, update
                                        ; the condition flags if the thumb state bit is set.
        LDRNEH  R2, [lr,#-2]            ; Yes: Load Register halfword (16-bit).
        BICNE   R2, R2, #0xFF00         ; Clears the upper byte of R2 to extract
                                        ; the SVC number from the last 8 bits of the halfword.
        LDREQ   R2, [lr,#-4]            ; No: Load Register word (32-bit).
        BICEQ   R2, R2, #0xFF000000     ; Clears the upper 3 bytes of R2 to extract
                                        ; the SVC number from the last 24 bits of the word.
        CMP R2, #1                      ; Compare R2 with 1.
        BEQ _case1                      ; Branch to _case1 if R2 == 1.
        B _default_case                 ; Branch to default_case if no match.

_case1: ; switch to system mode
        ORR     R3, R3, #0x0000001F     ; Bitwise OR operation for setting the
                                        ; the lower 5 bits of the register R3.
        MSR     SPSR_c, R3              ; The control fields of the SPSR are being
                                        ; updated by the value in R3.
        MOVS PC, LR                     ; Moves the value in the link Register
                                        ; (LR) to the Program Counter (PC) and copy
                                        ; the value in SPSP to CPSP.
_default_case: ; default case

        .endasmfunc

;-------------------------------------------------------------------------------
; This file contains an assembly function to return the operation mode
; https://developer.arm.com/documentation/ddi0460/d/Programmers-Model/
; Modes-of-operation-and-execution/Instruction-set-states?lang=en

        .def     DetectOperationMode
        .asmfunc                        ; The .asmfunc and .endasmfunc directives
                                        ; mark function boundaries

        .align 4
DetectOperationMode:
    MRS r2, CPSR                ; Move CPSR into register r2
    AND r0, r2, #0x1F           ; Mask all but the mode bits, the result will be
                                ; saved into r0, which value will be the returned
                                ; value from this function.
    BX lr                       ; Return from function call

        .endasmfunc

;-------------------------------------------------------------------------------

; Returns zero if caller was not in privileged mode.
; User mode:              10000b or 0x10
; Privileged/system mode: 11111b or 0x1F
        .def R5_IS_PRIVILEGED
        .asmfunc                        ; The .asmfunc and .endasmfunc directives
                                        ; mark function boundaries

        .align 4
R5_IS_PRIVILEGED:                      ; privileged mode equals system mode in documentation
                MRS r2, cpsr           ; Move CPSR into register r2
                mov r0, #0             ; Set return value to 0
                AND r1, r2, #0x1F      ; Mask all but the mode bits, the result will be
                                       ; saved into r1.
                CMP r1,#0x1F           ; compare to 0x1F = system mode
                bne notPrivileged      ; Return default vale (0) if not true.
                mov r0, #1             ; Return 1 if comparision was true.
notPrivileged   BX lr                  ; Return from function call
                .endasmfunc

.end    ; Terminates assembly, the assembler ignores any source statements that
        ; follows an '.end' directive.
