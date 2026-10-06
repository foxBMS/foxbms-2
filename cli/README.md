# `cli` Directory Description

## Directories

| Directory Name       | Long Name                   | Content Description                                                      |
| -------------------- | --------------------------- | ------------------------------------------------------------------------ |
| `cmd_*`              | Command_*                   | Actual implementation of the specific CLI command                        |
| `com`                | Communication               | Wrapper for different communication interfaces (e.g., CAN)               |
| `commands`           | Commands                    | Definition of all CLI commands                                           |
| `db`                 | Database                    | Light weight battery specific database implementation for foxBMS         |
| `helpers`            | Helpers                     | Helper functions that are used by several parts of the CLI tool          |
| `pre_commit_scripts` | pre-commit                  | Scripts that are run as part of the `pre-commit` framework               |

## Files

- ``__init__.py``: Python module
- ``__main__.py``: Executable Python module
- ``cli.py``: registers all commands.
- ``foxbms_version.py``: Reads the |foxbms| version information from the
  single source of truth for the version information, the ``wscript`` at the root
  of the repository.
- ``py.typed``: Package is typed
