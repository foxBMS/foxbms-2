.. include:: ../../../../macros.txt
.. include:: ../../../../units.txt

.. _HOW_TO_USE_THE_CAN_MODULE:

How to Use the CAN Module
=========================

How to add a new CAN Message
----------------------------

#. Add the new CAN message to the symbol file (``tools/dbc/foxbms.sym``).

   #. If the message is |foxbms| specific prefix it with ``foxBMS``.
   #. Set the DLC
   #. Add a comment that follows the pattern

      - ``optional comment text (in<file>:<function>, fv:tx) optional comment text``
        for files that are transmitted and
      - ``optional comment text (in<file>:<function>, fv:rx) optional comment text``
        for files that are received as seen by the |foxbms| hardware.

#. Export the symbol file as dbc file.
#. Declare the callback function in

   - in file ``src\app\driver\can\cbs\tx\can_cbs_tx.h`` for
     the transmit callbacks
   - in file ``src\app\driver\can\cbs\rx\can_cbs_rx.h`` for
     the receive callbacks

#. Implement a callback.
   If the message handling suites in any of the existing callback
   implementation files (``src/app/driver/can/cbs/tx/*`` or respectively
   ``src/app/driver/can/cbs/rx/*``), implement the callback there, otherwise
   create a new file (and accompanying test file) and implement the callback
   there.

#. Add the message ID and message details:

   - in file ``src/app/driver/config/can_cfg_tx-message-definitions.h`` for
     transmit messages
   - in file ``src/app/driver/config/can_cfg_rx-message-definitions.h`` for
     receive messages

#. Add the message to the callback array in

   - in file ``src/app/driver/config/can_cfg_tx.c`` for transmit messages in
     the array ``can_txMessages``
   - in file ``src/app/driver/config/can_cfg_rx.c`` for receive messages in the
     array ``can_rxMessages``

#. Verify that the CAN message is defined and implemented as expected by
   |foxbms| guidelines:

   .. code-block:: console
      :caption: Verify CAN message definition and callback implementation
      :name: can-message-implementation-verification

      C:\Users\vulpes\Documents\foxbms-2>python tests/can/check_ids.py
      C:\Users\vulpes\Documents\foxbms-2>python tests/can/check_implemented.py

.. warning::

   These script do no syntactical or similar checks of the implementation.
   These scripts do text based comparisons in order to help to get a consistent
   style for the CAN message implementations.
   **The correct implementation etc. must be checked by compiling and debugging.**

Example for a Transmit Message
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Creating the Transmit Message
"""""""""""""""""""""""""""""

In this example a message to transmit the |foxbms| system status is
implemented.
The name of the message is therefore ``BmsState`` (following PascalCase naming
convention).
In the symbol file the message is implemented as ``foxBMS_BmsState`` (prefix
``foxBMS`` followed by the message name) with the ID ``0x220``.
As the message is transmitted from the point of view of the BMS, the comment in
the symbol file must be
``Message containing the foxBMS system state (in:can_cbs_tx_state.c:CANTX_BmsState, fv:tx)``.
The new dbc file must be exported.

Required Macros
"""""""""""""""

The name of the macro to be implemented in
``src/app/driver/config/can_cfg_tx-message-definitions.h`` needs to be prefixed
with ``CANTX_ID`` (``CANTX`` is the module prefix) followed by the message name
in uppercase (``BMS_STATE``), i.e., the full macro name is
``CANTX_BMS_STATE_ID``.
The macro for the period of the transmitted message must be defined: Module
prefix ``CANTX`` followed by the message name (``BMS_STATE``) and the period in
milliseconds (``PERIOD_ms``), i.e., the full macro name is
``CANTX_BMS_STATE_PERIOD_ms``.
Last, the macro for the phase of the transmitted message must be defined:
Module prefix ``CANTX`` followed by the message name (``BMS_STATE``) and the
phase in milliseconds (``PHASE_ms``) i.e., the full macro name is
``CANTX_BMS_STATE_PHASE_ms``.

.. literalinclude:: ./can_how-to_tx.c
   :language: C
   :linenos:
   :start-after: example-can-message-id-macro-start
   :end-before: example-can-message-id-macro-end
   :caption: Adding the new message ID to the transmit message definition file
             ``src/app/driver/config/can_cfg_tx-message-definitions.h``
   :name: adding-the-new-message-id

Now the details of the ID need to be added in an additional macro, that is then
used for the initialization of the transmitted messages macro.
The macro must be named Module prefix (``CANTX``) followed by the message name
(``BMS_STATE``) and suffixed with ``MESSAGE``.

.. literalinclude:: ./can_how-to_tx.c
   :language: C
   :linenos:
   :start-after: example-can-message-id-details-start
   :end-before: example-can-message-id-details-end
   :caption: Adding the details of the new message ID to the transmit message
             definition file
             ``src/app/driver/config/can_cfg_tx-message-definitions.h``
   :name: adding-the-details-to-the-new-message

Callback Function
"""""""""""""""""

The callback declaration must be done in file
``src\app\driver\can\cbs\tx\can_cbs_tx.h``.

.. literalinclude:: ./can_how-to_tx.c
   :language: C
   :linenos:
   :start-after: example-can-tx-messages-callback-declaration-start
   :end-before: example-can-tx-messages-callback-declaration-end
   :caption: Declaration of the callback function in
             ``src\app\driver\can\cbs\tx\can_cbs_tx.h``
   :name: callback-declaration

The callback definition must be done in the appropriate implementation file,
i.e., for the BMS state message in
``src\app\driver\can\cbs\tx\can_cbs_tx_state.c``.

.. literalinclude:: ./can_how-to_tx.c
   :language: C
   :linenos:
   :start-after: example-can-tx-messages-callback-definition-start
   :end-before: example-can-tx-messages-callback-definition-end
   :caption: Definition of the callback function in
             ``src\app\driver\can\cbs\tx\can_cbs_tx_state.c``
   :name: callback-definition

Required Variables Adaptations
""""""""""""""""""""""""""""""

The message needs to be added the transmitted messages array as follows:

.. literalinclude:: ./can_how-to_tx.c
   :language: C
   :linenos:
   :start-after: example-can-tx-messages-configuration-start
   :end-before: example-can-tx-messages-configuration-end
   :caption: Adding the new message to the registry of transmitted messages
             ``src/app/driver/config/can_cfg_tx.c``
   :name: adding-the-the-new-message-to-the-tx-registry


Verification
""""""""""""

Run the check scripts to verify that the messages are implemented as described
in this how-to's guidelines.

.. code-block:: console
   :caption: Verify CAN transmit message definition and callback implementation

   C:\Users\vulpes\Documents\foxbms-2>python tests/can/check_ids.py
   C:\Users\vulpes\Documents\foxbms-2>python tests/can/check_implemented.py

Example for a Receive Message
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The implementation of a receive message is done analogous, by replacing ``tx``
with ``rx`` in paths, prefixes, function names etc.

Using the CAN4 interface
------------------------

If the CAN Module should be extended to the CAN4 interface, a workaround for a
bug in |halcogen| has to be applied.
For details, please refer to :ref:`HALCOGEN_TOOL_DOCUMENTATION`.

Further Reading
---------------

Implementation details of the CAN module are found in :ref:`CAN`.
