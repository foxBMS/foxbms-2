.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _BUILD_PROCESS:

Build Process
=============

This section addresses relevant steps in the build process that are important
to know when interacting with it.

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
