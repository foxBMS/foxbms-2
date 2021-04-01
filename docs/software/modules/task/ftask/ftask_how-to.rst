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

.. code-block:: c
   :linenos:
   :caption: Adding functionality to the 100ms User Code Function
   :name: adding-functionality-to-the-100ms-user-code-function

    void FTSK_UserCodeCyclic100ms(void) {
        /* user code */
        static uint32_t ftsk_cyclic_100ms_counter              = 0;
        static DATA_BLOCK_EXAMPLE_s ftsk_tableExampleCyclic100ms = {.header.uniqueId = DATA_BLOCK_ID_EXAMPLE};
        if ((ftsk_cyclic_100ms_counter % 10u) == 0u) {
            gioSetBit(gioPORTB, 6, gioGetBit(gioPORTB, 6) ^ 1);
        }

        DATA_READ_DATA(&ftsk_tableExampleCyclic100ms);
        ftsk_tableExampleCyclic100ms.dummy_value++;
        DATA_WRITE_DATA(&ftsk_tableExampleCyclic100ms);
        ftsk_cyclic_100ms_counter++;
    }

Further Reading
---------------

Implementation details of the FTASK module are found in :ref:`FTASK_MODULE`.
