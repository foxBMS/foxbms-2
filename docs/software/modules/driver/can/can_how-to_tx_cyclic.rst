Creating the Cyclic Transmit Message
""""""""""""""""""""""""""""""""""""

In this example a message to transmit the system status is implemented.
The name of the message is therefore ``BmsState`` (following PascalCase naming
convention).
In the symbol file the message is implemented as ``foxBMS_BmsState`` (prefix
``foxBMS`` followed by the message name) and the message ID ``0x220``.
As the message is transmitted (``tx``) from the point of view of the BMS, the
comment in the symbol file must be
``Message containing the foxBMS system state (in:can_cbs_tx_bms-state.c:CANTX_BmsState, fv:tx)``.
The changed symbol file must be saved and - as changes are applied - the dbc
file must be exported again.

Required Macros
"""""""""""""""

The name of the macros to be implemented in
``src/app/driver/config/can_cfg_tx-cyclic-message-definitions.h`` need to be
prefixed with ``CANTX`` (as ``CANTX`` is the module prefix) and the message
name in uppercase (``BMS_STATE``), i.e., all macro names start with
``CANTX_BMS_STATE``.
The message's

- ID with suffix ``_ID``,
- ID type with suffix ``_ID_TYPE``,
- period with suffix ``_PERIOD_ms``,
- phase with suffix ``_PHASE_ms``,
- endianness with suffix ``_ENDIANNESS`` and
- DLC with suffix ``_DLC``

need to be defined through macros:

.. literalinclude:: ./can_how-to_tx_cyclic.c
   :language: C
   :linenos:
   :start-after: example-can-tx-cyclic-message-id-macro-start
   :end-before: example-can-tx-cyclic-message-id-macro-end
   :caption: Adding the new message ID to the cyclic transmit message
             definition file
             ``src/app/driver/config/can_cfg_tx-cyclic-message-definitions.h``

Now the details of the ID need to be added in an additional macro, that is then
used for the initialization of the transmitted messages macro.
The macro must be named Module prefix (``CANTX``) followed by the message name
(``BMS_STATE``) and suffixed with ``MESSAGE``.

.. literalinclude:: ./can_how-to_tx_cyclic.c
   :language: C
   :linenos:
   :start-after: example-can-tx-cyclic-message-id-details-start
   :end-before: example-can-tx-cyclic-message-id-details-end
   :caption: Adding the details of the new message ID to the cyclic transmit
             message definition file
             ``src/app/driver/config/can_cfg_tx-cyclic-message-definitions.h``

Callback Function
"""""""""""""""""

The callback declaration must be added to the file
``src/app/driver/can/cbs/tx/can_cbs_tx_cyclic.h``.

.. literalinclude:: ./can_how-to_tx_cyclic.c
   :language: C
   :linenos:
   :start-after: example-can-tx-cyclic-messages-callback-declaration-start
   :end-before: example-can-tx-cyclic-messages-callback-declaration-end
   :caption: Declaration of the callback function in
             ``src/app/driver/can/cbs/tx/can_cbs_tx_cyclic.h``

The callback definition must be done in the appropriate implementation file,
i.e., for the BMS state message in
``src/app/driver/can/cbs/tx/can_cbs_tx_bms-state.c``.

.. literalinclude:: ./can_how-to_tx_cyclic.c
   :language: C
   :linenos:
   :start-after: example-can-tx-cyclic-messages-callback-definition-start
   :end-before: example-can-tx-cyclic-messages-callback-definition-end
   :caption: Definition of the callback function in
             ``src/app/driver/can/cbs/tx/can_cbs_tx_bms-state.c``

Required Variables Adaptations
""""""""""""""""""""""""""""""

The message needs to be added the transmitted messages array as follows:

.. literalinclude:: ./can_how-to_tx_cyclic.c
   :language: C
   :linenos:
   :start-after: example-can-tx-cyclic-messages-configuration-start
   :end-before: example-can-tx-cyclic-messages-configuration-end
   :caption: Adding the new message to the registry of transmitted messages
             ``src/app/driver/config/can_cfg_tx_cyclic.c``

Verification
""""""""""""

Run the check scripts to verify that the messages are implemented as described
in this how-to.

.. code-block:: powershell
   :caption: Verify CAN transmit message definition and callback implementation

   PS C:\Users\vulpes\Documents\foxbms-2> .\fox.ps1 run-script tests/can/check_ids.py
   PS C:\Users\vulpes\Documents\foxbms-2> .\fox.ps1 run-script tests/can/check_implemented.py
