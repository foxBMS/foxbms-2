# Lauterbach Debugger Scripts

This directory contains a template configuration for the debugging
tool Trace32 by Lauterbach. It is automatically converted into a working project
configuration by the waf tool `f_lauterbach.py` if the binary of the debugging
software has been found on the system.

For more information on using this, please search the foxBMS documentation
for `Lauterbach`.

## FAQ

- **Q:** I encounter "Debug port fail" when starting Trace32

  **A:** Decrease JTAG frequency
