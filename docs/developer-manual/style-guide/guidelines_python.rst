.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _PYTHON_CODING_GUIDELINES:

Python Coding Guidelines
========================

These coding guidelines **MUST** be applied to all ``Python`` files.

The source files **MUST** be successfully checked by running the ``pre-commit``
check before files can be merged into the master branch of the repository.

The following rules generally apply and follow the naming schema
``PYTHON:<ongoing-number>``.

.. _rule_python_filenames:

Filenames (``PYTHON:001``)
--------------------------

The following rules apply for filenames of Python scripts.

.. admonition:: Python Script Filenames

    - The general file naming rules **MUST** be applied  (see
      :numref:`rule_general_filenames`), except that dashes (``-``) **SHOULD**
      not be used.
    - Python scripts **MUST** use ``.py`` as file extension, except for Waf
      build scripts which use ``wscript`` as file name.

For example the valid file names for Python scripts are

- ``hello.py``
- ``my-script.py`` (**not recommended**, as the script is **not** importable)
- ``my_script.py`` (**recommended**, the script is importable)

.. _rule_python_header:

Header (``PYTHON:002``)
-----------------------

.. admonition:: Python file header

    Python source and header files **MUST** start with the following header:

   .. literalinclude:: ./../../../conf/tpl/python_script.py
      :language: python
      :linenos:
      :lines: 1-38
      :caption: File header for ``.py`` files.
      :name: file-header-py

.. _rule_python_syntax:

Syntax (``PYTHON:003``)
-----------------------

The following rules apply for syntax of Python scripts

.. admonition:: Python syntax rules

    - Python 2 only syntax **MUST NOT** be used.
    - Python 3 only syntax **MAY** be used.
    - Code **MUST** work in Python 3.12 or greater.

.. _rule_python_formatting:

Formatting (``PYTHON:004``)
---------------------------

Uniform formatting makes code easier to read to all developers.

.. admonition:: Python formatting rules

    Python source files are checked for correct formatting by ``ruff``.
    The ``ruff`` configuration can be found in ``pyproject.toml``.

.. _rule_python_spa:

Static program analysis (``PYTHON:005``)
----------------------------------------

Static program analysis helps to detected code smells and errors in an early
stage of development.

.. admonition:: Python static program analysis rules

    Python sources files are statically checked by ``pylint``.
    The ``pylint`` configuration can be found in ``pyproject.toml``.

.. _rule_no_platform_specific_code:

No platform specific code (``PYTHON:006``)
------------------------------------------

.. admonition:: No platform specific code

    Python scripts **MUST** use platform independent code where ever possible.
    If platform specific is required, it **MUST** be guarded.

Example :numref:`platform-specific-code` shows how to write platform acceptable
platform specific code.

.. literalinclude:: ./examples/python-006.py
    :language: python
    :linenos:
    :lines: 44-
    :caption: Handling of platform specific code
    :name: platform-specific-code

.. _rule_python_wscript_rules:

``wscript`` Specific rules (``PYTHON:007``)
-------------------------------------------

.. admonition:: Paths in ``wscript``

   - In a ``wscript`` paths that are meant to be used by task generators
     **MUST NOT** be ``path.Path`` objects, but strings using ``/`` as path
     separator.
   - ``includes`` and ``source`` or *might* be less readable if they are split
     over multiple lines (as ``ruff`` would format it like this on default).
     Therefore ``includes`` and ``source`` **MAY** be written as shown below,
     if it increases readability:

.. literalinclude:: ./examples/python-007.py
   :language: python
   :linenos:
   :start-after: start-include-in-docs
   :caption: Format ``includes`` in ``wscript``
   :name: format-includes-in-wscript

- ``# fmt: off`` disables formatting starting from that line and
  ``# pylint: disable=line-too-long`` disables the pylint error message
  starting from that line.
- ``# fmt: on`` re-activates on formatting starting from that line and
  ``# pylint: enable=line-too-long`` re-activates the pylint error message
  starting from that line.

.. _rule_python_typing:

Typing (``PYTHON:008``)
-----------------------

Typing information helps to detected errors in an early stage of development.

.. admonition:: Typing information

    Python scripts **SHALL** typing information where ever possible.
