.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _PYTHON_CODING_GUIDELINES:

Python Coding Guidelines
========================

These coding guidelines **MUST** be applied to all ``Python`` files.

The source files **MUST** be successfully checked by running the waf command
``check_guidelines`` before files can be merged into the master branch of the
repository.

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
    - Python scripts **MUST** use ``.py`` as file extension, except for waf
      build scripts which use ``wscript`` as file name.

For example the valid file names for batch scripts are

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
    - Code **MUST** work in Python 3.6 or greater.

.. _rule_python_formatting:

Formatting (``PYTHON:004``)
---------------------------

Uniform formatting makes code easier to read to all developers.

.. admonition:: Python formatting rules

    Python source files are checked for correct formatting by ``black``. The
    ``black`` configuration can be found in ``conf/fmt/pyproject.toml``.

.. _rule_python_spa:

Static program analysis (``PYTHON:005``)
----------------------------------------

Static program Analysis helps to detected code smells and errors in an early
stage of development.

.. admonition:: Python static program analysis rules

    Python sources files are statically checked by ``pylint``. The ``pylint``
    configuration can be found in ``conf/spa/.pylintrc``.

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

``includes`` and ``source`` or might be less readable if they are split over
multiple lines (as ``black`` would format it like this on default). Therefore
``includes`` and ``source`` might be written as follows, if it increases
readability:

.. literalinclude:: ./examples/python-007.py
    :language: python
    :linenos:
    :lines: 44-
    :caption: Format ``includes`` in ``wscript``
    :name: format-includes-in-wscript

- ``# fmt: off`` disables black on formatting starting from that line and
  ``# pylint: disable=line-too-long`` disables the pylint error message
  starting from that line.
- ``# fmt: on`` re-activates black on formatting starting from that line and
  ``# pylint: enable=line-too-long`` re-activates the pylint error message
  starting from that line.
