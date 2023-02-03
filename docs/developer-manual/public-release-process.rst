.. include:: ./../macros.txt
.. include:: ./../units.txt

.. _PUBLIC_RELEASE_PROCESS:

Public Release Process
======================

This description is for documenting purposes only, so that others know how we,
the :ref:`TEAM`, create public releases.
Releases are created from commits of the ``master`` branch of an Fraunhofer
internal git repository.
A release is assembled by running the described procedure after the ``master``
branch has been bumped to a version, which identifier matches the regular
expression ``\d+\.\d+\.\d+``
(the internal development version number is always ``x.y.z``, except for the
commit that bumps the version to a releasable version).

.. note::

   This description uses two different directories:

   - ``C:\release-bot\foxbms-2``: local checkout of the internal repository.
   - ``C:\release-bot\foxbms-2-publishing-mirror``: local checkout of the
     internal repository that is synced to |foxbms_repository|.

The release is internally created by the ``The foxBMS ReleaseBot`` and these
steps are automated.
The git configuration of the ``The foxBMS ReleaseBot`` is

.. code-block:: console

   C:\release-bot\foxbms-2-publishing-mirror>git config user.name "The foxBMS ReleaseBot"
   C:\release-bot\foxbms-2-publishing-mirror>git config user.mail "info@foxbms.org"

For this documentation we assume that the version to be created and released
is ``1.0.0``.

Defining release status of repository
+++++++++++++++++++++++++++++++++++++

#. Defining the version number of the release.
#. Creation of a version bump branch ``bump-version``

   .. code-block:: console

      C:\release-bot\foxbms-2>git checkout -b bump-version

#. Bump the version number in all relevant files and commit the changes to the
   new branch:

   .. code-block:: console

      C:\release-bot\foxbms-2>tools\utils\cmd\run-python-script.bat tools\utils\update_version.py --from x.y.z --to 1.0.0
      C:\release-bot\foxbms-2>tools\utils\cmd\run-python-script.bat tools\utils\update_doxygen_header.py
      C:\release-bot\foxbms-2>git add .
      C:\release-bot\foxbms-2>git commit -m "bump version to v1.0.0"
      C:\release-bot\foxbms-2>git tag -a v1.0.0-version-bump -m "bump version to v1.0.0"

#. Merge the branch back to ``master`` branch and remove the version bumping
   branch

   .. code-block:: console

      C:\release-bot\foxbms-2>git checkout master
      C:\release-bot\foxbms-2>git merge bump-version
      C:\release-bot\foxbms-2>git branch -D bump-version

#. Tag the ``master`` branch with ``v1.0.0``:

   .. code-block:: console

      C:\release-bot\foxbms-2>git tag -a v1.0.0 -m "v1.0.0"

#. Make the ``master`` branch a development branch again (note the reverted
   order of ``from`` and ``to``: ``--from 1.0.0 --to x.y.z``):

   .. code-block:: console

      C:\release-bot\foxbms-2>tools\utils\cmd\run-python-script.bat tools\utils\update_version.py --from 1.0.0 --to x.y.z

#. Push all this work:

   .. code-block:: console

      C:\release-bot\foxbms-2>git push origin master --follow-tags


Creation of the release branch
++++++++++++++++++++++++++++++

#. Creation of the release branch at the created version tag ``v1.0.0``:

   .. code-block:: console

      C:\release-bot\foxbms-2>git checkout v1.0.0
      C:\release-bot\foxbms-2>git checkout -b release-v1.0.0

#. Run script to remove confidential and non-releasable files and information.
#. Commit all changes to the release branch ``release-v1.0.0`` and tag this
   commit as release:

   .. code-block:: console

      C:\release-bot\foxbms-2>git add .
      C:\release-bot\foxbms-2>git commit -m "branch for GitHub release version 1.0.0"
      C:\release-bot\foxbms-2>git tag -a gh-1.0.0 -m "gh-1.0.0"

#. Make a clean build to make sure everything works as expected:

   .. code-block:: console

      C:\release-bot\foxbms-2>waf configure
      C:\release-bot\foxbms-2>waf build_all

#. Clean the repository from all generated files:

   .. code-block:: console

      C:\release-bot\foxbms-2>git clean -xdf


Publication of the release branch
+++++++++++++++++++++++++++++++++

#. Create release branch in the publishing mirror and check it out:

   .. code-block:: console

      C:\release-bot\foxbms-2-publishing-mirror>git checkout -b gh-release-v1.0.0
      C:\release-bot\foxbms-2-publishing-mirror>git commit -m "update branch for GitHub release version 1.0.0"

#. Remove all files from the current checkout:

   .. code-block:: console

      C:\release-bot\foxbms-2-publishing-mirror>git rm -r "*"

#. Copy files from release branch in the internal repository to the release
   branch of the publishing repository and add them:

   .. code-block:: console

      C:\release-bot\foxbms-2-publishing-mirror>xcopy C:\release-bot\foxbms-2 . /s /e
      C:\release-bot\foxbms-2-publishing-mirror>git add . -f
      C:\release-bot\foxbms-2-publishing-mirror>git commit -F docs\general\commit-msgs\release-v1.0.0.txt

#. Create a dummy tag (annotated or not does not make a difference here) and
   make sure everything works as expected (no problems are expected to happen):

   .. code-block:: console

      C:\release-bot\foxbms-2-publishing-mirror>git tag v1.0.0
      C:\release-bot\foxbms-2-publishing-mirror>waf configure
      C:\release-bot\foxbms-2-publishing-mirror>waf build_all

#. Delete the temporary tag ``v1.0.0``:

   .. code-block:: console

      C:\release-bot\foxbms-2-publishing-mirror>git checkout master
      C:\release-bot\foxbms-2-publishing-mirror>git tag -d v1.0.0

#. Merge the ``gh-release-v1.0.0`` branch to the ``master`` branch and create
   an annotated tag ``v1.0.0`` on ``master`` branch:

   .. code-block:: console

      C:\release-bot\foxbms-2-publishing-mirror>git merge gh-release-v1.0.0
      C:\release-bot\foxbms-2-publishing-mirror>git branch -D gh-release-v1.0.0

#. Push to the publishing repository:

   .. code-block:: console

      C:\release-bot\foxbms-2-publishing-mirror>git push origin master --follow-tags

#. The ``master`` branch of the publishing mirror is automatically synced to
   |foxbms_repository|.

Finishing the release
+++++++++++++++++++++

After that process, we

- create `GitHub release <https://github.com/foxBMS/foxbms-2/releases>`_,
- update the documentation on |docs.foxbms.org| and
- post a news entry on the website |foxbms.org|.
