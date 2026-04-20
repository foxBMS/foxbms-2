.. include:: ./../../../macros.txt

.. _FOX_INSTALL:

install
=======

The install tool can

* check the installation status of the software,
* return the location of the installation instructions, and
* install the |fox-cli| in a local |virtual-python-environment| as a
  package.

Usage
-----

The install command has two sets of options that each provide specific
functionalities.

.. include:: ./../../../../build/docs/fox_install_help.txt

The default options for each set are ``--no-check`` and ``--no-local``.
If a different functionality is intended, the corresponding option must be
specified as described below.

- Get the location of the installation instructions:

  .. tabs::

    .. group-tab:: Windows

        .. tabs::

          .. group-tab:: PowerShell

              .. code-block:: PowerShell

                .\fox.ps1 install

          .. group-tab:: Git bash

              .. code-block:: shell

                ./fox.sh install

    .. group-tab:: Linux

        .. code-block:: shell

          ./fox.sh install

- Check the installation status of the software:

  .. attention::

    If ``--check`` is specified as an option, the install tool will only check
    the installation status and ignore any other options.

  .. tabs::

    .. group-tab:: Windows

        .. tabs::

          .. group-tab:: PowerShell

              .. code-block:: PowerShell

                .\fox.ps1 install --check

          .. group-tab:: Git bash

              .. code-block:: shell

                ./fox.sh install --check

    .. group-tab:: Linux

        .. code-block:: shell

          ./fox.sh install --check

- Install the |fox-cli-package| in a local |virtual-python-environment|:

  .. note::

    See :ref:`FOX_CLI_PACKAGE` for information on the |fox-cli-package|.
    Instead of activating the |virtual-python-environment| as indicated there,
    open the terminal with the shortcut created by the install tool.

  .. tabs::

    .. group-tab:: Windows

        .. tabs::

          .. group-tab:: PowerShell

              .. code-block:: PowerShell

                .\fox.ps1 install --local

          .. group-tab:: Git bash

              .. code-block:: shell

                ./fox.sh install --local

    .. group-tab:: Linux

        .. code-block:: shell

          ./fox.sh install --local

  Running this command creates a |virtual-python-environment| in the
  directory ``C:\foxbms\envs\local\<foxbms-version-number>``
  (``/opt/foxbms/envs/local/<foxbms-version-number>`` for Linux).
  Additionally, two shortcuts are created in the directory ``foxbms-2`` in the
  Start Menu, named ``fox CLI - <foxbms-version-number>`` and
  ``fox CLI GUI - <foxbms-version-number>``.
  The former opens a terminal with the |virtual-python-environment| already
  activated, and the latter opens the GUI.
