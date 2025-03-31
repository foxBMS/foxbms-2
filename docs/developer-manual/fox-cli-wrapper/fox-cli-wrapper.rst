.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _FOX_CLI_IMPLEMENTATION:

########################
|fox-cli| Implementation
########################

The |fox-cli| is implemented in a two staged wrapper,

- first the ``fox.ps1`` wrapper for PowerShell and ``fox.sh`` wrapper for bash
  and
- secondly |fox-py| as a generic wrapper to call into the actual CLI
  when ``fox.ps1``\/``fox.sh`` succeeded.

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

   Block diagram of the fox CLI wrapper

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

   Block diagram of the fox.py wrapper
