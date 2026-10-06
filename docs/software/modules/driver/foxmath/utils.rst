.. include:: ./../../../../macros.txt
.. include:: ./../../../../units.txt

.. _UTILS_DRIVER:

Utils
=====

Module Files
------------

Driver
^^^^^^

- ``src/app/driver/foxmath/utils.c``
- ``src/app/driver/foxmath/utils.h``

Configuration
^^^^^^^^^^^^^

*none*

Unit Test
^^^^^^^^^

- ``tests/unit/app/driver/foxmath/test_utils.c``
- ``tests/unit/bootloader/driver/foxmath/test_utils.c``

Description
-----------

This module contains generic helper functionality used by several driver and
application modules.

Implemented functionality:

- String-to-integer packing via ``UTIL_ExtractCharactersFromString()``
- Pseudo-random number generation via
  ``UTIL_SeedRandomNumber()`` and ``UTIL_GetPseudoRandomNumber()``

String Character Extraction
^^^^^^^^^^^^^^^^^^^^^^^^^^^

``UTIL_ExtractCharactersFromString()`` packs a selected substring into a
``uint64_t``.
Each extracted character is appended by shifting the current result by one byte
and adding the next ASCII value.

The routine:

- checks all input arguments with ``FAS_ASSERT``
- limits extraction to ``stringLength``
- supports extraction of up to 8 characters (size of ``uint64_t``)

Pseudo-Random Number Utility
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The pseudo-random number utility is based on an internal state variable.
``UTIL_SeedRandomNumber()`` initializes this state.
``UTIL_GetPseudoRandomNumber()`` then updates it using a linear congruential
step and the current system tick (``OS_GetTickCount()``) as increment.

The returned value is derived from the updated state by shifting and masking
to 15 bits.

.. note::

   This routine is intended for lightweight pseudo-random behavior and not for
   cryptographic use cases.
