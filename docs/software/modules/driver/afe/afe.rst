.. include:: ./../../../../macros.txt
.. include:: ./../../../../units.txt

.. _ANALOG_FRONT_END_API:

Analog Front-End API
====================

|foxbms| supports various AFE from different manufacturers as
shown in :ref:`SUPPORTED_ANALOG_FRONT_ENDS`.
This is achieved by drivers that implement the
Analog Front-End API (AFE API).

This document describes how the AFE API works.

An example how to implement a new AFE API compatible driver is shown in
:ref:`HOW_TO_IMPLEMENT_AN_ANALOG_FRONT_END_DRIVER`.

Configuration
-------------

The AFE is configured through the |bms-config-file| described in
:ref:`BMS_APPLICATION_CONFIGURATION`.
As of now it is only possible to configure one AFE implementation.
The configuration is specified as e.g., in :numref:`afe-bms-json` for
the LTC6813-1.

.. code-block:: json
   :linenos:
   :emphasize-lines: 4,5
   :caption: snippet from |bms-config-file| specifying the AFE API
   :name: afe-bms-json

    {
        "bms-slave": {
            "analog-front-end": {
                "manufacturer": "ltc",
                "ic": "6813-1"
            }
        }
    }

The key ``manufacturer`` describes the AFE manufacturer.
The key ``ic`` describes the exact monitoring IC that implements the API.

Usage
-----

|tbc|

Internal implementation
-----------------------

|tbc|

.. toctree::
    :maxdepth: 1
    :caption: List of supported AFEs

    ./supported-afes.rst
