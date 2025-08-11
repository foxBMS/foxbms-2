.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _BUILD_ENVIRONMENT:

Build Environment
=================

In order to have a consistent build environment on every developer machine,
|foxbms| provides predefined Python `venv` environments, that ships some tools
that are additionally needed to build |foxbms|.

A certain |foxbms| version requires certain software with in exact versions as
the project specifies them.
By that, we are sure that every commit in the repository can be built the same
way on any machine that has all needed dependencies.

Until version 1.6.0 |foxbms| used `conda` environments.
Starting with version 1.7.0 |foxbms| uses now `venv` to create virtual
environments.
The following environments are/were used by |foxbms|.

+---------------------+--------+-------+-------+
| Environment Name    | From   | To    | Type  |
+=====================+========+=======+=======+
| 2025-06-pale-fox    | 1.10.0 | \-    | venv  |
+---------------------+--------+-------+-------+
| 2025-03-pale-fox    | 1.9.0  | 1.9.0 | venv  |
+---------------------+--------+-------+-------+
| 2025-01-pale-fox    | 1.8.0  | 1.8.0 | venv  |
+---------------------+--------+-------+-------+
| 2024-08-pale-fox    | 1.7.0  | 1.7.0 | venv  |
+---------------------+--------+-------+-------+
| 2023-02-fennec-fox  | 1.5.1  | 1.6.0 | conda |
+---------------------+--------+-------+-------+
| 2021-11-fennec-fox  | 1.2.1  | 1.5.0 | conda |
+---------------------+--------+-------+-------+
| 2021-08-arctic-fox  | 1.1.0  | 1.2.0 | conda |
+---------------------+--------+-------+-------+
| 2021-04-red-fox     | 1.0.0  | 1.0.2 | conda |
+---------------------+--------+-------+-------+
