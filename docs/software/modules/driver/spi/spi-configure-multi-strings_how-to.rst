.. include:: ../../../../macros.txt
.. include:: ../../../../units.txt

.. _HOW_TO_CONFIGURE_SPI_FOR_MULTI_STRING:

How to configure SPI for multi string support
=============================================

By default, the first string is already configured for each AFE version.
This How-to guide aims to show how to add configurations for the SPI
communication needed to support additional strings.

On the Hardware side, each AFE driver module communicates with the AFE slave
strings via a slave-specific SPI/TPL transceiver ICs on the interface board.
In Software, these transceivers are addressed from within the low level AFE
driver parts by calls to the SPI driver module with an
``SPI_INTERFACE_CONFIG_s`` struct that represents the SPI communication interface.
While most AFEs (like LTC slaves) use one SPI interface per transceiver for
both Rx and Tx, there are some variants (like NXP slaves) that need an SPI
interface for Rx and Tx each, so two per string.


.. _adding_an_spi_interface:

Adding an SPI interface
-----------------------
The interfaces for the different AFE versions are configured hard-coded in the
file ``src/app/driver/config/spi_cfg.c`` as an array with one
``SPI_INTERFACE_CONFIG_s`` entry per string.
To configure an additional string, one has to add a new struct
initialization and specify the correct SPI node (as found in the hardware
schematics) in the pNode and pGioPort entries (PC3 Latch should always be kept
as the Pin Output Latch Address).

To enable addressing multiple strings using the same SPI hardware modules,
a Chip Select (CS) line is used to specify which SPI-slave on the bus (and thus
which AFE string) is currently active.
Each SPI node has multiple CS pins, so the associated pin for the correct
target also has to be added to the ``SPI_INTERFACE_CONFIG_s`` entry.

Furthermore, each ``SPI_INTERFACE_CONFIG_s`` also is filled with its own
``spiDAT1_t`` entry that specifies the SPI communication structure.
Since this entry should not change between strings, the ``spiDAT1_t``
configured as default can simply be duplicated and then be referenced from the
SPI interface config.

For AFE variants that use different SPI configs for Rx and Tx, each of the
configs also have to be extended by a version for Rx and Tx communication,
where their SPI node and CS pin can differ for both data directions.
In order to still support using only one string, all additional config
entries should be framed in compiler defines.


.. _initializing_the_chip_select_pins:

Initializing the Chip Enable pins
---------------------------------
Additional to the SPI chip select lines, each transceiver chip usually has an
enable pin that is independent from the SPI configuration and can deactivate
the transceiver completely.
Since the transceiver addressing is done via the SPI chip select lines,
the transceiver enable pin can be held active all the time.
This has to be set only once during initialization of the AFE driver and thus
is implemented in the ``<AFE-name>_Initialize`` function in the file
``src/app/driver/afe/<AFE-manufacturer>/<AFE-IC>/api/<AFE-prefix>_afe.c``.
These enable pins are connected to the foxbms master via its port expander (PEX).

.. _cyclically switching to a new string:

Cyclically switching to a new string
------------------------------------
The AFE driver modules typically cycle through the different strings after all
measurements for all modules on one string are finished.

When there is more than one string used, the AFE driver software has to make
sure that for each SPI communication request, the correct SPI interface is
used.
This should already be implemented by saving a reference to the start of
the SPI interface config array explained in :ref:`adding_an_spi_interface`
during initialization,
and then by cyclically indexing the following interfaces being addressed
by pointer-increments.

See Also
--------

- :ref:`SPI_DRIVER`
- :ref:`ANALOG_FRONT_END_API`
