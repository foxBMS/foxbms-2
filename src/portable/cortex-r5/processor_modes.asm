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
; vPortIsPrivileged

; Returns zero if caller was not in privileged mode.
; User mode:              10000b or 0x10
; Privileged/system mode: 11111b or 0x1F
                .def R5_IS_PRIVILEGED
                .asmfunc
R5_IS_PRIVILEGED            ; privileged mode equals system mode in documentation
                MRS r2, cpsr           ; Move CPSR into register r2
                mov r0, #0             ; Set return value to 0
                AND r1, r2, #0x1F      ; Mask all but the mode bits, the result will be
                                       ; saved into r1.
                CMP r1,#0x1F           ; compare to 0x1F = system mode
                bne notPrivileged      ; Return default vale (0) if not true.
                mov r0, #1             ; Return 1 if comparision was true.
notPrivileged   BX lr                  ; Return from function call
                .endasmfunc
