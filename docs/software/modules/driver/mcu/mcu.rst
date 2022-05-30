.. include:: ./../../../../macros.txt
.. include:: ./../../../../units.txt

.. _MCU:

MCU
===

Module Files
------------

Driver
^^^^^^

- ``src/app/driver/mcu/mcu.c`` (`API <../../../../_static/doxygen/src/html/mcu_8c.html>`__, `source <../../../../_static/doxygen/src/html/mcu_8c_source.html>`__)
- ``src/app/driver/mcu/mcu.h`` (`API <../../../../_static/doxygen/src/html/mcu_8h.html>`__, `source <../../../../_static/doxygen/src/html/mcu_8h_source.html>`__)

Configuration
^^^^^^^^^^^^^

*none*

Unit Test
^^^^^^^^^

- ``tests/unit/app/driver/mcu/test_mcu.c`` (`API <../../../../_static/doxygen/tests/html/test__mcu_8c.html>`__, `source <../../../../_static/doxygen/tests/html/test__mcu_8c_source.html>`__)

Description
-----------

The ``MCU`` module supplies an API that helps using certain functions of the MCU.
As an example, it can be used to access the current value of the free running
counter of the real time interrupt module.

This can be used as a quick and reliable method for measuring code execution
time.
The following example :ref:`mcu-measure-time-rti` can help finding states in a
line of code that takes longer than expected for execution.

.. code-block:: c
   :linenos:
   :caption: Measuring execution time with the ``RTI``
   :name: mcu-measure-time-rti

    const uint32_t entry   = MCU_GetFreeRunningCount();
    /* code under test here */
    const uint32_t exit    = MCU_GetFreeRunningCount();
    const uint32_t time_us = MCU_ConvertFrcDifferenceToTimespan_us(exit - entry);
    if (time_us > 250u) {
        /* do something that the compiler won't optimize away */
        volatile uint8_t test = 0u;
        /* set breakpoint for example here */
        test++;
    }

The ``MCU`` module also supplies the function ``MCU_Delay_us()``.
It waits blocking for the time in microseconds given as parameter.
As this function uses the free running counter, it can be interrupted by the
operating system.
After its execution resumes, it will exit if the time given as parameter has
elapsed.
A timeout has also been implemented to avoid blocking the system with the
function.
If the time given as parameter is higher than the time resulting from the
timeout, the function will exit after the timeout and will not wait for the
time given as parameter.
This means that the timeout value should be checked.
