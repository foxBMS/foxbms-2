# Variant Build Configuration

The naming schema for simple build variant configuration files is:

1. Operating System
1. Analog Front-End
   1. Manufacturer
   1. Chip
1. Balancing strategy
1. SoX algorithms:
   1. SoC algorithm
   1. SoE algorithm
   1. SoF algorithm
   1. SoH algorithm
1. Insulation Measurement Device

Therefore for a **FreeRTOS** based build, using an **LTC 6813-1**
AFE, with **voltage-based balancing**, with **coulomb counting** as
basis for **SoC and SoE**, with **trapezoid** for **SoF**, with
**no SoH** estimation, without IMD, the bms configuration file
must be named:

``freertos_ltc-6813-1_vbb_cc-cc-tr-none_none-no-imd.json``

For more complex build variants a directory must be created the all required
files must be placed in that directory. See the library build in ``lib-build``
to see how it can be done.
