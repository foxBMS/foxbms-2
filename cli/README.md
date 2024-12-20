# `cli` Directory Description

| Directory Name   | Long Name                   | Content Description                                                      |
| ---------------- | --------------------------- | ------------------------------------------------------------------------ |
| `cmd_*`          | Command_*                   | Actual implementation of the specific CLI command                        |
| `commands`       | Commands                    | Defintion of all CLI commands                                            |
| `helpers`        | Helpers                     | Helper functions that are used by several parts of the CLI tool          |
| `pre-commit`     | pre-commit                  | Scripts that are run as part of the `pre-commit` framework               |

## How to implement a new command?

The new command `my-command` would be implemented as follows:

- Add a new file `cmd_my_command/my_command_impl.py`
- Add a new file `commands/c_my_command.py`
  Add

  ```python
  import click

  from ..cmd_my_command import my_command_impl

  # Add the new CLI commands afterwards using click, e.g., as follows:
  @click.command("my-command")
  @click.pass_context
  def run_my_command(ctx: click.Context) -> None:
      """Add help message here"""
      # my_command_impl.do_something must return a `SubprocessResult` object
      # pass the CLI arguments to `do_something` if additional arguments and/or
      # options are needed
      ret = my_command_impl.do_something()
      ctx.exit(ret.returncode)
  ```

- Implement the required feature(s) in `cmd_my_command/my_command_impl.py` (add
  as many files/submodules as needed in `cmd_my_command/`).
