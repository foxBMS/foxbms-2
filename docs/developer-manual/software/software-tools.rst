.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _SOFTWARE_TOOLS:

Software Tools
==============

General
-------

At every commit of the development the repository strictly defines the tools
and their versions that **MUST** be used when working on the project.

All developers **MUST** use exactly these tools and versions.

The continuous integration test runners **MUST** use exactly these tools and
versions.

The tools and versions that are used **MUST** be defined in the documentation.

Third Party Tools
-----------------

Supporting tools for software analysis **SHOULD** be ISO 9001 qualified.
The following supporting tool classes are required to be used:

- version control system
- compiler
- text editor
- build automation system
- unit testing framework
- static program analysis tools
- documentation tools
- test runner

The following tools are shown in
:numref:`criticality-mapping-of-third-party-tools`.

.. csv-table:: Criticality mapping of the third party tools
   :name: criticality-mapping-of-third-party-tools
   :delim: ;
   :file: ./criticality-mapping-of-third-party-tools.csv

Repository Inherent Tools
-------------------------

General
^^^^^^^

Tools that are specifically developed to support this project **MUST** be
included in the repository.

These tools **MUST** be checked by language appropriate checkers:

- linters to reduce the presence of programmatically detectable error (if
  available)
- source code formatters to simplify code reviews (if available)

Unit tests **MUST** be implemented for these tools.

.. _supporting_languages:

Supporting Programming and Scripting Languages
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Supporting programming and scripting languages that support the projects
objective **MUST** be chosen from the following list in order to limit the
number of external dependencies:

- Python 3 (preferred)
- Ruby 3
- shell scripts (for ``pwsh`` or ``bash``)
