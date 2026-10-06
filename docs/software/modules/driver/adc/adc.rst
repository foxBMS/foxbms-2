.. include:: ./../../../../macros.txt
.. include:: ./../../../../units.txt

.. _ADC_DRIVER:

ADC
===

Module Files
------------

Driver
^^^^^^

- ``src/app/driver/adc/adc.c``
- ``src/app/driver/adc/adc.h``

Configuration
^^^^^^^^^^^^^

*none*

Unit Test
^^^^^^^^^

- ``tests/unit/app/driver/adc/test_adc.c``

Description
-----------

The |adc| module controls periodic conversion of MCU ADC1 group-1 channels and
provides converted voltages in millivolt through the database.

The driver is implemented as a small state machine in ``ADC_Control()``:

- ``ADC_START_CONVERSION``:
  starts a new ADC conversion sequence using ``adcStartConversion()``.
- ``ADC_WAIT_CONVERSION_FINISHED``:
  polls conversion completion via ``adcIsConversionComplete()``.
- ``ADC_CONVERSION_FINISHED``:
  reads raw ADC samples with ``adcGetData()``, converts each channel to mV and
  writes the result to ``DATA_BLOCK_ADC_VOLTAGE``.

Voltage Conversion
^^^^^^^^^^^^^^^^^^

Raw |adc| counts are converted with ``ADC_ConvertVoltage()`` using the
configured reference range and 12-bit conversion factors.

The converted values are stored in
``adc1ConvertedVoltages_mV[MCU_ADC1_MAX_NR_CHANNELS]`` and published to the
database with ``DATA_WRITE_DATA()``.
