:orphan:

.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _MANUAL_LLVM_INSTALLATION:

########################
Manual LLVM Installation
########################

.. note::

   The LLVM installer does not support installing different versions of
   LLVM.
   If there is already some LLVM version installed into
   ``C:\Program Files\LLVM\*`` this installation must be copied to some
   temporary folder, and then can be copied back into
   ``C:\Program Files\LLVM\other-LLVM-version`` after the installation
   has finished.

#. Download LLVM (version |version_llvm|) from
   `github.com/LLVM`_
   version 13.0.0 <https://github.com/llvm/llvm-project/releases/tag/llvmorg-13.0.0>`_
   from the project's GitHub release page.
#. Copy the existing LLVM installation to some temporary directory.
#. Remove the LLVM installation directory.
#. Run the installer.
   Choose |path_llvm| as installation directory
   Do not add LLVM to ``PATH``.
#. Copy the previous LLVM installations back to the original place.
