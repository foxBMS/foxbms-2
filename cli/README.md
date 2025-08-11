# `cli` Directory Description

## Directories

| Directory Name   | Long Name                   | Content Description                                                      |
| ---------------- | --------------------------- | ------------------------------------------------------------------------ |
| `cmd_*`          | Command_*                   | Actual implementation of the specific CLI command                        |
| `commands`       | Commands                    | Definition of all CLI commands                                           |
| `fallback`       | Fallback                    | Fallback for the fox CLI wrappers when the environment is missing        |
| `helpers`        | Helpers                     | Helper functions that are used by several parts of the CLI tool          |
| `pre-commit`     | pre-commit                  | Scripts that are run as part of the `pre-commit` framework               |

## Files

- ``cli/cli.py``: registers all commands.
- ``cli/foxbms_version.py``: Reads the |foxbms| version information from the
  single source of truth for the version information, the ``wscript`` at the root
  of the repository.
