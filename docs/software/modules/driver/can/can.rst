.. include:: ./../../../../macros.txt
.. include:: ./../../../../units.txt

.. _CAN:

CAN
===

Module Files
------------

Driver
^^^^^^

- ``src/app/driver/can/can.c``
- ``src/app/driver/can/can.h``
- ``src/app/driver/can/cbs/can_cbs.h``
- ``src/app/driver/can/cbs/can_helper.c``
- ``src/app/driver/can/cbs/can_helper.h``

Configuration
^^^^^^^^^^^^^

- ``src/app/driver/can/cbs/rx/can_cbs_rx.h``
- ``src/app/driver/can/cbs/rx/can_cbs_rx_aerosol-sensor.c``
- ``src/app/driver/can/cbs/rx/can_cbs_rx_bms-state-request.c``
- ``src/app/driver/can/cbs/rx/can_cbs_rx_current-sensor.c``
- ``src/app/driver/can/cbs/rx/can_cbs_rx_debug.c``
- ``src/app/driver/can/cbs/rx/can_cbs_rx_imd-info.c``
- ``src/app/driver/can/cbs/rx/can_cbs_rx_imd-response.c``
- ``src/app/driver/can/cbs/tx-async/can_cbs_tx_crash-dump.c``
- ``src/app/driver/can/cbs/tx-async/can_cbs_tx_crash-dump.h``
- ``src/app/driver/can/cbs/tx-async/can_cbs_tx_debug-build-configuration.c``
- ``src/app/driver/can/cbs/tx-async/can_cbs_tx_debug-build-configuration.h``
- ``src/app/driver/can/cbs/tx-async/can_cbs_tx_debug-response.c``
- ``src/app/driver/can/cbs/tx-async/can_cbs_tx_debug-response.h``
- ``src/app/driver/can/cbs/tx-async/can_cbs_tx_debug-unsupported-multiplexer-values.c``
- ``src/app/driver/can/cbs/tx-async/can_cbs_tx_debug-unsupported-multiplexer-values.h``
- ``src/app/driver/can/cbs/tx-async/can_cbs_tx_imd-request.c``
- ``src/app/driver/can/cbs/tx-async/can_cbs_tx_imd-request.h``
- ``src/app/driver/can/cbs/tx-async/can_cbs_tx_imd-request.c``
- ``src/app/driver/can/cbs/tx-cyclic/can_cbs_tx_bms-state-details.c``
- ``src/app/driver/can/cbs/tx-cyclic/can_cbs_tx_bms-state.c``
- ``src/app/driver/can/cbs/tx-cyclic/can_cbs_tx_cell-temperatures.c``
- ``src/app/driver/can/cbs/tx-cyclic/can_cbs_tx_cell-voltages.c``
- ``src/app/driver/can/cbs/tx-cyclic/can_cbs_tx_cyclic.h``
- ``src/app/driver/can/cbs/tx-cyclic/can_cbs_tx_pack-limits.c``
- ``src/app/driver/can/cbs/tx-cyclic/can_cbs_tx_pack-minimum-maximum-values.c``
- ``src/app/driver/can/cbs/tx-cyclic/can_cbs_tx_pack-state-estimation.c``
- ``src/app/driver/can/cbs/tx-cyclic/can_cbs_tx_pack-values-p0.c``
- ``src/app/driver/can/cbs/tx-cyclic/can_cbs_tx_pack-values-p1.c``
- ``src/app/driver/can/cbs/tx-cyclic/can_cbs_tx_string-minimum-maximum-values.c``
- ``src/app/driver/can/cbs/tx-cyclic/can_cbs_tx_string-state-estimation.c``
- ``src/app/driver/can/cbs/tx-cyclic/can_cbs_tx_string-state.c``
- ``src/app/driver/can/cbs/tx-cyclic/can_cbs_tx_string-values-p0.c``
- ``src/app/driver/can/cbs/tx-cyclic/can_cbs_tx_string-values-p1.c``
- ``src/app/driver/config/can_cfg.c``
- ``src/app/driver/config/can_cfg.h``
- ``src/app/driver/config/can_cfg_rx-message-definitions.h``
- ``src/app/driver/config/can_cfg_rx.c``
- ``src/app/driver/config/can_cfg_tx-async-message-definitions.h``
- ``src/app/driver/config/can_cfg_tx-cyclic-message-definitions.h``
- ``src/app/driver/config/can_cfg_tx_cyclic.c``

Unit Test
^^^^^^^^^

- ``tests/unit/app/driver/can/cbs/rx/test_can_cbs_rx_aerosol-sensor.c``
- ``tests/unit/app/driver/can/cbs/rx/test_can_cbs_rx_bms-state-request.c``
- ``tests/unit/app/driver/can/cbs/rx/test_can_cbs_rx_current-sensor.c``
- ``tests/unit/app/driver/can/cbs/rx/test_can_cbs_rx_debug.c``
- ``tests/unit/app/driver/can/cbs/rx/test_can_cbs_rx_imd-info.c``
- ``tests/unit/app/driver/can/cbs/rx/test_can_cbs_rx_imd-response.c``
- ``tests/unit/app/driver/can/cbs/tx-async/test_can_cbs_tx_crash-dump.c``
- ``tests/unit/app/driver/can/cbs/tx-async/test_can_cbs_tx_debug-build-configuration.c``
- ``tests/unit/app/driver/can/cbs/tx-async/test_can_cbs_tx_debug-response.c``
- ``tests/unit/app/driver/can/cbs/tx-async/test_can_cbs_tx_debug-unsupported-multiplexer-values.c``
- ``tests/unit/app/driver/can/cbs/tx-async/test_can_cbs_tx_imd-request.c``
- ``tests/unit/app/driver/can/cbs/test_can_helper.c``
- ``tests/unit/app/driver/can/cbs/test_can_helper.h``
- ``tests/unit/app/driver/can/cbs/tx-cyclic/test_can_cbs_tx_bms-state-details.c``
- ``tests/unit/app/driver/can/cbs/tx-cyclic/test_can_cbs_tx_bms-state.c``
- ``tests/unit/app/driver/can/cbs/tx-cyclic/test_can_cbs_tx_cell-temperatures.c``
- ``tests/unit/app/driver/can/cbs/tx-cyclic/test_can_cbs_tx_cell-voltages.c``
- ``tests/unit/app/driver/can/cbs/tx-cyclic/test_can_cbs_tx_pack-limits.c``
- ``tests/unit/app/driver/can/cbs/tx-cyclic/test_can_cbs_tx_pack-minimum-maximum-values.c``
- ``tests/unit/app/driver/can/cbs/tx-cyclic/test_can_cbs_tx_pack-state-estimation.c``
- ``tests/unit/app/driver/can/cbs/tx-cyclic/test_can_cbs_tx_pack-values-p0.c``
- ``tests/unit/app/driver/can/cbs/tx-cyclic/test_can_cbs_tx_pack-values-p1.c``
- ``tests/unit/app/driver/can/cbs/tx-cyclic/test_can_cbs_tx_string-minimum-maximum-values.c``
- ``tests/unit/app/driver/can/cbs/tx-cyclic/test_can_cbs_tx_string-state-estimation.c``
- ``tests/unit/app/driver/can/cbs/tx-cyclic/test_can_cbs_tx_string-state.c``
- ``tests/unit/app/driver/can/cbs/tx-cyclic/test_can_cbs_tx_string-values-p0.c``
- ``tests/unit/app/driver/can/cbs/tx-cyclic/test_can_cbs_tx_string-values-p1.c``
- ``tests/unit/app/driver/can/test_can.c``
- ``tests/unit/app/driver/can/test_can_1.c``
- ``tests/unit/app/driver/can/test_can_2.c``
- ``tests/unit/app/driver/config/test_can_cfg.c``
- ``tests/unit/app/driver/config/test_can_cfg_rx.c``
- ``tests/unit/app/driver/config/test_can_cfg_tx_cyclic.c``
- ``tests/unit/app/engine/diag/cbs/test_diag_cbs_can.c``
- ``tests/unit/support/test_can_mpu_prototype_queue_create_stub.h``

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
- repetition time, period of transmission in ms.
  Must be a multiple of 10.
- repetition phase, delay for the first transmission.
  Avoids sending all messages with same period at the same time.
- byte order, endianness (big or little endian) of CAN data.
- callback function, pointer to the function that is called when the message
  has to be sent.
- multiplexer, pointer to a number.
  This is used to multiplex data in CAN messages.
  A static variable must be defined to be used as multiplexer.

The data of the CAN message is divided into signals.
Data for each signal is prepared within the callback function.
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
- signal data. Fox example, if data is a float, it must be cast
  to an integer before being passed to the function.
- endianness (big or little endian) of CAN data.

Once the CAN message is ready, the function
``CAN_TxSetCanDataWithMessageData()`` must be called.
It will store the CAN message in the variable used by the low-level driver for
the actual transmission.

The function ``CAN_PeriodicTransmit()`` is called every 10ms by the 10ms task.
It parses all the elements of ``can_txMessages[]``.
If the time has been reached to send the messages, the corresponding callback
function is called.

The message is then sent with the function ``CAN_DataSend()``.
The function ``CAN_DataSend()`` can also be used to send a CAN message directly
anywhere else in the code.

If all mailboxes on the CAN are full when the message is sent, it cannot be
transmitted. In this case, it will be stored in the queue
``ftsk_canTxUnsentMessagesQueue``. With the next call of the function
``CAN_PeriodicTransmit()``, the unsent messages will be pulled from this queue
and resent with the function ``CAN_DataSend()``.

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

A receive queue was used because usually the developer needs to access the
database in the callback of the receive function, but this must not be done in
an interrupt routine.
With the current implementation, the receive interrupt routine sends the
received data to the queue.
The ``CAN_ReadRxBuffer()`` function retrieves the messages from the queue and
calls the callbacks outside of the interrupt routine.

Configuration in |ti-halcogen|
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

64 messages boxes are available for each CAN interface. 32 of them are
configured as transmit message boxes, the other half as receive message boxes
(with |ti-halcogen|), so the macro ``CAN_NR_OF_TX_MESSAGE_BOX`` is set to 32.
It must be adapted if the number of transmit message boxes is changed.

Unfortunately, using |ti-halcogen|, only the complete CAN interface and not the
individual message boxes can be configured if standard or extended identifiers
shall be used.

For this reason, four receive mailboxes (mailboxes 61-64) are hard-coded during
the initialization and overwrite the respective |ti-halcogen| configuration for
these mailboxes.
The mailboxes are configured to receive all CAN messages
with an extended 29-bit identifier. This configuration is done in function
``CAN_ConfigureRxMailboxesForExtendedIdentifiers()``

Configuration errors in |ti-halcogen|
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Using the CAN1 mailbox 42
-------------------------

Additionally, there is a bug in |ti-halcogen| regarding the CAN1 mailbox 42 as
described in :ref:`HALCOGEN_TOOL_DOCUMENTATION`.
The missing configuration for this mailbox is also done in function
``CAN_ConfigureRxMailboxesForExtendedIdentifiers()``.
Mailbox 42 is configured to receive all CAN messages with a standard 11-bit
identifier.

Using the CAN4 interface
------------------------

If the CAN Module should be extended to the CAN4 interface, a workaround for a
bug in |ti-halcogen| has to be applied.
For details, please refer to :ref:`HALCOGEN_TOOL_DOCUMENTATION`.

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
