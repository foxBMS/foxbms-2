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

      +-------------------+------------------------------------------------------------------------------+
      | Message direction | Comment                                                                      |
      +===================+==============================================================================+
      | transmit          | ``optional comment text (in<file>:<function>, fv:tx) optional comment text`` |
      +-------------------+------------------------------------------------------------------------------+
      | receive           | ``optional comment text (in<file>:<function>, fv:rx) optional comment text`` |
      +-------------------+------------------------------------------------------------------------------+

      The message direction is specified as seen from the BMS view.

#. Export the symbol file as dbc file to ``tools/dbc/foxbms.dbc``.
#. Declare the callback function for the message in the appropriate file:

   +---------------------------------+-----------------------------------------------------------------+
   | Message type                    | File                                                            |
   +=================================+=================================================================+
   | cyclic transmit callbacks       | ``src/app/driver/can/cbs/tx-cyclic/can_cbs_tx_cyclic.h``        |
   +---------------------------------+-----------------------------------------------------------------+
   | asynchronous transmit callbacks | ``src/app/driver/can/cbs/tx-async/can_cbs_tx_<my-file-name>.h`` |
   +---------------------------------+-----------------------------------------------------------------+
   | receive callbacks               | ``src/app/driver/can/cbs/rx/can_cbs_rx.h``                      |
   +---------------------------------+-----------------------------------------------------------------+

   **Note 1**: every asynchronous transmit callback is implemented in an
   separate file, that **only** consists of this one, specific callback.

   **Note 2**: the name *callback* is kept for the asynchronous transmit
   callbacks for consistency reasons although they are not callbacks in the
   classical meaning of the word.

   **Note 3**: Only the transmitted messages are separated in cyclic and
   asynchronous callbacks, therefore there is only one file for receive
   callbacks.
   This file includes the callbacks for the cyclic and asynchronously received
   messages.
   The same reason applies for splitting things between
   *asynchronous transmit*, *cyclic transmit* and *receive* is consistently
   through the document and code.

#. Implement a callback in a new file and the accompanying test file/files in
   the appropriate directories at:

   +---------------------------------+------------------------------------------------------------------------------------------+
   | Message type                    | New file                                                                                 |
   +=================================+==========================================================================================+
   | cyclic transmit callbacks       | ``src/app/driver/can/cbs/tx-cyclic/*`` and ``tests/unit/app/driver/can/cbs/tx-cyclic/*`` |
   +---------------------------------+------------------------------------------------------------------------------------------+
   | asynchronous transmit callbacks | ``src/app/driver/can/cbs/tx-async/*`` and ``tests/unit/app/driver/can/cbs/tx-async/*``   |
   +---------------------------------+------------------------------------------------------------------------------------------+
   | receive callbacks               | ``src/app/driver/can/cbs/rx/*`` and ``tests/unit/app/driver/can/cbs/rx/*``               |
   +---------------------------------+------------------------------------------------------------------------------------------+

#. Add the message ID and message details

   +---------------------------------+-------------------------------------------------------------------+
   | Message type                    | Declaration file                                                  |
   +=================================+===================================================================+
   | cyclic transmit callbacks       | ``src/app/driver/config/can_cfg_tx-cyclic-message-definitions.h`` |
   +---------------------------------+-------------------------------------------------------------------+
   | asynchronous transmit callbacks | ``src/app/driver/config/can_cfg_tx-async-message-definitions.h``  |
   +---------------------------------+-------------------------------------------------------------------+
   | receive callbacks               | ``src/app/driver/config/can_cfg_rx-message-definitions.h``        |
   +---------------------------------+-------------------------------------------------------------------+

#. Add the message to the respective callback array:

+---------------------------------+-------------------------------------------------------------------------------------+
| Message type                    |                                                                                     |
+=================================+=====================================================================================+
| cyclic transmit callbacks       | array ``can_txMessages`` in the file ``src/app/driver/config/can_cfg_tx_cyclic.c``  |
+---------------------------------+-------------------------------------------------------------------------------------+
| asynchronous transmit callbacks | The callbacks for asynchronous transmit messages are not stored in some array.      |
|                                 | These callbacks are only called asynchronously in some specific code paths and      |
|                                 | so there is no central "processing" function for these callbacks.                   |
+---------------------------------+-------------------------------------------------------------------------------------+
| receive callbacks               | array ``can_rxMessages`` in the file ``src/app/driver/config/can_cfg_rx.c``         |
+---------------------------------+-------------------------------------------------------------------------------------+

#. Verify that the CAN message is defined and implemented as expected:

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

.. warning::

   These script do no syntactical or similar checks of the implementation.
   These scripts do text based comparisons in order to help to get a consistent
   style for the CAN message implementations.
   **The correct implementation etc. must be checked by compiling and **
   **debugging.**

Example for a Cyclic Transmit Message
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. include:: ./can_how-to_tx_cyclic.rst

Example for an Asynchronous Transmit Message
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. include:: ./can_how-to_tx_async.rst

Example for a Receive Message
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. include:: ./can_how-to_rx.rst


Multi-string Support when using Isabellenhuette IVT Current Sensors
-------------------------------------------------------------------

In Multi-string systems multiple current sensors must be used in order to
measure the current in each string.
In case CAN-based Isabellenhuette IVT current sensors are used, the CAN message
IDs indicate which current sensor is in which string.

.. note::

   This of course means, that the current sensors must be correctly configured,
   i.e., that all current sensors in the system use unique CAN messages IDs.
   Furthermore, the current sensors must be placed in the correct string
   (accordingly to their CAN message IDs), as otherwise the current
   measurements would be assigned to the wrong strings.

The *string-selection* code in the function ``CANRX_CurrentSensor`` right at
the beginning of the function must be adapted as shown in
:numref:`current-sensor-string-selection`.

.. code-block:: c
   :linenos:
   :caption: String selection for the current sensor measurement in
             multi-string systems based on the CAN message ID
   :name: current-sensor-string-selection

    if (message.id <= CANRX_STRING0_ENERGY_COUNTER_ID) {
        stringNumber = 0u;
    } else if (message.id <= CANRX_STRING1_ENERGY_COUNTER_ID) {
        stringNumber = 1u;
    } else {
        FAS_ASSERT(FAS_TRAP);
    }


Further Reading
---------------

Implementation details of the CAN module are found in :ref:`CAN`.
