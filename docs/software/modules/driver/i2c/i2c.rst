.. include:: ../../../../macros.txt
.. include:: ../../../../units.txt

.. _I2C_MODULE:

I2C Module
==========

Module Files
------------

Driver
^^^^^^

- ``src/app/driver/i2c/i2c.c`` (`API <../../../../_static/doxygen/src/html/i2c_8c.html>`__, `source <../../../../_static/doxygen/src/html/i2c_8c_source.html>`__)
- ``src/app/driver/i2c/i2c.h`` (`API <../../../../_static/doxygen/src/html/i2c_8h.html>`__, `source <../../../../_static/doxygen/src/html/i2c_8h_source.html>`__)

Unit Test
^^^^^^^^^

- ``tests/unit/app/driver/can/test_i2c.c`` (`API <../../../../_static/doxygen/tests/html/test__i2c_8c.html>`__, `source <../../../../_static/doxygen/tests/html/test__i2c_8c_source.html>`__)

Detailed Description
--------------------

The driver consists of READ functions and WRITE functions. In all cases,
``slaveAddress`` are the seven bits of the |I2C| slave address.

There are two cases for READ:

- ``I2C_Read()``: implements the usual way of reading. First a write operation
  is made after the START condition, to write the address (``readAddress``)
  to read from. Then a REPEATED START condition issued on the bus and
  ``nrBytes`` are read. The result is stored in the data pointed by
  ``readData``.

- ``I2C_ReadDirect()``: a START condition is issued on the bus and
  ``nrBytes`` are read directly. The result is stored in the data pointed by
  ``readData``.

Similarly, there are two cases for WRITE:

- ``I2C_WriteDirect()``: a START condition is issued on the bus and
  ``nrBytes`` are written directly. The data to be written is pointed by
  ``writeData``.

- ``I2C_Write()``: like the preceding function, a START condition is issued
  on the bus and bytes are written. The difference is that ``nrBytes+1`` are
  written: first ``writeAddress`` is written, then the data to be written
  pointed by ``writeData``. This function is symmetric to ``I2C_Read()``,
  where first the register address to access is written.

Two DMA functions are also implemented, similar to the functions where
the register address is written first before read or write.
``dmaGroupANotification()`` is called after the bytes are written to or
read from |I2C| to deactivate the corresponding DMA transfers.
Currently the DMA functions lack transaction control: in case the |I2C|
transaction does not finish or take place, this is not signaled to the
driver.
