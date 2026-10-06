.. include:: ./../../../../macros.txt
.. include:: ./../../../../units.txt

.. _CONTACTOR:

Contactor
=========

Module Files
------------

Driver
^^^^^^

- ``src/app/driver/contactor/contactor.c``
- ``src/app/driver/contactor/contactor.h``

Configuration
^^^^^^^^^^^^^

- ``src/app/driver/config/contactor_cfg.c``
- ``src/app/driver/config/contactor_cfg.h``

A how-to for the configuration of the contactor module can be found in
:ref:`HOW_TO_USE_THE_CONTACTOR_DRIVER`.

Unit Test
^^^^^^^^^

- ``tests/unit/app/driver/contactor/test_contactor.c``
- ``tests/unit/app/driver/config/test_contactor_cfg.c``

Description
-----------

The contactor module provides a high-level interface to control configured
contactors and to evaluate their feedback state.

The runtime state of all contactors is stored in the central configuration
registry ``cont_contactorStates`` (configured in ``contactor_cfg.c``).
Each entry contains:

- requested/set state,
- feedback state,
- feedback type,
- string assignment,
- contactor type (plus, minus, precharge),
- SPS channel mapping,
- preferred current-breaking direction.

Switching Interface
^^^^^^^^^^^^^^^^^^^

The module offers per-string switching functions:

- ``CONT_CloseContactor()`` / ``CONT_OpenContactor()`` for explicit contactor
  types,
- ``CONT_ClosePrecharge()`` / ``CONT_OpenPrecharge()`` for precharge handling,
- ``CONT_OpenAllPrechargeContactors()`` and ``CONT_OpenAllContactors()`` for
  global safe-state opening.

Requests are forwarded to the SPS driver through
``SPS_RequestContactorState()`` while the set state in the registry is
updated accordingly.

Feedback Handling and Diagnostics
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

``CONT_CheckFeedback()`` updates feedback for all configured contactors and
checks set state versus feedback state.
Depending on the configured feedback type, feedback is derived from:

- no dedicated feedback (assume requested state),
- SPS current-based feedback,
- SPS digital feedback via normally-open pin logic,
- SPS digital feedback via normally-closed pin logic.

For each configured contactor type, corresponding DIAG events are reported
based on match/mismatch between set and feedback state.

Initialization
^^^^^^^^^^^^^^

``CONT_Initialize()`` validates configuration consistency, including SPS channel
range checks and channel affiliation checks for all registered contactors.
