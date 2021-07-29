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

Lookup program location of assertion
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
In the case that the program has asserted, the location of the assertion will
be written to ``fas_assertLocation.pc`` as mentioned in
:ref:`DEBUGGING_THE_APPLICATION`.

With Ozone, the location of this assertion in the program code can be looked up
as follows:

* The variable ``fas_assertLocation`` has to be viewable in ``Global Data`` or
  in ``Watched Data``.
* It has to be updated (ideally by pausing program execution).
* Unfold ``fas_assertLocation`` so that the members are shown. Do not unfold
  ``pc``.
* Right-click on the ``Value`` of ``pc`` and select "Show Value in Source".
* Ozone will show the code location from where the failing assertion originated.

Break on an assertion
^^^^^^^^^^^^^^^^^^^^^
It can be helpful to configure Ozone to break when a new assertion location
is written to ``fas_assertLocation.pc``. This can be achieved by setting a data
breakpoint on this variable.

The configuration that is supplied with this project automatically adds
``fas_assertLocation`` to the watch window and sets a data breakpoint on
``fas_assertLocation.line``. (It uses ``.line`` and not ``.pc``, because this
member is written secondly and therefore ``.pc`` is already written when the
debugger halts.) In order to prevent that this breakpoint is
triggered during the decompression of the RAM, the debugger is configured to
automatically clear the breakpoint before a reset and to set it when the
probe has detected a completed initialization phase.

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

J-Flash
=======
Segger supplies with their debug probes also a software called `J-Flash`.
This software allows to download software into a target without a debug
session.

The |foxbms| toolchain has a wrapper for J-Flash that allows to use the utility
directly from waf. This allows the user to call waf with the ``install_bin``
command in order to build and directly download in the connected target. This
feature can be used for integrated tests that have to download the compiled
software into a target. For the user's convenience, the |code| workspace
contains a ``Flash:Binary`` build target that allows to call this action
directly from the IDE.
