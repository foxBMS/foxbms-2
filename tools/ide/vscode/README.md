# VS Code Setting Configuration

## Workspace

The `jinja2`-files in this directory contain *configuration* files for VS
Code settings.
These files get properly configured when running `fox.ps1 waf configure` or
`./fox.sh waf configure` in the root of the repository.
Please note that doing so, will overwrite the contents of `.vscode` in
the root of the repository with these templates.

## Snippets

### Installation

Snippets are installed by copying the snippets files into the VS Code snippets
directory.
This directory is

- on Windows: `%APPDATA%\Code\User\snippets` and
- on Linux: `$HOME/.config/Code/User/snippets`.

### C

`C`-snippets are provided to simplify the creation of style guide conforming
`for` loops regarding number of strings, modules and cells per module.
The snippet file is `c.json`.

### Python

Simple `main`-function snippet for `Python`
The snippet file is `python.json`.
