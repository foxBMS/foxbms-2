:orphan:

.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _MANUAL_RUBY_INSTALLATION:

########################
Manual Ruby Installation
########################

.. note::

   The RubyInstaller does not support installing different versions of Ruby.
   If there is already some Ruby version installed into
   ``C:\Ruby\RubyX.Y.Z.\*`` this installation must be copied to some
   temporary folder, and then can be copied back into
   ``C:\Ruby\RubyX.Y.Z.`` after the installation has finished.

#. Download the |ruby| installer version ``2.7.2`` including the Devkit from
   `rubyinstaller.org <https://rubyinstaller.org/>`_. Use
   ``C:\Ruby\Ruby272-x64`` as installation directory and do not add Ruby to
   ``PATH``.

   .. figure:: img/ruby-installer-0.png
      :alt: |ruby| installation routine
      :align: center

      |Ruby| installation routine

#. Install the required gems for ``Ceedling``:

   .. code-block:: console
      :caption: Installing required gems for Ceedling

      C:\Users\vulpes\Documents\foxbms-2>cd tools\vendor\ceedling
      C:\Users\vulpes\Documents\foxbms-2>C:\Ruby\Ruby272-x64\bin\bundle install

   Add ``--http-proxy=http://user:password@server:port`` (adapted to your
   settings) to the shown command if you are behind a proxy.
