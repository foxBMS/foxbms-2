# Cortex-R5 FreeRTOS port notes

## Scope

This directory contains the TI TMS570 Cortex-R5 compiler port layer used by the
FreeRTOS kernel integration in this repository:

- [port.c](port.c)
- [portasm.asm](portasm.asm)
- [portmacro.h](portmacro.h)

The local files are based on the FreeRTOS Cortex-R port and contain project-
specific adaptations, including TI HALCoGen and MPU related changes.

## Verified behavior

### Deferred yield while in a critical section

The implementation defers a context switch when a software interrupt yield
arrives while critical nesting is non-zero.

Why:

- The critical section depth counter is a global variable:
  [port.c](port.c) declares ulCriticalNesting.
- The pending-yield flag is also global:
  [port.c](port.c) declares ulPortYieldPending.

How:

1. SWI yield enters swiPortYield.
2. If ulCriticalNesting is not zero, swiPortYield sets ulPortYieldPending and
   returns without switching.
3. The outermost swiPortExitCritical re-enables IRQs, checks
   ulPortYieldPending, clears it, and branches back to swiPortYield to perform
   the switch.

Reference: [portasm.asm](portasm.asm).

### Yield-from-ISR path remains interrupt based

portYIELD_FROM_ISR() still uses SSIR1 (software set interrupt register):
[portmacro.h](portmacro.h).

This is intentionally different from the API-internal yield path, which should
switch with SWI immediately (not deferred by a later taskENTER_CRITICAL()).

## Previous behavior and reason for the change

### Previous behavior

Previously, API-internal yield requests were triggered through SSIR1
(interrupt-driven path), equivalent to deferring the switch until IRQ service.
In that setup, requesting a yield and executing the context switch were not
guaranteed to happen at the same instant.

Reference: rationale comment in [portmacro.h](portmacro.h).

### Why this needed to change

In critical sections, IRQs are masked by design. If the API-internal yield path
depends on SSIR1, a yield can remain pending while code assumes the switch has
already happened.

This timing mismatch can break API expectations for immediate handover and was
observed as cache/timing related failures in this branch context.

The fix is to keep API-internal yield on SWI (`portYIELD()` path) and defer the
actual switch explicitly with ulPortYieldPending only while
ulCriticalNesting > 0, then perform the switch at the outermost
portEXIT_CRITICAL().

References:

- [portmacro.h](portmacro.h)
- [portasm.asm](portasm.asm)

## Short examples

### Example 1: Yield requested inside critical section

If ulCriticalNesting is 1 when SWI #0 is raised:

- swiPortYield does not switch context immediately.
- ulPortYieldPending is set to 1.
- The switch happens when the matching portEXIT_CRITICAL() reaches nesting 0.

Reference: [portasm.asm](portasm.asm).

### Example 2: Yield requested from ISR

When xHigherPriorityTaskWoken is true in portYIELD_FROM_ISR(x):

- SSIR1 is written,
- barriers are executed (DSB/ISB),
- and the scheduler runs from the IRQ-driven path.

Reference: [portmacro.h](portmacro.h).
