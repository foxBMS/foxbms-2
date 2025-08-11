.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _DEVELOPING_FOX_CLI:

####################
Developing |fox-cli|
####################

|fox-cli| is primary a tool to interact with the |foxbms| repository through
the command line.
All commands must therefore be implemented as a part of it.
The |fox-cli| implementation is found in the ``cli/``-directory at the root
of the repository.

*********************
Directory Description
*********************

Directories
-----------

.. csv-table:: Directory description of the ``cli`` directory
   :file: ./cli-dir.csv
   :header-rows: 1
   :delim: ;
   :widths: 20 20 60

Files
-----

- ``cli/cli.py``: registers all commands.
- ``cli/foxbms_version.py``: Reads the |foxbms| version information from the
  single source of truth for the version information, the ``wscript`` at the root
  of the repository.

*******************************
How to implement a new command?
*******************************

When a new tool, i.e., a command needs to be implemented the following steps
need to be done (exemplary new command ``my-command``):

- Add a new file ``cli/cmd_my_command/__init__.py``
- Add a new file ``cli/cmd_my_command/my_command_impl.py`` which implements the
  main entrance function of this command.

- Add a new file ``cli/commands/c_my_command.py`` that implements the command
  line interface of the tool.

  .. code-block:: python

   import click

   from ..cmd_my_command import my_command_impl

   # Add the new CLI commands afterwards using click, e.g., as follows:
   @click.command("my-command")
   @click.pass_context
   def run_my_command(ctx: click.Context) -> None:
       """Add help message here"""
       # my_command_impl.do_something must return a `SubprocessResult` object
       # pass the CLI arguments to `do_something` if additional arguments
       # and/or options are needed
       ret = my_command_impl.do_something()
       ctx.exit(ret.returncode)

- In ``cli/cli.py`` add the new command to the cli:

  .. code-block:: python

     # add import of the command
     from .commands.c_my_command import my_command
     # add the command to the application
     # ...
     main.add_command(my_command)
     # ...

Adhere to the following rules when using ``click``:

- Always import click as ``import click`` and do not use
  ``from click import ...``, except when you are

  - only using ``echo`` and/or ``secho`` or
  - are testing click application using ``CliRunner``.
- Use ``cli/helpers/click_helpers.py`` for common click options (except for CAN,
  see next point)
- When the tool uses CAN communication use the ``cli/helpers/fcan.py`` to
  handle the CLI arguments.
- When using a verbosity flag, ``@verbosity_option`` **SHALL** be the second
  last decorator.
- ``@click.pass_context`` **SHALL** be the last decorator.

***********************************
## How to add a command to the GUI?
***********************************

The command that shall be added is `my-command` from the example above.

- Add a new file ``cmd_gui/frame_my_command/__init__.py``
- Add a new file ``cmd_gui/frame_my_command/my_command_gui.py``
- Add a new file ``commands/c_my_command.py`` with the following structure as
  starting point

  .. code-block:: python

     """Implements the 'my_command' frame"""

     from tkinter import ttk


     # pylint: disable-next=too-many-instance-attributes, too-many-ancestors
     class MyCommandFrame(ttk.Frame):
         """'My Command' frame"""

         def __init__(self, parent) -> None:
             super().__init__(parent)
             # ...

- Add the new frame to the main GUI in ``cli/cmd_gui/gui_impl.py`` as follows:

  .. code-block:: python

     from .frame_my_command.my_command_gui import MyCommandFrame

     # Add a 'Notebook' (i.e., tab support)
     self.notebook = ttk.Notebook(self)

     # other frames are already added here
     # add the new one at the appropiate position
     tab_my_command = MyCommandFrame(self.notebook)
     self.notebook.add(tab_my_command, text="My Command")

**********
Unit Tests
**********

- Unit tests for the |fox-cli| shall be implemented in ``tests/cli``.
- Functions called by the function under test should be mocked.
- The command line interface for each command shall be tested in
  ``tests/cli/commands/*``, where each command uses its own file to tests its
  interface.
- Each tool shall then be tested in the appropiate subdirectory, e.g.,
  ``cli/cmd_etl/batetl/etl/can_decode.py``
  is then tested in
  ``tests/cli/cmd_etl/batetl/etl/test_can_decode.py``.
- Every test case shall only test one test, i.e., if a function has an
  ``if...else`` branch, two test functions shall be used.
- Foreach test appropriate
  `assert methods <https://docs.python.org/3/library/unittest.html
  #unittest.TestCase.assertEqual>`_ shall be used to provide a verbose error
  message in case a test fails.

.. csv-table:: Assert Usage
   :file: ./assert_usage.csv
   :header-rows: 1
   :delim: ;
   :widths: 20 20

- For every test the following shall be considered to avoid boilerplate code:

  - In every test case it shall be considered to use the
    `setUp <https://docs.python.org/3/library/unittest.html
    #unittest.TestCase.setUp>`_ and
    `tearDown <https://docs.python.org/3/library/unittest.html
    #unittest.TestCase.tearDown>`_ method.
  - If similar
    `patch <https://docs.python.org/3/library/unittest.mock.html#patch>`_
    decorators are used in each test of a test case, it shall
    be considered to apply the patch decorators on the test class.

|fox-cli| Unit Test Example
---------------------------

Consider the the file ``foo.py``, which implements a class ``Foo`` and two
methods.

.. literalinclude:: ./foo.py
   :language: python
   :start-after: start-include-in-docs
   :linenos:
   :caption: ``foo.py``

The include section should look something like this:

.. literalinclude:: ./test_foo.py
   :language: python
   :start-after: start-include-in-docs-imports
   :end-before: stop-include-in-docs-imports
   :linenos:
   :caption: ``test_foo.py``

Each method, include the dunder methods, shall have a separate test case
implemented through a ``unittest.TestCase`` class.
In this example, there are then three test cases

- function ``__init__`` implements its tests in class ``TestFooInstantiation``
- function ``add_two`` implements its tests in class ``TestFooAddTwo``
- function ``print_attr`` implements its tests in class ``TestFooPrintAttr``

Testing the ``__init__`` method:

.. literalinclude:: ./test_foo.py
   :language: python
   :start-after: start-include-in-docs-instantiation
   :end-before: stop-include-in-docs-instantiation
   :linenos:
   :caption: ``test_foo.py``

Testing the ``add_two`` method:

.. literalinclude:: ./test_foo.py
   :language: python
   :start-after: start-include-in-docs-add-two
   :end-before: stop-include-in-docs-add-two
   :linenos:
   :caption: ``test_foo.py``

Testing the ``print_attr`` method:

.. literalinclude:: ./test_foo.py
   :language: python
   :start-after: start-include-in-docs-print-attr
   :end-before: stop-include-in-docs-print-attr
   :linenos:
   :caption: ``test_foo.py``
