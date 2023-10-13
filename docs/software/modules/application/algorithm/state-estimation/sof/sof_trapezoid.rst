.. include:: ./../../../../../../macros.txt
.. include:: ./../../../../../../units.txt

.. _SOF__TRAPEZOID:

SOF: Trapezoid
==============

Module Files
------------

Driver
^^^^^^

- ``src/app/application/algorithm/state_estimation/sof/trapezoid/sof_trapezoid.c`` (`API <./../../../../../../_static/doxygen/src/html/sof_8c.html>`__, `source <./../../../../../../_static/doxygen/src/html/sof__trapezoid_8c_source.html>`__)
- ``src/app/application/algorithm/state_estimation/sof/trapezoid/sof_trapezoid.h`` (`API <./../../../../../../_static/doxygen/src/html/sof_8h.html>`__, `source <./../../../../../../_static/doxygen/src/html/sof__trapezoid_8h_source.html>`__)

Configuration
^^^^^^^^^^^^^

- ``src/app/application/algorithm/state_estimation/sof/trapezoid/sof_trapezoid_cfg.c`` (`API <./../../../../../../_static/doxygen/src/html/sof__cfg_8c.html>`__, `source <./../../../../../../_static/doxygen/src/html/sof__trapezoid__cfg_8c_source.html>`__)
- ``src/app/application/algorithm/state_estimation/sof/trapezoid/sof_trapezoid_cfg.h`` (`API <./../../../../../../_static/doxygen/src/html/sof__cfg_8h.html>`__, `source <./../../../../../../_static/doxygen/src/html/sof__trapezoid__cfg_8h_source.html>`__)

Unit Test
^^^^^^^^^

- ``tests/unit/app/application/algorithm/state_estimation/sof/trapezoid/test_sof_trapezoid.c`` (`API <./../../../../../../_static/doxygen/tests/html/test__sof_8c.html>`__, `source <./../../../../../../_static/doxygen/tests/html/test__sof__trapezoid_8c_source.html>`__)
- ``tests/unit/app/application/algorithm/state_estimation/sof/trapezoid/test_sof_trapezoid_cfg.c`` (`API <./../../../../../../_static/doxygen/tests/html/test__sof__cfg_8c.html>`__, `source <./../../../../../../_static/doxygen/tests/html/test__sof__trapezoid__cfg_8c_source.html>`__)

Detailed Description
--------------------

The state of function estimation (SOF) calculates current derating values to
operate the battery system within the safe operating area. These charge and
discharge derating values are calculated according to battery cell specific
and battery system related constraints.

For this, the operating limits for the cell voltage and cell temperature are
taken into account. Two derating points are defined for each of these limits.
A **cut-off** value where the derating begins and the allowed current starts to
be decreased and a **limit** value where no current flow is allowed anymore.
The allowed current is linearly decreased between these two points.

Additionally, the electrical string current capability are consider and the
allowed current is limited below this value. This value needs to be configured
according to the design of the battery system. Important factors for this are
for example the selected contactors and connectors or the busbar/wire cross
section.
