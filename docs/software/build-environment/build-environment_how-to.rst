.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _CHANGING_AND_EXTENDING_THE_BUILD_ENVIRONMENT:

Changing and Extending the Build Environment
============================================

If packages are needed that are not included in development environment they
can simply be added.

- Chose a new name for the environment
- Create the new virtual environment
  ``py -m venv %USERPROFILE%\foxbms-envs\<name-of-the-new-env>``
- Activate the new environment.
- Install the required packages in the environment using
  ``python -m pip install ...``
- Run ``python -m pip freeze > requirements.txt``
- Update the license list in
  ``docs/general/license-tables/license-info_python_packages.csv``
- Commit all changes.
- Add a changelog entry that tells the user to update the environment.

Existing Environments
=====================

Until version 1.6.0 |foxbms| used `conda` environments.
Starting with version 1.7.0 |foxbms| uses now `venv` to create virtual
environments.

+---------------------+-------+-------+-------+
| Environment Name    | From  | To    | Type  |
+=====================+=======+=======+=======+
| 2025-01-pale-fox    | x.y.z | \-    | venv  |
+---------------------+-------+-------+-------+
| 2024-08-pale-fox    | 1.7.0 | 1.7.0 | venv  |
+---------------------+-------+-------+-------+
| 2023-02-fennec-fox  | 1.5.1 | 1.6.0 | conda |
+---------------------+-------+-------+-------+
| 2021-11-fennec-fox  | 1.2.1 | 1.5.0 | conda |
+---------------------+-------+-------+-------+
| 2021-08-arctic-fox  | 1.1.0 | 1.2.0 | conda |
+---------------------+-------+-------+-------+
| 2021-04-red-fox     | 1.0.0 | 1.0.2 | conda |
+---------------------+-------+-------+-------+

Further Reading
---------------

An explanation why build environments are used is found in
:ref:`BUILD_ENVIRONMENT`.
