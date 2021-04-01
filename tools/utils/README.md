# Copy of HALCoGen generated HAL files

The script ``copy_halcogen_files.py`` copies all necessary HAL files generated
by the HALCoGen project from ``conf/hcg`` to to the ``src/hal`` directory.

- It copies all ``*.c``, ``*.h`` and ``*.asm`` files.
- It does **not** copy the linker scripts ``*.cmd``.
- It does **not** copy the FreeRTOS sources (``*.c`` and ``*.h`` files. Instead
it parses ``src/os/freertos/include/FreeRTOSConfig.h`` and sets the correct
value for the define ``configCPU_CLOCK_HZ``.
