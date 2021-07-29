.. include:: ../../macros.txt
.. include:: ../../units.txt

.. _HOW_TO_USE_GENERATED_SOURCES_FROM_HALCOGEN:

How to Use Generated Sources from HALCoGen
==========================================

.. note::
   A documentation of the tool |halcogen| can be found in
   :ref:`HALCOGEN_TOOL_DOCUMENTATION`, the waf-tool wrapper for this project in
   :ref:`WAF_TOOL_HALCOGEN` and information on configuring |halcogen| in the
   context of the toolchain of this project in :ref:`HAL_CONFIGURATION`.

The following steps need to be applied:

#. Removing the |halcogen| dependency in the compiler tool:

   - remove loading the the ``f_hcg`` tool in the ``configure`` step.
   - in the ``class search_swi(Task.Task)`` the class attribute ``after`` needs
     to be modified. Remove the entry ``hcg_compiler`` from the list.

#. Adding the sources:

   - The generated HAL files need to be copied into the ``src/hal`` directory
   - The ``wscript`` in ``src/hal`` needs to be modified:

     .. literalinclude:: without-halcogen/wscript
        :language: python
        :linenos:
        :lines: 43-
        :caption: Building the HAL library without using |halcogen| on the fly
        :name: building-without-halcogen

   - Add the ``include`` directory in all ``wscript``-files where needed. This
     will affect most ``wscript``-files. This looks e.g., like this for a
     FreeRTOS and LTC6806 configuration:

     .. literalinclude:: without-halcogen/without-halcogen_diff-includes.diff
        :language: diff
        :linenos:
        :caption: The diff of includes
        :name: include-diff
