.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _BUILD_PROCESS:

Build Process
=============

..
    Comments:
    none

.. spelling::
    SWI

This section addresses relevant steps in the build process that are important
to know when interacting with it.

.. _database_variable_checking:

Checking Database Entries
-------------------------
The build process automatically checks instances of database entries if they
are correctly initialized. For correct operation of the database functions
it is vital that each variable representing a database table has its member
``header`` set to the correct value. For more information on the usage of the
database module please refer to :numref:`DATABASE_MODULE`.

In case that the declaration and definition of a variable for a database table
does not match the required pattern, the build process will fail with an
error message that guides into the direction of a fix for the issue.
:numref:`using-the-wrong-header-element-in-a-database-table` shows an
example where the variable ``mrc_tableCellVoltages`` of type
``DATA_BLOCK_CELL_VOLTAGE_s`` has been initialized with a wrong header element.
This failure is reported by the build system.

.. code-block::
   :linenos:
   :caption: Using the wrong header element in a database table
   :name: using-the-wrong-header-element-in-a-database-table

    C:\Users\vulpes\Documents\foxbms-2\src\app\application\redundancy\redundancy.c:71 uses database variable mrc_tableCellVoltages (type: 'DATA_BLOCK_CELL_VOLTAGE_s') without correct initialization.

    Something like

            DATA_BLOCK_CELL_VOLTAGE_s mrc_tableCellVoltages = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE};
    or
            DATA_BLOCK_CELL_VOLTAGE_s mrc_tableCellVoltages = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_BASE};
    or
            DATA_BLOCK_CELL_VOLTAGE_s mrc_tableCellVoltages = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_REDUNDANCY0};

    is required, where the ID is from typedef enum DATA_BLOCK_ID_e.
    Details: Regex
            ^[\s]{0,}(?:static[\s]{1,}|extern[\s]{1,})?[\s]{0,}DATA_BLOCK_([A-Z_0-9]{1,})_s[\s]{1,}([A-za-z0-9_]{0,})[\s]{0,}=[\s]{0,}\{[\s]{0,}(\.header\.uniqueId)[\s]{0,}=[\s]{0,}(DATA_BLOCK_ID_)(CELL_VOLTAGE)(_BASE|_REDUNDANCY[0-9]{1,})?[\s]{0,}\}[\s]{0,};[\s]{0,}$
    did not match on
            static DATA_BLOCK_CELL_VOLTAGE_s mrc_tableCellVoltages         = {.header.uniqueId = DATA_BLOCK_ID_MIN_MAX};


.. _afe_library_build:

Building the Analog Front-End Library
-------------------------------------

In order to easily switch between different AFEs the |foxbms| build
system implements a mechanic for swapping implementations through a
configuration file.
The configuration file is described in :numref:`BMS_APPLICATION`.

The build system will automatically select the correct driver files depending
on the configuration.

External Libraries
------------------

A How-to is found in
:ref:`HOW_TO_BUILD_A_LIBRARY_AND_LINK_IT_IN_A_FOXBMS_2_PROJECT`.

.. _swi_info:

SWI Aliases
-----------

The build process automatically generates a list of the SWI-alias and function
name mapping. It is found in variant
build directory.

It is used as follows:

.. code-block:: python
   :caption: Searching for SWI-alias
   :name: swi-checking-example

        bld(
            features="swi-check",
            files=bld.path.ant_glob("src/**/*.c src/**/*.h"),
            jump_table_file=bld.path.find_node(
                "src/os/freertos/portable/ccs/arm_cortex-r5/portasm.asm"
            ),
        )

For implementation details see :py:meth:`f_ti_arm_cgt.get_swi_aliases` and
:py:meth:`f_ti_arm_cgt.print_swi_aliases`.
