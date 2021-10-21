.. include:: ./../../../../macros.txt
.. include:: ./../../../../units.txt

.. _PWM:

PWM
===

Module Files
------------

Driver
^^^^^^

- ``src/app/driver/pwm/pwm.c`` (`API <../../../../_static/doxygen/src/html/pwm_8c.html>`__, `source <../../../../_static/doxygen/src/html/pwm_8c_source.html>`__)
- ``src/app/driver/pwm/pwm.h`` (`API <../../../../_static/doxygen/src/html/pwm_8h.html>`__, `source <../../../../_static/doxygen/src/html/pwm_8h_source.html>`__)

Configuration
^^^^^^^^^^^^^

*none*

Unit Test
^^^^^^^^^

- ``tests/unit/app/driver/test_pwm.c`` (`API <../../../../_static/doxygen/tests/html/test__pwm_8c.html>`__, `source <../../../../_static/doxygen/tests/html/test__pwm_8c_source.html>`__)

Description
-----------

The PWM module uses the enhanced PWM module in the MCU in order to generate
a PWM clock. Currently, only the PWM channel 1A is used and controlled.
For usage details, please refer to the doxygen documentation of this module.
