.. include:: ./../../../../macros.txt
.. include:: ./../../../../units.txt

.. _RTC:

RTC
===

Module Files
------------

Driver
^^^^^^

- ``src/app/driver/rtc/rtc.c`` (`API <../../../../_static/doxygen/src/html/rtc_8c.html>`__, `source <../../../../_static/doxygen/src/html/rtc_8c_source.html>`__)
- ``src/app/driver/rtc/rtc.h`` (`API <../../../../_static/doxygen/src/html/rtc_8h.html>`__, `source <../../../../_static/doxygen/src/html/rtc_8h_source.html>`__)

Configuration
^^^^^^^^^^^^^

*none*

Unit Test
^^^^^^^^^

- ``tests/unit/app/driver/rtc/test_rtc.c`` (`API <../../../../_static/doxygen/tests/html/test__rtc_8c.html>`__, `source <../../../../_static/doxygen/tests/html/test__rtc_8c_source.html>`__)

Description
-----------

The Real Time Clock (RTC) module uses an NXP PCF2131 IC as RTC.
The communication with the IC is made via |I2C|.
A struct ``RTC_TIME_DATA_s`` is defined in ``rtc.h``.
It holds the RTC data in the format of the IC.

To avoid reading the RTC IC frequently, an RTC system timer is defined
in ``rtc.c`` with the name ``rtc_SystemTime``.
It has the format ``RTC_SYSTEM_TIMER_EPOCH_s``. This struct has two elements:

 - the time in Unix format, which means seconds since January 1st 1970
 - an additional field with milliseconds

The function ``RTC_IncrementSystemTime()`` is used to increment the RTC
system timer.
It must be called every millisecond.
It does the following:

- increment the milliseconds field if its value is under 999
- otherwise set the millisecond field to 0 and increment the
  time in Unix format by one second

A getter and a setter function have been defined:
``RTC_GetSystemTimeRtcFormat()`` and ``RTC_SetSystemTimeRtcFormat()``.
They get or return an element of the type ``RTC_TIME_DATA_s``.

The RTC system timer is set at startup via the function
``RTC_InitializeSystemTimeWithRtc()``.
It reads the RTC time from the RTC IC over |I2C| and uses the value to set
the RTC system timer.

To set the RTC Time over CAN, the queue ``ftsk_rtcSetTimeQueue`` is used.
If a message to set the RTC is received, the data is written in the queue.
The function ``RTC_SetOverCanMessage()`` reads the queue if it is not empty,
and uses the data to set the RTC time of the IC over |I2C| and then to set
the RTC system timer value.

The ``RTC_Trigger()`` function is called periodically. It calls
``RTC_SetOverCanMessage()``.
