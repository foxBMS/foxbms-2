.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _PROGRAMMING_LANGUAGE:

Programming Language
====================

.. _system_programming_language_and_standards:

System Programming Language and Standards
-----------------------------------------

Special care has to be taken for coding C-code as the core functions of the
BMS software rely on this language. This project uses the ISO C11 standard
(a working document is for example available in :cite:`ISO-9899:2011`) as a
language basis.

The C programming language is very flexible. This flexibility has to be
reduced to a safe language sub-set which is done by additional coding
standards. Code contributed to this project **MUST** be compatible with
MISRA C:2012 which is described in :cite:`MISRA-C:2012`.

.. admonition:: Abstract of System Programming Language and Standards

   - The programming language of the embedded system software **MUST** be ISO
     C.
   - The programming language standard version of the embedded system software
     **MUST** be C11.
   - The source code **SHOULD** not make use of language extensions.
   - The source code **MUST** be compiled in the strictest standards way the
     compiler supports.
   - The source code **MUST** compile without any error or warning messages.
