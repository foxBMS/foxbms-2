.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _SEGGER_OZONE_DEBUGGER:

Segger Ozone debugger
=====================

Segger
`Ozone <https://www.segger.com/products/development-tools/ozone-j-link-debugger/>`_
is the debugging software that is supplied by Segger with the
higher tier debug probes such as the Segger J-Link PLUS.
Please refer to the
`Ozone-manual <https://www.segger.com/downloads/jlink/UM08025_Ozone.pdf>`_
and read it carefully before using the software.

After setting up the hardware connection you should be able to load the
Ozone configuration. For details on where to find a ready configuration for
|foxbms|, please refer to :numref:`WAF_TOOL_OZONE`.

Common pitfalls
^^^^^^^^^^^^^^^
When creating too many cyclically updated entries in the watch window, the
debugger seems to stop and read the target so often that it impacts the system
performance. Being aware of this issue and keeping an eye on the number of
automatically updated variable watches should mitigate this issue.

Tracing with Segger J-Trace PRO Cortex
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Apart from debug probes, Segger also supplies trace probes. With these devices
it is possible to stream internal information from the MCU such as the program
counter directly to the debugger. For the TMS570LC4357 target it is necessary
to use the Segger J-Trace PRO Cortex model. According to its documentation it
is able to trace with a data width of 4 bit.

Information on how to set up the trace probe can be found in the
`Segger Wiki <https://wiki.segger.com/TI_TMS570Lx#Tracing_on_TI_TMS570LC4357>`_.
Please note, that as of now, only tracing with
:ref:`Lauterbach devices<LAUTERBACH_TRACE32_DEBUGGER>` has been tested by
`Fraunhofer IISB`_.
