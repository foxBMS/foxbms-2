.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _RESTRUCTUREDTEXT_CODING_GUIDELINES:

reStructuredText Coding Guidelines
==================================

These coding guidelines **MUST** be applied to all ``reStructuredText`` source
files.

The following rules generally apply and follow the naming schema
``RST:<ongoing-number>``.

.. _rule_rst_filenames:

Filenames (``RST:001``)
-----------------------

Additional to the general file naming rules the following **MUST**
be applied.

.. admonition:: File name rules

    - ``reStructuredText`` source files **MUST** use ``.rst`` as file
      extension.

For example the valid file names for a reStructuredText sources are

- ``macros.txt``
- ``software-installation.rst``

.. _rule_rst_linelength:

Linelength (``RST:002``)
------------------------

Each line of text in your code **SHOULD** be at most 80
characters long.
A line **MAY** exceed 80 characters if it is

- a comment line which is not feasible to split without harming readability,
  ease of cut and paste or auto-linking, e.g., if a line contains an example
  command or a literal URL longer than 120 characters or
- a raw-string literal with content that exceeds 120 characters.
  Except for test code, such literals should appear near the top of a file.

.. _rule_rst_include:

Include (``RST:003``)
----------------------

Include macros to have consistent style for repetitive words.

.. admonition:: Include rules

    - Macros **SHOULD** be used where ever it is possible within the build
      toolchain.
    - ``macros.txt`` **MUST** be included where ever possible
    - File local macros **MAY** be used. However, if a term is used in more
      than one file, the macro **MUST** be transferred to ``macros.txt``.

.. _rule_rst_links:

Links (``RST:004``)
-------------------

Links **MUST** be checked for availability.
This is ensured by running sphinx in ``linkcheck`` mode.

.. admonition:: Links rules

    - All links **MUST** be reachable.
    - Separated link and target definition **SHOULD** be used.
      If link and target definition are separated the target **MUST** be
      defined at the end of the file after two blank lines.

Examples:

.. code-block:: rst
   :linenos:
   :emphasize-lines: 1,6,7
   :caption: description of external links in rst-files
   :name: rst-external-links

    |foxbms| is developed by the `Battery Systems Group`_ at `Fraunhofer IISB`_.

    more documentation
    even more documentation

    .. Battery Systems Group: https://www.iisb.fraunhofer.de/en/research_areas/intelligent_energy_systems/stationary_battery_systems.html
    .. Fraunhofer IISB: https://www.iisb.fraunhofer.de

.. _rule_rst_headings:

Headings (``RST:005``)
----------------------

We follow the convention of the Python Developer's Guide for
`Documenting Python <https://devguide.python.org/documenting/#sections>`_. Use
the following rules to create headings:

- # with overline, for parts
- \* with overline, for chapters
- =, for sections
- -, for subsections
- ^, for subsubsections
- ", for paragraphs

.. admonition:: heading

    - all ``reStructuredText`` files **MUST** have a heading
    - the heading **MUST** be two lines after the file label
    - the heading **MUST** be underlined with =

.. code-block:: rst
   :linenos:
   :emphasize-lines: 3,5,6
   :caption: heading of guidelines_rst.rst
   :name: rst-heading

    .. include:: ./../../macros.txt

    .. _RESTRUCTUREDTEXT_CODING_GUIDELINES:

    reStructuredText Coding Guidelines
    ==================================

.. _rule_rst_orphan:

Orphan (``RST:006``)
--------------------

.. admonition:: orphan

    - ``reStructuredText`` files which are not included in other ``.rst`` files
      **MUST** start with :orphan:

.. _rule_rst_sentence:

Sentence (``RST:006``)
----------------------

.. admonition:: Sentence

    - Every new sentence must start on a new line.

File Templates
--------------

This file template below show how these rules are correctly applied.
It **SHOULD** be used as basis for new files.

- reStructuredText file :download:`rst.rst <../../../conf/tpl/rst.rst>`
