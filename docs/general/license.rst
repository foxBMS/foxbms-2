.. include:: ./../macros.txt
.. include:: ./../units.txt

.. _LICENSE:

#######
License
#######

..
    Comments:
    ccs is the abbreviation for "Code Composer Studio"
    gcc is the abbreviation for "GNU Compiler Collection"
    GPL is the abbreviation for "GNU General Public License"
    SPDX is the abbreviation for "Software Package Data Exchange"
    TSPA is the abbreviation for "TECHNOLOGY AND SOFTWARE PUBLICLY AVAILABLE"

.. spelling::
    ccs
    gcc
    GPL
    GPLv
    halcogen
    SPDX
    TSPA
    pdf
    txt
    Tcl
    tk
    GCCRLE
    LGPL

.. include:: ./license-tables/license-packages-conda-env-spelling.txt
.. include:: ./license-tables/license-packages-conda-env-spelling-build-strings.txt

*****************
|foxbms| Licenses
*****************

The license texts are included verbatim in this documentation.

|foxbms| Software License
=========================

- |foxbms| software is licensed under the BSD 3-Clause License.
- The SPDX short identifier is
  `BSD-3-Clause <https://spdx.org/licenses/BSD-3-Clause.html>`_.

The license text is included in this documentation at
:ref:`BSD_3_CLAUSE_LICENSE__BSD_3_CLAUSE__NEW__OR__REVISED__LICENSE_`.

|foxbms| Hardware and Documentation License
===========================================

- |foxbms| documentation is licensed under the
  Creative Commons Attribution 4.0 International License.
  The license can be obtained from
  https://creativecommons.org/licenses/by/4.0/.
- The SPDX short identifier is
  `CC-BY-4.0 <https://spdx.org/licenses/CC-BY-4.0.html>`_.

The license text is included in this documentation at
:ref:`CREATIVE_COMMONS_ATTRIBUTION_4_0_INTERNATIONAL_LICENSE`.

.. _OPEN_SOURCE_HARDWARE_CERTIFICATION:

Open Source Hardware Certification
==================================

|foxbms| has been certified as open source hardware by the
Open Source Hardware Association under the OSHWA UID
`DE000128 <https://certification.oshwa.org/de000128.html>`_.

Further Information
===================

We kindly request you to use one or more of the following phrases to refer to
foxBMS in your hardware, software, documentation or advertising materials:

- "This product uses parts of foxBMS®"
- "This product includes parts of foxBMS®"
- "This product is derived from foxBMS®"

If you use foxBMS in your products, we encourage you to contact us at:

.. code-block:: text

   Fraunhofer IISB
   Schottkystrasse 10
   91058 Erlangen, Germany
   mailto:info@foxbms.org
   https://foxbms.org

.. _BSD_3_CLAUSE_LICENSE__BSD_3_CLAUSE__NEW__OR__REVISED__LICENSE_:

BSD 3-Clause License (BSD 3-Clause "New" or "Revised" License)
==============================================================

.. literalinclude:: ./../../BSD-3-Clause.txt
    :language: none


.. _CREATIVE_COMMONS_ATTRIBUTION_4_0_INTERNATIONAL_LICENSE:

Creative Commons Attribution 4.0 International License
======================================================

.. literalinclude:: ./../../CC-BY-4.0.txt
    :language: none


********************
Third Party Licenses
********************

.. note::

    This is not legal advice.
    The license list hereafter shows the licenses used in |foxbms| to our best
    knowledge.

This is a list of third party licenses used by |foxbms|.
This is split into licenses for third party sources and software that are
included within the repository and software that must be installed in order to
develop BMS applications within or with |foxbms|.

Software listed under `Third Party Software`_ are typically only used for
|foxbms| development and do not need to be altered.

Third Party Sources and Software
================================

Ceedling
--------

.. csv-table::
   :name: license-info_ceedling
   :widths: 10 50
   :delim: ;
   :file: ./license-tables/license-info_ceedling.csv

|freertos|
----------

.. csv-table::
   :name: license-info_freertos
   :widths: 10 50
   :delim: ;
   :file: ./license-tables/license-info_freertos.csv

WAF
---

Binary
^^^^^^

.. csv-table::
   :name: license-info_waf-binary
   :widths: 10 50
   :delim: ;
   :file: ./license-tables/license-info_waf-binary.csv

Unit Tests
^^^^^^^^^^

.. csv-table::
   :name: license-info_waf-unit-tests
   :widths: 10 50
   :delim: ;
   :file: ./license-tables/license-info_waf-unit-tests.csv

Third Party Software
====================

|ti| |code-composer-studio|
---------------------------

.. csv-table::
   :name: license-info_ti-ccs
   :widths: 10 50
   :delim: ;
   :file: ./license-tables/license-info_ti-ccs.csv

|ti| |halcogen|
---------------

.. csv-table::
   :name: license-info_ti-hcg
   :widths: 10 50
   :delim: ;
   :file: ./license-tables/license-info_ti-hcg.csv

Miniconda
---------

.. csv-table::
   :name: license-info_miniconda
   :widths: 10 50
   :delim: ;
   :file: ./license-tables/license-info_miniconda.csv

Miniconda ships a `Python <https://www.python.org>`_ interpreter.
The Python interpreter license is found in
``<miniconda-install-dir>/LICENSE_PYTHON.txt``.

The conda environments shipped within the |foxbms| repository
(``conf/enc/conda_env_win32.yaml``) specifies a couple of conda and pip
packages.
These packages and their licenses are listed in
:numref:`license-info_packages-conda-env-win32` and
:numref:`license-info_packages-conda-env-linux`.
This list is generated based on ``conda list --show-channel-urls``.

- For ``pypi`` packages the license information is taken from
  https://pypi.org/.
- For conda packages the  license information is obtained by running
  ``conda info package_name --info`` or directly looked up in the conda build
  recipe.
- As a help for updating the list you can call
  ``./tools/utils/bash/run-python-script.sh ./tools/utils/generate_license_list.py -f ./docs/general/license-tables/license-info_packages-conda-env-win32.csv > ./docs/general/license-tables/license-info_packages-conda-env-new.csv``.

.. csv-table:: Conda and pip packages and licenses in the Windows environment
   :name: license-info_packages-conda-env-win32
   :header-rows: 1
   :delim: ;
   :file: ./license-tables/license-info_packages-conda-env-win32.csv

.. csv-table:: Conda and pip packages and licenses in the Linux environment
   :name: license-info_packages-conda-env-linux
   :header-rows: 1
   :delim: ;
   :file: ./license-tables/license-info_packages-conda-env-linux.csv

LLVM
----

.. csv-table::
   :name: license-info_llvm
   :widths: 10 50
   :delim: ;
   :file: ./license-tables/license-info_llvm.csv

RubyInstaller
-------------

.. csv-table::
   :name: license-info_ruby-installer
   :widths: 10 50
   :delim: ;
   :file: ./license-tables/license-info_ruby-installer.csv

Visual Studio Code
------------------

.. csv-table::
   :name: license-info_vs-code
   :widths: 10 50
   :delim: ;
   :file: ./license-tables/license-info_vs-code.csv

Visual Studio Code Extensions
-----------------------------

.. csv-table::
   :name: license-info_vs-code_extensions
   :header-rows: 1
   :delim: ;
   :file: ./license-tables/license-info_vs-code_extensions.csv

MinGW-w64 - for 32 and 64 bit Windows
-------------------------------------

.. csv-table::
   :name: license-info_mingw64
   :header-rows: 1
   :delim: ;
   :file: ./license-tables/license-info_mingw64.csv
