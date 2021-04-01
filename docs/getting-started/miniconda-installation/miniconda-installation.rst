:orphan:

.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _MANUAL_MINICONDA_INSTALLATION:

#############################
Manual Miniconda Installation
#############################

Installing Miniconda consists of two steps, the actual miniconda installation
and setting up the ``.condarc`` file.

#. Download `Miniconda`_ (any Python 3.x 64bit version) and install it into
   the default installation directory.

   - If your **user name does NOT include whitespace**: During installation
     select ``Just Me`` and **do not change** the ``Destination Folder``
     (``%USERPROFILE%\miniconda3``).
   - If your **user name does include whitespace**: During installation
     select ``Just Me`` and **change** the ``Destination Folder`` to
     ``C:\miniconda3``.
   - Unselect ``Register Anaconda as my default Python 3.x`` when prompted.

   The installation is illustrated in the figure below.

   .. note::

     - If any Miniconda x64 version with a Python version greater than 3.x is
       already installed **in the indicated directory**, this step can be
       skipped.

   .. figure:: img/miniconda-installer.gif
      :alt: Miniconda installation routine
      :name: miniconda-installation-routine
      :align: center

      Miniconda installation routine

#. Proceed with :ref:`MANUAL_CONDA_CONFIGURATION`.
