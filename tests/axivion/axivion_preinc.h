/**
 *
 * @copyright &copy; 2010 - 2023, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * We kindly request you to use one or more of the following phrases to refer to
 * foxBMS in your hardware, software, documentation or advertising materials:
 *
 * - &Prime;This product uses parts of foxBMS&reg;&Prime;
 * - &Prime;This product includes parts of foxBMS&reg;&Prime;
 * - &Prime;This product is derived from foxBMS&reg;&Prime;
 *
 */

/* Required by Axivion */

#pragma diag_suppress unrecognized_pragma, unrecognized_attribute, invalid_error_tag, invalid_error_number, 69, 951

/* We use GNU mode to emulate extensions such as __attribute__,
   but we can't define __GNUC__ or the system headers go down the wrong code path. */
#undef __GNUC__
#undef __GNUG__

/* clang-format off */

#define __near
#define __far
#define __aligned(x)
#define __inline inline
#define __intaddr__ __INTADDR__

#define __cregister
#define __interrupt
#define __restrict

#define cregister __cregister
#define interrupt __interrupt
/* #define restrict __restrict */

/* TI internally uses _assert() which returns and does not have [[noreturn]] semantics.
 * However our Externals-CStdLib.NoReturn by default treats _assert() as noreturn.
 * Define _assert to a different name here in the preinc (before first use).  */
#define _assert(x,y) __axivion_ti_assert(x,y)

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__TI_ARM__)
/* TI Arm Clang Compiler Tools User Guide, Release v2.1.0.LTS
   2.3. Migrating C and C++ Source Code: Conversions that Require More Attention */
extern void _dmb(void);
/* https://developer.arm.com/documentation/dui0491/i/Compiler-specific-Features/--clrex-intrinsic */
extern void __clrex(void);
/* https://developer.arm.com/documentation/dui0491/i/Compiler-specific-Features/--ldrex-intrinsic */
extern unsigned int __ldrexw(volatile void*);
/* TI Arm Clang Compiler Tools User Guide, Release v2.1.0.LTS
   2.3. Migrating C and C++ Source Code: Non-ACLE Compiler Intrinsics */
extern unsigned int _disable_interrupts(void);
extern void _restore_interrupts(unsigned int);
extern void _assert(int, const char*);
extern void _nassert(int src);

/* TI _atomic.h redefines __atomic_compare_exchange(), however cafe has already
 * the correct one built-in, so make the macro immutable. */
#pragma immutable_macro __atomic_compare_exchange

/* cafe has a different __atomic_is_lock_free() built-in, so redefine the
 * name in TI headers to avoid ambiguitiy. */
#define __atomic_is_lock_free __atomic_is_lock_free_no_builtin

#define __va_parmadr(P) (&(P))
#define __va_argcsu(TYPE) 0u


#endif /* __TI_ARM__ */
#ifdef __cplusplus
} /* extern "C" */
#endif

/* foxBMS 2 specific adaptions                                              */
/* SPNU151V-January1998-RevisedFebruary2020                                  */
/* 5.14 ARM Instruction Intrinsics                                           */
/* Table 5-3. ARM Intrinsic Support by Target                                */
/* entries for ARM V7R (Cortex-R4)                                           */

/* Cortex-R4 has: __clz                                                      */
/* Cortex-R4 has: __delay_cycle                                              */
/* Cortex-R4 has: __ldrex                                                    */
/* Cortex-R4 has: __ldrexb                                                   */
/* Cortex-R4 has: __ldrexd                                                   */
/* Cortex-R4 has: __ldrexh                                                   */
/* Cortex-R4 has: __MCR                                                      */
/* Cortex-R4 has: __MRC                                                      */
/* Cortex-R4 has: __nop                                                      */
/* Cortex-R4 has: _norm                                                      */
/* Cortex-R4 has: __rev                                                      */
/* Cortex-R4 has: __rev16                                                    */
/* Cortex-R4 has: __revsh                                                    */
/* Cortex-R4 has: __rbit                                                     */
/* Cortex-R4 has: __ror                                                      */
/* Cortex-R4 has: _pkhbt                                                     */
/* Cortex-R4 has: _pkhtb                                                     */
/* Cortex-R4 has: _qadd16                                                    */
/* Cortex-R4 has: _qadd8                                                     */
/* Cortex-R4 has: _qaddsubx                                                  */
/* Cortex-R4 has: _qsub16                                                    */
/* Cortex-R4 has: qsub8                                                      */
/* Cortex-R4 has: _qsubaddx                                                  */
/* Cortex-R4 has: _sadd                                                      */
/* Cortex-R4 has: _sadd16                                                    */
/* Cortex-R4 has: _sadd8                                                     */
/* Cortex-R4 has: _saddsubx                                                  */
/* Cortex-R4 has: _sdadd                                                     */
/* Cortex-R4 has: _sdsub                                                     */
/* Cortex-R4 has: _sel                                                       */
/* Cortex-R4 has: _shadd16                                                   */
/* Cortex-R4 has: _shadd8                                                    */
/* Cortex-R4 has: _shsub16                                                   */
/* Cortex-R4 has: _shsub8                                                    */
/* Cortex-R4 has: _smac                                                      */
/* Cortex-R4 has: _smlabb                                                    */
/* Cortex-R4 has: _smlabt                                                    */
/* Cortex-R4 has: _smlad                                                     */
/* Cortex-R4 has: _smladx                                                    */
/* Cortex-R4 has: _smlalbb                                                   */
/* Cortex-R4 has: _smlalbt                                                   */
/* Cortex-R4 has: _smlald                                                    */
/* Cortex-R4 has: _smlaldx                                                   */
/* Cortex-R4 has: _smlaltb                                                   */
/* Cortex-R4 has: _smlaltt                                                   */
/* Cortex-R4 has: _smlatb                                                    */
/* Cortex-R4 has: _smlatt                                                    */
/* Cortex-R4 has: _smlawb                                                    */
/* Cortex-R4 has: _smlawt                                                    */
/* Cortex-R4 has: _smlsd                                                     */
/* Cortex-R4 has: _smlsdx                                                    */
/* Cortex-R4 has: _smlsld                                                    */
/* Cortex-R4 has: _smlsldx                                                   */
/* Cortex-R4 has: _smmla                                                     */
/* Cortex-R4 has: _smmlar                                                    */
/* Cortex-R4 has: _smmls                                                     */
/* Cortex-R4 has: _smmlsr                                                    */
/* Cortex-R4 has: _smmul                                                     */
/* Cortex-R4 has: _smmulr                                                    */
/* Cortex-R4 has: _smuad                                                     */
/* Cortex-R4 has: _smuadx                                                    */
/* Cortex-R4 has: _smusd                                                     */
/* Cortex-R4 has: _smusdx                                                    */
/* Cortex-R4 has: _smpy                                                      */
/* Cortex-R4 has: _smsub                                                     */
/* Cortex-R4 has: _smulbb                                                    */
/* Cortex-R4 has: _smulbt                                                    */
/* Cortex-R4 has: _smultb                                                    */
/* Cortex-R4 has: _smultt                                                    */
/* Cortex-R4 has: _smulwb                                                    */
/* Cortex-R4 has: _smulwt                                                    */
/* Cortex-R4 has: __sqrt                                                     */
/* Cortex-R4 has: __sqrtf                                                    */
/* Cortex-R4 has: _ssat16                                                    */
/* Cortex-R4 has: _ssata                                                     */
/* Cortex-R4 has: _ssatl                                                     */
/* Cortex-R4 has: _ssub                                                      */
/* Cortex-R4 has: _ssub16                                                    */
/* Cortex-R4 has: _ssub8                                                     */
/* Cortex-R4 has: _ssubaddx                                                  */
/* Cortex-R4 has: __strex                                                    */
/* Cortex-R4 has: __strexb                                                   */
/* Cortex-R4 has: __strexd                                                   */
/* Cortex-R4 has: __strexh                                                   */
/* Cortex-R4 has: _subc                                                      */
/* Cortex-R4 has: _sxtab                                                     */
/* Cortex-R4 has: _sxtab16                                                   */
/* Cortex-R4 has: _sxtah                                                     */
/* Cortex-R4 has: _sxtb                                                      */
/* Cortex-R4 has: _sxtb16                                                    */
/* Cortex-R4 has: _sxth                                                      */
/* Cortex-R4 has: _uadd16                                                    */
/* Cortex-R4 has: _uadd8                                                     */
/* Cortex-R4 has: _uaddsubx                                                  */
/* Cortex-R4 has: _uhadd16                                                   */
/* Cortex-R4 has: _uhadd8                                                    */
/* Cortex-R4 has: _uhsub16                                                   */
/* Cortex-R4 has: _uhsub8                                                    */
/* Cortex-R4 has: _umaal                                                     */
/* Cortex-R4 has: _uqadd16                                                   */
/* Cortex-R4 has: _uqadd8                                                    */
/* Cortex-R4 has: _uqaddsubx                                                 */
/* Cortex-R4 has: _uqsub16                                                   */
/* Cortex-R4 has: _uqsub8                                                    */
/* Cortex-R4 has: _uqsubaddx                                                 */
/* Cortex-R4 has: _usad8                                                     */
/* Cortex-R4 has: _usat16                                                    */
/* Cortex-R4 has: _usata                                                     */
/* Cortex-R4 has: _usatl                                                     */
/* Cortex-R4 has: _usub16                                                    */
/* Cortex-R4 has: _usub8                                                     */
/* Cortex-R4 has: _usubaddx                                                  */
/* Cortex-R4 has: _uxtab                                                     */
/* Cortex-R4 has: _uxtab16                                                   */
/* Cortex-R4 has: _uxtah                                                     */
/* Cortex-R4 has: _uxtb                                                      */
/* Cortex-R4 has: _uxtb16                                                    */
/* Cortex-R4 has: _uxth                                                      */
/* Cortex-R4 has: __wfe                                                      */
/* Cortex-R4 has: __wfi                                                      */

/* foxBMS 2 specific adaptions                                              */
/* SPNU151V-January1998-RevisedFebruary2020                                  */
/* 5.14 ARM Instruction Intrinsics                                           */
/* Table 5-4. ARM Compiler Intrinsics                                        */

extern int _clz(int src) __attribute__((const));
extern void __delay_cycles(unsigned int cycles) __attribute__((const));
extern unsigned int __get_MSP(void) __attribute__((const));
extern unsigned int __get_PRIMASK(void) __attribute__((const));
extern unsigned int __ldrex(void *src) __attribute__((const));
extern unsigned int __ldrexb(void *src) __attribute__((const));
extern unsigned long long __ldrexd(void *src) __attribute__((const));
extern unsigned int __ldrexh(void *src) __attribute__((const));
extern void __MCR(unsigned int coproc, unsigned int opc1, unsigned int src, unsigned int coproc_reg1, unsigned int coproc_reg2, unsigned int opc2) __attribute__((const));
extern unsigned int __MRC(unsigned int coproc, unsigned int opc1, unsigned int coproc_reg1, unsigned int coproc_reg2, unsigned int opc2) __attribute__((const));
extern void __nop(void) __attribute__((const));
extern int _norm(int src) __attribute__((const));
extern int _pkhbt(int src1, int src2, int shift) __attribute__((const));
extern int _pkhtb(int src1, int src2, int shift) __attribute__((const));
extern int _qadd16(int src1, int src2) __attribute__((const));
extern int _qadd8(int src1, int src2) __attribute__((const));
extern int _qaddsubx(int src1, int src2) __attribute__((const));
extern int _qsub16(int src1, int src2) __attribute__((const));
extern int _qsub8(int src1, int src2) __attribute__((const));
extern int _qsubaddx(int src1, int src2) __attribute__((const));
extern int __rbit(int src) __attribute__((const));
extern int __rev(int src) __attribute__((const));
extern int __rev16(int src) __attribute__((const));
extern int __revsh(int src) __attribute__((const));
extern int __ror(int src, int shift) __attribute__((const));
extern int _sadd(int src1, int src2) __attribute__((const));
extern int _sadd16(int src1, int src2) __attribute__((const));
extern int _sadd8(int src1, int src2) __attribute__((const));
extern int _saddsubx(int src1, int src2) __attribute__((const));
extern int _sdadd(int src1, int src2) __attribute__((const));
extern int _sdsub(int src1, int src2) __attribute__((const));
extern int _sel(int src1, int src2) __attribute__((const));
extern void __set_MSP(unsigned int src) __attribute__((const));
extern unsigned int __set_PRIMASK(unsigned int src) __attribute__((const));
extern int _shadd16(int src1, int src2) __attribute__((const));
extern int _shadd8(int src1, int src2) __attribute__((const));
extern int _shsub16(int src1, int src2) __attribute__((const));
extern int _shsub8(int src1, int src2) __attribute__((const));
extern int _smac(int dst, int src1, int src2) __attribute__((const));
extern int _smlabb(int dst, short src1, short src2) __attribute__((const));
extern int _smlabt(int dst, short src1, int src2) __attribute__((const));
extern int _smlad(int src1, int src2, int acc) __attribute__((const));
extern int _smladx(int src1, int src2, int acc) __attribute__((const));
extern long long _smlalbb(long long dst, short src1, short src2) __attribute__((const));
extern long long _smlalbt(long long dst, short src1, int src2) __attribute__((const));
extern long long _smlald(long long acc, int src1, int src2) __attribute__((const));
extern long long _smlaldx(long long acc, int src1, int src2) __attribute__((const));
extern long long _smlaltb(long long dst, int src1, short src2) __attribute__((const));
extern long long _smlaltt(long long dst, int src1, int src2) __attribute__((const));
extern int _smlatb(int dst, int src1, short src2) __attribute__((const));
extern int _smlatt(int dst, int src1, int src2) __attribute__((const));
extern int _smlawb(int src1, short src2, int acc) __attribute__((const));
extern int _smlawt(int src1, short src2, int acc) __attribute__((const));
extern int _smlsd(int src1, int src2, int acc) __attribute__((const));
extern int _smlsdx(int src1, int src2, int acc) __attribute__((const));
extern long long _smlsld(long long acc, int src1, int src2) __attribute__((const));
extern long long _smlsldx(long long acc, int src1, int src2) __attribute__((const));
extern int _smmla(int src1, int src2, int acc) __attribute__((const));
extern int _smmlar(int src1, int src2, int acc) __attribute__((const));
extern int _smmls(int src1, int src2, int acc) __attribute__((const));
extern int _smmlsr(int src1, int src2, int acc) __attribute__((const));
extern int _smmul(int src1, int src2, int acc) __attribute__((const));
extern int _smmulr(int src1, int src2, int acc) __attribute__((const));
extern int _smpy(int src1, int src2) __attribute__((const));
extern int _smsub(int src1, int src2) __attribute__((const));
extern int _smuad(int src1, int src2) __attribute__((const));
extern int _smuadx(int src1, int src2) __attribute__((const));
extern int _smulbb(int src1, int src2) __attribute__((const));
extern int _smulbt(int src1, int src2) __attribute__((const));
extern int _smultb(int src1, int src2) __attribute__((const));
extern int _smultt(int src1, int src2) __attribute__((const));
extern int _smulwb(int src1, short src2, int acc) __attribute__((const));
extern int _smulwt(int src1, short src2, int acc) __attribute__((const));
extern int _smusd(int src1, int src2) __attribute__((const));
extern int _smusdx(int src1, int src2) __attribute__((const));
extern double __sqrt(double) __attribute__((const));
extern float __sqrtf(float) __attribute__((const));
extern int _ssat16(int src, int bitpos) __attribute__((const));
extern int _ssata(int src, int shift, int bitpos) __attribute__((const));
extern int _ssatl(int src, int shift, int bitpos) __attribute__((const));
extern int _ssub(int src1, int src2) __attribute__((const));
extern int _ssub16(int src1, int src2) __attribute__((const));
extern int _ssub8(int src1, int src2) __attribute__((const));
extern int _ssubaddx(int src1, int src2) __attribute__((const));
extern int __strex(unsigned int src, void *dst) __attribute__((const));
extern int __strexb(unsigned char src, void *dst) __attribute__((const));
extern int __strexd(unsigned long long src, void *dst) __attribute__((const));
extern int __strexh(unsigned short src, void *dst) __attribute__((const));
extern int _subc(int src1, int src2) __attribute__((const));
extern int _sxtab(int src1, int src2, int rotamt) __attribute__((const));
extern int _sxtab16(int src1, int src2, int rotamt) __attribute__((const));
extern int _sxtah(int src1, int src2, int rotamt) __attribute__((const));
extern int _sxtb(int src1, int rotamt) __attribute__((const));
extern int _sxtb16(int src1, int rotamt) __attribute__((const));
extern int _sxth(int src1, int rotamt) __attribute__((const));
extern int _uadd16(int src1, int src2) __attribute__((const));
extern int _uadd8(int src1, int src2) __attribute__((const));
extern int _uaddsubx(int src1, int src2) __attribute__((const));
extern int _uhadd16(int src1, int src2) __attribute__((const));
extern int _uhadd8(int src1, int src2) __attribute__((const));
extern int _uhsub16(int src1, int src2) __attribute__((const));
extern int _uhsub8(int src1, int src2) __attribute__((const));
extern int _umaal(long long acc, int src1, int src2) __attribute__((const));
extern int _uqadd16(int src1, int src2) __attribute__((const));
extern int _uqadd8(int src1, int src2) __attribute__((const));
extern int _uqaddsubx(int src1, int src2) __attribute__((const));
extern int _uqsub16(int src1, int src2) __attribute__((const));
extern int _uqsub8(int src1, int src2) __attribute__((const));
extern int _uqsubaddx(int src1, int src2) __attribute__((const));
extern int _usad8(int src1, int src2) __attribute__((const));
extern int _usat16(int src, int bitpos) __attribute__((const));
extern int _sxtab(int src1, int src2, int rotamt) __attribute__((const));
extern int _sxtab16(int src1, int src2, int rotamt) __attribute__((const));
extern int _sxtah(int src1, int src2, int rotamt) __attribute__((const));
extern int _sxtb(int src1, int rotamt) __attribute__((const));
extern int _sxtb16(int src1, int rotamt) __attribute__((const));
extern int _sxth(int src1, int rotamt) __attribute__((const));
extern int _uadd16(int src1, int src2) __attribute__((const));
extern int _uadd8(int src1, int src2) __attribute__((const));
extern int _uaddsubx(int src1, int src2) __attribute__((const));
extern int _uhadd16(int src1, int src2) __attribute__((const));
extern int _uhadd8(int src1, int src2) __attribute__((const));
extern int _uhsub16(int src1, int src2) __attribute__((const));
extern int _uhsub8(int src1, int src2) __attribute__((const));
extern int _umaal(long long acc, int src1, int src2) __attribute__((const));
extern int _uqadd16(int src1, int src2) __attribute__((const));
extern int _uqadd8(int src1, int src2) __attribute__((const));
extern int _uqaddsubx(int src1, int src2) __attribute__((const));
extern int _uqsub16(int src1, int src2) __attribute__((const));
extern int _uqsub8(int src1, int src2) __attribute__((const));
extern int _uqsubaddx(int src1, int src2) __attribute__((const));
extern int _usad8(int src1, int src2) __attribute__((const));
extern int _usat16(int src, int bitpos) __attribute__((const));

/* foxBMS 2 specific adaptions                                              */
/* SPNU151V-January1998-RevisedFebruary2020                                  */
/* 6.9 Built-In Functions                                                    */

extern void *__curpc(void);
extern int __run_address_check(void);

/* foxBMS 2 specific adaptions                                              */
/* source: https://e2e.ti.com/support/tools/code-composer-studio-group/ccs/f/code-composer-studio-forum/666101/compiler-tms570ls3137-need-definition-for-the-_hi-f-_lo-f-used-in-the-xxdftype-h-file*/
extern unsigned long _hi(long double);
extern unsigned long _lo(long double);

/* foxBMS 2 specific adaptions                                              */
extern unsigned _ftoi (float src);
