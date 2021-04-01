.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _YAML_CODING_GUIDELINES:

YAML Coding Guidelines
======================

These coding guidelines **MUST** be applied to all ``YAML`` files.

The following rules generally apply and follow the naming schema
``YAML:<ongoing-number>``.

.. _rule_yaml_filenames:

Filenames (``YAML:001``)
------------------------

Additional to the general file naming rules the following **MUST**
be applied.

.. admonition:: File name rules

    - The general file naming rules **MUST** be applied (see
      :numref:`rule_general_filenames`).
    - ``YAML`` files **MUST** use ``.yaml`` or ``.yml`` as file extension.

.. _rule_yaml_header:

Header (``YAML:002``)
---------------------

.. admonition:: YAML file header

    YAML source and header files **MUST** start with the following header:

   .. literalinclude:: ./../../../conf/tpl/yaml.yaml
      :language: yaml
      :linenos:
      :lines: 1-35
      :caption: File header for ``YAML`` files.
      :name: file-header-yaml
