.. include:: ../../../../macros.txt
.. include:: ../../../../units.txt

.. _BALANCING_MODULE:

Balancing Module
================

Module Files
------------

Driver
^^^^^^

- ``src/app/application/bms/history/bal_strategy_history.c``
- ``src/app/application/bms/none/bal_strategy_none.c``
- ``src/app/application/bms/voltage/bal_strategy_voltage.c``
- ``src/app/application/bms/voltage/bal_strategy_voltage.h``
- ``src/app/application/bms/bal.c``
- ``src/app/application/bms/bal.h``

Configuration
^^^^^^^^^^^^^

- ``src/app/application/config/bal_cfg.h``

Unit Test
^^^^^^^^^
- ``tests/unit/app/application/bal/history/test_bal_strategy_history.c``
- ``tests/unit/app/application/bal/none/test_bal_strategy_none.c``
- ``tests/unit/app/application/bal/voltage/test_bal_strategy_voltage.c``
- ``tests/unit/app/application/bal/test_bal.c``

Detailed Description
--------------------

.. _BALANCING_MODULE_VOLTAGE_BASED_BALANCING:

Voltage-based Balancing
^^^^^^^^^^^^^^^^^^^^^^^

In voltage-based balancing, the balancing module takes the minimum battery cell voltage of the complete battery pack
and activates balancing for all the cells whose voltage is above the minimum + ``BAL_GetBalancingThreshold_mV()``. Once all cells
have been balanced, the threshold is set to ``BAL_GetBalancingThreshold_mV()`` + ``BAL_HYSTERESIS_mV`` to avoid an oscillating
behavior between balancing and not balancing.

.. _BALANCING_MODULE_HISTORY_BASED_BALANCING:

History-based Balancing
^^^^^^^^^^^^^^^^^^^^^^^

The SOC history-based balancing works as follows: at one point in time, when no current is flowing and the cell
voltages have fully relaxed (e.g., after 3 hours rest time), the voltages of all cells are measured. With a suitable
SOC versus voltage look-up table, the voltages are converted to their respective SOC. These SOC are then translated to
Depth-of-Discharge (DOD) using the nominal capacity, with:

.. math::

   DOD = Capacity \times (1-SOC)

The cell with the highest DOD is taken as a reference, since it is the most discharged cell in the battery pack.
Its charge difference is set to 0. For all other cells, the charge difference is computed via:

.. math::

   Charge_{\mathrm{difference}}(C_{\mathrm{c}}) =  DOD(C_\mathrm{r}) - DOD(C_{\mathrm{c}})

where :math:`C_{\mathrm{c}}` is the considered cell and :math:`C_{\mathrm{r}}` is the reference cell.

Balancing is then switched on for all cells. Every second, for each cell, the voltage is taken and the balancing
current :math:`I_{\mathrm{balancing}}` computed by

.. math::

   I_{\mathrm{balancing}} = U_{\mathrm{C}} / R_{\mathrm{balancing}}

where :math:`U_{\mathrm{C}}` is the cell voltage and :math:`R_{\mathrm{balancing}}` is the balancing resistance.

The balancing quantity :math:`I_{\mathrm{balancing}} \times 1s` is subtracted from the charge difference. Balancing is
stays turned on until the charge difference reaches 0.

In SOC history-based balancing, ``SLV_BALANCING_RESISTANCE_ohm`` must be defined identically to the balancing
resistances soldered on the Slave Board. When the imbalances are computed, they are set to a non-zero value to balance
each specific cell only if its cell voltage is above the minimum cell voltage of the battery pack plus a threshold. The
threshold is set in this case to ``BAL_GetBalancingThreshold_mV()`` + ``BAL_HYSTERESIS_mV``.

The correspondence between cell voltage and SOC must be defined by the user depending on the specific battery cells
used.
Currently, it is done in the function ``SE_GetStateOfChargeFromVoltage()`` in
``soc_counting.c``, ``soc_debug.c``, ``soc_none.c``, or ``state_estimation.h``.
This function gets a voltage in V and return an SOC between 0.0 and 1.0.

.. _BALANCING_MODULE_NO_BALANCING:

No Balancing
^^^^^^^^^^^^

No balancing is performed on any cell.
