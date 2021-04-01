.. include:: ../../../../macros.txt
.. include:: ../../../../units.txt

.. _HOW_TO_USE_THE_DATABASE_MODULE:

How to Use the Database Module
==============================

This How-to shows how to add new entries to the database, and how to read and write database entries.


How to Add a Database Entry?
----------------------------

Four steps need to be undertaken to add an entry to the database.

First, an identification number (ID) for this database entry must be added to enum
``DATA_BLOCK_ID_e`` in file ``src/app/engine/config/database_cfg.h``. This enumeration
value respectively the identification number functions as ``uniqueId`` for each
database entry to synchronize database entries with local copies during read
and write access to the database.

.. note::

    The new ID must start with ``DATA_BLOCK_ID_``. Before any source is
    compiled the build process checks that a database variable is
    initialized with the matching ID from ``DATA_BLOCK_ID_e``. For the details
    of the checker implementation see :ref:`database_variable_checking`.

In this example the name of the new database entry is **example**, therefore the
corresponding ID must be the prefix and the name in uppercase, which
gives ``DATA_BLOCK_ID_EXAMPLE``.

.. code-block:: c
   :linenos:
   :caption: Adding a new element **example** to the ``DATA_BLOCK_ID_e`` enum
   :name: adding-a-new-element-to-the-data-block-id-e-enum

    /** data block identification numbers */
    typedef enum DATA_BLOCK_ID {
        DATA_BLOCK_ID_EXAMPLE, /**< some example database entry */
        /* ...
           ...
           ...
        */
        DATA_BLOCK_ID_MAX, /**< DO NOT CHANGE, MUST BE THE LAST ENTRY */
    } DATA_BLOCK_ID_e;


Second, a struct definition of the database entry has to be added in file
``src/app/engine/config/database_cfg.h``. This structure needs to contain a variable
``header`` of type ``DATA_BLOCK_HEADER_s`` at the beginning of the struct. This
header contains a ``uniqueID`` for this database entry as well as timestamps. These
timestamps (``timestamp`` and ``previousTimestamp``) are automatically updated each
time new data is stored in this database entry. The remaining struct consists
of all the data needed for the entry.

.. note::

    The new database entry must start with ``DATA_BLOCK_``. Before any source is
    compiled the build process checks that a database variable is
    initialized with the matching ID from ``DATA_BLOCK_ID_e``. For the details
    of the checker implementation see :ref:`database_variable_checking`.

Again, the name of the new database entry is **example**, therefore the
corresponding data block must be the prefix followed by the name in uppercase and
the ``_s`` suffix as the typedef is a struct, which gives ``DATA_BLOCK_EXAMPLE_s``.

.. code-block:: c
   :linenos:
   :caption: Creating a new database struct
   :name: creating-a-new-database-struct

    /**
    * data block struct of example
    */
    typedef struct DATA_BLOCK_EXAMPLE {
        /* This struct needs to be at the beginning of every database entry. During
        * the initialization of a database struct, uniqueId must be set to the
        * respective database entry representation in enum DATA_BLOCK_ID_e. */
        DATA_BLOCK_HEADER_s header;
        uint16_t dummyValue;
    } DATA_BLOCK_EXAMPLE_s;

The database checker basically checks that if a database struct is found (something matching
``DATA_BLOCK_(A_Z_0-9)_s``) that it is initialized with a corresponding ID name.

Examples:

+-------------------------------+--------------------------------+
| Struct                        | Id                             |
+-------------------------------+--------------------------------+
| DATA_BLOCK_**EXAMPLE**_s      | DATA_BLOCK_ID_**EXAMPLE**      |
+-------------------------------+--------------------------------+
| DATA_BLOCK_**CELL_VOLTAGE**_s | DATA_BLOCK_ID_**CELL_VOLTAGE** |
+-------------------------------+--------------------------------+


Third, in ``src/app/engine/config/database_cfg.c``, a static variable with
the structure type must be declared. In this declaration the ``uniqueId`` of the
database header must be initialized with the identification number added in the
first step. This structure ``data_blockExample`` contains the actual values of
this database entry during once the BMS is running.

.. code-block:: c
   :linenos:
   :caption: Initialization of the static variable inside the database module
   :name: initialization-of-the-static-variable-inside-the-database-module

    static DATA_BLOCK_EXAMPLE_s data_blockExample = {.header.uniqueId = DATA_BLOCK_ID_EXAMPLE};


Last, this declared struct must be linked to the actual database implementation
by adding an additional entry to the structure ``DATA_BASE_s data_database[]``
in file ``src/app/engine/config/database_cfg.c``

.. code-block:: c
   :linenos:
   :caption: Adding the struct to the database
   :name: adding-the-struct-to-the-database

    /**
    * @brief   channel configuration of database (data blocks)
    * @details all data block managed by database are listed here (address, size,
    *          consistency type)
    */
    DATA_BASE_s data_database[] = {
        {
            (void*)(&data_blockExample),
            sizeof(DATA_BLOCK_EXAMPLE_s)
        },
        /* ...
           ...
           ...
        */
    };

How to Read a Database Entry?
-----------------------------

Data inside the database can not be accessed directly. The data from a database
entry has to be copied inside a local representation of the requested entry.
For the local structure of the database entry, the ``uniqueId`` of the database
entry header has to be initialized with the identification number of the
requested database entry. Once this is done, a database entry can simply be
accessed by calling function ``DATA_READ_DATA()`` and passing the pointer to
the local database entry structure.

.. code-block:: c
   :linenos:
   :caption: Reading a database entry
   :name: reading-a-database-entry
   :emphasize-lines: 4

    /* Create local structure of database entry */
    DATA_BLOCK_EXAMPLE_s tableLocalExampleEntry = { .header.uniqueId = DATA_BLOCK_ID_EXAMPLE };
    /* Read data from database entry and update local variable */
    DATA_READ_DATA(&tableLocalExampleEntry);


Up to four database entries can be accessed on the same time with one call of
``DATA_READ_DATA()``.

.. code-block:: c
   :linenos:
   :caption: Reading multiple database entries
   :name: reading-multiple-database-entries
   :emphasize-lines: 6

    DATA_BLOCK_EXAMPLE_0_s tableLocalExampleEntry0 = { .header.uniqueId = DATA_BLOCK_ID_EXAMPLE_0 };
    DATA_BLOCK_EXAMPLE_1_s tableLocalExampleEntry1 = { .header.uniqueId = DATA_BLOCK_ID_EXAMPLE_1 };
    DATA_BLOCK_EXAMPLE_2_s tableLocalExampleEntry2 = { .header.uniqueId = DATA_BLOCK_ID_EXAMPLE_2 };
    DATA_BLOCK_EXAMPLE_3_s tableLocalExampleEntry3 = { .header.uniqueId = DATA_BLOCK_ID_EXAMPLE_3 };
    /* Read data from 4 database entries and update local variables */
    DATA_READ_DATA(&tableLocalExampleEntry0, &tableLocalExampleEntry1, &tableLocalExampleEntry2, &tableLocalExampleEntry3);


How to Write a Database Entry?
------------------------------

It is not possible to directly write data into a database entry. Only the
content of a local representation of the requested database entry can be copied
into the database. The ``uniqueId`` of the database entry header has to be
initialized with the identification number of the requested database entry for
the local structure of the database entry. Once this is done, a database entry
can be written by calling function ``DATA_WRITE_DATA()`` and passing the
pointer to the local database entry structure. Up to four database entries can
be written with one call of ``DATA_WRITE_DATA()``.

.. code-block:: c
   :linenos:
   :caption: Writing a database entry
   :name: writing-a-database-entry
   :emphasize-lines: 6

    /* Create local structure of database entry */
    DATA_BLOCK_EXAMPLE_s tableLocalExampleEntry = { .header.uniqueId = DATA_BLOCK_ID_EXAMPLE };
    /* Set dummy value to 100 */
    tableLocalExampleEntry.dummyValue = 100;
    /* Update database entry with changed dummy value */
    DATA_WRITE_DATA(&tableLocalExampleEntry);

After a call of ``DATA_WRITE_DATA`` the timestamp variables in the database
entry header are automatically updated database entry **AND** the local
database entry structure.

Up to four database entries can be written on the same time with one call of
``DATA_WRITE_DATA()``.

.. code-block:: c
   :linenos:
   :caption: Writing multiple database entries
   :name: writing-multiple-database-entries
   :emphasize-lines: 11

    DATA_BLOCK_EXAMPLE_0_s tableLocalExampleEntry0 = { .header.uniqueId = DATA_BLOCK_ID_EXAMPLE_0 };
    DATA_BLOCK_EXAMPLE_1_s tableLocalExampleEntry1 = { .header.uniqueId = DATA_BLOCK_ID_EXAMPLE_1 };
    DATA_BLOCK_EXAMPLE_2_s tableLocalExampleEntry2 = { .header.uniqueId = DATA_BLOCK_ID_EXAMPLE_2 };
    DATA_BLOCK_EXAMPLE_3_s tableLocalExampleEntry3 = { .header.uniqueId = DATA_BLOCK_ID_EXAMPLE_3 };
    /* Set dummy values to 100 */
    tableLocalExampleEntry0.dummyValue = 100;
    tableLocalExampleEntry1.dummyValue = 100;
    tableLocalExampleEntry2.dummyValue = 100;
    tableLocalExampleEntry3.dummyValue = 100;
    /* Update database entries with changed dummy values */
    DATA_WRITE_DATA(&tableLocalExampleEntry0, &tableLocalExampleEntry1, &tableLocalExampleEntry2, &tableLocalExampleEntry3);


Further Reading
---------------

Implementation details of the database module are found in :ref:`DATABASE_MODULE`.
