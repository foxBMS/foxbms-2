.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _CHANGING_AND_EXTENDING_THE_BUILD_ENVIRONMENT:

Changing and Extending the Build Environment
============================================

If packages are needed that are not included in development environment they
can simply be added.
This how-to explains it for Windows.
If there is a reference to |conda_env_config_win32| and you are on Linux
replace it by |conda_env_config_linux|.

The basic required packages are listed in
``conf/env/conda_env_win32-pkgs.yaml``.
If a package should be added or removed, it needs to be done here.
This file only defines the major Python version that should be used.

The steps are basically:

- Add new packages and/or remove no longer needed packages and update the
  environment name, for this example ``example-env``.
- Create a new pseudo-base environment that includes all needed Python packages
  for the project.
- Export the exact environment definition.
- Update the test suite.
- Commit the new environment to the repository.
- Add a changelog entry that tells the user to run the environment update
  script.

These steps in details:

#. Add packages/remove packages and update environment name.
#. Create a new pseudo-base environment and wait for the solver to succeeded.

   .. code-block:: console

      C:\Users\vulpes>%USERPROFILE%\miniconda3\Scripts\activate base
      (base) C:\Users\vulpes>conda env create -f conf\env\conda_env_win32-pkgs.yaml

#. Export the new development environment:

   .. code-block:: console

      (base) C:\Users\vulpes>conda env export -n example-env > conf\env\conda_env_win32.yaml

#. Remove the ``Prefix`` entry from |conda_env_config_win32|.
#. Adapt the test suite as needed and run it afterwards.
#. Commit the new environment file to the repository.
#. Add changelog entry.

Further Reading
---------------

An explanation why build environments are used is found in
:ref:`BUILD_ENVIRONMENT`.
