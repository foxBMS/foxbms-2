.. include:: ../../../../macros.txt
.. include:: ../../../../units.txt

.. _HOW_TO_IMPLEMENT_AN_ANALOG_FRONT_END_DRIVER:

How to Implement an Analog Front-End Driver
===========================================

This section gives a guideline how add a new AFE to the project.

The Example
-----------

The AFE is from the manufacturer called *Super BMS ICs (SB-IC)* and
the the IC is named *Vulpes G 2000 (VG2000)* from the IC family *VG*.

Basic Directory Structure
-------------------------

#. Adapt ``conf/bms/schema/bms.schema.json`` to recognize the new manufacturer
   and the IC.
#. Add the *SB-IC* manufacturer directory ``sb-ic``:
   ``src/app/driver/afe/sb-ic``.
#. Add the API directory ``api`` for *SB-IC* manufacturer:
   ``src/app/driver/afe/sb-ic/api``.
#. Add the IC ``vg2`` directory:
   ``src/app/driver/afe/sb-ic/vg2000``.
#. Add the ``api`` directory:
   ``src/app/driver/afe/sb-ic/vg2000/api``.
#. Add the ``config`` directory:
   ``src/app/driver/afe/sb-ic/vg2000/config``.
#. If there is special code code **from the IC vendor** for that IC: Add the
   ``vendor`` directory ``src/app/driver/afe/sb-ic/vg2000/vendor``.
   The vendor code should be placed here verbatim without any changes to the
   source code or to the source code organization.
#. If there is shared code between all ICs or family of ICs from that
   manufacturer, add the ``common`` directory:
   ``src/app/driver/afe/sb-ic/common``.
#. If there is common shared code **from the IC vendor**: Add the ``vendor``
   directory ``src/app/driver/afe/sb-ic/common/vendor``. The vendor
   code should be placed here verbatim without any changes to the source code
   or to the source code organization.
#. If there needs to be common shared code **developed** for the family: Add
   the ``vg`` directory ``src/app/driver/afe/sb-ic/common/vg``.

After that the directories are setup correctly.

Example: If there is

- common code from the vendor,
- common code for the family from the vendor,
- special code for the IC from the vendor,
- common code developed,
- common code for the family developed and
- special code for the IC developed

then the directory structure is as follows:

.. code-block::

   src
   └── app
       └── src
           └── driver
               └── afe
                   └── sb-ic
                       ├── api
                       ├── common
                       │   ├── vendor  # vendor: common shared vendor code without modification
                       │   ├── vg      # developed: family common shared code
                       │   └── .       # developed: manufacturer common shared code
                       └── vg2000
                           ├── api     # api files go here
                           ├── config  # configuration files go here
                           ├── vendor  # vendor: special code for the IC vendor goes here
                           └── .       # developed: driver code developed

Required Files
--------------

The files listed here are the required minimum to keep the different
manufacturer driver consistent.

Driver
++++++

- ``sb-ic_vg2000.c``
- ``sb-ic_vg2000.h``
- ``sb-ic_vg2000.json``
- ``wscript``
- Additional files **MAY** be added as needed.

API Files
+++++++++

- ``api/sb-ic_vg2000_afe.h``
- ``api/sb-ic_vg2000_afe.c``
- ``api/sb-ic_vg2000_afe_dma.c``
- ``api/sb-ic_vg2000_afe_dma.h``
- There **SHOULD** be no additional files.

Configuration Files
+++++++++++++++++++

- ``sb-ic_vg2000_cfg.c``
- ``sb-ic_vg2000_cfg.h``
- ``sb-ic_vg2000_afe_dma_cfg.c``
- ``sb-ic_vg2000_afe_dma_cfg.h``
- There **SHOULD** be no additional files.

The file structure should look like this (for the sake of simplicity no common
shared code and no vendor code):

.. code-block::

   src
   └── app
       └── src
           └── driver
               └── afe
                   └── sb-ic
                       └── vg2000
                           ├── api
                           │   ├──sb-ic_vg2000_afe.h
                           │   ├──sb-ic_vg2000_afe.c
                           │   ├──sb-ic_vg2000_afe_dma.c
                           │   └──sb-ic_vg2000_afe_dma.h
                           ├── config
                           │   ├──sb-ic_vg2000_cfg.c
                           │   ├──sb-ic_vg2000_cfg.h
                           │   ├──sb-ic_vg2000_afe_dma_cfg.c
                           │   └──sb-ic_vg2000_afe_dma_cfg.h
                           ├── sb-ic_vg2000.c
                           ├── sb-ic_vg2000.h
                           ├── sb-ic_vg2000.json
                           └── wscript
