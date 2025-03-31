.. include:: ./../macros.txt
.. include:: ./../units.txt

.. _DEPENDENCY_MANAGEMENT:

Dependency Management
=====================

|foxbms| uses other tools in order to build embedded sources, unit tests,
documentation and other tools.
These dependencies are either part of the repository (e.g., FreeRTOS)
or are programs that need to be installed in order work with the repository
(e.g., Python).

The following sections list these dependencies, describes how to update them
and includes a maintenance plan to keep dependencies up-to-date.

Internal Dependencies
---------------------

Internal dependencies referr to dependencies where the dependency (i.e., some
files) is actually commited to the repository, not just a configuration file.

- |freertos-kernel|: ``src/os/freertos/freertos``
- |freertos-plus-tcp|: ``src/os/freertos/freertos-plus/freertos-plus-tcp``
- |waf|: ``tools/waf``

External Dependencies
---------------------

Dependencies that are installed locally on the developer's machine.

- |doxygen|
- |drawio-desktop|
- |mingw-w64|
- |git|
- |graphviz|
- |pre-commit| hooks: defined through ``.pre-commit-config.yaml``.
- |python|
- |python| packages: defined through ``requirements.txt``.
- |ruby|
- |ruby-gems|
- |ti-code-composer-studio|
- |ti-halcogen|
- |vs-code|

Management Plan
---------------

- All dependencies **SHALL** be updated at the end of every quarter by the
  Dependency Update Responsible.
- The Dependency Update Responsible **SHOULD** be changed in every update
  cycle, so that all team members know how to update the dependencies.
- Every update cycle **SHALL** have a ticket that follows the naming
  conventions ``Dependency update: <XXXX>/Q<Y>``, where ``<XXXX>`` is the year
  ``<Y>`` the quarter.
  Use ``.gitlab/issue_templates/issue-dependency-update.md`` as template for
  the issue.
- After closing the ticket for the current update cycle, the ticket for the new
  update cycle **SHALL** be opened.
- If a dependency can not be updated, it shall be document in the ticketing
  system for the **next** update cycle why a dependency has not been updated.
- Every dependency **SHALL** be updated in a separate commit.
  Use ``.gitlab/merge_request_templates/mr-dependency-update.md`` as template
  for the merge request.
  The commit message (i.e., the merge request title) **SHALL** be
  ``Dependency update: <XXXX>/Q<Y>: <Dependency name> <version number>``.

.. note::

   Check that the following documentation sources are aligned:

   - ``.gitlab/issue_templates/issue-dependency-update.md``,
   - ``.gitlab/merge_request_templates/mr-dependency-update.md``
   - ``INSTALL.md``,
   - ``docs/getting-started/software-installation.rst`` and the files linked in
     it, and
   - ``docs/general/license.rst`` and the files linked in it.

How to Update the Individual Dependencies
-----------------------------------------

.. |snippet_verify_license| replace:: Verify that the license of the latest release is aligned with our licensing policy. If not, open a ticket to discuss with the team members on how to proceed with that dependency (finding a replacement etc.), otherwise start the update process
.. |snippet_download| replace:: Download the latest release from
.. |snippet_install| replace:: Install the latest release of
.. |update-installation-instructions| replace:: Update the installation instructions in ``docs/getting-started/software-installation.rst`` and ``INSTALL.md``
.. |snippet_update_license| replace:: Update the relevant license information in
.. |snippet_changelog_entry| replace:: Add a changelog entry. If updating the dependency required other changes than the dependency itself, document this in this changelog entry.
.. |snippet_commit| replace:: Add the changes and commit the changes using the following commit message

Updating Internal Dependencies
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

|freertos-kernel|
+++++++++++++++++

.. include:: ./update-processes/internal/update-freertos-kernel.txt

|freertos-plus-tcp|
+++++++++++++++++++

.. include:: ./update-processes/internal/update-freertos-plus-tcp.txt


|waf|
+++++

.. include:: ./update-processes/internal/update-waf.txt

Updating External Dependencies
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

|doxygen|
+++++++++

.. include:: ./update-processes/external/update-doxygen.txt

|drawio-desktop|
++++++++++++++++

.. include:: ./update-processes/external/update-drawio-desktop.txt

|mingw-w64|
+++++++++++

.. include:: ./update-processes/external/update-mingw-w64.txt

|pre-commit| Hooks
++++++++++++++++++

.. include:: ./update-processes/external/update-pre-commit-hooks.txt

|python|
++++++++

.. include:: ./update-processes/external/update-python.txt

|python| Packages
+++++++++++++++++

.. include:: ./update-processes/external/update-python-packages.txt

|ruby|
++++++

.. include:: ./update-processes/external/update-ruby.txt

|ruby-gems|
+++++++++++

.. include:: ./update-processes/external/update-ruby-gems.txt

|ti-code-composer-studio|
+++++++++++++++++++++++++

.. include:: ./update-processes/external/update-ti-code-composer-studio.txt

|ti-halcogen|
+++++++++++++++

.. include:: ./update-processes/external/update-ti-halcogen.txt
