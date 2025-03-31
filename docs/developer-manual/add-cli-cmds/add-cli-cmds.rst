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

********************
Adding a New Command
********************

When a new tool, i.e., a command needs to be implemented the following steps
need to be done (exemplary new command ``my-command``):

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

**********
Unit Tests
**********

- Unit tests for the |fox-cli| shall be implemented in ``tests/cli``.
- The command line interface for each command shall be tested in
  ``tests/cli/commands/*``, where each command uses its own file to tests its
  interface.
- Each tool shall then be tested in the appropiate subdirectory, e.g.,
  ``cli/cmd_etl/batetl/etl/can_decode.py``
  is then tested in
  ``tests/cli/cmd_etl/batetl/etl/test_can_decode.py``.
