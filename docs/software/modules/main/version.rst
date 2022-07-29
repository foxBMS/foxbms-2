.. include:: ../../../macros.txt
.. include:: ../../../units.txt

.. _VERSION_MODULE:

Version Module
==============

The ``wscript`` of this project retrieves the current version (if available)
from version control and generates during compilation of the project a
``version_cfg.c/h`` file containing information about the current version.

In order to be able to find the version easily in any foxBMS binary, the linker script
places this information always at the same location in memory.
It uses the linker section ``VER_VERSION_INFORMATION`` (for details see
:ref:`LINKER_SCRIPT`).
