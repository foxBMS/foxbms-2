# VS Code Setting Configuration

The ``jinja2``-files in this directory contain *configuration* files for VS
Code settings. These files get properly configured when running
``waf.bat configure`` or ``waf.sh configure`` in the root of the repository.
Please note that doing so, will overwrite the contents of `.vscode/` in
the root of the repository with these templates.

To simplify the creation of style guide conforming for loops regarding to
number of strings, modules and cells per module, just copy the content from
``snippets/c.json`` into the user snippet configuration directory:
- Windows: ``%APPDATA%\Code\User\snippets\c.json``
- Linux: ``$HOME/.config/Code/User/snippets/c.json``

There is also a snippet configuration for Python (``snippets/python.json``).
