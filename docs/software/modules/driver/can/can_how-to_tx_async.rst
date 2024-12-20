Creating the Asynchronous Transmit Message
""""""""""""""""""""""""""""""""""""""""""

In this example a message to asynchronously transmit some debug information on
request is implemented.
The name of the message is therefore ``DebugResponse`` (following PascalCase
naming convention).
In the symbol file the message is implemented as ``foxBMS_DebugResponse``
(prefix ``foxBMS`` followed by the message name) and the message ID ``0x227``.
As the message is transmitted (``tx``) from the point of view of the BMS, the
comment in the symbol file must be
``Message containing some debug information (in:can_cbs_tx_debug-response.c:CANTX_DebugResponse, fv:tx)``.
The changed symbol file must be saved and - as changes are applied - the dbc
file must be exported again.

Required Macros
"""""""""""""""

The name of the macros to be implemented in
``src/app/driver/config/can_cfg_tx-async-message-definitions.h`` need to be
prefixed with ``CANTX`` (as ``CANTX`` is the module prefix) and the message
name in uppercase (``DEBUG_RESPONSE``), i.e., all macro names start with
``CANTX_DEBUG_RESPONSE``.
The message's

- ID with suffix ``_ID``,
- ID type with suffix ``_ID_TYPE``,
- endianness with suffix ``_ENDIANNESS`` and
- DLC with suffix ``_DLC``

need to be defined through macros:

.. literalinclude:: ./can_how-to_tx_async.c
   :language: C
   :linenos:
   :start-after: example-can-tx-asynchronous-message-id-macro-start
   :end-before: example-can-tx-asynchronous-message-id-macro-end
   :caption: Adding the new message ID to the cyclic transmit message
             definition file
             ``src/app/driver/config/can_cfg_tx-async-message-definitions.h``

Callback Function
"""""""""""""""""

The callback declaration must be done in file
``src/app/driver/can/cbs/tx-async/can_cbs_tx_debug-response.h``.

.. literalinclude:: ./can_how-to_tx_async.h
   :language: C
   :linenos:
   :start-after: example-can-tx-asynchronous-messages-callback-declaration-start
   :end-before: example-can-tx-asynchronous-messages-callback-declaration-end
   :caption: Declaration of the callback function in
             ``src/app/driver/can/cbs/tx-async/can_cbs_tx_debug-response.h``

The callback definition must be done in the appropriate implementation file,
i.e., for the BMS state message in
``src/app/driver/can/cbs/tx-async/can_cbs_tx_debug-response.c``.

.. literalinclude:: ./can_how-to_tx_async.c
   :language: C
   :linenos:
   :start-after: example-can-tx-asynchronous-messages-callback-definition-start
   :end-before: example-can-tx-asynchronous-messages-callback-definition-end
   :caption: Definition of the callback function in
             ``src/app/driver/can/cbs/tx-async/can_cbs_tx_debug-response.c``

As last step, the function ``CANTX_DebugResponse`` needs to be called in the
appropriate code path of the user code.
This cannot be covered by the how to as this is too user specific.
See already implemented asynchronous transmitted messages as starting point.

Verification
""""""""""""

Run the check scripts to verify that the messages are implemented as described
in this how-to.

.. tabs::

   .. group-tab:: Win32/PowerShell

      .. code-block:: powershell

         .\fox.ps1 run-script tests\can\check_ids.py
         .\fox.ps1 run-script tests\can\check_implemented.py

   .. group-tab:: Win32/cmd.exe

      .. code-block:: bat

         fox.bat run-script tests\can\check_ids.py
         fox.bat run-script tests\can\check_implemented.py

   .. group-tab:: Win32/Git bash

      .. code-block:: shell

         ./fox.sh run-script tests/can/check_ids.py
         ./fox.sh run-script tests/can/check_implemented.py

   .. group-tab:: Linux

      .. code-block:: shell

         ./fox.sh run-script tests/can/check_ids.py
         ./fox.sh run-script tests/can/check_implemented.py
