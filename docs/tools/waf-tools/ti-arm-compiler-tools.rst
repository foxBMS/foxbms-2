.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _TI_ARM_CGT_TOOL:

TI ARM CGT Tool
---------------

The compiler tool is split up into different sub modules to simplify testing
and maintenance. The implementations are found in :numref:`ti-arm-cgt-tools`.

.. csv-table:: TI ARM CGT compiler tools
   :name: ti-arm-cgt-tools
   :header-rows: 1
   :delim: ;
   :file: ./ti-arm-compiler-tools.csv

The implementation documentation is found hereafter:

.. toctree::
    :titlesonly:
    :maxdepth: 1
    :caption: Compiler Tool Implementations

    ./compiler-tool/f_ti_arm_cgt
    ./compiler-tool/f_ti_arm_cgt_cc_options
    ./compiler-tool/f_ti_arm_helper
    ./compiler-tool/f_ti_arm_tools
    ./compiler-tool/f_ti_color_arm_cgt
