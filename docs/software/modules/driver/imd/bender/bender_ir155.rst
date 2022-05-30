.. include:: ./../../../../../macros.txt
.. include:: ./../../../../../units.txt

.. _BENDER_IR155:

Bender IR155
============

Module Files
------------

Driver
^^^^^^

- ``src/app/driver/imd/bender/ir155/bender_ir155.c`` (`API <../../../../../_static/doxygen/src/html/bender__ir155_8c.html>`__, `source <../../../../../_static/doxygen/src/html/bender__ir155_8c_source.html>`__)
- ``src/app/driver/imd/bender/ir155/bender_ir155.h`` (`API <../../../../../_static/doxygen/src/html/bender__ir155_8h.html>`__, `source <../../../../../_static/doxygen/src/html/bender__ir155_8h_source.html>`__)
- ``src/app/driver/imd/bender/ir155/bender_ir155_helper.c`` (`API <../../../../../_static/doxygen/src/html/bender__ir155__helper_8c.html>`__, `source <../../../../../_static/doxygen/src/html/bender__ir155__helper_8c_source.html>`__)
- ``src/app/driver/imd/bender/ir155/bender_ir155_helper.h`` (`API <../../../../../_static/doxygen/src/html/bender__ir155__helper_8h.html>`__, `source <../../../../../_static/doxygen/src/html/bender__ir155__helper_8h_source.html>`__)

Configuration
^^^^^^^^^^^^^

- ``src/app/driver/imd/bender/ir155/config/bender_ir155_cfg.h`` (`API <../../../../../_static/doxygen/src/html/bender__ir155__cfg_8h.html>`__, `source <../../../../../_static/doxygen/src/html/bender__ir155__cfg_8h_source.html>`__)

Unit Test
^^^^^^^^^

- ``tests/unit/app/driver/imd/bender/ir155/test_bender_ir155.c`` (`API <../../../../../_static/doxygen/tests/html/test__bender__ir155_8c.html>`__, `source <../../../../../_static/doxygen/tests/html/test__bender__ir155_8c_source.html>`__)
- ``tests/unit/app/driver/imd/bender/ir155/test_bender_helper_ir155.c`` (`API <../../../../../_static/doxygen/tests/html/test__bender__ir155__helper_8c.html>`__, `source <../../../../../_static/doxygen/tests/html/test__bender__ir155__helper_8c_source.html>`__)


Description of the IMD state machine
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The Bender IR155 driver is interfaced and controlled by the superimposed
:ref:`IMD state machine<INSULATION_MEASUREMENT_DEVICE>`.
The required functionality, that needs to be provided by this driver is:

* Initialize the Bender IR155 device
* Activate the insulation measurement
* Deactivate the insulation measurement
* Measurement of the insulation resistance

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
