# TI AFE Support

The foxBMS project supports the following TI AFEs

- [Dummy](#dummy) (API reference implementation; **non functional**)

This document is just there to provide a short overview.
For detailed information please see the documentation.

## Common

- AFE DMA API implementation
  [``src/app/driver/afe/api/afe_dma.h``](../api/afe_dma.h):

  Common implementation for all TI AFEs.
  Defined at [``src/app/driver/afe/ti/common/api/ti_bq79xxx_afe_dma.c``](common/api/ti_bq79xxx_afe_dma.c)

- AFE API implementation
  [``src/app/driver/afe/api/afe.h``](../api/afe.h):

  Implementation of the **AFE** to **TI** wrapper:

  - [src/app/driver/afe/ti/api/ti_afe.c](api/ti_afe.c):
    Defines the the ``AFE_*`` functions and wraps them to ``TI_*`` functions.
  - [src/app/driver/afe/ti/api/ti_afe.h](api/ti_afe.h): Declares the ``TI_*``
    functions

## Dummy

- AFE DMA API implementation:

  Implementation not required as there is a generic one (see
  [Common](#common)).

- TI AFE API implementation
  [``src/app/driver/afe/ti/api/ti_afe.h``](api/ti_afe.h):

  Implementation of the **TI** to **TIDUM** wrapper:

  - [src/app/driver/afe/ti/dummy/api/ti_dummy_afe.c](dummy/api/ti_dummy_afe.c):
    Defines the ``TI_*`` functions and wraps them to ``TIDUM_*`` functions.
  - [src/app/driver/afe/ti/dummy/ti_dummy.h](dummy/ti_dummy.h):
    Declares the ``TIDUM_*`` functions.
  - [src/app/driver/afe/ti/dummy/ti_dummy.c](dummy/ti_dummy.c):
    Defines the ``TIDUM_*`` functions.
