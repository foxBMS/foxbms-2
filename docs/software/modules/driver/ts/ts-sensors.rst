.. include:: ./../../../../macros.txt
.. include:: ./../../../../units.txt

.. _SUPPORTED_TEMPERATURE_SENSORS:

Supported Temperature Sensors
=============================

..
    Comments:
    Manufacturer names and their short names

.. spelling::
    Epcos
    Epc
    Fak
    Vishay
    Vis

|foxbms| supports various Temperature sensors from different manufacturers as
the list below shows.

These temperature sensors can be used inside the AFE drivers as they
implement the :ref:`temperature_sensor_api`.

.. toctree::
    :maxdepth: 1
    :caption: List of supported temperature sensors

    ./epcos/b57251v5103j060.rst
    ./epcos/b57861s0103f045.rst
    ./fake/none.rst
    ./vishay/ntcalug01a103g.rst
    ./vishay/ntcle317e4103sba.rst

These temperature sensors have *short name* in order to be able to write
shorter function and variable names. The *short names* are list in
:numref:`temperature-sensor-short-names`.

.. csv-table:: Temperature sensor short names
   :name: temperature-sensor-short-names
   :header-rows: 1
   :delim: ;
   :file: ./ts-short-names.csv
