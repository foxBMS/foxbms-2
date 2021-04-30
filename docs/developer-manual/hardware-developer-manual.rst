.. include:: ./../macros.txt
.. include:: ./../units.txt

.. _HARDWARE_DEVELOPER_MANUAL:

Hardware Developer Manual
=========================

This developer manual describes the development workflow of the |foxbms|
hardware components. Changes to this document have to be tracked in version
control and have to go through change management.

Project naming scheme
---------------------
A design **MUST** have a name that clearly distinguishes it from other
designs. The name **SHOULD** be constructed from the project acronym, the
type or family of design and an identifier.

As an example the master board of a BMS in the project XMPL could be named
``xmpl-master``. If necessary an additional identifier could be added if
several similar designs exist.

Design and development process
------------------------------

The designer has to adhere to best practice during the design. This entails
using the pre-defined set of templates, design rules and release processes.
Releasing through the release process of Altium helps to make sure that all
changes are committed and that no design rule violations remain.

Violations should only be waived or suppressed in justified cases. This
justification **MUST** be noted in the waived rule.


.. _RELEASE_PROCESS:

Release process
---------------

The release process has the following steps:

#. Create the necessary footprints and components in a versioned DBLib.
#. Derive a new project from a project template and put it under version
   control.
#. Define the schematic of the project and the design of the printed
   circuit board.
#. Review the project together with a team member.
#. Create a release with the predefined set of output jobs.
#. Store the release in a pre-defined location so that it can be found.
#. Mark the released version in version control with a tag.


Modification process
--------------------

If modifications to the hardware are necessary, the following process has to
be followed:

#. Create an issue in the issue tracker of the hardware component project.
#. Describe the necessary changes as precise as possible.
#. Describe the impact of the changes to the software.
#. Describe the impact of the changes to the tests.
#. Enter the hardware release process :numref:`RELEASE_PROCESS` with
   these changes.


Checklist for releases
----------------------

This checklist should be used for reviewing a release.

Basics:
^^^^^^^

* Is a layer marker on the PCB?
* Is a white field for marking the assembled board on the PCB?
* Is the name of the component on the PCB?
* Is a release version on the board? Is the version corresponding to the
  parameter in the Altium project?
* Are fiducial markers on the board?
* Are mounting points specified in the board?

BOM:
^^^^

* Is everything on the BOM available?
* Is the correct distributor selected for each component?
* Are similar components grouped into one (e.g., reduce amount of different
  footprints for capacitors with same value)?
