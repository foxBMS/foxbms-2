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

Supporting tools for software analysis **SHOULD** be ISO 9001 qualified. The
following supporting tool classes are required to be used:

- version control system
- compiler
- text editor
- build automation system
- unit testing framework
- static program analysis tools
- documentation tools
- test runner

The following tools are shown in :numref:`mapping-of-third-party-tools`.

.. table:: Third Party Tools
   :name: mapping-of-third-party-tools
   :widths: grid

   +-----------------+-------------------------+-------------+----------------------------------------+
   | Tool            | Class                   | Criticality | Criticality comment                    |
   +=================+=========================+=============+========================================+
   | |git|           | version control system  | T3          |                                        |
   +-----------------+-------------------------+-------------+----------------------------------------+
   | TI ARM CGT      | compiler                | T3          | used to build code for the target      |
   +-----------------+-------------------------+-------------+----------------------------------------+
   | TI ARM CGT      | linker                  | T3          | used to link code for the target       |
   +-----------------+-------------------------+-------------+----------------------------------------+
   | GCC             | compiler                | T2          | used to build and link code            |
   |                 |                         |             | for unit tests                         |
   +-----------------+-------------------------+-------------+----------------------------------------+
   | VS Code         | text editor             | T1          |                                        |
   +-----------------+-------------------------+-------------+----------------------------------------+
   | waf             | build automation system | T3          | controls build process; generates      |
   |                 |                         |             | version dependent code                 |
   +-----------------+-------------------------+-------------+----------------------------------------+
   | Axivion Suite   | static program          | T2          |                                        |
   |                 | analysis tools          |             |                                        |
   +-----------------+-------------------------+-------------+----------------------------------------+
   | Cppcheck        | static program          | T1          | just a helper for the static analysis; |
   |                 | analysis tools          |             | Axivion is used as the main tool       |
   +-----------------+-------------------------+-------------+----------------------------------------+
   | sphinx          | documentation tools     | T1          | just a rendering tool for              |
   |                 |                         |             | the documentation text files           |
   +-----------------+-------------------------+-------------+----------------------------------------+
   | ceedling        | unit testing framework  | T2          |                                        |
   +-----------------+-------------------------+-------------+----------------------------------------+
   | *Custom HIL     | test runner             | T3          |                                        |
   | implementation* |                         |             |                                        |
   +-----------------+-------------------------+-------------+----------------------------------------+

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

Programming and scripting languages that support the projects objective
**MUST** be chosen from the following list in order to limit the number of
external dependencies:

- Python 3
- Ruby 2.7
- shell scripts (for ``cmd.exe`` or ``bash``)
