.. include:: ./../../../../../macros.txt
.. include:: ./../../../../../units.txt

.. _BENDER_ISO165C:

Bender iso165c
==============

Module Files
------------

Driver
^^^^^^

- ``src/app/driver/imd/bender/iso165c/bender_iso165c.c`` (`API <../../../../../_static/doxygen/src/html/bender__iso165c_8c.html>`__, `source <../../../../../_static/doxygen/src/html/bender__iso165c_8c_source.html>`__)
- ``src/app/driver/imd/bender/iso165c/bender_iso165c.h`` (`API <../../../../../_static/doxygen/src/html/bender__iso165c_8h.html>`__, `source <../../../../../_static/doxygen/src/html/bender__iso165c_8h_source.html>`__)

Configuration
^^^^^^^^^^^^^

- ``src/app/driver/imd/bender/iso165c/bender_iso165c.c`` (`API <../../../../../_static/doxygen/src/html/bender__iso165c__cfg_8c.html>`__, `source <../../../../../_static/doxygen/src/html/bender__iso165c_8c_source.html>`__)
- ``src/app/driver/imd/bender/iso165c/bender_iso165c.h`` (`API <../../../../../_static/doxygen/src/html/bender__iso165c__cfg_8h.html>`__, `source <../../../../../_static/doxygen/src/html/bender__iso165c_8h_source.html>`__)

Unit Test
^^^^^^^^^

- ``tests/unit/app/driver/imd/bender/iso165c/test_bender_iso165c.c`` (`API <../../../../../_static/doxygen/tests/html/test__bender__iso165c_8c.html>`__, `source <../../../../../_static/doxygen/tests/html/test__bender__iso165c_8c_source.html>`__)

Description
-----------

Description of the IMD state machine
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The Bender iso165C driver is interfaced and controlled by the superimposed
:ref:`IMD state machine<INSULATION_MEASUREMENT_DEVICE>`.
The required functionality, that needs to be provided by this driver is:

* Initialize the Bender iso165C device
* Activate the insulation measurement
* Deactivate the insulation measurement
* Measurement of the insulation resistance

Each functionality is implemented in a dedicated state machine that is
explained in detail in the following.

Initialization procedure
""""""""""""""""""""""""

The state flow diagram of the initialization procedure of the Bender iso165C
state machine is depicted below.

.. graphviz:: state-diagrams/iso165c_state_diagram_initialization.dot
    :caption: iso165C initialization state flow diagram
    :name: iso165c-init-state-diagram

The initialization process is implemented in function `I165C_Initialize`.

Activation of insulation measurement
""""""""""""""""""""""""""""""""""""

The state flow diagram of the enabling procedure of the Bender iso165C
state machine is depicted below.

.. graphviz:: state-diagrams/iso165c_state_diagram_enable.dot
    :caption: iso165C enable state flow diagram
    :name: iso165c-enable-state-diagram

The enabling process is implemented in function `I165C_Enable`.

Disable insulation measurement
""""""""""""""""""""""""""""""

The state flow diagram of the disabling procedure of the Bender iso165C
state machine is depicted below.

.. graphviz:: state-diagrams/iso165c_state_diagram_disable.dot
    :caption: iso165C disable state flow diagram
    :name: iso165c-disable-state-diagram

The disabling process is implemented in function `I165C_Disable`.

Insulation measurement
""""""""""""""""""""""


The state flow diagram of the periodic insulation measurement procedure of the
Bender iso165C state machine is depicted below.

.. graphviz:: state-diagrams/iso165c_state_diagram_running.dot
    :caption: iso165C running state flow diagram
    :name: iso165c-running-state-diagram

The measurement process is implemented in function `I165C_Running`.
