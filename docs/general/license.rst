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

.. include:: licenses-packages-conda-env-spelling.txt
.. include:: licenses-packages-conda-env-spelling-build-strings.txt

*****************
|foxbms| Licenses
*****************

The license texts are included verbatim in this documentation.

|foxbms| Software License
=========================

- foxBMS 2 software is licensed under the BSD 3-Clause License.
- The SPDX short identifier is
  `BSD-3-Clause <https://spdx.org/licenses/BSD-3-Clause.html>`_.

The license text is included in this documentation at
:ref:`BSD_3_CLAUSE_LICENSE__BSD_3_CLAUSE__NEW__OR__REVISED__LICENSE_`.

|foxbms| Hardware and Documentation License
===========================================

- foxBMS 2 hardware and documentation is licensed under the
  Creative Commons Attribution 4.0 International License.
  The license can be obtained from
  https://creativecommons.org/licenses/by/4.0/.
- The SPDX short identifier is
  `CC-BY-4.0 <https://spdx.org/licenses/CC-BY-4.0.html>`_.

The license text is included in this documentation at
:ref:`CREATIVE_COMMONS_ATTRIBUTION_4_0_INTERNATIONAL_LICENSE`.

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

    This is not legal advice. The license list hereafter shows the licenses
    used in |foxbms| to our best knowledge.

This is a list of third party licenses used by |foxbms|. This is split into
licenses for third party sources and software that are included within the
repository and software that must be installed in order to develop BMS
applications within or with |foxbms|.

Software listed under `Third Party Software`_ are typically only used for
|foxbms| development and do not need to be altered.

Third Party Sources and Software
================================

Ceedling
--------

+---------------------------+-----------------------------------------------------------------------+
| Location in repository    | ``./tools/vendor/ceedling``                                           |
+---------------------------+-----------------------------------------------------------------------+
| License                   | MIT-like License                                                      |
+---------------------------+-----------------------------------------------------------------------+
| License URL               | https://github.com/ThrowTheSwitch/Ceedling/blob/v0.31.1/license.txt   |
+---------------------------+-----------------------------------------------------------------------+
| Latest license check      | 2021-07-21                                                            |
+---------------------------+-----------------------------------------------------------------------+
| Website                   | https://github.com/ThrowTheSwitch/Ceedling                            |
+---------------------------+-----------------------------------------------------------------------+
| Gem                       | https://rubygems.org/gems/ceedling                                    |
+---------------------------+-----------------------------------------------------------------------+

FreeRTOS
--------

+---------------------------+---------------------------------------+
| Location in repository    | ``./src/os/freertos``                 |
+---------------------------+---------------------------------------+
| License                   | MIT License                           |
+---------------------------+---------------------------------------+
| SPDX short identifier     | MIT                                   |
+---------------------------+---------------------------------------+
| License URL               | https://www.freertos.org/a00114.html  |
+---------------------------+---------------------------------------+
| Latest license check      | 2020-06-23                            |
+---------------------------+---------------------------------------+
| Website                   | https://www.freertos.org/             |
+---------------------------+---------------------------------------+

WAF
---


Binary
^^^^^^

+---------------------------+---------------------------------------------------+
| Location in repository    | ``./tools/waf``                                   |
+---------------------------+---------------------------------------------------+
| License                   | BSD                                               |
+---------------------------+---------------------------------------------------+
| License URL               | https://waf.io/book/#_license_and_redistribution  |
+---------------------------+---------------------------------------------------+
| Latest license check      | 2020-06-23                                        |
+---------------------------+---------------------------------------------------+
| Website                   | https://waf.io/                                   |
+---------------------------+---------------------------------------------------+

Unit Tests
^^^^^^^^^^

+---------------------------+---------------------------------------------------------------------------------------------------+
| Location in repository    | ``./tests/scripts/waf-core``                                                                      |
+---------------------------+---------------------------------------------------------------------------------------------------+
| License                   | BSD                                                                                               |
+---------------------------+---------------------------------------------------------------------------------------------------+
| License URL               | https://gitlab.com/ita1024/waf/-/blob/3f8bb163290eb8fbfc3b26d61dd04aa5a6a29d4a/waf-light#L6-30    |
+---------------------------+---------------------------------------------------------------------------------------------------+
| Latest license check      | 2021-03-11                                                                                        |
+---------------------------+---------------------------------------------------------------------------------------------------+
| Website                   | https://waf.io/                                                                                   |
+---------------------------+---------------------------------------------------------------------------------------------------+

Third Party Software
====================

|texas-instruments| |code-composer-studio|
------------------------------------------

+-----------------------+---------------------------------------------------+
| License               | Modified TSPA                                     |
+-----------------------+---------------------------------------------------+
| License File          | ``<ccs-install-dir>/ccs/doc/TSPA_Modified.pdf``   |
+-----------------------+---------------------------------------------------+
| Latest license check  | 2020-06-23                                        |
+-----------------------+---------------------------------------------------+
| Website               | https://www.ti.com/tool/CCSTUDIO                  |
+-----------------------+---------------------------------------------------+

|texas-instruments| |halcogen|
------------------------------

+-----------------------+-----------------------------------------------------------------------+
| License File          | ``<halcogen-install-dir>/Production_License_Agreement_062612.pdf``    |
+-----------------------+-----------------------------------------------------------------------+
| Latest license check  | 2020-06-23                                                            |
+-----------------------+-----------------------------------------------------------------------+
| Website               | https://www.ti.com/tool/HALCOGEN                                      |
+-----------------------+-----------------------------------------------------------------------+

Miniconda
---------

+-----------------------+---------------------------------------------------+
| License               | 3-clause BSD                                      |
+-----------------------+---------------------------------------------------+
| SPDX short identifier | BSD-3-Clause                                      |
+-----------------------+---------------------------------------------------+
| License URL           | https://docs.conda.io/en/latest/license.html      |
+-----------------------+---------------------------------------------------+
| Latest license check  | 2020-06-23                                        |
+-----------------------+---------------------------------------------------+
| Website               | https://docs.conda.io/en/latest/miniconda.html    |
+-----------------------+---------------------------------------------------+

Miniconda ships a `Python <https://www.python.org>`_ interpreter. The Python
interpreter license is found in ``<miniconda-install-dir>/LICENSE_PYTHON.txt``.

The conda environments shipped within the |foxbms| repository
(``conf/enc/conda_env_win32.yaml``) specifies a couple of conda and pip
packages. These packages and their licenses are listed in
:numref:`conda-and-pip-packages-and-licenses-win32` and
:numref:`conda-and-pip-packages-and-licenses-linux`.
This list is generated based on ``conda list --show-channel-urls``.

- For ``pypi`` packages the license information is taken from
  https://pypi.org/.
- For conda packages the  license information is obtained by running
  ``conda info package_name --info`` or directly looked up in the conda build
  recipe.
- As a help for updating the list you can call
  ``./tools/utils/bash/run-python-script.sh ./tools/utils/generate_license_list.py -f ./docs/general/licenses-packages-conda-env-win32.csv > ./docs/general/licenses-packages-conda-env-new.csv``.

.. csv-table:: Conda and pip packages and licenses in the Windows environment
   :name: conda-and-pip-packages-and-licenses-win32
   :header-rows: 1
   :delim: ;
   :file: ./licenses-packages-conda-env-win32.csv

.. csv-table:: Conda and pip packages and licenses in the Linux environment
   :name: conda-and-pip-packages-and-licenses-linux
   :header-rows: 1
   :delim: ;
   :file: ./licenses-packages-conda-env-linux.csv

LLVM
----

+-----------------------+----------------------------------------------+
| License               | LLVM Exceptions to the Apache 2.0 License    |
+-----------------------+----------------------------------------------+
| SPDX short identifier | LLVM-exception                               |
+-----------------------+----------------------------------------------+
| License URL           | https://releases.llvm.org/11.0.0/LICENSE.TXT |
+-----------------------+----------------------------------------------+
| Latest license check  | 2020-06-23                                   |
+-----------------------+----------------------------------------------+
| Website               | https://llvm.org/                            |
+-----------------------+----------------------------------------------+


Cppcheck
--------

+-----------------------+-----------------------------------+
| License               | GPLv3                             |
+-----------------------+-----------------------------------+
| License File          | <cppcheck-install-dir>/GPLv3.txt  |
+-----------------------+-----------------------------------+
| Latest license check  | 2020-06-23                        |
+-----------------------+-----------------------------------+
| Website               | http://cppcheck.sourceforge.net/  |
+-----------------------+-----------------------------------+

RubyInstaller
-------------

+-----------------------+-------------------------------+
| License               | 3-clause BSD                  |
+-----------------------+-------------------------------+
| SPDX short identifier | BSD-3-Clause                  |
+-----------------------+-------------------------------+
| Latest license check  | 2020-06-23                    |
+-----------------------+-------------------------------+
| Website               | https://rubyinstaller.org/    |
+-----------------------+-------------------------------+

Visual Studio Code
------------------

+-----------------------+---------------------------------------+
| License URL           | https://code.visualstudio.com/License |
+-----------------------+---------------------------------------+
| Latest license check  | 2020-06-23                            |
+-----------------------+---------------------------------------+
| Website               | https://code.visualstudio.com         |
+-----------------------+---------------------------------------+
| Development           | https://github.com/microsoft/vscode   |
+-----------------------+---------------------------------------+

:numref:`vscode-extensions-and-licenses` shows the licenses of VS Code
extensions.

.. csv-table:: VS Code extensions and licenses
   :name: vscode-extensions-and-licenses
   :header-rows: 1
   :delim: ;
   :file: ./licenses-vscode-extensions.csv

MinGW-w64 - for 32 and 64 bit Windows
-------------------------------------

+-----------------------+-----------------------------------------------+
| License Files         | <gcc-install-dir>/licenses/*                  |
+-----------------------+-----------------------------------------------+
| Latest license check  | 2020-06-23                                    |
+-----------------------+-----------------------------------------------+
| Website               | https://sourceforge.net/projects/mingw-w64    |
+-----------------------+-----------------------------------------------+
