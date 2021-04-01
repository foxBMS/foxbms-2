# Variant Build Configuration

The naming schema for simple build variant configuration files is:

1. operating system
1. measurement IC
   1. manufacturer
   1. chip
1. balancing strategy
1. SoX algorithms:
   1. SoC algorithm
   1. SoE algorithm
   1. SoH algorithm
1. Insulation Measurement Device

Therefore for a an **FreeRTOS** based build, using and **LTC 6813-1**
measurement IC with **voltage-based balancing** and **coulomb counting** as
basis for **SoC and SoE** but **no SoH** estimation and IMD, the bms
configuration file must be named:

``freertos_ltc-6813-1_vbb_cc-cc-none_none-no-imd.json``

For more complex build variants a directory must be created the all required
files must be placed in that directory. See the library build in ``lib-build``
to see how it can be done.
