.. include:: ./../../../../macros.txt
.. include:: ./../../../../units.txt

.. _PWM:

PWM
===

Module Files
------------

Driver
^^^^^^

- ``src/app/driver/pwm/pwm.c``
- ``src/app/driver/pwm/pwm.h``

Configuration
^^^^^^^^^^^^^

*none*

Unit Test
^^^^^^^^^

- ``tests/unit/app/driver/test_pwm.c``

Description
-----------

The PWM module uses the enhanced PWM module in the MCU in order to generate
a PWM clock. Currently, only the PWM channel 1A is used and controlled.
For usage details, please refer to the doxygen documentation of this module.
