.. include:: ./../macros.txt
.. include:: ./../units.txt

.. _NAMING_CONVENTIONS:

##################
Naming Conventions
##################

When working with battery systems, it is vital to use consistent wording.
The naming conventions used  throughout this documentation for the parts of a
battery system are illustrated in :numref:`battery-system-elements-naming`.

.. drawio-figure:: img/naming-convention.drawio
   :format: svg
   :alt: Naming conventions for battery system elements
   :name: battery-system-elements-naming
   :width: 100 %

   Naming conventions for battery system elements

- A **battery cell** or **cell** is the smallest unit in a battery system. It
  has one positive terminal and one negative terminal.
- Battery cells are usually connected in parallel to enhance the current
  capability of the system.
- **Battery cells** connected in parallel form a **(cell-)block**.
- **Cells** or **cell blocks** put in series form a **battery module**.
- **Several modules** connected in series form a **string**.
- **Several strings** connected in parallel form a **battery pack**.
- The highest unit is the **battery system**. It is formed by the conjunction
  of a **battery pack** and a **battery junction box (BJB)**. The BJB contains
  the elements used to control the current flow like power contactors and
  fuses.

The BMS consists of two main components,

- one **BMS-Master** and
- one or more **BMS-Slave**\ (s).

Generally, the |bms-master| is connected to the |bms-slaves| via an
communication interface.
The |bms-slaves| are wired to the battery cells of a module, so that the
|bms-slaves| are able to measure specific physical quantities on the module
(see :numref:`bms-components-naming`).

.. drawio-figure:: img/bms-components.drawio
   :format: svg
   :alt: Naming conventions for BMS components
   :name: bms-components-naming
   :width: 80 %

   Naming conventions for BMS components
