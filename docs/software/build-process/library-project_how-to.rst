.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _HOW_TO_BUILD_A_LIBRARY_AND_LINK_IT_IN_A_FOXBMS_2_PROJECT:

How to Build a Library and Link it in a foxBMS 2 Project
========================================================

Sometimes source code can not be shared between parties.
For these cases the |foxbms| project provides a mechanism to ship a minimal
development project to the other party.
This allows the partner to build a static library.
This library and the accompanying headers can then be shared with the partner
and be included in the application.

The following example describes the workflow. In this scenario **Partner A**
develops on |foxbms| while **Partner B** should only provide a library to
**Partner A**.

Bootstrapping a minimal development Project
-------------------------------------------

- **Partner A** bootstraps a minimal development project.
  This minimal project is named ``library-project.tar.gz``.

  .. tabs::

     .. group-tab:: Win32/PowerShell

        .. code-block:: powershell

           .\fox.ps1 waf bootstrap-library-project

     .. group-tab:: Win32/Git bash

        .. code-block:: shell

           ./fox.ps1 waf bootstrap-library-project

     .. group-tab:: Linux

        .. code-block:: shell

           ./fox.ps1 waf bootstrap-library-project

- **Partner A** shares the archive ``library-project.tar.gz`` with
  **Partner B**.

Building a Library
-------------------

- **Partner B** installs a |ti-code-composer-studio| as described in
  :ref:`css_install`.
- **Partner B** installs a Python environment as described in
  :ref:`SOFTWARE_INSTALLATION`.
- **Partner B** builds a library by adding sources etc. to the minimal project
  as needed and builds the library.

  .. tabs::

     .. group-tab:: Win32/PowerShell

        .. code-block:: powershell

           .\fox.ps1 waf build

     .. group-tab:: Win32/Git bash

        .. code-block:: shell

           ./fox.ps1 waf build

     .. group-tab:: Linux

        .. code-block:: shell

           ./fox.ps1 waf build

- **Partner B** shares the library and accompanying headers with **Partner A**.

Including the Library
---------------------

- **Partner A** saves the library and accompanying headers and adds the path to
  the library to the search path, the library name the list of used libraries
  and the path to the headers to the include path (in ``cc-options.yaml``).

- For including the library the library path has to be updated:
   .. code-block:: powershell

      LIBRARY_PATHS:
         win32:
         - ..\..\src\app\driver\<library folder name>
         linux:

- In the section libraries the name of the library has to be added:
   .. code-block:: powershell

      LIBRARIES:
         ST: # libraries following the pattern "lib<name>.a"
         TARGET: # libraries following the pattern "<name>.lib"
         - <name>

- For the changes to take effect execute:
  ./fox.ps1 waf configure
- Now the project should build with:
  ./fox.ps1 waf build_app_embedded
