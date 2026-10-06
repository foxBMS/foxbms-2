.. include:: ../../macros.txt
.. include:: ../../units.txt

.. _HOW_TO_USE_GENERATED_SOURCES_FROM_HALCOGEN:

How to Use Generated Sources from HALCoGen
==========================================

.. note::

   A documentation of the tool |ti-halcogen| can be found in
   :ref:`TI_HALCOGEN_TOOL`, the Waf tool wrapper for this project in
   :doc:`/tools/waf_tools/waf_tools_autosummary/waf_tools.hcg` and
   information on configuring |ti-halcogen| in the context of the toolchain
   of this project in :ref:`HAL_CONFIGURATION`.

The following steps need to be applied:

#. Removing the |ti-halcogen| dependency in the compiler tool by removing
   to load the ``hcg`` tool in the ``configure`` step.
#. Build the HAL sources as a library.
   The HAL library target needs to be ``f"{bld.env.APPNAME.lower()}-hal"``.
