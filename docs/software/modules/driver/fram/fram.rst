.. include:: ./../../../../macros.txt
.. include:: ./../../../../units.txt

.. _FRAM:

FRAM
====

Module Files
------------

Driver
^^^^^^

- ``src/app/driver/fram/fram.c`` (`API <../../../../_static/doxygen/src/html/fram_8c.html>`__, `source <../../../../_static/doxygen/src/html/fram_8c_source.html>`__)
- ``src/app/driver/fram/fram.h`` (`API <../../../../_static/doxygen/src/html/fram_8h.html>`__, `source <../../../../_static/doxygen/src/html/fram_8h_source.html>`__)

Configuration
^^^^^^^^^^^^^

- ``src/app/driver/config/fram_cfg.c`` (`API <../../../../_static/doxygen/src/html/fram__cfg_8c.html>`__, `source <../../../../_static/doxygen/src/html/fram__cfg_8c_source.html>`__)
- ``src/app/driver/config/fram_cfg.h`` (`API <../../../../_static/doxygen/src/html/fram__cfg_8h.html>`__, `source <../../../../_static/doxygen/src/html/fram__cfg_8h_source.html>`__)

Unit Test
^^^^^^^^^

- ``tests/unit/app/driver/fram/test_fram.c`` (`API <../../../../_static/doxygen/tests/html/test__fram_8c.html>`__, `source <../../../../_static/doxygen/tests/html/test__fram_8c_source.html>`__)

Description
-----------

The FRAM (ferroelectric random access memory) used is the model CY15B102QN.
It is a 2 Mbit logically organized as 256K × 8 bits.
It is non-volatile, does not need to be supplied to retain its data and can
be written a virtually unlimited number of times.
The FRAM is driven by an SPI interface.

FRAM data variables and entries are defined in the ``fram_cfg.c`` file.
These must also be declared as extern in the ``fram_cfg.h`` file.
In ``fram_cfg.c``, the structure ``fram_base_header`` contains a list of all
variables and entries, along with their size.
In fram_cfg.h, an enum called ``FRAM_BLOCK_ID_e`` is declared. One element
in the enum must be defined as ID for each FRAM variable and entry.

.. warning::
    The entries in the enum ``FRAM_BLOCK_ID_e`` must have the same order as
    their definition in the structure ``fram_base_header``. There must be the
    same number of entries in the structure ``fram_base_header`` as
    in the enum ``FRAM_BLOCK_ID_e``.

.. warning::
    The enum must always end with ``FRAM_BLOCK_MAX``.

The function ``FRAM_Initialize()`` computes the address of each entry in
the FRAM. If the computed address is greater than the highest address
of the FRAM memory, an error is thrown.

.. warning::
    The function ``FRAM_Initialize()`` must be called before using the FRAM.

There are two main functions to interact with the FRAM:
``FRAM_WriteData()`` and ``FRAM_ReadData()``. They must simply be called with
one of the IDs defined in the enum called ``FRAM_BLOCK_ID_e``.
When ``FRAM_WriteData()`` is called, the content of the corresponding variable
defined in the ``fram_cfg.c`` file is written to the FRAM.
When ``FRAM_ReadData()`` is called, the content of the FRAM is read and written
to the corresponding variable defined in the ``fram_cfg.c`` file.
These functions use SPI and are blocking. The SPI used for the FRAM can be
configured in ``spi_cfg.c``.

.. warning::
    The SPI used for the communication with the FRAM must be configured
    with ``SPI_HARDWARE_CHIP_SELECT_DISABLE_ALL`` as the Chip Select pin is
    driven via software.

When writing a variable or entry, the FRAM module first computes a CRC of the
data written and stores it in the FRAM before the data. When reading, the
FRAM module reads the CRC and the data, computes the CRC of the read data
and compares it with the read CRC. This mechanism is transparent when using
the FRAM so it is not necessary to define a CRC field in the variables and
entries.

When changing the FRAM layout, the alignment will not match anymore and this
will be detected with the CRC. The function ``FRAM_ReinitializeAllEntries()``
is available. It will write all variables and entries with their current
state. The consequence is that the alignment will be restored. This function
is useful when making developments that involve changes in the FRAM data
layout.

The read and write functions return one of the following four values:

 - ``FRAM_ACCESS_OK``: the write or read access was successful and
   the CRC read matches with the CRC calculated on the read data.
 - ``FRAM_ACCESS_SPI_BUSY``: the SPI is busy, the transaction could not
   take place.
 - ``FRAM_ACCESS_CRC_BUSY``: the CRC hardware is busy, the transaction
   could not take place.
 - ``FRAM_ACCESS_CRC_ERROR``: the data was read from the FRAM but the read
   CRC does not match with the CRC computed on the read data.
