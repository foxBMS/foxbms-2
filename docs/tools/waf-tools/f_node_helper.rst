.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _WAF_TOOL_NODE_HELPER:

Node Helper
===========

The tool is located in ``tools/waf-tools``.

.. automodule:: f_node_helper
    :members:

    .. autofunction:: f_node_helper.convert_to_node(self, _node, path=None)

       Convert input to node object

    .. autofunction:: f_node_helper.convert_to_nodes(self, lst, path=None)

       Flatten the input list of string/nodes/lists into a list of nodes (based
       on ``TaskGen.to_nodes``, but for any node type, see
       `3536dfecf8061c6d99bac338837997c4862ee89b/waflib/TaskGen.py#L495-527 <https://gitlab.com/ita1024/waf/-/blob/3536dfecf8061c6d99bac338837997c4862ee89b/waflib/TaskGen.py#L495-527>`_)
