.. include:: ./../macros.txt
.. include:: ./../units.txt

.. _SYSTEM_INTRODUCTION:

###################
System Introduction
###################

Conditions that lead to a transition to `ERROR` state
=====================================================

Logging after 20th event for errors connected related to the contactor
feedback.
This value is chosen to be so large because of the time delay between the
request for a state and the actual physical response.
It is caused by the SPI transaction to the SPS module, the rise time of the
control signal and the actual opening/closing of the contactor.
Only then can the feedback be read correctly, which also takes some additional
time depending on the selected feedback source.

.. csv-table:: Diagnosis entries with severity: ERROR
   :file: ./system-error-table.csv
   :header-rows: 1
   :delim: ;
   :name: diagnosis-entries-errors
