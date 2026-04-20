.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _DEVELOPING_FOX_CLI:

####################
Developing |fox-cli|
####################

The |fox-cli| consists of three parts:

- ``cli/*``: the actual |fox-cli| implementation
- a two layer wrapper:

  - ``fox.ps1``\/``fox.sh``: a wrapper to support different shells
  - ``fox.py``: a Python wrapper as entrance point when the |fox-cli| is
    used in the repository (called by the shell wrappers).

The |fox-cli-package| only contains part of the |fox-cli| implementation and
provides ``fox-cli`` as an entry point, which can be used similarly to the
|fox-cli| wrappers.

.. toctree::
    :maxdepth: 1

    ./fox-cli-wrapper.rst
    ./add-cli-cmds.rst
    ./fox-cli-api.rst

|fox-cli| Fundamentals
######################

The |foxbms| repository is large and offers a lot of different things to do.
These things shall be explorable and usable.
One thing to enable this is documentation, but a simple interface directly
embedded into the repository simplifies this furthermore and enables a good
interaction with the project.
That interface is the |fox-cli|.

The |fox-cli| shall e.g., enable

- building embedded binaries
- updating the BMS software
- analyzing and plotting BMS data

and many more things.
All this

- shall work in a terminal,
- shall be accessible in a GUI,
- shall be cross platform (Windows and Linux),
- shall not rely on environment variables,
- shall not require any setup of a terminal.

Getting all these different requirements to work, required some implementations
that at first glance are not immediately clear and some of the reasoning
shall be explained in the following, so that - in combination with the source
code - the implementation is understandable and comprehensible.

This is a lose list, that makes most sense to read and understand when also
looking at the |fox-cli| implementation.

Standard Streams
================

PowerShell and Bash handle standard streams differently.
Depending on the specific command behavior, ``fox.ps1`` handles ``stdin``
differently to Bash (i.e., this PowerShell and Windows only).
This affects the base commands ``bms`` and ``install``.

GUI
===

.. note::

  The GUI is currently not supported on Linux.

The GUI can be started through ``fox.ps1`` and ``fox.sh``.
On Windows, there is ``pythonw`` to start Python GUI applications.
``pythonw`` does not support standard streams and this behavior needs to be
reflected in some implementations, e.g., the GUI.

The standard way of using the GUI shall be to **not** block the terminal.
For that reason the default is to start the GUI using ``pythonw``, as this just
starts Python and exits.
However, when debugging, the standard streams must be available.
The correct interpreter is determined based on the provided options
after the base command ``gui``
(these are ``-h``, ``--help`` and ``--debug-gui``) by the terminal wrappers.

Some commands that can be run in the GUI require ``stdout``\/``stderr`` to be
available, which of course would contradict the usage of ``pythonw``.
To overcome this issue for such commands, the GUI uses subprocesses and
redirects ``stdout``\/``stderr`` to file streams that are then read back to the
GUI output.
To make this consistently work, all ``stdout``\/``stderr`` output must be
written through ``echo``, ``recho``, and ``secho`` provided by
``cli/helpers/click_helpers.py``.
Only ``cli/helpers/python_setup.py`` shall use ``print``.

There is a wrapper provided for starting the GUI on Windows (``gui.lnk``),
which is a shortcut to run ``fox.ps1 gui``.


Dependencies
============

In case Python or the Python development environment are not available, the
shell wrappers should guide the user on what to do, in order to get a working
setup.

This includes:

- Check whether the activate script for the virtual environment exists
- If so, activate it, ensure that it worked, and run the provided command
- If not, check which part is missing

  - Generally Python
  - Specifically Python 3.12
  - Pinned Python 3.12 development environment

- This check is always run when using the wrappers ``fox.ps1`` and ``fox.sh``.
  When only the pinned Python development environment is missing, the user can
  directly install it through the wrapper in an interactive mode.
  On Windows, this check and interactive installation is also implemented for
  the GUI, i.e., when running ``gui.lnk``, but if the aforementioned
  dependencies are missing graphical error messages or installation
  confirmations are provided.

For a deeper understanding see ``fox.ps1``, ``fox.sh``, and
``cli/helpers/python_setup.py``, as well as
:numref:`block-diagram-fox-cli-ps1-wrapper`.

|fox-cli-package|
=================

This sections only covers information for developers.
More general information on the package can be found in the following sections:

- :ref:`INSTALL_FOX_CLI_PACKAGE`
- :ref:`FOX_CLI_PACKAGE`

Configuration
*************

The |fox-cli-package| is built using
`hatchling <https://hatch.pypa.io/latest/>`_.
All configuration is managed via the ``pyproject.toml`` and ``hatch_build.py``
files.

Some files required to run commands are not part of the |fox-cli|.
These files are included in the ``package_data`` directory.

The file ``cli/foxbms_version.py`` typically reads the ``wscript`` located at
the repository root to determine the |foxbms| version.
However, since hatchling requires a strictly numerical version number, which
the |foxbms| version does not always provide, the file ``version.py`` is
created at the start of the package build.
This file only contains the numerical version number.
When using the |fox-cli-package|, ``foxbms_version.py`` reads the version
information from this generated ``version.py`` file.

Package Structure
*****************

The |fox-cli-package| is structured as shown in :numref:`package-dir-structure` and
the files shown in :numref:`package-file-structure` are essential for the
|fox-cli| to function properly within the package.

.. csv-table:: Content description of the |fox-cli-package| directories
  :file: ./pkg-dir.csv
  :header-rows: 1
  :delim: ;
  :widths: 20 20 70
  :name: package-dir-structure

.. csv-table:: Content description of relevant |fox-cli-package| files
  :file: ./pkg-files.csv
  :header-rows: 1
  :delim: ;
  :widths: 20 50
  :name: package-file-structure

How to use the |fox-cli-package|?
*********************************

The |fox-cli-package| can be used both from the command line, with the command
``fox-cli``, and directly in Python code, where the import-name is ``fox_cli``.
An example for using the |fox-cli-package| in Python code is given in
:numref:`example-usage`, where the class ``ComInterface`` is used.

.. code-block:: python
  :caption: Implementation of a new communication interface
  :name: example-usage

  from fox_cli.com.com_interface import ComInterface, ProcessInterface

  class NewCom(ComInterface):
    """New communication interface using 'ComInterface' from the package."""

    def __init__(self, name) -> None:
      super().__init__(name)
      # ...
