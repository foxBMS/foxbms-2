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

     C:\Users\vulpes\Documents\foxbms-2>waf configure

#. Now all build variants are available, e.g., to build the binaries
   (``build_bin``) and the documentation (``build_docs``):

   .. code-block:: console

     C:\Users\vulpes\Documents\foxbms-2>waf build_bin build_docs

   Waf commands can be concatenated and are executed in the order of their
   appearance.

Important available commands are listed in :numref:`waf-available-commands`.

.. csv-table:: Available build and clean commands
   :name: waf-available-commands
   :widths: 25 75
   :header-rows: 1
   :delim: ;
   :file: ./waf-available-commands.csv

You can profile the build process by running the following command:

.. code-block:: console

   C:\Users\vulpes\Documents\foxbms-2>waf build_bin --profile
   C:\Users\vulpes\Documents\foxbms-2>gprof2dot -f pstats profile.txt | dot -Tpng -o profile.png

There are additional commands that are not typically needed when developing.
This and other help on the build system is obtained by running

.. code-block:: console

   C:\Users\vulpes\Documents\foxbms-2>waf --help
