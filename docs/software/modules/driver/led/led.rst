.. include:: ../../../../macros.txt
.. include:: ../../../../units.txt

.. _LED_DRIVER:

LED
===

Module Files
------------

Driver
^^^^^^

- ``src/app/driver/led/led.c``
- ``src/app/driver/led/led.h``

Configuration
^^^^^^^^^^^^^

*none*

Unit Test
^^^^^^^^^

- ``tests/unit/app/driver/led/test_led.c``

Detailed Description
--------------------

This module controls the debug LED connected directly to a MCU HET pin.
It provides a simple interface to set the LED during startup and to generate
cyclic blinking patterns during runtime.

Main Behavior
^^^^^^^^^^^^^

- ``LED_SetDebugLed()`` sets the debug LED output to ON.
  This is intended for startup indication.
- ``LED_SetToggleTime(onOffTime_ms)`` configures the ON and OFF duration
  (half-cycle) in milliseconds.
- ``LED_Trigger()`` toggles the LED when the configured duration has elapsed.
  The function is designed for periodic invocation.

Timing Model
^^^^^^^^^^^^

The implementation assumes that ``LED_Trigger()`` is called with the
``FTSK_TASK_CYCLIC_100MS_CYCLE_TIME`` period (100 ms).
For this reason, ``LED_SetToggleTime()`` asserts that the requested
``onOffTime_ms`` is a non-zero multiple of 100 ms.

Two symbolic timing values are provided by the API:

- ``LED_NORMAL_OPERATION_ON_OFF_TIME_ms`` (500 ms),
- ``LED_ERROR_OPERATION_ON_OFF_TIME_ms`` (100 ms).
