.. include:: ./../macros.txt
.. include:: ./../units.txt

.. _REPOSITORY_STRUCTURE:

Repository Structure
********************

..
    Comments:
    Jupyter is a python tool

.. spelling::
    Jupyter

The |foxbms| repository is structured into the following parts (see
:numref:`introduction-repository-structure`). All directory names are
abbreviated (e.g., ``conf`` for ``configuration``) to ensure a compact
repository structure and avoid long command lines during build steps.

.. table:: |foxbms| repository structure
   :name: introduction-repository-structure
   :widths: grid

   +-----------------+-----------------------------+--------------------------------------+
   | directory       | long name                   | content description                  |
   +=================+=============================+======================================+
   | ``.``           | repository root             | the repository root contains the     |
   |                 |                             | license information, a changelog     |
   |                 |                             | and the main build script            |
   +-----------------+-----------------------------+--------------------------------------+
   | ``conf``        | Configuration               | contains all high level              |
   |                 |                             | configurations                       |
   +-----------------+-----------------------------+--------------------------------------+
   | ``docs``        | Documentation               | documentation source files           |
   +-----------------+-----------------------------+--------------------------------------+
   | ``hardware``    | Hardware                    | hardware schematics and layouts      |
   +-----------------+-----------------------------+--------------------------------------+
   | ``src``         | Source                      | parent directory for all source      |
   |                 |                             | files for the BMS embedded software  |
   +-----------------+-----------------------------+--------------------------------------+
   | ``tests``       | Tests                       | tests for embedded sources, the tool |
   |                 |                             | chain,  the conda environment and    |
   |                 |                             | scripts                              |
   +-----------------+-----------------------------+--------------------------------------+
   | ``tools``       | Tools                       | tools needed to build |foxbms|       |
   |                 |                             | binaries and additional tools to     |
   |                 |                             | work with |foxbms|                   |
   +-----------------+-----------------------------+--------------------------------------+


Configuration directory ``conf``
================================

The ``conf`` directory is structured into the following parts (see
:numref:`introduction-repository-structure-conf-details`).

.. table:: Detailed description of the ``conf`` directory
   :name: introduction-repository-structure-conf-details
   :widths: grid

   +-----------------+-----------------------------+--------------------------------------+
   | directory       | long name                   | content description                  |
   +=================+=============================+======================================+
   | ``conf/bms``    | Battery Management System   | configuration of the BMS             |
   |                 |                             | (e.g., which AFE is used)            |
   +-----------------+-----------------------------+--------------------------------------+
   | ``conf/cc``     | C compiler                  | compiler configuration               |
   |                 |                             | (e.g., compiler flags)               |
   +-----------------+-----------------------------+--------------------------------------+
   | ``conf/env``    | Environment                 | environment specifications           |
   |                 |                             | configuration files                  |
   |                 |                             | (e.g., Conda package dependencies)   |
   +-----------------+-----------------------------+--------------------------------------+
   | ``conf/fmt``    | Formatting                  | formatting rules for source files    |
   +-----------------+-----------------------------+--------------------------------------+
   | ``conf/gl``     | Guidelines                  | Guideline check configuration        |
   +-----------------+-----------------------------+--------------------------------------+
   | ``conf/hcg``    | |halcogen|                  | |halcogen| configuration files for   |
   |                 |                             | the Hardware Abstraction Layer       |
   +-----------------+-----------------------------+--------------------------------------+
   | ``conf/spa``    | Static Program Analysis     | configuration and template files for |
   |                 |                             | static program analysis              |
   +-----------------+-----------------------------+--------------------------------------+
   | ``conf/tpl``    | Templates                   | template files for source files      |
   +-----------------+-----------------------------+--------------------------------------+
   | ``conf/unit``   | Unit tests                  | unit test configuration files        |
   +-----------------+-----------------------------+--------------------------------------+

Documentation directory ``docs``
================================

This directory contains the general documentation and links the API
documentation generated by Doxygen.

Hardware directory ``hardware``
===============================

This directory contains the hardware schematics and layouts.

Source directory ``src``
========================

The ``src`` directory is structured into the following parts (see
:numref:`introduction-repository-structure-src-details`).

.. table:: Detailed description of the ``src`` directory
   :name: introduction-repository-structure-src-details
   :widths: grid

   +-----------------+-----------------------------+--------------------------------------+
   | directory       | long name                   | content description                  |
   +=================+=============================+======================================+
   | ``src/app``     | Application                 | sources files for the BMS            |
   |                 |                             | application                          |
   +-----------------+-----------------------------+--------------------------------------+
   | ``src/hal``     | Hardware                    | source files of the Hardware         |
   |                 | Abstraction                 | Abstraction Layer                    |
   |                 | Layer                       |                                      |
   +-----------------+-----------------------------+--------------------------------------+
   | ``src/opt``     | Optional                    | optional source files (e.g.,         |
   |                 |                             | specific cell parameters             |
   +-----------------+-----------------------------+--------------------------------------+
   | ``src/os``      | Operating System            | source files of the Operating        |
   |                 |                             | System                               |
   +-----------------+-----------------------------+--------------------------------------+

Tests directory ``tests``
=========================

The ``tests`` directory is structured into the following parts (see
:numref:`introduction-repository-structure-tests-details`).

.. table:: Detailed description of the ``tests`` directory
   :name: introduction-repository-structure-tests-details
   :widths: grid

   +--------------------+--------------------------+--------------------------------------+
   | directory          | long name                | content description                  |
   +====================+==========================+======================================+
   | ``tests/axivion``  | Axivion                  | configuration files to describe the  |
   |                    |                          | architecture of |foxbms|. Additional |
   |                    |                          | static program analysis              |
   |                    |                          | configurations are stored here       |
   |                    |                          | (e.g., MISRA-C)                      |
   +--------------------+--------------------------+--------------------------------------+
   | ``tests/scripts``  | scripts                  | unit test of scripts (e.g., Python,  |
   |                    |                          | shell)                               |
   +--------------------+--------------------------+--------------------------------------+
   | ``tests/unit``     | unit tests               | unit tests for embedded sources      |
   +--------------------+--------------------------+--------------------------------------+

Tools directory ``tools``
=========================

The ``tools`` directory is structured into the following parts (see
:numref:`introduction-repository-structure-tools-details`).

.. table:: Detailed description of the ``tools`` directory
   :name: introduction-repository-structure-tools-details
   :widths: grid

   +---------------------+----------------------------+--------------------------------------+
   | directory           | long name                  | content description                  |
   +=====================+============================+======================================+
   | ``tools/src``       | Cyclic redundancy check    | Jupyter notebooks to help generate   |
   |                     |                            | CRC-functions                        |
   +---------------------+----------------------------+--------------------------------------+
   | ``tools/debugger``  | Debugger                   | Debugger setups and usage            |
   +---------------------+----------------------------+--------------------------------------+
   | ``tools/gui``       | Graphical User Interface   | GUI to interact with |foxbms| and    |
   |                     |                            | |foxbms| generated data              |
   +---------------------+----------------------------+--------------------------------------+
   | ``tools/ide``       | Editor/Integrated          | Setups for editors and IDE           |
   |                     | Development Environment    |                                      |
   +---------------------+----------------------------+--------------------------------------+
   | ``tools/utils``     | utils                      | unspecific helper tools and scripts  |
   +---------------------+----------------------------+--------------------------------------+
   | ``tools/waf-tools`` | waf tools                  | waf tools that are not part of       |
   |                     |                            | vendored waf                         |
   +---------------------+----------------------------+--------------------------------------+
   | ``tools/waf``       | waf                        | waf binary                           |
   +---------------------+----------------------------+--------------------------------------+
