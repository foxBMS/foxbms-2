:orphan:

.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _MANUAL_CONDA_CONFIGURATION:

##########################
Manual conda Configuration
##########################

Setup of the |conda| configuration in |condarc|.

*****************
Creating the File
*****************

#. Create the ``.condarc`` file in your user directory (|condarc|).

.. note::

   If you are unable to create a file with a leading *dot* (``.``) but no file
   extension in the file explorer, see
   https://www.hanselman.com/blog/how-to-create-a-file-with-a-dot-prefix-in-windows-explorer

.. _condarc_configuration:

*************
Configuration
*************

#. To add the |conda| channel for ``foxbms``, open the |condarc| file and
   add ``foxbms`` (all letters in lowercase) to the ``channels`` section.
   If the ``defaults`` channel is missing, add it first:

   .. code-block:: yaml
      :caption: ``defaults`` and ``foxbms`` are added to channels section

      channels:
         - defaults
         - foxbms

   .. note::

      If the ``foxbms`` channel and the ``defaults`` entries are already
      existing in the |conda| channels section this step can be skipped.

#. If |conda| is used behind a proxy, the |conda| proxy configuration must
   be adapted accordingly. Open |condarc| and edit the ``http`` and
   ``https`` proxy information, according to your network:

   .. code-block:: yaml
      :caption: Example ``proxy_servers`` in |condarc|

      proxy_servers:
         http: http://user:pass@corp.com:8080
         https: https://user:pass@corp.com:8080

   .. note::

      If the |conda| proxy is already correctly set, this step can be
      skipped.

#. A correct setup |conda| configuration in |condarc| might then look
   like that:

   .. code-block:: yaml
      :caption: Example |conda| configuration in |condarc|

      channels:
         - defaults
         - foxbms

      proxy_servers:
         http: my-company-http:8030
         https: my-company-https:8030
