.. include:: ./../../../../macros.txt
.. include:: ./../../../../units.txt

.. _SPI:

SPI
===

Module Files
------------

Driver
^^^^^^

- ``src/app/driver/spi/spi.c`` (`API <../../../../_static/doxygen/src/html/spi_8c.html>`__, `source <../../../../_static/doxygen/src/html/spi_8c_source.html>`__)
- ``src/app/driver/spi/spi.h`` (`API <../../../../_static/doxygen/src/html/spi_8h.html>`__, `source <../../../../_static/doxygen/src/html/spi_8h_source.html>`__)

Configuration
^^^^^^^^^^^^^

- ``src/app/driver/config/spi_cfg.c`` (`API <../../../../_static/doxygen/src/html/spi__cfg_8c.html>`__, `source <../../../../_static/doxygen/src/html/spi__cfg_8c_source.html>`__)
- ``src/app/driver/config/spi_cfg.h`` (`API <../../../../_static/doxygen/src/html/spi__cfg_8h.html>`__, `source <../../../../_static/doxygen/src/html/spi__cfg_8h_source.html>`__)
- ``src/app/driver/spi/spi_cfg-helper.h`` (`API <../../../../_static/doxygen/src/html/spi__cfg_helper_8h.html>`__, `source <../../../../_static/doxygen/src/html/spi__cfg_helper_8h_source.html>`__)

Unit Test
^^^^^^^^^

- ``tests/unit/app/driver/spi/test_spi.c`` (`API <../../../../_static/doxygen/tests/html/test__spi_8c.html>`__, `source <../../../../_static/doxygen/tests/html/test__spi_8c_source.html>`__)
- ``tests/unit/app/driver/config/test_spi_cfg.c`` (`API <../../../../_static/doxygen/tests/html/test__spi__cfg_8c.html>`__, `source <../../../../_static/doxygen/tests/html/test__spi__cfg_8c_source.html>`__)

Description
-----------

There are three main functions in the SPI API:

- :ref:`spi-transmit`: used to transmit data without DMA, blocking
- :ref:`spi-transmit-receive`: used to transmit and receive data without DMA,
  blocking
- :ref:`spi-transmit-receive-dma`: used to transmit and receive data with DMA,
  non-blocking

Three other functions are available:

- :ref:`spi-dummy`: used to send a dummy byte without DMA, blocking
- :ref:`spi-fram`: wrapper function used in the FRAM module, works without DMA, blocking
- :ref:`spi-slave-receive-dma`: used to configure an SPI interface as a slave device receiving
  data over SPI, non blocking.

.. literalinclude:: ./../../../../../src/app/driver/spi/spi.h
   :language: C
   :linenos:
   :start-after: /* INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE spi-documentation-transmit-start-include */
   :end-before: /* INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE spi-documentation-transmit-stop-include */
   :caption: SPI function to transmit data
   :name: spi-transmit

.. literalinclude:: ./../../../../../src/app/driver/spi/spi.h
   :language: C
   :linenos:
   :start-after: /* INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE spi-documentation-transmit-receive-start-include */
   :end-before: /* INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE spi-documentation-transmit-receive-stop-include */
   :caption: SPI function to transmit and receive data
   :name: spi-transmit-receive

.. literalinclude:: ./../../../../../src/app/driver/spi/spi.h
   :language: C
   :linenos:
   :start-after: /* INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE spi-documentation-transmit-receive-dma-start-include */
   :end-before: /* INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE spi-documentation-transmit-receive-dma-stop-include */
   :caption: SPI function to transmit and receive data with DMA
   :name: spi-transmit-receive-dma

.. literalinclude:: ./../../../../../src/app/driver/spi/spi.h
   :language: C
   :linenos:
   :start-after: /* INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE spi-documentation-dummy-start-include */
   :end-before: /* INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE spi-documentation-dummy-stop-include */
   :caption: SPI function to receive data with DMA as a slave device
   :name: spi-dummy

.. literalinclude:: ./../../../../../src/app/driver/spi/spi.h
   :language: C
   :linenos:
   :start-after: /* INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE spi-documentation-fram-start-include */
   :end-before: /* INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE spi-documentation-fram-stop-include */
   :caption: SPI wrapper function for FRAM
   :name: spi-fram

.. literalinclude:: ./../../../../../src/app/driver/spi/spi.h
   :language: C
   :linenos:
   :start-after: /* INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE spi-documentation-slave-receive-dma-start-include */
   :end-before: /* INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE spi-documentation-slave-receive-dma-stop-include */
   :caption: SPI function to receive data with DMA as a slave device
   :name: spi-slave-receive-dma

To configure an SPI interface, a structure of the type :ref:`spi-configuration`
must be used.
One important parameter is the type of the Chip Select pin, to be chosen within
the enum :ref:`spi-cs-type`. Available possibilities:

- Hardware Chip Select
- Software Chip Select

.. warning::
    Currently the DMA functions presented above can only be used with a
    hardware Chip Select.

.. literalinclude:: ./../../../../../src/app/driver/config/spi_cfg.h
   :language: C
   :linenos:
   :start-after: /* INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE spi-documentation-configuration-start-include */
   :end-before: /* INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE spi-documentation-configuration-stop-include */
   :caption: Configuration for the SPI interface
   :name: spi-configuration

.. literalinclude:: ./../../../../../src/app/driver/config/spi_cfg.h
   :language: C
   :linenos:
   :start-after: /* INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE spi-documentation-cs-type-start-include */
   :end-before: /* INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE spi-documentation-cs-type-stop-include */
   :caption: Type of Chip Select used
   :name: spi-cs-type

.. warning::
    The DMA transmit/receive function works only to transmit 3 words and more.
    To transmit 1 or 2 words, the function without DMA must be used.

The register |spidat1| is used to transmit data. It is made out of two bit
groups:

- bits 0 to 15, containing the data to send (up to 16 bits)
- bits 16 to 31, containing the configuration to be used for the SPI
  transmission

When using DMA, only the data group is written, so the configuration is not
taken into account.
This is problematic, as the configuration contains among other the hardware
Chip Select pin to use.
Another issue is that very often SPI transmissions require the Chip Select pin
to be held active.
To realize this, the field |cshold| in the configuration group must be written
with 1, except for the last word sent, where it must be written with 0.
To overcome these issues, the first word and the last word are written to the
|spidat1| register without DMA. This way the configuration group is written
with the first word.
When DMA writes the subsequent words, the configuration group remains
untouched.
When the last word is written, the |cshold| bit is set to 0.
If this was not done, the Chip Select pin would remains active after the
transmission.
This is the reason why DMA can only be used when transmitting 3 words or more.
