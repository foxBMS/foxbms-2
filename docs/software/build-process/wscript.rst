.. include:: ./../../macros.txt

.. _BUILD_SYSTEM_ENTRY_POINT:

Entry Point
===========

The top-level ``wscript`` is the entry point for the |waf| build system.
It registers all build variants, configures the toolchains, and dispatches
builds to the correct sub-directories.

.. automodule:: wscript
    :members: options, configure, build
