# Formatting Configurations

This directory contains the configurations for the formatting rules applied to
the foxBMS 2 repository.

## C

foxBMS 2 uses [clang-format](https://clang.llvm.org/docs/ClangFormat.html) to
automatically format the code correctly.

The formatting configuration file ``.clang-format`` is not in the directory as
clang-format has a different way of searching the configuration file
(closest configuration file to the to-be-formatted-source file is chosen).
The ``.clang-format`` is therefore stored in the repository root.

Directories that should not be automatically formatted must include a
``.clang-format`` file to disable automatic formatting:

**``.clang-format``**
```yaml
DisableFormat: true
SortIncludes: false
```

## Python
All Python formatting rules are defined in ``pyproject.toml``.
