.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _HOW_TO_BUILD_A_LIBRARY_AND_LINK_IT_IN_A_FOXBMS_2_PROJECT:

How to Build a Library and Link it in a |foxbms| Project
========================================================

Sometimes source code can not be shared between parties.
For these cases the |foxbms| project provides a mechanism to ship a minimal
development project to the other party.
This allows the partner to build a static library.
This library and the accompanying headers can then be shared with the partner
and be included in the application.

The following example describes the workflow.
In this scenario **Partner A** develops on |foxbms| while **Partner B** should
only provide a library to **Partner A**.

Bootstrapping a minimal development Project
-------------------------------------------

- **Partner A** bootstraps a minimal development project.
  This minimal project is named ``library-project.tar.gz``.

  .. tabs::

     .. group-tab:: Win32/PowerShell

        .. code-block:: powershell

           .\fox.ps1 waf configure
           .\fox.ps1 waf bootstrap_library_project

     .. group-tab:: Win32/Git bash

        .. code-block:: shell

           ./fox.sh waf configure
           ./fox.sh waf bootstrap_library_project

     .. group-tab:: Linux

        .. code-block:: shell

           ./fox.sh waf configure
           ./fox.sh waf bootstrap_library_project

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
- **Partner B** extract the archive
- **Partner B** builds the library

  .. tabs::

     .. group-tab:: Win32/PowerShell

        .. code-block:: powershell

           .\fox.ps1 waf configure
           .\fox.ps1 waf build

     .. group-tab:: Win32/Git bash

        .. code-block:: shell

           ./fox.sh waf configure
           ./fox.sh waf build

     .. group-tab:: Linux

        .. code-block:: shell

           ./fox.sh waf configure
           ./fox.sh waf build

- **Partner B** shares the library and accompanying headers with **Partner A**.

Including the Library
---------------------

- **Partner A** copies the provided library and headers to a suitable location.
- **Partner A** updates the include path for headers and search path for
  libraries in the build configuration by updating ``INCLUDES`` and
  ``STLIBPATH``.
- **Partner A** adds a configuration step that make the library available for
  linking by using
  ``conf.check_cc(stlib=<lib-name>", uselib_store="<reference>")``.
- **Partner A** can now use functions etc. from the library by including the
  headers and linking against the library by adding ``<reference>`` to the
  ``use`` parameter in the build step that requires the library.
- **Partner A** configures and builds the application as usual.

  .. tabs::

     .. group-tab:: Win32/PowerShell

        .. code-block:: powershell

           .\fox.ps1 waf configure
           .\fox.ps1 waf build_app_ti_arm_cgt

     .. group-tab:: Win32/Git bash

        .. code-block:: shell

           ./fox.sh waf configure
           ./fox.sh waf build_app_ti_arm_cgt

     .. group-tab:: Linux

        .. code-block:: shell

           ./fox.sh waf configure
           ./fox.sh waf build_app_ti_arm_cgt

- If the build is successful, the external library is correctly linked into
  the |foxbms| application.

A working minimal example of a library and its integration into the project can
be found in ``tests/variants/lib-build``.
