.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _FOX_CLI_IMPLEMENTATION:

|fox-cli| Implementation
========================

The |fox-cli| is implemented in a two staged wrapper,

- first the ``fox.ps1`` wrapper for PowerShell and ``fox.sh`` wrapper for bash
  and
- secondly |fox-py| as a generic wrapper to call into the actual CLI
  when ``fox.ps1``\/``fox.sh`` succeeded.

The |fox-cli-package| provides the ``fox-cli`` command to call into the CLI.

*****************
|fox-cli| Wrapper
*****************

The workflow of the |fox-cli| wrapper is shown in
:numref:`block-diagram-fox-cli-wrapper`.

.. drawio-figure:: fox-cli-wrapper.drawio
   :format: svg
   :alt: Block diagram of the fox CLI wrapper
   :name: block-diagram-fox-cli-wrapper
   :width: 520px

   Block diagram of the |fox-cli| wrapper

==========================
|fox-cli| ``.ps1`` Wrapper
==========================

The workflow of the |fox-cli| ``.ps1`` wrapper is shown in
:numref:`block-diagram-fox-cli-ps1-wrapper`.

.. drawio-figure:: fox-cli-wrapper.ps1.drawio
   :format: svg
   :alt: Detailed block diagram of the ps1 fox CLI wrapper
   :name: block-diagram-fox-cli-ps1-wrapper
   :width: 600px

   Detailed block diagram of the |fox-cli| ``.ps1`` wrapper

=========================
|fox-cli| ``.sh`` Wrapper
=========================

The workflow of the |fox-cli| ``.sh`` wrapper is shown in
:numref:`block-diagram-fox-cli-sh-wrapper`.

.. drawio-figure:: fox-cli-wrapper.sh.drawio
   :format: svg
   :alt: Detailed block diagram of the sh fox CLI wrapper
   :name: block-diagram-fox-cli-sh-wrapper
   :width: 520px

   Detailed block diagram of the |fox-cli| ``.sh`` wrapper

****************
|fox-py| Wrapper
****************

The purpose of and workflow of the |fox-py| file is shown in
:numref:`block-diagram-fox-py`.

.. drawio-figure:: fox.py.drawio
   :format: svg
   :alt: Block diagram of the fox.py wrapper
   :name: block-diagram-fox-py
   :width: 240px

   Block diagram of the ``fox.py`` wrapper

*****************************
|fox-cli-package| Entry Point
*****************************

The ``fox-cli`` command can only be used if the |virtual-python-environment|
is activated.
Its workflow is shown in :numref:`block-diagram-fox-cli-package`.

.. drawio-figure:: fox-cli-package-entry-point.drawio
   :format: svg
   :alt: Block diagram of the fox-cli entry point
   :name: block-diagram-fox-cli-package
   :width: 320px

   Block diagram of the ``fox-cli`` entry point for the |fox-cli-package|
