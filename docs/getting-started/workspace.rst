.. include:: ./../macros.txt
.. include:: ./../units.txt

.. _CREATING_A_WORKSPACE:

Creating a Workspace
====================

If |code| is available, a ``.vscode`` directory with all needed
configurations is generated when running ``waf configure``. The
generated workspace is properly configured.


Launching |code|
----------------

It is best to use the provided launcher script ``ide.bat`` to open |code| as
then all environment variables are automatically correctly set.

If using the launcher scripts is not desired the following steps need to be
taken into account:

#. All required build tools etc. need to be in path and environment variables
   need to be correctly set.
#. Open |code|.
#. Goto ``File->Open Folder`` and select the cloned |foxbms| repository.


Build Tasks
-----------
Build commands are executed by running ``Build Tasks``. A ``Build Task`` is
executed either selecting ``Terminal->Run Build Task`` from the menu or
pressing :kbd:`Ctrl + Shift + B`. A list of possible build commands is shown
and the desired one can be executed by clicking with the cursor or navigating
with the arrow keys and pressing :kbd:`Enter`.

Test Explorer
-------------
With the Test Explorer (which is listed among the recommended extensions)
you can directly interact with the :ref:`Ceedling unit tests <UNIT_TESTS>`.
The Test Explorer is found in the activity bar under ``Test``.

Hints
-----

These are the default shortcuts in |code|. They can be adapted to fit the
user's needs.

#. Jumping to definition: :kbd:`F12`
#. Show all references: :kbd:`Shift + F12`
#. Find file: :kbd:`Ctrl + P`
#. Open command palette: :kbd:`Ctrl + Shift + P`
#. Search within active file: :kbd:`Ctrl + F`
#. Search within whole project: :kbd:`Ctrl + Shift + F`
#. Run Build Tasks: :kbd:`Ctrl + Shift + B`
#. Open header file to active source file: :kbd:`Alt + O`
#. Navigate history: :kbd:`Ctrl + Tab`
#. Navigate back: :kbd:`Alt + Left`
#. Navigate forward: :kbd:`Alt + Right`
