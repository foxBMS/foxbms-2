.. include:: ./../../../../../../macros.txt
.. include:: ./../../../../../../units.txt

.. _SOC__COULOMB_COUNTING:

SOC: Coulomb Counting
=====================

Module Files
------------

Driver
^^^^^^

- ``src/app/application/algorithm/state_estimation/soc/counting/soc_counting.c`` (`API <./../../../../../../_static/doxygen/src/html/soc__counting_8c.html>`__, `source <./../../../../../../_static/doxygen/src/html/soc__counting_8c_source.html>`__)
- ``src/app/application/algorithm/state_estimation/soc/counting/soc_counting.h`` (`API <./../../../../../../_static/doxygen/src/html/soc__counting_8h.html>`__, `source <./../../../../../../_static/doxygen/src/html/soc__counting_8h_source.html>`__)

Configuration
^^^^^^^^^^^^^

- ``src/app/application/algorithm/state_estimation/soc/counting/soc_counting_cfg.c`` (`API <./../../../../../../_static/doxygen/src/html/soc__counting__cfg_8c.html>`__, `source <./../../../../../../_static/doxygen/src/html/soc__counting__cfg_8c_source.html>`__)
- ``src/app/application/algorithm/state_estimation/soc/counting/soc_counting_cfg.h`` (`API <./../../../../../../_static/doxygen/src/html/soc__counting__cfg_8h.html>`__, `source <./../../../../../../_static/doxygen/src/html/soc__counting__cfg_8h_source.html>`__)

Unit Test
^^^^^^^^^

- ``tests/unit/app/application/algorithm/state_estimation/soc/counting/test_soc_counting.c`` (`API <./../../../../../../_static/doxygen/tests/html/test__soc__counting_8c.html>`__, `source <./../../../../../../_static/doxygen/tests/html/test__soc__counting_8c_source.html>`__)
- ``tests/unit/app/application/algorithm/state_estimation/soc/counting/test_soc_counting_cfg.c`` (`API <./../../../../../../_static/doxygen/tests/html/test__soc__counting__cfg_8c.html>`__, `source <./../../../../../../_static/doxygen/tests/html/test__soc__counting__cfg_8c_source.html>`__)

Detailed Description
--------------------

By default, |foxbms| uses the internal integrator of the current sensor.
If the current sensor integrator is not available, the integration is made
directly in the |foxbms| software.

The check of the presence of the integrator and the software integration are
made in the :ref:`cc-function-name`.


.. literalinclude:: ./../../../../../../../src/app/application/algorithm/state_estimation/soc/counting/soc_counting.c
   :language: C
   :linenos:
   :start-after: /* INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE cc-documentation-start-include */
   :end-before: /* INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE cc-documentation-stop-include */
   :caption: Function implementing Coulomb-counting
   :name: cc-function-name
