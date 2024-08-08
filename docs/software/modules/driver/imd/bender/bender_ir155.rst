.. include:: ./../../../../../macros.txt
.. include:: ./../../../../../units.txt

.. _BENDER_IR155:

Bender IR155
============

Module Files
------------

Driver
^^^^^^

- ``src/app/driver/imd/bender/ir155/bender_ir155.c``
- ``src/app/driver/imd/bender/ir155/bender_ir155.h``
- ``src/app/driver/imd/bender/ir155/bender_ir155_helper.c``
- ``src/app/driver/imd/bender/ir155/bender_ir155_helper.h``

Configuration
^^^^^^^^^^^^^

- ``src/app/driver/imd/bender/ir155/config/bender_ir155_cfg.h``

Unit Test
^^^^^^^^^

- ``tests/unit/app/driver/imd/bender/ir155/test_bender_ir155.c``
- ``tests/unit/app/driver/imd/bender/ir155/test_bender_helper_ir155.c``

Description of the IMD state machine
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The Bender IR155 driver is interfaced and controlled by the superimposed
:ref:`IMD state machine<INSULATION_MEASUREMENT_DEVICE>`.
The required functionality, that needs to be provided by this driver is:

- Initialize the Bender IR155 device
- Activate the insulation measurement
- Deactivate the insulation measurement
- Measurement of the insulation resistance

Each functionality is implemented in a dedicated state machine that is
explained in detail in the following.

Initialization procedure
""""""""""""""""""""""""

|tbc|

The initialization process is implemented in function `IR155_InitializeModule`.

Activation of insulation measurement
""""""""""""""""""""""""""""""""""""

The Bender IR155 device is activated via a MOSFET setting supply enable pin
(`IMD_SUP_EN`) to high.

Disable insulation measurement
""""""""""""""""""""""""""""""

The Bender IR155 device is deactivated via a MOSFET setting supply enable pin
(`IMD_SUP_EN`) to low.

Insulation measurement
""""""""""""""""""""""

|tbc|

The measurement process is implemented in function `IR155_MeasureInsulation`.
