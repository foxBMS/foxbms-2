.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _WAF_TOOL_HALCOGEN:

HALCoGen
========

.. note::
   A documentation of the tool |halcogen| can be found in
   :ref:`HALCOGEN_TOOL_DOCUMENTATION`, the waf-tool wrapper for this project in
   this section, information on configuring |halcogen| in the context of the
   toolchain of this project in :ref:`HAL_CONFIGURATION` and an example on how
   to use a pre-generated HAL instead of generating it in the compilation step
   in :ref:`HOW_TO_USE_GENERATED_SOURCES_FROM_HALCOGEN`.

..
    Comments:
    bld is the waf object
    hcg and dil are file extensions for HALCoGen projects

.. spelling::
    bld
    hcg
    dil


The tool is located in ``tools/waf-tools``.

.. automodule:: f_hcg
    :members:
    :show-inheritance:
