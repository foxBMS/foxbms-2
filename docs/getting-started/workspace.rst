.. include:: ./../macros.txt
.. include:: ./../units.txt

.. cspell:ignore Folder

.. _CREATING_A_WORKSPACE:

Creating a Workspace
====================

If |vs-code| is available, ``.vscode`` directories with all needed
configurations are generated when running ``.\fox.ps1 waf configure``.
The generated workspaces are already properly configured for the specific
directories.

Launching |vs-code|
-------------------

It is best to use the provided launcher script ``.\fox.ps1 ide`` to open
|vs-code| as then all environment variables are automatically correctly set.

If using |fox-cli| is not desired the following steps need to be taken into
account:

#. All required build tools etc. need to be in path and environment variables
   need to be correctly set.
#. Open |vs-code|.
#. Goto ``File->Open Folder`` and select the cloned |foxbms| repository.

Build Tasks
-----------

Build commands are executed by running ``Build Tasks``.

A ``Build Task`` is executed either selecting ``Terminal->Run Build Task`` from
the menu or pressing :kbd:`Ctrl + Shift + B`.

A list of possible build commands is shown and the desired one can be executed
by clicking with the cursor or navigating with the arrow keys and pressing
:kbd:`Enter`.
