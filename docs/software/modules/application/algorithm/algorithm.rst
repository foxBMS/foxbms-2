.. include:: ../../../../macros.txt
.. include:: ../../../../units.txt

.. _ALGORITHM_APPLICATION:

Algorithm
=========

Overview of included algorithms

.. toctree::
    :maxdepth: 1

    ./state-estimation/state-estimation.rst

The algorithm module provides the common execution framework for
application-level algorithms.
It is configuration-driven: individual algorithm functions are registered in
the ``algo_algorithms`` table and executed by the central scheduler logic.

Execution Model
---------------

- ``ALGO_MainFunction()`` is called periodically (tick: ``ALGO_TICK_ms``).
- Each configured algorithm is executed either:
  - cyclically when its cycle time elapses, or
  - as soon as possible if its cycle time is set to ``0``.
- Algorithms transition through defined states

Initialization and Reinitialization
-----------------------------------

- ``ALGO_UnlockInitialization()`` requests initialization handling.
- During initialization, each registered algorithm is checked and optional
  initialization callbacks are executed.
- Algorithms can request reinitialization via ``ALGO_MarkAsReinit()``;
  the framework then schedules a new initialization cycle.

Runtime Monitoring
------------------

``ALGO_MonitorExecutionTime()`` supervises algorithm execution duration.
If an algorithm exceeds its configured maximum calculation time while in
``ALGO_RUNNING``, it is set to ``ALGO_BLOCKED``.

Configuration Note
------------------

The set of algorithms executed at runtime is defined in
``src/app/application/algorithm/config/algorithm_cfg.c``.
The documentation pages linked above describe available algorithm modules,
while the active runtime set depends on the current configuration.
