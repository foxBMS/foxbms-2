.. include:: ./../macros.txt
.. include:: ./../units.txt

.. _RELEASES:

Releases
========

..
    Comments:
    vx version dummy

.. spelling::
    vx

|foxbms| Releases
-----------------

The following tables describe the different versions of |foxbms| that were
released. The first line is the most recent one, the last one the oldest one.

The changes between two releases are described in the :ref:`CHANGELOG`.

.. csv-table:: |foxbms| releases
   :name: foxbms-2-releases
   :header-rows: 1
   :delim: ;
   :file: ./releases.csv

Release Information
-------------------

Releases are commits on the master branch with an annotated tag. If we would
release for example version 0.0.42, we would check out the master branch and
then execute ``git tag -a v0.0.42``.
