.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _HOW_TO_BUILD_A_LIBRARY_AND_LINK_IT_IN_A_FOXBMS_2_PROJECT:

How to Build A Library and Link it in a foxBMS 2 Project
========================================================

Sometimes source code can not be shared between parties. For these cases the
|foxbms| project provides a mechanism to ship a minimal development project to
the other party. This allows the partner to build a static library. This
library and the accompanying headers can then be shared with the partner and be
included in the application.

The following example describes the workflow. In this scenario partner **A**
develops on |foxbms| while Part **B** should only provide a library to **A**.

- **A** bootstraps a minimal development project. This minimal project is named
  ``library-project.tar.bz2``.

  .. code-block:: console
    :caption: Bootstrapping the minimal library project

    C:\Users\Partner_A\Documents\foxbms-2>waf.bat bootstrap-library-project

- **A** shares the archive ``library-project.tar.bz2`` with **B**.
- **B** installs a |code-composer-studio| as described in :ref:`css_install`.
- **B** installs a Python environment as described in
  :ref:`miniconda_install_and_configuration`.
- **B** builds a library by adding sources etc. to the minimal project as
  needed and builds the library.

  .. code-block:: console
    :caption: Building the Library

    C:\Users\Partner_B\Documents\foxbms-2>waf.bat build

- **B** shares the library and accompanying headers with **A**.
- **A** saves the library and accompanying headers and adds the path to the
  library to the search path, the library name the list of used libraries and
  the path to the headers to the include path (in ``cc-options.yaml``).
