.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _LAUTERBACH_TRACE32_DEBUGGER:

Lauterbach Trace32 Debugger
===========================

..
    Comments:
    Mictor is a connector

.. spelling::
    Mictor

Required Hardware
^^^^^^^^^^^^^^^^^

For debugging the |foxbms| application with the Lauterbach tools the
following hardware is required:
- Lauterbach LA-3505 debugger
- Lauterbach LA-3253 Adapter for Cortex-A/R cores (successor of the LA-7843)
- Lauterbach Mictor-38 adapter LA-3722

Debugger Software
^^^^^^^^^^^^^^^^^

The debugger software is available at
`https://www.lauterbach.com <https://www.lauterbach.com/frames.html?download_trace32.html>`_.

Lauterbach Manual
^^^^^^^^^^^^^^^^^

Refer to the
`basic manual <https://www.lauterbach.com/pdf/training_debugger.pdf>`_
for working with the Lauterbach debugger and tools.

Setup
^^^^^

After setting up the hardware connection you should be able to load
the Trace32 application.
For details on where to find a ready configuration for |foxbms|, please refer
to :numref:`WAF_TOOL_LAUTERBACH`.

.. figure:: lauterbach-buttons.png
   :alt: Buttons in the Lauterbach Trace32 environment
   :name: lauterbach-buttons
   :width: 175

   Most relevant buttons in Trace32 from left to right: Select ELF file,
   Download ELF to target, Load symbols, Unload symbols, Up and Go the target,
   Up the target, Down the target

:numref:`lauterbach-buttons` shows the most relevant buttons in Trace32.
For a normal download onto the target with a subsequent debug session you have
to select the ELF file, then set the target to "Up"-state, download the ELF
file onto the target and then press the "Up and Go"-button in order to reset
the target and start with an attached debugger.

Lauterbach Command Line Commands
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The subsections below show some important commands.

Flash Manipulation
------------------

#. Unlock flash: ``FLASH.ReProgram ALL``
#. Manipulate flash as needed
#. Lock flash: ``FLASH.ReProgram OFF``


Deleting Flash
--------------

#. Unlock flash: ``FLASH.ReProgram ALL``
#. Erase

   #. Complete flash: ``FLASH.Erase ALL``
   #. Only certain memory range: ``FLASH.Erase (xxxxxx)--(yyyyyy)``

#. Lock flash: ``FLASH.ReProgram OFF``

Saving Flash to File
--------------------

#. Save flash to file: ``DATA.SAVE.BINARY filename.bin 0x08000000--0x081FFFFF``

The file is saved in Lauterbach installation directory if a relative path is
used.

Writing to Flash from File
--------------------------

#. Erase flash: ``FLASH.Erase ALL``
#. Unlock flash: ``FLASH.Program ALL``
#. Load and flash file:
   ``DATA.LOAD.BINARY filename.bin 0x08000000--0x081FFFFF``
#. Lock flash: ``FLASH.Program off``

Using a trace probe
^^^^^^^^^^^^^^^^^^^
If debugging alone is not sufficient and additional trace capabilities are
required, Lauterbach supplies in addition to the debug probes also a trace
probe for the TMS570LC4357 target.
Please refer to their customer support for details.

The trace  setup of `Fraunhofer IISB`_ consists of a LA-3505 debugger connected
to a LA-3580 trace probe with the LA-7992 parallel preprocessor for ARM/Cortex
architectures.
|foxbms| features a compliant Mictor-38 connector in order to be able to
connect trace ports with up to 8 bit. If you need more bandwidth, please
contact us.
The standard design can be adapted to up to 32 bit trace bandwidth.
