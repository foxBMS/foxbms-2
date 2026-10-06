.. include:: ../../../../macros.txt
.. include:: ../../../../units.txt

.. _IO_DRIVER:

IO
==

Module Files
------------

Driver
^^^^^^

- ``src/app/driver/io/io.c``
- ``src/app/driver/io/io.h``

Configuration
^^^^^^^^^^^^^

*none*

Unit Test
^^^^^^^^^

- ``tests/unit/app/driver/io/test_io.c``

Detailed Description
--------------------

The IO module provides small helper functions for bit-based access to
memory-mapped peripheral registers used as GPIO-like control and status
registers.

It abstracts common operations used throughout the driver layer:

- configure a pin direction as output,
- configure a pin direction as input,
- set an output pin,
- reset an output pin,
- read and decode a pin state.

API Behavior
^^^^^^^^^^^^

All functions validate their inputs using assertions:

- register pointer must not be ``NULL_PTR``,
- pin index must be within the supported MCU bit range.

Implementation Details
^^^^^^^^^^^^^^^^^^^^^^

The implementation in ``io.c`` performs direct bit manipulation on the passed
register address:

- output direction/set operations use bitwise OR,
- input direction/reset operations use bitwise AND with inverted mask,
- pin read operation extracts the addressed bit and returns ``STD_PIN_LOW`` or
  ``STD_PIN_HIGH``.

The module does not own any state and does not perform hardware
initialization by itself; it is used by other modules that pass the
appropriate register addresses and pin numbers.
