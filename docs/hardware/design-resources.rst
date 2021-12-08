.. include:: ./../macros.txt
.. include:: ./../units.txt

.. _DESIGN_RESOURCES:

################
Design Resources
################

The hardware design packages for the |bms-master|, the |bms-extension|, the
|bms-interface| and the |bms-slaves| are available from the
|foxbms_repository|.
The packages include:

* Altium Designer Source Files

    * schematics
    * layout
    * active bill of materials

* Assembly files

    * BOM in Excel format
    * 3D model of PCB in step format
    * schematics in PDF format

* Fabrication files

    * PCB manufacturing data in ODB++ format
    * PCB layer stack in PDF format

.. note::

    To open the schematic and layout files, please use
    `Altium Designer <https://www.altium.com/altium-designer>`_.

.. note::

    To manufacture the printed circuit boards, the BOM (Microsoft Excel file)
    and the ODB++ files in each corresponding folder should be used.
    Before sending the board layout to a PCB manufacturer, the layout files
    must be checked against the design rules provided by this manufacturer,
    since some board layout settings may depend on its specific design rules
    and may cause violations (e.g., pad layout).
