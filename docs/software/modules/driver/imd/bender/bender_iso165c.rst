.. include:: ./../../../../../macros.txt
.. include:: ./../../../../../units.txt

.. _BENDER_ISO165C:

Bender iso165c
==============

Module Files
------------

Driver
^^^^^^

- ``src/app/driver/imd/bender/iso165c/bender_iso165c.c``
- ``src/app/driver/imd/bender/iso165c/bender_iso165c.h``

Configuration
^^^^^^^^^^^^^

- ``src/app/driver/imd/bender/iso165c/bender_iso165c.c``
- ``src/app/driver/imd/bender/iso165c/bender_iso165c.h``

Unit Test
^^^^^^^^^

- ``tests/unit/app/driver/imd/bender/iso165c/test_bender_iso165c.c``

Description
-----------

Communication
^^^^^^^^^^^^^

The Bender iso165C is connected to the BMS via CAN-bus, the handling
of the messages is implemented in the can callback files.

Description of the IMD state machine
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The Bender iso165C driver is interfaced and controlled by the superimposed
:ref:`IMD state machine<INSULATION_MEASUREMENT_DEVICE>`.
The required functionality, that needs to be provided by this driver is:

- Initialize the Bender iso165C device
- Activate the insulation measurement
- Deactivate the insulation measurement
- Measurement of the insulation resistance

Each functionality is implemented in a dedicated state machine that is
explained in detail in the following.

Initialization procedure
""""""""""""""""""""""""

The state flow diagram of the initialization procedure of the Bender iso165C
state machine is depicted below.

.. drawio-figure:: state-diagrams/iso165c_state_diagram.drawio
   :page-name: initialization
   :format: svg
   :alt: iso165C initialization state flow diagram
   :name: iso165c-init-state-diagram
   :width: 720px

   iso165C initialization state flow diagram

The initialization process is implemented in function `I165C_Initialize`.

Activation of insulation measurement
""""""""""""""""""""""""""""""""""""

The state flow diagram of the enabling procedure of the Bender iso165C
state machine is depicted below.

.. drawio-figure:: state-diagrams/iso165c_state_diagram.drawio
   :page-name: enable
   :format: svg
   :alt: iso165C initialization state flow diagram
   :name: iso165c-enable-state-diagram
   :width: 240px

   iso165C initialization state flow diagram

The enabling process is implemented in function `I165C_Enable`.

Disable insulation measurement
""""""""""""""""""""""""""""""

The state flow diagram of the disabling procedure of the Bender iso165C
state machine is depicted below.

.. drawio-figure:: state-diagrams/iso165c_state_diagram.drawio
   :page-name: disable
   :format: svg
   :alt: iso165C disable state flow diagram
   :name: iso165c-disable-state-diagram
   :width: 240px

   iso165C disable state flow diagram

The disabling process is implemented in function `I165C_Disable`.

Insulation measurement
""""""""""""""""""""""

The state flow diagram of the periodic insulation measurement procedure of the
Bender iso165C state machine is depicted below.

.. drawio-figure:: state-diagrams/iso165c_state_diagram.drawio
   :page-name: running
   :format: svg
   :alt: iso165C running state flow diagram
   :name: iso165c-running-state-diagram
   :width: 240px

   iso165C running state flow diagram

The measurement process is implemented in function `I165C_Running`.
