.. include:: ./../macros.txt
.. include:: ./../units.txt

.. _USE_CASE:

########
Use Case
########

As a first step to designing a BMS, it is necessary to define a use case.
While it sounds like an attractive goal to design a general-purpose BMS for
each and every application, this is not possible as each implementation has to
be adapted to its targeted application.

A simple example is that depending on the application, cells with high power or
cells with high energy will be used. These cells will have different
chemistries and behave differently.

Another example is a stationary battery storage used for peak shaving compared
to the BMS used to monitor the cells in a vehicle: in the first case, it is
easily possible to disconnect the battery in the case that a malfunction is
detected. In the second case, this will probably not be possible: if the
vehicle is driving at higher speeds in the traffic on a highway, it is not
possible to simply disconnect the battery in the case that a malfunction is
detected, because this would put the passengers in danger if the vehicle stops
in the middle of the high speed traffic.

In this documentation and in the application implemented by default in the
code, a stationary storage is considered. In case of error, the contactors
will open and disconnect the battery.

Three power contactors are used to connect and disconnect the battery modules
from the load:

 - Main Contactor Plus
 - Main Contactor Minus
 - Pre-charge contactor

There is no separate charge path, discharging and charging are made through
the main contactors.

*******
Example
*******

An example a battery system is shown in :numref:`example-battery-system`.

.. figure:: img/example-battery-system.png
   :alt: Example of a battery system
   :name: example-battery-system
   :width: 100 %

   Example of a battery system
