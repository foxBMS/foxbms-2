.. include:: ./../../../../macros.txt
.. include:: ./../../../../units.txt

.. _CRC:

CRC
===

Module Files
------------

Driver
^^^^^^

- ``src/app/driver/crc/crc.c`` (`API <../../../../_static/doxygen/src/html/crc_8c.html>`__, `source <../../../../_static/doxygen/src/html/crc_8c_source.html>`__)
- ``src/app/driver/crc/crc.h`` (`API <../../../../_static/doxygen/src/html/crc_8h.html>`__, `source <../../../../_static/doxygen/src/html/crc_8h_source.html>`__)

Unit Test
^^^^^^^^^

- ``tests/unit/app/driver/crc/test_crc.c`` (`API <../../../../_static/doxygen/tests/html/test__crc_8c.html>`__, `source <../../../../_static/doxygen/tests/html/test__crc_8c_source.html>`__)

Description
-----------

The CRC module uses the hardware of the MCU to compute the CRC of a data
flow.

The CRC hardware is based on a 64 bit polynomial. The data is written
in 64 bit chunks to the registers ``PSA_SIGREGL1`` and ``PSA_SIGREGH1``.
The data flow does not need to be aligned on 64 bit: if is is not the case,
the last chunks of data will be padded with zeros to reach a size of 64 bit.

The seed for the CRC computation is defined in ``crc.h`` with
``CRC_SEED_LOW`` and ``CRC_SEED_HIGH``, low referring to the LSB and high
to the MSB.

The function ``CRC_CalculateCrc()`` takes three arguments:

 - a ``uint64_t *`` pointer where the calculated 64 bit CRC is written
 - a ``uint8_t *`` pointer to the data
 - the length in bytes of the data

.. warning::
    This function is not re-entrant: as it uses hardware registers, it
    must only be called once before its execution has finished, otherwise
    the CRC computation will be wrong.
    Currently it is only used with the FRAM, which only makes one access
    at the same time, ensuring that the function is only called once.
    If the function is to be used elsewhere, this point must be taken into
    account.

An internal re-entrance counter is used. If the function was only called
once, it returns ``STD_OK``, otherwise it returns ``STD_NOT_OK`` and the
CRC value is set to zero.
