.. include:: ./../../../../macros.txt
.. include:: ./../../../../units.txt

.. _INTERLOCK:

Interlock
=========

Module Files
------------

Driver
^^^^^^

- ``src/app/driver/interlock/interlock.c`` (`API <../../../../_static/doxygen/src/html/interlock_8c.html>`__, `source <../../../../_static/doxygen/src/html/interlock_8c_source.html>`__)
- ``src/app/driver/interlock/interlock.h`` (`API <../../../../_static/doxygen/src/html/interlock_8h.html>`__, `source <../../../../_static/doxygen/src/html/interlock_8h_source.html>`__)

Configuration
^^^^^^^^^^^^^

- ``src/app/driver/config/interlock_cfg.c`` (`API <../../../../_static/doxygen/src/html/interlock__cfg_8c.html>`__, `source <../../../../_static/doxygen/src/html/interlock__cfg_8c_source.html>`__)
- ``src/app/driver/config/interlock_cfg.h`` (`API <../../../../_static/doxygen/src/html/interlock__cfg_8h.html>`__, `source <../../../../_static/doxygen/src/html/interlock__cfg_8h_source.html>`__)

Unit Test
^^^^^^^^^

- ``tests/unit/app/driver/interlock/test_interlock.c`` (`API <../../../../_static/doxygen/tests/html/test__interlock_8c.html>`__, `source <../../../../_static/doxygen/tests/html/test__interlock_8c_source.html>`__)

Description
-----------

.. graphviz:: interlock-schematic.dot
    :caption: Simplified schematic of the interlock circuit
    :name: interlock-circuit-simplified

The purpose of the interlock circuit is that the embedded software can
recognize that an external safety circuit has been opened.
This is achieved through a monitor current that is supplied to an external
connector.
The application expects that the rest of the safety system closes this path
when it is working as expected.
Typically, this is used with emergency stop switches or similar devices.
A simplified schematic of the interlock circuit is shown in
:numref:`interlock-circuit-simplified`.

Circuit description
^^^^^^^^^^^^^^^^^^^

The interlock circuit consists of a main path that is continuously supplied by
the static power supply of the |master|.
In :numref:`interlock-circuit-simplified` this is called
``static low-power supply``.
This supply does not have to be enabled and is powered by the 5V rail of the
peripheral supply.

The interlock is closed through the connector that makes the external
connection of ``interlock connector high`` with ``interlock connector low``.

As a diagnostic measure, the ``low-side current sense`` allows to monitor the
current on the low-side of the interlock connector.
The monitoring circuit scales 0mA to 100mA from 0V to 4V with a linear relation.

For the case that additional diagnostic features are enabled, the
``low-side current regulation`` limits the low-side current to 50mA.

As a last point in the measurement chain, a ``low-side current threshold``
comparator circuit, pulls a pin low in the case that the threshold of 10mA
interlock current is surpassed.
This is the main detection method of detecting a closed interlock.

Additional diagnostic features consist of the voltage measurements of the high
and low pins of the external interlock connector.
Moreover, dedicated current measurements at the low- and high-side of the
interlock connector increase the diagnostic possibilities.

The ``high-side voltage sense`` and the ``low-side voltage sense`` allow the
measurement of the voltages of the respective interlock connector pins.
Together with the low-side current sense circuit this allows an estimation of
the resistance of the external part of the interlock.
All voltages are measured using a resistor divider consisting of a 5k6 ohm and
a 3k9 ohm resistor.

An additional switchable power supply imprints a higher current on the
interlock path to increase the robustness of the interlock. This current can
be measured using the ``high-side current sense`` measurement.

The measurements of voltage and current on both high-side and low-side can be
used for checking the plausibility of measured values.

Software description
^^^^^^^^^^^^^^^^^^^^

The interlock state is read via a feedback input pin. If this pin is low,
then the interlock is closed. If the pin is high, the interlock is
open.

The feedback is checked in the interlock state machine. It reports an error
via the ``DIAG`` module if the interlock is open.

The interlock feedback can be deactivated via the define
``BS_IGNORE_INTERLOCK_FEEDBACK``. If set to ``true``, the feedback function
will always return ``ILCK_SWITCH_ON``.

In addition to the feedback pins, the ADC inputs of the MCU also monitor the
voltages and currents described in the section `Circuit description`_.
Currently, these readings are only stored in the database
(:numref:`DATABASE_MODULE`).
They can be used for further diagnostics.
