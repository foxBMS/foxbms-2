.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _BUILDING_THE_APPLICATION:

Building the Application
------------------------

.. note::

   There are shell scripts ``waf.bat``/``waf.sh`` that activate the correct
   build environment before starting the build process. It is recommended to
   use these scripts to interact with the build system.
   If these shell scripts are not used to call the build tool the following
   steps are required:

   - activating the correct environment
   - the current working directory must be the repository root
   - instead of ``waf <some-command>`` use ``python3 tools/waf <some-command>``

   The following examples are using the shell script ``waf.bat``

#. Open a terminal and change into a checkout of the foxbms repository
#. Configuration of the project

   .. code-block:: console

     C:\Users\vulpes\Documents\foxbms-on-tms570>waf configure

#. Now all build variants are available, e.g., to build the binaries
   (``build_bin``) and the documentation (``build_docs``):

   .. code-block:: console

     C:\Users\vulpes\Documents\foxbms-on-tms570>waf build_bin build_docs

   Waf commands can be concatenated and are executed in the order of their
   appearance.

Important available commands are listed in :numref:`waf-available-commands`.

.. table:: Available build and clean commands
   :name: waf-available-commands
   :widths: grid

   +-----------------------------+------------------------------------------------------------------------------------+
   | Command/Option              | Description                                                                        |
   +=============================+====================================================================================+
   | ``build_all``               | Shortcut to run all available build commands                                       |
   +-----------------------------+------------------------------------------------------------------------------------+
   | ``build_bin``               | Builds the binaries                                                                |
   +-----------------------------+------------------------------------------------------------------------------------+
   | ``build_docs``              | Builds the documentation                                                           |
   +-----------------------------+------------------------------------------------------------------------------------+
   | ``build_static_analysis``   | Runs a static code analysis on all C sources that are not third party sources      |
   +-----------------------------+------------------------------------------------------------------------------------+
   | ``build_unit_test``         | Builds and runs all unit tests for the embedded sources                            |
   +-----------------------------+------------------------------------------------------------------------------------+
   | ``clean_all``               | Shortcut to run all available clean commands                                       |
   +-----------------------------+------------------------------------------------------------------------------------+
   | ``clean_bin``               | Cleans all binaries                                                                |
   +-----------------------------+------------------------------------------------------------------------------------+
   | ``clean_docs``              | Cleans the rendered documentation                                                  |
   +-----------------------------+------------------------------------------------------------------------------------+
   | ``clean_pylint``            | Cleans all output files written during ``build_pylint``                            |
   +-----------------------------+------------------------------------------------------------------------------------+
   | ``clean_static_analysis``   | Cleans all output files written during ``build_static_analysis``                   |
   +-----------------------------+------------------------------------------------------------------------------------+
   | ``clean_unit_test``         | Cleans all output files written during ``build_unit_test``                         |
   +-----------------------------+------------------------------------------------------------------------------------+

You can profile the build process by running the following command:

.. code-block:: console

   C:\Users\vulpes\Documents\foxbms-2>waf build_bin --profile
   C:\Users\vulpes\Documents\foxbms-2>gprof2dot -f pstats profile.txt | dot -Tpng -o profile.png

There are additional commands that are not typically needed when developing. This and other help on the build system is
obtained by running

.. code-block:: console

   C:\Users\vulpes\Documents\foxbms-2>waf --help
