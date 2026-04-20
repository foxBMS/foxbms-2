.. include:: ./../../../macros.txt
.. include:: ./../../../units.txt

.. _FOX_DB:

db
==

The ``db`` module implements an interface for retrieving cell specifications
and model parameters.
Data are read from a standardized cell database.
The following sections explain how to access the information via the command-line
tool and programmatically, and how to define the database format.

Usage
-----

.. important::

  The ``db`` module validates input data and exits with an error message
  if types or values are invalid, if linked sources are missing, or if
  duplicate cell identifiers are found.

The ``db`` module supports two main commands:

``db list``: Lists all available cells in the database.

.. tabs::

   .. group-tab:: Win32/PowerShell

      .. code-block:: powershell

        .\fox.ps1 db list \path\to\db-root

   .. group-tab:: Win32/Git Bash

      .. code-block:: shell

        ./fox.sh db list /path/to/db-root

   .. group-tab:: Linux

      .. code-block:: shell

        ./fox.sh db list /path/to/db-root


``db show``: Shows detailed information for a single cell by its identifier.

.. tabs::

   .. group-tab:: Win32/PowerShell

      .. code-block:: powershell

        .\fox.ps1 db show \path\to\db-root --cell-id "CELL-ID"

   .. group-tab:: Win32/Git Bash

      .. code-block:: shell

        ./fox.sh db show /path/to/db-root --cell-id "CELL-ID"

   .. group-tab:: Linux

      .. code-block:: shell

        ./fox.sh db show /path/to/db-root --cell-id "CELL-ID"

.. note::

  The database root (``db-root``) can be:

    - A directory containing subdirectories (one per cell), or
    - A ZIP archive containing the same directory structure.

.. note::

  The cell identifier is composed of ``<manufacturer>-<name>``.

Download
--------

To download the latest |foxbms| cell database and save it in your current
working directory, use:

.. tabs::

   .. group-tab:: Win32/PowerShell

      .. code-block:: powershell

        curl "ftp://myftpsite/foxDB_latest.zip"

   .. group-tab:: Win32/Git Bash

      .. code-block:: shell

        curl "ftp://myftpsite/foxDB_latest.zip"

   .. group-tab:: Linux

      .. code-block:: shell

        curl "ftp://myftpsite/foxDB_latest.zip"

The naming pattern for the |foxbms| cell database is ``foxDB-<version>.zip``,
with ``<version>`` following Semantic Versioning (see
`versioning <https://semver.org/>`_), or ``latest`` to indicate the most recent
version of the cell database.

Database Format
---------------

The cell database is a (zipped) directory containing subdirectories for each
cell.
Each cell directory **must** contain the following structure:

| cell-id
| ├── sources
| │ ├── example_measurement_data.parquet
| │ ├── ...
| │ └── example_source_parameter.npy
| ├── cell_spec.json
| ├── cell_datasheet.pdf
| ├── model_parameter.json
| └── README.md

Sources
*******

Place measurement data and complex model parameters in matrix form in the
``sources`` directory. These files are referenced from the corresponding sections
in ``model_parameter.json``.
No mandatory file formats are specified for sources, but ``.parquet`` (for
measurement data) and ``.npy`` (for matrices) are recommended.

.. warning::

  The CLI exits with an error message if a linked source cannot be found.

Root
****

The root of the cell directory **must** contain at least the following four files:

- ``cell_spec.json``: Specifications of the cell derived from ``cell_datasheet.pdf``.
- ``cell_datasheet.pdf``: Specifications provided by the manufacturer.
- ``model_parameter.json``: Model parameters (e.g., current limits).
- ``README.md``: Detailed information about all supported models, the parameters they use and model sources.

Cell Specification
******************

Each cell must provide a ``cell_spec.json`` with the following fields:

.. csv-table:: Mandatory Cell Specifications
   :file: examples/cell_spec.csv
   :widths: 50 50 50 50
   :header-rows: 1
   :align: center

.. literalinclude:: examples/cell_spec.json
    :language: json
    :caption: Example ``cell_spec.json``

.. important::

  All values in the cell specifications **must** be in SI base units.

Model Parameters
****************

The ``model_parameter.json`` defines the parameters of each model attached
to a cell.
Parameters are provided as dictionaries containing the mandatory keys
``name`` and ``sources``.
Each dictionary is an element of a list of model parameters.
Below is an example of parameters of a ``"current limits"`` model, which
describes charge and discharge current limits as a function of temperature.

.. literalinclude:: examples/model_parameter.json
    :language: json
    :caption: Example ``model_parameter.json``

API
---

The ``db`` module provides, for each cell, a ``Cell`` dataclass with two attributes:

- ``cell_spec``: The specifications of the cell as a dataclass (``CellSpec``).
- ``model_parameters``: A list of dataclasses (``BaseModel``) containing the parameters of each available model.

To obtain a ``Cell`` dataclass for a specific cell in the database and use it
in another tool:

1. Import the ``cli.db.FoxDB`` class.
2. Create a ``FoxDB`` instance with a valid database root.
3. Access ``FoxDB.cells``, which is a list of ``Cell`` instances.

.. literalinclude:: examples/api_example.py
    :language: python
    :caption: Example Usage of the FoxDB API
    :lines: 40-56

Adding Cells and Models
-----------------------

New cells can be added by creating an additional cell directory in the database
root directory with the required structure described above.
To add new models (and their parameters), add a new |python| file to
``cli/db/model_parameter``.
This |python| file **must** define one or more dataclasses containing the model
parameters, and the top-level dataclass **must** inherit from ``BaseModel`` in
``cli/db/model_parameter/__init__.py``.
It is recommended to use the ``__post_init__`` method to validate each
attribute of the dataclass.
As last step, in the ``cli/db/setup.py`` the ``creates_models`` method **must**
be modified to add the new model parameters to the ``Cell`` instances.

Validation
----------

To validate a database directory or archive, run ``db list``.
It will parse all cells and report errors for invalid types/values, missing
linked sources, or duplicate cell identifiers.

Architecture
------------

.. drawio-figure:: fox-cli-db-architecture.drawio
   :format: svg
   :alt: Class diagram of db module
   :align: center
