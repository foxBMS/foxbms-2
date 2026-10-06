.. include:: ./../macros.txt
.. include:: ./../units.txt

.. _SYSTEM_INTRODUCTION:

###################
System Introduction
###################

Conditions that lead to a transition to *ERROR* state
=====================================================

Logging after 20th event for errors connected related to the contactor
feedback.
This value is chosen to be so large because of the time delay between the
request for a state and the actual physical response.
It is caused by the SPI transaction to the SPS module, the rise time of the
control signal and the actual opening/closing of the contactor.
Only then can the feedback be read correctly, which also takes some additional
time depending on the selected feedback source.

For each diagnosis entry listed below, the **severity**, **sensitivity**, and
**delay** can be configured individually.

The **severity** defines the criticality level of the entry (Fatal Error,
Warning, or Info).
The **sensitivity** defines the number of fault events that must occur
before the diagnosis entry is confirmed and logged.
The **delay** configures the time before the transition to the *ERROR* state
occurs after a fault has been confirmed.
The delay is not relevant if the severity is set to Fatal Error.

.. csv-table:: Diagnosis entries
   :file: ../../build/docs/docs/system/diag_array_cfg.csv
   :header-rows: 1
   :delim: ;
   :name: diagnosis-entries
