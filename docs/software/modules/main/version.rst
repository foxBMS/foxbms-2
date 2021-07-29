.. include:: ../../../macros.txt
.. include:: ../../../units.txt

.. _VERSION_MODULE:

Version Module
==============

The ``wscript`` of this project retrieves the current version (if available)
from version control and generates during compilation of the project a
``version_cfg.c/h`` containing information about the current version.

In order to be able to find this struct in unknown binaries, the linker script
places this information always at the same location in memory. It uses the
linker section ``VERSIONINFO`` that starts at address ``0x003FFF40``.
