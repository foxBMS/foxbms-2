.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _LAUTERBACH_TRACE32_DEBUGGER:

Lauterbach Trace32 debugger
===========================

Some information can be found in ``tools/debugger/lauterbach/commands.md``.
Please carefully read the
`basic manual <https://www2.lauterbach.com/pdf/training_debugger.pdf>`_
for Lauterbach.

After setting up the hardware connection you should be able to load
the Trace32 application. For details on where to find a ready configuration
fo |foxbms|, please refer to :numref:`WAF_TOOL_LAUTERBACH`.

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
