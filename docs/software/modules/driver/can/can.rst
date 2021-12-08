.. include:: ./../../../../macros.txt
.. include:: ./../../../../units.txt

.. _CAN:

CAN
===

Module Files
------------

Driver
^^^^^^

- ``src/app/driver/can/can.c`` (`API <../../../../_static/doxygen/src/html/can_8c.html>`__, `source <../../../../_static/doxygen/src/html/can_8c_source.html>`__)
- ``src/app/driver/can/can.h`` (`API <../../../../_static/doxygen/src/html/can_8h.html>`__, `source <../../../../_static/doxygen/src/html/can_8h_source.html>`__)
- ``src/app/driver/can/cbs/can_helper.c`` (`API <../../../../_static/doxygen/src/html/can__helper_8c.html>`__, `source <../../../../_static/doxygen/src/html/can__helper_8c_source.html>`__)
- ``src/app/driver/can/cbs/can_helper.h`` (`API <../../../../_static/doxygen/src/html/can__helper_8h.html>`__, `source <../../../../_static/doxygen/src/html/can__helper_8h_source.html>`__)
- ``src/app/driver/can/cbs/can_cbs.h`` (`API <../../../../_static/doxygen/src/html/can__cbs_8h.html>`__, `source <../../../../_static/doxygen/src/html/can__cbs_8h_source.html>`__)

Configuration
^^^^^^^^^^^^^

- ``src/app/driver/config/can_cfg.c`` (`API <../../../../_static/doxygen/src/html/can__cfg_8c.html>`__, `source <../../../../_static/doxygen/src/html/can__cfg_8c_source.html>`__)
- ``src/app/driver/config/can_cfg.h`` (`API <../../../../_static/doxygen/src/html/can__cfg_8h.html>`__, `source <../../../../_static/doxygen/src/html/can__cfg_8h_source.html>`__)

Unit Test
^^^^^^^^^

- ``tests/unit/app/driver/can/test_can.c`` (`API <../../../../_static/doxygen/tests/html/test__can_8c.html>`__, `source <../../../../_static/doxygen/tests/html/test__can_8c_source.html>`__)
- ``tests/unit/app/driver/config/test_can_cfg.c`` (`API <../../../../_static/doxygen/tests/html/test__can__cfg_8c.html>`__, `source <../../../../_static/doxygen/tests/html/test__can__cfg_8c_source.html>`__)

Description
-----------

In ``can_cfg.c``, CAN messages are defined in two tables:

- ``can_txMessages[]`` for messages to be sent.
- ``can_rxMessages[]`` for messages to be received.

Messages to send
^^^^^^^^^^^^^^^^

The sent message parameters are:

- CAN ID of message to be sent.
- data length code, number of bytes to send. Default 8, maximum 8.
- repetition time, period of transmission in ms. Must be a multiple of 10.
- repetition phase, delay for the first transmission. Avoids sending all
  messages with same period at the same time.
- byte order, endianness (big or little endian) of CAN data.
- callback function, pointer to the function that is called when the message
  has to be sent.
- multiplexer, pointer to a number. This is used to multiplex data in CAN
  messages. A static variable must be defined to be used as multiplexer.

The data of the CAN message is divided into signals. Data for each signal
is prepared within the callback function.
The developer must implement the signals as needed by the application.

Two helper functions are defined for this task,
``CAN_TxSetMessageDataWithSignalData()`` and
``CAN_TxSetCanDataWithMessageData()``.
In the callback function, a ``uint64_t variable`` must be defined, which
represents the CAN message.
With the function ``CAN_TxSetMessageDataWithSignalData()``, the signal data
can be stored in the message data. The parameters are

- pointer to 64-bit CAN message.
- bit position in CAN message where the signal data must be stored.
- length of the signal data in the CAN message.
- signal data. If data is e.g. a float, it must be cast
  to an integer before being passed to the function.
- endianness (big or little endian) of CAN data.

Once the CAN message is ready, the function
``CAN_TxSetCanDataWithMessageData()`` must be called. It will
store the CAN message in the variable used by the low-level driver for
the actual transmission.

The function ``CAN_PeriodicTransmit()`` is called every 10ms by the 10ms task.
It parses all the elements of ``can_txMessages[]``. If the time has been
reached to send the messages, the corresponding callback function is called.

The message is then sent with the function ``CAN_DataSend()``. The function
``CAN_DataSend()`` can also be used to send a CAN message directly anywhere
else in the code.

Messages to receive
^^^^^^^^^^^^^^^^^^^

The received message parameters are:

- CAN ID of message to be received.
- data length code, number of bytes to receive. Default 8, maximum 8.
- byte order, endianness (big or little endian) of CAN data.
- callback function: pointer to the function that is called when the message
  is received. The data of the CAN message is available within this
  function.


A receive queue called ``ftsk_canRxQueue`` is used as shown in
:ref:`queue-can-receive-code-queue` and
:ref:`queue-can-receive-code-vars`.

.. literalinclude:: ./../../../../../src/app/task/ftask/freertos/ftask_freertos.c
   :language: C
   :linenos:
   :start-after: /* INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE can-documentation-rx-queue-handle-start-include */
   :end-before: /* INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE can-documentation-rx-queue-handle-stop-include */
   :caption: Queue handle for CAN receive
   :name: queue-can-receive-code-queue

.. literalinclude:: ./../../../../../src/app/task/ftask/freertos/ftask_freertos.c
   :language: C
   :linenos:
   :start-after: /* INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE can-documentation-rx-queue-vars-start-include */
   :end-before: /* INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE can-documentation-rx-queue-vars-stop-include */
   :caption: Supporting variables for the queue of the CAN receive module
   :name: queue-can-receive-code-vars

When CAN messages are received, the CAN interrupt callback calls
``CAN_RxInterrupt()``. The message received is sent to the queue.
The function ``CAN_ReadRxBuffer()`` is called every 1ms by the 1ms task.
For each element in the queue, it checks if the CAN message ID matches
an ID of the RX message list ``can_rxMessages[]``. If this is the case,
the corresponding callback function is called.

In the callback function, a ``uint64_t variable`` must be defined, which
represents the CAN message. The helper function
``CAN_RxGetMessageDataFromCanData()`` **MUST** be called at the beginning.
It copies the CAN data retrieved by the low level driver into the message
variable. If necessary, the other helper function
``CAN_RxGetSignalDataFromMessageData()`` can be used to retrieve signal data
easily from the message. The parameters are

- 64-bit CAN message.
- bit position in CAN message where the signal data
  must be stored.
- length of the signal data in the CAN message.
- pointer to signal data.
- endianness (big or little endian) of CAN data.

The signal data is then available in the variable pointed to by the signal
data pointer.

A receive queue was used because usually the developer needs
to access the database in the callback of the receive function, but this
must not be done in an interrupt routine. With the current implementation,
the receive interrupt routine sends the received data to the queue.
The ``CAN_ReadRxBuffer()`` function retrieves the messages from the queue
and calls the callbacks outside of the interrupt routine.

Configuration in |halcogen|
^^^^^^^^^^^^^^^^^^^^^^^^^^^

64 messages boxes are available for each CAN interface. 32 of them are
configured as transmit message boxes, the other half as receive message boxes
(with |halcogen|), so the macro ``CAN_NR_OF_TX_MESSAGE_BOX`` is set to 32.
It must be adapted if the number of transmit message boxes is changed.

When using the CAN interface ``CAN4``, special care has to be taken because of
a bug in |halcogen|. For more information, refer to
:ref:`HALCOGEN_TOOL_DOCUMENTATION`.

Callback definition
^^^^^^^^^^^^^^^^^^^

If a new file is needed for a new callback, it must be added in the
directory ``./src/app/driver/can/cbs``. The corresponding header is the file
``can_cbs.h`` in the same directory. The new callback file must also be added
to the ``wscript`` file in ``./src/app/driver``. For instance, if the file
``can_my_callback.c`` is added, the line

.. code-block:: python

    os.path.join("can", "cbs", "can_my_callback.c"),

must be added.
