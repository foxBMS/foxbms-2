.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _BUILDING_THE_APPLICATION:

Building the Application
------------------------

.. note::

   As mentioned in :ref:`FOX_PY` it is recommended to use one of the shell
   scripts to interact with the repository, as these activate the correct build
   environment and prepare the ``PATH`` variable before starting the build
   process.
   If these shell scripts are not used to call the build tool the following
   steps are required:

   - activating the correct environment
   - make all tools available in ``PATH``
   - the current working directory must be the repository root
   - instead of ``.\fox.ps1 waf <some-command>`` use ``python3 tools/waf <some-command>``


#. Open a terminal and change into a checkout of the foxbms repository
#. Configuration of the project

   .. code-block:: powershell

     PS C:\Users\vulpes\Documents\foxbms-2> .\fox.ps1 waf configure

#. Now all build variants are available, e.g., to build the binaries
   (``build_bin``) and the documentation (``build_docs``):

   .. code-block:: powershell

     PS C:\Users\vulpes\Documents\foxbms-2> .\fox.ps1 waf build_bin build_docs

   Waf commands can be concatenated and are executed in the order of their
   appearance.

Important available commands are listed in :numref:`waf-available-commands`.

.. csv-table:: Available build and clean commands
   :name: waf-available-commands
   :widths: 25 75
   :header-rows: 1
   :delim: ;
   :file: ./waf-available-commands.csv

There are additional commands that are not typically needed when developing.
This and other help on the build system is obtained by running

.. code-block:: powershell

   PS C:\Users\vulpes\Documents\foxbms-2> .\fox.ps1 waf --waf-help
