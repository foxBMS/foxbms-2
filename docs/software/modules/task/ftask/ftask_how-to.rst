.. include:: ../../../../macros.txt
.. include:: ../../../../units.txt

.. _HOW_TO_USE_THE_FTASK_MODULE:

How to Use the FTASK Module
===========================

Adding a Simple Functionality to a Task
---------------------------------------

This simple example shows how to add a blinking LED every 100ms (assuming the
hardware supports this feature) and incrementing a counter from a database
variable:

.. literalinclude:: ./ftask_how-to.c
   :language: C
   :linenos:
   :start-after: ftask-example-cyclic-100ms-start
   :end-before: ftask-example-cyclic-100ms-end
   :caption: Adding functionality to the 100ms User Code Function
   :name: adding-functionality-to-the-100ms-user-code-function

Further Reading
---------------

Implementation details of the FTASK module are found in :ref:`FTASK_MODULE`.
