.. include:: ./../../macros.txt

.. _BOOTLOADER:

##########
Bootloader
##########

Description
-----------

A `bootloader <https://de.wikipedia.org/wiki/Bootloader>`__
is a program to load initialization code.
In our case, it is the initial segment of the program located at the start of
the flash of the microcontroller (|ti-tms570lc4357|), and its primary purpose
is to update the |foxbms| application via interfaces like CAN on the fly
without the use of a debugger.

With the help of the bootloader, the compiled binary file of the |foxbms|
application can be easily uploaded to the |bms-master| by using the bootloader
PC application that is integrated into the :ref:`FOX_CLI` tool.
This tool not only provides the function to upload the binary of |foxbms| into
the hardware but can also be used to check the current status of the bootloader
and start the uploaded |foxbms| application manually.
In addition, the user can also use it to reset the boot process if any
unexpected issues appear.

In summary, this bootloader PC application provides the following
functionalities:

- checking the status of the bootloader
- updating/flashing the |foxbms| application
- resetting the boot process
- starting the |foxbms| application

To enable an automatic start of the |foxbms| application,  a timeout check has
been implemented in the initial phase of the bootloader, as shown in
:numref:`description_of_timeout_at_the_beginning`.
If there is no valid application onboard, the bootloader will return to its
original routine, and the timeout will no longer be activated.
Since the timeout duration is very short, the commands to load application and
reset the bootloader should be executed before powering on the |bms-master|.

.. drawio-figure:: img/description_of_timeout_at_the_beginning.drawio
   :format: svg
   :alt: Description of timeout at the beginning
   :name: description_of_timeout_at_the_beginning
   :width: 55 %
   :align: center

   Initial timeout check for bootloader

Before using this PC application, the bootloader should be compiled and flashed
into the hardware if there is no bootloader pre-installed in the |bms-master|.
Similar to building the |foxbms| application in
:ref:`BUILDING_THE_APPLICATION`, the fox CLI tool can be used to build the
bootloader application by using the following command:

   .. tabs::

      .. group-tab:: Win32/PowerShell

         .. code-block:: powershell

            .\fox.ps1 waf build_bootloader_embedded

      .. group-tab:: Win32/Git bash

         .. code-block:: shell

            ./fox.sh waf build_bootloader_embedded

      .. group-tab:: Linux

         .. code-block:: shell

            ./fox.sh waf build_bootloader_embedded

After flashing the binary of the bootloader into the |bms-master|, it is
possible to use the bootloader PC application to communicate with it.

.. _how_to_use_it:

How to Use It?
--------------

To build the bootloader binary, use the command variant
``build_bootloader_embedded``.
After the binary is successfully built, user can flash it into the |bms-master|
board using a debugger.
Once the binary is flashed, you can control it using commands available in the
fox CLI tool.
This bootloader PC application provides the following commands to interact
with the bootloader:

#. Check the status of the bootloader:

   .. tabs::

      .. group-tab:: Win32/PowerShell

         .. code-block:: powershell

            .\fox.ps1 bootloader check

      .. group-tab:: Win32/Git bash

         .. code-block:: shell

            ./fox.sh bootloader check

      .. group-tab:: Linux

         .. code-block:: shell

            ./fox.sh bootloader check

#. Upload a new |foxbms| application into the flash memory of |bms-master|:

   .. tabs::

      .. group-tab:: Win32/PowerShell

         .. code-block:: powershell

            .\fox.ps1 bootloader load-app

      .. group-tab:: Win32/Git bash

         .. code-block:: shell

            ./fox.sh bootloader load-app

      .. group-tab:: Linux

         .. code-block:: shell

            ./fox.sh bootloader load-app

   (To use this function, a |foxbms| binary should be built in advance
   following the instructions described in :ref:`BUILDING_THE_APPLICATION`.
   In addition, the command should be executed before powering on the
   |bms-master|, and the board should be powered on first if the instruction
   `"Waiting bootloader to be powered on ..."` is displaying in the terminal.)

#. Reset the boot process:

   .. tabs::

      .. group-tab:: Win32/PowerShell

         .. code-block:: powershell

            .\fox.ps1 bootloader reset

      .. group-tab:: Win32/Git bash

         .. code-block:: shell

            ./fox.sh bootloader reset

      .. group-tab:: Linux

         .. code-block:: shell

            ./fox.sh bootloader reset

   (Like the command to load a new |foxbms| application, the reset command
   should also be executed before powering on the |bms-master|.
   The board should then be powered on after the instruction
   `"Waiting bootloader to be powered on..."` is displaying in the terminal.)

   (In the case of an error status, a reset command or a power-on restart
   should resolve the problem. If not, the user could contact foxBMS team for
   further support.)

#. Start the |foxbms| application on |bms-master| manually:

   .. tabs::

      .. group-tab:: Win32/PowerShell

         .. code-block:: powershell

            .\fox.ps1 bootloader run-app

      .. group-tab:: Win32/Git bash

         .. code-block:: shell

            ./fox.sh bootloader run-app

      .. group-tab:: Linux

         .. code-block:: shell

            ./fox.sh bootloader run-app

#. To get more information, add `-vv` after command, for example:

   .. tabs::

      .. group-tab:: Win32/PowerShell

         .. code-block:: powershell

            .\fox.ps1 bootloader check -vv

      .. group-tab:: Win32/Git bash

         .. code-block:: shell

            ./fox.sh bootloader check -vv

      .. group-tab:: Linux

         .. code-block:: shell

            ./fox.sh bootloader check -vv

.. _communication_between_pc_and_bootloader_while_uploading_the_binary_of_foxbms:

Description of the |foxbms| Application Update Process Using the Bootloader
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

To transfer the application binary to the |bms-master|, the binary file needs
to be parsed into small sectors first, and a CRC signature is calculated for
each sector.
The 'sector' used in this context has been defined based on the data block that
can be stored in the corresponding flash sector of the physical flash memory.
To enable data transfer via CAN messages, which have the maximum size of 8
bytes, each sector is further divided into subsectors that contain 1024 * 8
bytes of data, as shown in :numref:`from_bin_file_to_8_bytes_data`:

.. drawio-figure:: img/from_bin_file_to_8_bytes_data.drawio
   :format: svg
   :alt: From bin file to 8 bytes data
   :name: from_bin_file_to_8_bytes_data
   :width: 60 %
   :align: center

   The division of the |foxbms| application binary file during data transfer

The communication between the PC application and the bootloader has been
implemented via pre-defined CAN messages.
As shown in :numref:`communication_between_pc_and_bootloader`, to
transfer a program, the PC application will send a CAN request
"command to transfer program" to inform the bootloader that a program is going
to be transferred.
Once the bootloader receives the command, it will reply with an "ACK message"
to inform the PC application that it has received this command and is prepared
for the next step.
The PC application will then start sending the information relevant to the
current data transfer process.
After that, the PC application must receive an ACK message from the bootloader
to ensure it has processed the program information and is ready for the binary
data.

Next, the PC application starts transferring the binary data.
It iteratively sends one data sector after another as depicted in
:numref:`from_bin_file_to_8_bytes_data`.
As shown, each subsector is transferred by sending the loop number and 8 bytes
of data in every iteration.
Once the bootloader has successfully received 1024 * 8 bytes of data, it
responds with an "ACK message" to signal the PC application to send the next
subsector.

.. drawio-figure:: img/communication_between_pc_and_bootloader.drawio
   :format: svg
   :alt: Communication between PC and bootloader
   :name: communication_between_pc_and_bootloader
   :width: 50 %
   :align: center

   Communication between the PC-side application and the bootloader

Once a sector transfer is completed, the program sector will be written into
its relevant flash space, and a CRC signature will be calculated.
Next, the CRC signature for this program sector will be sent to the host
computer and compared with the calculated CRC signature to ensure that the
flashed section data is correct.
If both CRC signatures are equal, the next sector will be transferred.
Otherwise, the bootloader will wait for the same sector data again.

After all sectors are successfully sent, the vector table will be sent in four
parts together with a CRC signature to the bootloader to ensure the validity of
the vector table.

Project Structure
-----------------

This bootloader project contains two parts: the bootloader itself and the PC
application, which communicates with the bootloader.
The file structures of these two parts is as following:

.. table:: C Part (On-board code)
   :name: c-part-on-board-code
   :widths: grid

   +-----------------------------+--------------------------------------------+
   | Module                      | Description                                |
   +=============================+============================================+
   | driver                      | Contains low level driver modules to       |
   |                             | control the on-board resources.            |
   +-----------------------------+--------------------------------------------+
   | engine                      | Contains mid-level engine modules to       |
   |                             | control the overall program flow.          |
   +-----------------------------+--------------------------------------------+
   | hal                         | Contains the build script and the hash     |
   |                             | code for HALCoGen.                         |
   +-----------------------------+--------------------------------------------+
   | main                        | Contains the files where the 'main'        |
   |                             | function and '_c_int00' function are       |
   |                             | located.                                   |
   |                             | In addition, it also contains the linker   |
   |                             | script for configuring the memory          |
   |                             | distribution and the files supported at    |
   |                             | the system level.                          |
   +-----------------------------+--------------------------------------------+

.. table:: Python Part (PC-side code)
   :name: python-part-pc-side-code
   :widths: grid

   +-----------------------------+--------------------------------------------+
   | Module                      | Description                                |
   +=============================+============================================+
   | bootloader.py               | Contains the Bootloader class, which       |
   |                             | serves as the main entry point for         |
   |                             | sending application data or requests to    |
   |                             | the bootloader.                            |
   +-----------------------------+--------------------------------------------+
   | bootloader_can.py           | Contains the BootloaderInterfaceCan class, |
   |                             | which enables high-level communication     |
   |                             | with the bootloader via CAN.               |
   +-----------------------------+--------------------------------------------+
   | bootloader_can_basics.py    | Contains the BootloaderCanBasics class,    |
   |                             | where the basic CAN communication          |
   |                             | functions, including sending and receiving |
   |                             | specified messages, are implemented.       |
   +-----------------------------+--------------------------------------------+
   | bootloader_binary_file.py   | Contains the BootloaderBinaryFile class,   |
   |                             | responsible for managing the application   |
   |                             | binary file and providing functions to     |
   |                             | perform operations on it, such as          |
   |                             | calculating CRC, extracting data, and      |
   |                             | more.                                      |
   +-----------------------------+--------------------------------------------+
   | bootloader_can_messages.py  | Contains all enums of CAN messages and     |
   |                             | functions to get specified CAN messages in |
   |                             | a dictionary.                              |
   +-----------------------------+--------------------------------------------+

Memory Configuration
--------------------

The microcontroller (|ti-tms570lc4357|) has two independent flash banks.
Each of them consists of 16 sectors and has a storage area of 2MB.
While the sizes of the sectors in the second flash bank are uniform, the sizes
of the sectors in the first flash bank are not identical.
More details can be found in
`Technical Reference Manual of TMS570LC43 <https://www.ti.com/lit/ug/spnu563a/spnu563a.pdf?ts=1725549128780&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FTMS570LC4357%253FHQS%253Dti-null-null-verifimanuf_manuf-manu-pf-octopart-wwe>`__
.

In this project, the flash memory (from ``0x0x00000000`` to ``0x00400000``)
has been divided into 5 regions, as shown in :numref:`memory-configuration`.
The initial vector table is saved in the memory labeled
``VECTORS_TABLE_INIT``, and the second vector table, where the actual exception
entries are implemented, is saved in the memory labeled
``VECTORS_TABLE``.
More details about the vector tables can be found in
:numref:`about_vector_table`.

The program of bootloader (except for its vector table) has been saved in the
memory labeled ``BOOTLOADER``.
The binary of |foxbms| is supposed to be put in the memory labeled
``PROGRAM_PLACE_HOLDER``.
The information of the program will be saved to memory labeled
``PROGRAM_INFO_AREA``.

In this project, the RAM of the microcontroller is configured as shown in
:numref:`memory-configuration`.
As the names indicate, the memory labeled ``STACK`` refers to the space
allocated for stack usage, while the memory labeled ``RAM`` serves as the
general RAM space.

The memory labeled ``RAM_FLASH`` serves as the section buffer to temporarily
store transferred data before it is written to flash sector.
The flash-related functions and libraries will run from the memory labeled
``RAM_FLASH_API``.
More details can be found in :numref:`how_to_load_flash_api_to_sram`.

The memory labeled with the ``ECC`` prefix is where the calculated error
correction codes (ECC) are saved.
More details about the ECC can be found in
`Technical Reference Manual of TMS570LC43 <https://www.ti.com/lit/ug/spnu563a/spnu563a.pdf?ts=1725549128780&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FTMS570LC4357%253FHQS%253Dti-null-null-verifimanuf_manuf-manu-pf-octopart-wwe>`__
.

.. table:: Memory Configuration
   :name: memory-configuration
   :widths: grid
   :align: center

   +------------------------------+----------------+-------------------+--------+----------------+
   | NAME                         | ORIGIN         | LENGTH            | ATTR   | FILL           |
   +==============================+================+===================+========+================+
   | ``VECTORS_TABLE_INIT``       | ``0x00000000`` | ``0x00000020``    | ``X``  | ``0xFFFFFFFF`` |
   +------------------------------+----------------+-------------------+--------+----------------+
   | ``BOOTLOADER``               | ``0x00000020`` | ``0x00017FE0``    | ``RX`` | ``0xFFFFFFFF`` |
   +------------------------------+----------------+-------------------+--------+----------------+
   | ``PROGRAM_INFO_AREA``        | ``0x00018000`` | ``0x00007FE0``    | ``RX`` | ``0xFFFFFFFF`` |
   +------------------------------+----------------+-------------------+--------+----------------+
   | ``VECTORS_TABLE``            | ``0x0001FFE0`` | ``0x00000020``    | ``X``  | ``0xFFFFFFFF`` |
   +------------------------------+----------------+-------------------+--------+----------------+
   | ``PROGRAM_PLACE_HOLDER``     | ``0x00020000`` | ``0x003E0000``    | ``RX`` | ``0xFFFFFFFF`` |
   +------------------------------+----------------+-------------------+--------+----------------+
   | ``STACK``                    | ``0x08000000`` | ``0x00010000``    | ``RW`` |                |
   +------------------------------+----------------+-------------------+--------+----------------+
   | ``RAM``                      | ``0x08010000`` | ``0x00020000``    | ``RWX``|                |
   +------------------------------+----------------+-------------------+--------+----------------+
   | ``RAM_FLASH``                | ``0x08030000`` | ``0x00040000``    | ``RWX``|                |
   +------------------------------+----------------+-------------------+--------+----------------+
   | ``RAM_FLASH_API``            | ``0x08070000`` | ``0x00010000``    | ``RWX``|                |
   +------------------------------+----------------+-------------------+--------+----------------+
   | ``ECC_VECTORS_TABLE_INIT``   | ``0xF0400000`` | ``0x00000004``    | ``R``  |                |
   +------------------------------+----------------+-------------------+--------+----------------+
   | ``ECC_BOOTLOADER``           | ``0xF0400004`` | ``0x00002FFC``    | ``R``  |                |
   +------------------------------+----------------+-------------------+--------+----------------+
   | ``ECC_PROGRAM_INFO_AREA``    | ``0xF0403000`` | ``0x00000FFC``    | ``R``  |                |
   +------------------------------+----------------+-------------------+--------+----------------+
   | ``ECC_VECTORS_TABLE``        | ``0xF0403FFC`` | ``0x00000004``    | ``R``  |                |
   +------------------------------+----------------+-------------------+--------+----------------+
   | ``ECC_PROGRAM_PLACE_HOLDER`` | ``0xF0404000`` | ``0x0007C000``    | ``R``  |                |
   +------------------------------+----------------+-------------------+--------+----------------+

Functional Mechanisms
---------------------

The functions of the bootloader are implemented through the cooperation of two
independent final state machines (FSMs).
One FSM is called the boot FSM, as it directly controls the boot process of the
bootloader.
The other is called the CAN FSM because it helps control the CAN communication
and ensures the correct sequence of the data transfer process.

This section will first present the CAN FSM and the boot FSM separately, using
their corresponding state diagrams.
After that, the functional mechanism of the bootloader will be demonstrated by
explaining the principles of starting the |foxbms| application, resetting the
boot process, and uploading the |foxbms| application into the flash
memory of the |bms-master|.

CAN FSM
^^^^^^^
The entire state diagram of the CAN FSM state machine is shown in :numref:`can_fsm_state`.

.. drawio-figure:: img/can_fsm_state.drawio
   :format: svg
   :name: can_fsm_state
   :align: center
   :alt: CAN FSM state
   :width: 90 %

   State diagram of the CAN FSM

Boot FSM
^^^^^^^^

As shown in :numref:`boot_fsm_state`, at the start of the program, the state of
the boot FSM is initialized to ``BOOT_FSM_STATE_WAIT``.
From this state, the state of the boot FSM can change to
``BOOT_FSM_STATE_LOAD``, ``BOOT_FSM_STATE_RUN``, or ``BOOT_FSM_STATE_RESET`` in
response to changes in the CAN FSM.
If any error happens during the state ``BOOT_FSM_STATE_LOAD``,
``BOOT_FSM_STATE_RUN`` or ``BOOT_FSM_STATE_RESET``, the state of the boot FSM
will change to  ``BOOT_FSM_STATE_ERROR``.

.. drawio-figure:: img/boot_fsm_state.drawio
   :format: svg
   :name: boot_fsm_state
   :align: center
   :width: 65 %
   :alt: Boot FSM state

   State diagram of the boot FSM

Start the |foxbms| Application
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The |foxbms| application starts automatically, if no CAN request to the
``CAN_FSM_STATE_MACHINE`` has been received within the defined timeout.
The application will start earlier if the corresponding request is sent to the
bootloader via the CAN bus.
Once the CAN module has received this CAN message, it will change its state
from the initial state ``CAN_FSM_STATE_NO_COMMUNICATION`` to
``CAN_FSM_STATE_RUN_PROGRAM`` as shown in :numref:`can_boot_run`.
If the boot FSM is in the expected state (``BOOT_FSM_STATE_WAIT``), at this
moment, a validation process will be initiated to check if the flashed |foxbms|
application is valid.
The bootloader will only jump into the application if the validation process
was successful.
Otherwise, it will inform the host PC that there is no valid program available.

.. drawio-figure:: img/can_boot_run.drawio
   :format: svg
   :name: can_boot_run
   :align: center
   :width: 60 %
   :alt: Interaction between the boot FSM and the CAN FSM after a "run-app" command is issued

   Interaction between the boot FSM and the CAN FSM after a "run-app" command is issued

Reset the Boot Process
^^^^^^^^^^^^^^^^^^^^^^

If a reset request (see :numref:`how_to_use_it`) has been sent via the CAN bus,
the CAN FSM state will change to ``CAN_FSM_STATE_RESET_BOOT`` from any state
as shown in :numref:`can_boot_reset`.
After that, the bootloader will reset its boot-relevant configurations,
including global variables that contain the boot- and CAN-relevant program
information.
Additionally, the sector buffer RAM area where the sector data is temporarily
stored will be cleared, and the flash sectors where the application data is
supposed to be written will be erased.
Finally, a software reset will be performed to reset the MCU.

.. drawio-figure:: img/can_boot_reset.drawio
   :format: svg
   :name: can_boot_reset
   :align: center
   :width: 60 %
   :alt: Interaction between the boot FSM and the CAN FSM after a "reset" command is issued

   Interaction between the boot FSM and the CAN FSM after a "reset" command is issued

Transfer the |foxbms| Application
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If the incoming CAN message indicates a start of the data transfer process, the
CAN FSM state will change to ``CAN_FSM_STATE_WAIT_FOR_INFO``, and the boot FSM
state will change to ``BOOT_FSM_STATE_LOAD``, as shown in
:numref:`can_boot_load`.

.. drawio-figure:: img/can_boot_load.drawio
   :format: svg
   :name: can_boot_load
   :align: center
   :width: 60 %
   :alt: Interaction between the boot FSM and the CAN FSM after a "load-app" command is issued

   Interaction between the boot FSM and the CAN FSM after a "load-app" command is issued

Once the information of the program has been successfully transferred, the
state of the CAN FSM will change from ``CAN_FSM_STATE_WAIT_FOR_INFO``
to ``CAN_FSM_STATE_WAIT_FOR_DATA_LOOPS``.
To ensure that the transferred data (8 bytes each time) is correct, in the
innermost loop, a corresponding loop number needs to be sent before the
transmission of the corresponding data in 8 bytes, as shown in
:numref:`communication_between_pc_and_bootloader`.
Only if the loop number is the one the CAN module expects, the data bytes will
be considered correct and will be written into the sector buffer.
Essentially, the loop number can also be understood as the key to enabling the
reception of the corresponding 8-byte program data.

To transmit the data efficiently and precisely, the entire program will be
divided into sectors, as mentioned in
:numref:`communication_between_pc_and_bootloader_while_uploading_the_binary_of_foxbms`.
The program sector size is identical to the size of corresponding physical
flash sector section.
If a whole sector has been transferred and written into the sector buffer, the
corresponding flash sector will be written using the data that is temporarily
saved in the sector buffer.
Immediately afterwards, the written flash sector will be validated by comparing
the received CRC signature (8 bytes) with the calculated CRC signature
(8 bytes).
If the result is invalid, the variables involved in data transfer and the
sector buffer will be reset to their original state at the start of the data
transfer for this sector.
Additionally, the CAN FSM state will reset to either
``CAN_FSM_STATE_WAIT_FOR_DATA_LOOPS`` or
``CAN_FSM_STATE_RECEIVED_8_BYTES_CRC``.

Once all data has been received, written, and validated, the CAN FSM state will
be set to ``CAN_FSM_STATE_FINISHED_FINAL_VALIDATION``, as shown in
:numref:`can_fsm_state`.
Meanwhile, the CRC signature of the last sector, which is also the CRC
signature of the entire application binary, will be saved in the flash memory
where the program information is stored.

After the entire program has been received and validated, the vector table for
this program will also be transferred and validated.
Meanwhile, its state will be updated accordingly, as shown in
:numref:`can_fsm_state`.

Developer Notes
---------------

What Should Be Considered/Modified While Configuring the Flash Memory?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

To configure the memory of the microcontroller (|ti-tms570lc4357|), several
parts need to be considered/configured:

#. The linker script of the application (|foxbms|) ``app.cmd``
   and ``app_hex.cmd``.
#. The linker script of the bootloader ``bootloader.cmd``.
#. The address jump between the first vector table and the second vector table,
   which is defined in ``intvecs.asm``.
#. The corresponding macros defined in the bootloader header file
   : ``boot_cfg.h``.

.. _about_vector_table:

About Vector Table
^^^^^^^^^^^^^^^^^^

The
`vector table <https://www.ti.com/lit/an/spna236/spna236.pdf?ts=1724301653744>`__
is usually placed at the start address (``0x00``) of the flash and has a length
of ``0x20``.
It contains eight 32-bit ARM instructions in our case.

In the bootloader, there are two vector tables (``VECTORS_TABLE_INIT`` and
``VECTORS_TABLE``)
located at ``0x00`` and ``0x0001FFE0`` of the flash memory.
The first vector table only reroutes the undefined entry, the SVC
(supervisor call) entry, the prefetch entry, the data abort entry, and the
phantom interrupt entry, to these inside the second vector table
(``VECTORS_TABLE``).
The second vector table hosts the real functions entries to handle these
exception entries using
`b xxx <https://developer.arm.com/documentation/den0042/a/Exceptions-and-Interrupts/Exception-priorities/The-Vector-table>`__.

Different from handlers listed above, the reset entry points always to the ``_c_int00``
function which will also be called first before any other functions.
The IRQ and FIQ interrupt table will be loaded by
`ldr pc, [pc, #-0x1b0] <https://developer.arm.com/documentation/den0042/a/Exceptions-and-Interrupts/Exception-priorities/The-Vector-table>`__
inside the first vector table.
The configuration and initialization of the vectored interrupt manager is done
in ``_c_int00`` by ``vimInit()``.

During booting, the ``_c_int00`` function is first called, but the actual
working exception entries (except for IRQ and FIQ) will be the ones defined the
second vector table.
After the |foxbms| application is flashed, the |foxbms| application vector
table will overwrite the second vector table ``VECTORS_TABLE``.
This means that by jumping into the second vector table, the ``_c_int00``
function of the application will be called, where the configuration for, e.g.,
VIM will be reset for the application.
Meanwhile, the real entries for the exceptions will be replaced by the entries
shipped with the application.

Change Operation Mode
^^^^^^^^^^^^^^^^^^^^^
Since some functions inside flash and CRC modules change values in the
protected flash area, such as register values, certain privileges need to be
claimed before calling these functions.

To raise the privilege, SVC handlers are implemented to change the value of
`the program status register <https://developer.arm.com/documentation/ddi0460/d/Programmers-Model/Program-status-registers?lang=en>`__.

.. _how_to_load_flash_api_to_sram:

How to Load the "Flash API" into the SRAM?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

To erase and write the flash bank where the bootloader is located, the relevant
flash API and all functions that use the flash API need to be executed from
SRAM rather than flash.
More details can be found in
`here <https://www.ti.com/lit/ug/spnu501h/spnu501h.pdf?ts=1725300688204>`__,
which is also called
`run-time relocation <https://software-dl.ti.com/codegen/docs/tiarmclang/compiler_tools_user_guide/compiler_manual/program_loading_and_running/run-time-relocation-stdz0694629.html#stdz0694629>`__.
To execute program code from SRAM, the following steps must be done:

#. Change the MPU configuration for the region from ``0x08000000`` to
   ``0x0807FFFF`` to ``PRIV_RW_USER_RW_EXEC`` to enable calling the functions
   in this area without triggering any error.
   More details can be found in
   `here <https://www.ti.com/lit/an/spna238/spna238.pdf?ts=1718810487618&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FTMS570LC4357%253Fbm-verify%253DAAQAAAAJ_____32ivajprZvDL3z6rbQIE5s075poxTO-MCK52TSVps_v6YYqaPSWjYQ8ZJlLdypjTy-Tqv1nypZNJa1qxYZE777ZbDRShl_L8Q1uxIgT9PjWdBBcqMH_O4ldH-G8PpDeULXMzVm45DMrLCW8TZVoHk_o-JO4xL6_YoJ_V544VamYSzvz1R-2oBwOWyiZuirQHIT5IREJYlRnmJ7kRaSYlO-hR7Az-4W2mZbI37U0s3Gsu6HSHoGDeozZKUhPR2Nmjswq-T-k5BuK7K-96wpqYDwRG1TQDbKaZYJ1WRYw-Dk-Of3GwVAufbiX23E>`__
#. Configure loading the flash API and flash relevant functions to flash memory
   and run them from SRAM by using build-in link operators in linker script.
   More details can be found in
   `here <https://software-dl.ti.com/codegen/docs/tiarmclang/compiler_tools_user_guide/compiler_manual/linker_description/08_using_linker_generated_copy_tables/using-built-in-link-operators-in-copy-tables-stdz0757585.html>`__.

How to Use the Onboard CRC Module in Semi-AUTO Mode?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

There is an onboard CRC controller available on the TMS570LC43.
It offers three modes of operation: Auto, Semi-CPU, and Full-CPU.
In our case, the Semi-CPU mode is used to calculate the CRC.
Unlike Auto mode, where the CRC calculation and evaluation are performed
without CPU intervention, in Semi-CPU mode, the generated CRC signature must be
compared with a pre-determined CRC value with the assistance of the CPU.

More detailed information regarding CRC onboard module and the CRC algorithm
used within the CRC onboard module can be found in
`here <https://www.ti.com/lit/an/spna235/spna235.pdf>`__.

How to Jump to a Certain Address?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

When you power-up the BMS, the bootloader will start first because the
bootloader's reset vector that is located at the first flash address ``0x00``.

From the address area where the bootloader program is located, it is possible
to jump to another address using the following code example:

.. code:: c

   uint32_t boot_jumpAddress;
   boot_jumpAddress = (uint32_t)APP_START_ADDRESS;

   ((void (*)(void))boot_jumpAddress)();

This code performs the following actions:

.. code:: c

   boot_jumpAddress = (uint32_t)APP_START_ADDRESS;

assigns the value of ``APP_START_ADDRESS`` (``0x00020020``) to the
``boot_jumpAddress`` variable after casting it to an unsigned 32-bit
integer.

.. code:: c

   ((void (*)(void))boot_jumpAddress)();

is a function pointer cast and call operation. Here's what it does :

#. ``(void (*)(void))boot_jumpAddress`` casts the
   ``boot_jumpAddress`` variable to a function pointer. This cast
   assumes that the address stored in ``boot_jumpAddress`` points to
   a function with no arguments and no return value (i.e., a function
   that takes ``void`` as both its argument and return types).
#. ``()`` immediately invokes (calls) the function pointed to by the
   casted function pointer.

In summary, this code converts an address (``boot_jumpAddress``) into
a function pointer and then calls the function at that address.
