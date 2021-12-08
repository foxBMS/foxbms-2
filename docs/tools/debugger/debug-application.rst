.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _DEBUGGING_THE_APPLICATION:

#########################
Debugging the Application
#########################

During development it is necessary to be able to deeply inspect internals
of the BMS application.
This is only possible through a debugger.
Apart from that a debugger is a required tool for downloading code into the BMS.

A debugger attaches to the microcontroller on the BMS which will be called
`target` in the following text. The interface through which the connection
to the target is established is called `JTAG`. The debugger can halt execution
of the program on the target, download register values and restart execution.
Moreover it is possible to mark locations in the code in such a way that the
target will automatically halt once it reaches this location. This is called
a breakpoint.

For more in-depth analysis of program flow it can be necessary to attach a
trace probe to the target. This trace probe is a specialized hardware that
is capable of parsing compressed information that the target can output
through its trace port. This is not necessary for normal debug sessions.

How to debug
------------

This manual is not able to describe in depth how to debug an application on
an embedded target as this requires education and experience that have to be
obtained through other means.
Nevertheless, this section tries to walk through the main points of debugging
this specific application. As this relies on basic debugger features it should
work with any debugger.

Software structure
^^^^^^^^^^^^^^^^^^

The main structure of |foxbms| are several interacting finite-state machines.
For more information on the system structure please refer to
:numref:`SOFTWARE_MODULES` of this documentation.

In order to debug behavior of these state machines it is often helpful to keep
track of the struct that contains the state of the state machine. The state
of a state machine is tracked in a variable that is most of the time called
``*_state``. For details on the anatomy of this style of code please refer to
:numref:`implementation_basics`.

The second central point to look into are the database tables that contain
all values that are passed between state machines. As an example the table
``data_blockCellVoltage`` contains the consolidated cell voltages that are
used by the BMS for decisions. For more information on the database refer
to the database documentation in :numref:`DATABASE_MODULE`.

For both of these variable types it can be helpful to monitor them
continuously.
Most debugger know a concept that is often called `watch window`. This watch
window can be configured to monitor and continuously show updates of
certain variables. This way it can be observed which values these variables
take. Please note that even though the updates of a watch window might seem
to be real-time, they are obtained by continuously halting the target and
sampling the registers of the target. That means that firstly the shown
values do not necessarily have to be all values (there might be values
in-between) and secondly showing too many values might impact the performance
of the target depending on the debugger implementation and thus alter real-time
behavior during the debug session.

Assertions
^^^^^^^^^^

Assertions are a method of ensuring that conditions that are thought to be
true are actually true. |foxbms| uses assertions and the standard behavior
is to completely stop the system in case of an assertion. That means that
all interrupts are disabled and the system will come to a halt.

This state can be detected by checking if the system stops and if it does
controlling the value of the struct ``fas_assertLocation``. It stores the
program counter and the line number of the location of an assertion that has
failed. The value of the program counter can be used to find the exact
location in code where the assertion has failed.

It is recommended to add the ``fas_assertLocation`` variable to the watch
window or to set a write breakpoint that trips on writing to this variable in
order to easily see when an assertion has failed.

Version struct
^^^^^^^^^^^^^^

Each binary is automatically marked with the software version from which it
has been built. The current version is stored in the global variable
``foxbmsVersionInfo`` and can be inspected with the debugger.

Choice of debugger
------------------

The choice of the correct debugger depends on many factors and can not be
decided based solely on the information of this manual.
The authors of this manual use and support two types of debuggers. While other
options exist and may yield similar results, we recommend to choose one of
these toolchains if you have to rely on our support for the setup of the
debugger, as we cannot support debugger toolchains that we do not know or
cannot recommend. Feel free to contact us, if you need more information.

* The first one is the
  `Segger J-Link PLUS <https://www.segger.com/j-link-plus.html>`_. A cheaper
  debugger solution is the
  `Segger J-Link BASE <https://www.segger.com/jlink_base.html>`_. Please note
  that the debug software
  `Ozone <https://www.segger.com/products/development-tools/ozone-j-link-debugger/#ozone-licensing>`_
  is not licensed with the base version of J-Link.

* The second type of debugger are the debugger by
  `Lauterbach <http://www.lauterbach.com>`_. The Lauterbach
  debugger performs under certain conditions better, but is also more expensive
  than the J-Link. Tracing has been tested and set up only with trace-probes
  by Lauterbach.

The following sections describe configuration and usage of the debuggers
Segger Ozone and Lauterbach Trace32.

.. toctree::
    :maxdepth: 1

    ./debugger-ozone.rst
    ./debugger-lauterbach.rst
