.. include:: ../../macros.txt
.. include:: ../../units.txt

.. _LINKER_SCRIPT:

Linker Script
=============

Memory Map
----------

The relevant parts of the memory map of the TMS 570LC4375 for configuring the
linker script are::

    ┌────────────────────────────────────────┬────────────┐
    │              ...                       │            │
    │                                        │            │
    ├────────────────────────────────────────┼────────────┤
    │              Flash                     │ 0xF047FFFF │
    │  (Flash ECC, OTP and EEPROM accesses)  │ 0xF0000000 │
    ├────────────────────────────────────────┼────────────┤
    │              ...                       │            │
    │                                        │            │
    ├────────────────────────────────────────┼────────────┤
    │              RAM (512KB)               │ 0x0807FFFF │
    │                                        │ 0x08000000 │
    ├────────────────────────────────────────┼────────────┤
    │              ...                       │            │
    │                                        │            │
    ├────────────────────────────────────────┼────────────┤
    │              Flash (4MB)               │ 0x003FFFFF │
    │                                        │ 0x00000000 │
    └────────────────────────────────────────┴────────────┘

The Flash bank is divided into two equally sized banks with a length of
``0x200000`` (i.e., ``FLASH_BANK_LENGTH``), i.e., the total flash size is
``2 * 0x200000``.

Detail information for the Flash ECC::

    ┌────────────────────────────────────────┬────────────┐
    │          Flash Data Space ECC          │ 0xF05FFFFF │
    │                                        │ 0xF047FFFF │
    └────────────────────────────────────────┴────────────┘

Linker Script
-------------

The main linker script is found at ``src\app\main\linker_script_elf.cmd``.
A derived hex-linker script is found at ``src\app\main\linker_script_hex.cmd``.

The ``MEMORY`` and ``SECTIONS`` directives can be used to configure the
allocation process.
The ``ECC`` directive can be used to configure the Error-correcting code of the
memory.

The ``MEMORY`` directive tells the linker the location and the size of memory
blocks.
The task of the ``SECTIONS`` directive is to tell the linker how to map input
sections into output sections, and how to place the output sections in memory,
i.e., specific sections can be placed in particular memory regions.

Memory Layout Configuration
+++++++++++++++++++++++++++

Flash::

    ┌────────────────────────────────────────┬────────────┐
    │              VECTORS_TABLE             │ 0x00000000 │
    │              (0x20)                    │ 0x0000001F │
    ├────────────────────────────────────────┼────────────┤
    │              KERNEL_FUNCTIONS          │ 0x00000020 │
    │              (0x8000)                  │ 0x0000801F │
    ├────────────────────────────────────────┼────────────┤
    │              FLASH                     │ 0x00008020 │
    │              (0x3F7F20)                │ 0x003FFF3F │
    ├────────────────────────────────────────┼────────────┤
    │              VER_VERSION_INFORMATION   │ 0x003FFF40 │
    │              (0xC0)                    │ 0x003FFFFF │
    └────────────────────────────────────────┴────────────┘

RAM::

    ┌────────────────────────────────────────┬────────────┐
    │              STACKS                    │ 0x08000000 │
    │              (0x800)                   │ 0x080007FF │
    ├────────────────────────────────────────┼────────────┤
    │              KERNEL_DATA               │ 0x08000800 │
    │              (0x800)                   │ 0x08000FFF │
    ├────────────────────────────────────────┼────────────┤
    │              RAM                       │ 0x08001000 │
    │              (0x7E000)                 │ 0x0807EFFF │
    ├────────────────────────────────────────┼────────────┤
    │              SHARED_RAM                │ 0x0807F000 │
    │              (0x1000)                  │ 0x0807FFFF │
    └────────────────────────────────────────┴────────────┘

Flash ECC::

    ┌────────────────────────────────────────┬────────────┐
    │              ECC_VECTORS_TABLE         │ 0xF0400000 │
    │              (0x4)                     │ 0xF0400003 │
    ├────────────────────────────────────────┼────────────┤
    │              ECC_KERNEL_FUNCTIONS      │ 0xF0400004 │
    │              (0x1000)                  │ 0xF0401003 │
    ├────────────────────────────────────────┼────────────┤
    │              ECC_FLASH                 │ 0xF0401004 │
    │              (0x7EFE4)                 │ 0xF047FFE7 │
    ├────────────────────────────────────────┼────────────┤
    │              ECC_VERSION_INFORMATION   │ 0xF047FFE8 │
    │              (0x18)                    │ 0xF047FFFF │
    └────────────────────────────────────────┴────────────┘

Common Linker Script related Abbreviations
------------------------------------------

.. csv-table:: Common Linker Script related Abbreviations
   :name: common-linker-script-related-abbreviations
   :header-rows: 1
   :delim: ;
   :file: ./linker-script-definitions.csv
