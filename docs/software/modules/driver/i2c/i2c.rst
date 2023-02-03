.. include:: ../../../../macros.txt
.. include:: ../../../../units.txt

.. _I2C_MODULE:

I2C Module
==========

.. spelling::
    cacheable

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

The driver consists of READ functions and WRITE functions.
In all cases, ``slaveAddress`` are the seven bits of the |I2C| slave address.

Reading
^^^^^^^

There are two cases for READ:

- ``I2C_WriteRead()``: implements the usual way of reading.
  First a write operation is made after the START condition and
  ``nrBytesWrite`` are written directly.
  The data to be written is pointed by ``writeData``.
  Then a REPEATED START condition issued on the bus and ``nrBytesRead`` are
  read.
  The result is stored in the data pointed by ``readData``.

- ``I2C_Read()``: a START condition is issued on the bus and ``nrBytes`` are
   read directly.
   The result is stored in the data pointed by ``readData``.

Writing
^^^^^^^

There is one function to WRITE:

- ``I2C_Write()``: a START condition is issued on the bus and
  ``nrBytes`` are written directly. The data to be written is pointed by
  ``writeData``.

DMA
^^^

Three similar DMA functions are also implemented, with the suffix ``DMA`` at
the end of the function name.
``dmaGroupANotification()`` is called after the bytes are written to or read
from |I2C| to deactivate the corresponding DMA transfers.
The DMA functions lack transaction control: in case the |I2C| transaction does
not finish or take place, this is not signaled to the driver.
This is detected because the OS function waiting for the tasks to be notified
times out instead of receiving a notification.

.. warning::
    When using the DMA functions, the read and write variables must be declared
    in a non-cacheable area.

Details on the implementation
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The functions all use the |I2C| interface as a master.

To send bytes with a stop condition at the end, the repeat mode of the |I2C|
interface is deactivated.
The counter register is set with the number of bytes to send and the stop
condition is generated.
With this configuration, the counter is decremented each time a byte is sent.
When the counter reaches zero, a stop bit is sent.

To read bytes, the repeat mode must be activated.
When the start condition is set, a start bit is sent and the master starts
receiving bytes.
The byte counter is not used.
When the stop condition is generated, the stop bit is send, but due to the
double buffer on the receiver side, the stop condition must be issued after
reading the (message size-1)\ :sup:`th`\ |_| byte.
Due to this, the DMA receive functions use the DMA last transfer started
interrupt to issue the stop condition, but the stop bit is sent after the
last byte is received by DMA.
To avoid receiving one byte more than needed, when receiving ``N`` bytes,
``N-1`` received bytes are transferred with DMA.
The last byte is copied by the CPU in the DMA receive transfer finished
interrupt.
As a consequence, the following must be considered:

.. warning::
    The DMA receive functions works only to receive two bytes and more.
    To receive one byte, the functions without DMA must be used.

Task running the functions with DMA
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

As the DMA functions are non blocking, the synchronization must be taken care
of.
A specific task has been implemented for the communication with |I2C| devices.
It runs continuously like the AFE task.
At the end of the DMA function, the task will be blocked, waiting for
a notification.
The notification is made in the DMA complete interrupt: the task then wakes up.
If the notification does not come within ``I2C_NOTIFICATION_TIMEOUT_ms``
milliseconds, the task is unblocked and the I2C communication is declared to
have failed.
To leave CPU time for the other tasks, the |I2C| task should be blocked for at
least 1 millisecond after each |I2C| transaction.
