.. include:: ../../../../macros.txt
.. include:: ../../../../units.txt

.. _DATABASE_MODULE:

Database Module
===============

Module Files
------------

Driver
^^^^^^

- ``src/app/engine/database/database.c``
- ``src/app/engine/database/database.h``

Configuration
^^^^^^^^^^^^^

- ``src/app/engine/config/database_cfg.c``
- ``src/app/engine/config/database_cfg.h``

Unit Test
^^^^^^^^^

- ``tests/unit/app/engine/config/test_database_cfg.c``
- ``tests/unit/app/engine/database/test_database.c``

Detailed Description
--------------------

The data-exchange module, sometimes also called database ensures a reliable
and safe asynchronous data exchange between different tasks and/or software
modules.
It is implemented based on a producer/consumer pattern.
The exchanged data is always produced by a single data producer and then
stored in the data-exchange module.
Afterwards, it can be used by multiple consumers while the data integrity is
always ensured.
The concept is shown in :numref:`sw-database-concept`.

.. drawio-figure:: img/sw-database-concept.drawio
   :format: svg
   :alt: Database Concept
   :name: sw-database-concept
   :width: 800px
   :align: center

   |foxbms| Data-Exchange Module Concept

.. note::
   It has to be ensured by SW architecture that every data entry in the
   Data-Exchange Module is only written by a single data producer! Data
   integrity can **NOT** be ensured if multiple data producers write into the
   same data entry.

Further Reading
---------------

A How-to is found in :ref:`HOW_TO_USE_THE_DATABASE_MODULE`.
