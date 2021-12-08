.. include:: ./macros.txt
.. include:: ./units.txt

.. _THE_FOXBMS_2_DOCUMENTATION:

The foxBMS 2 Documentation
==========================

Welcome to the |foxbms| documentation. |foxbms| is the modular and open source
Battery Management System (BMS) development platform from `Fraunhofer IISB`_.
This is the second generation of foxBMS. Its is a free, open and flexible
development environment to design beyond state-of-the-art complex battery
management systems.

.. note::

   The present version of the |foxbms| documentation is |version_foxbms|. It is
   available in HTML. More detailed information on the version history can be
   found in the :ref:`RELEASES` section. This documentation was generated on
   |timestamp|.

.. warning::

   The |foxbms| hardware and software is under permanent development. The free
   and open research and development platform |foxbms| as presented in the
   documentation hereafter is not provided to be used without any adaptions
   (e.g., to fulfill mandatory regulations) in consumer products, electric
   vehicles, production environments or any similar usages: it is only
   intended to be used by skilled professionals trained in designing battery
   system prototypes.

The documentation is divided in the following parts:

#. :ref:`GENERAL_INFORMATION_OVERVIEW`
#. :ref:`DESIGN_CONSIDERATIONS`
#. :ref:`GETTING_STARTED_OVERVIEW`
#. :ref:`SOFTWARE_DOCUMENTATION_OVERVIEW`
#. :ref:`HARDWARE_DOCUMENTATION_OVERVIEW`
#. :ref:`SYSTEM_DOCUMENTATION_OVERVIEW`
#. :ref:`TOOLS_DOCUMENTATION_OVERVIEW`
#. :ref:`DEVELOPER_MANUAL`

In the first part of the documentation, general information about |foxbms|
can be found (:ref:`GENERAL_INFORMATION_OVERVIEW`):

- Information about the versions and the related release notes of |foxbms|
- Licenses of the hardware, software and documentation of |foxbms|
- Motivation behind the |foxbms| initiative
- Technology roadmap of the upcoming hardware and software components
- Team involved in the development of |foxbms|

The second parts gives an overview about battery systems and their design in
general (:ref:`DESIGN_CONSIDERATIONS`):

- Abbreviations and naming conventions used in the documentation
- Overview of the platform
- Use case on which the documentation is based

The third part of the documentation (:ref:`GETTING_STARTED_OVERVIEW`) acts as a
guide through the setup of the |foxbms| development environment:

- How to install the required software
- How to set a workspace to use the sources

To work on the software, the :ref:`SOFTWARE_DOCUMENTATION_OVERVIEW` presents
the structure of the software, how the basic tasks are running and how to call
user-defined functions. Further, a how-to is provided in order to answer the
most common questions. A detailed description of the embedded software is found
in the |doxygen| documentation.

Next, the :ref:`HARDWARE_DOCUMENTATION_OVERVIEW` gives the details needed to
understand the hardware platform developed for |foxbms|.

The :ref:`SYSTEM_DOCUMENTATION_OVERVIEW` provides information on the
integration of |foxbms| in an application.

Numerous tools have been developed or adapted to support developing with the
|foxbms| platform. These tools and their usage are described in
:ref:`TOOLS_DOCUMENTATION_OVERVIEW`.

Finally, the :ref:`DEVELOPER_MANUAL` explains which rules need to be applied
when developing for the |foxbms| platform.

All sections are listed here:

.. _GENERAL_INFORMATION_OVERVIEW:

.. toctree::
    :titlesonly:
    :numbered:
    :maxdepth: 1
    :caption: General Information

    ./general/releases.rst
    ./general/changelog.rst
    ./general/motivation.rst
    ./general/safety/safety.rst
    ./general/license.rst
    ./general/team.rst

.. _DESIGN_CONSIDERATIONS:

.. toctree::
    :titlesonly:
    :numbered:
    :maxdepth: 1
    :caption: Introduction

    ./introduction/abbreviations-definitions.rst
    ./introduction/naming-conventions.rst
    ./introduction/bms-overview.rst
    ./introduction/use-case.rst

.. _GETTING_STARTED_OVERVIEW:

.. toctree::
    :titlesonly:
    :numbered:
    :maxdepth: 1
    :caption: Getting Started

    ./getting-started/getting-started.rst
    ./getting-started/repository-structure.rst
    ./getting-started/software-installation.rst
    ./getting-started/workspace.rst
    ./getting-started/first-steps-on-hardware.rst

.. _SOFTWARE_DOCUMENTATION_OVERVIEW:

.. toctree::
    :titlesonly:
    :numbered:
    :maxdepth: 1
    :caption: Software Documentation

    ./software/configuration/configuration.rst
    ./software/build/build.rst
    ./software/overview/sw-overview.rst
    ./software/modules/modules.rst
    ./software/unit-tests/unit-tests.rst
    ./software/build-process/build-process.rst
    ./software/build-environment/build-environment.rst
    ./software/how-to/how-to.rst
    ./software/architecture/architecture.rst
    ./software/api/overview.rst

.. _HARDWARE_DOCUMENTATION_OVERVIEW:

.. toctree::
    :titlesonly:
    :numbered:
    :maxdepth: 1
    :caption: Hardware Documentation

    ./hardware/hardware.rst
    ./hardware/design-resources.rst
    ./hardware/connectors.rst
    ./hardware/master.rst
    ./hardware/slaves.rst
    ./hardware/interfaces.rst

.. _SYSTEM_DOCUMENTATION_OVERVIEW:

.. toctree::
    :titlesonly:
    :numbered:
    :maxdepth: 1
    :caption: System Documentation

    ./system/system-introduction.rst
    ./system/system-voltage-and-current-monitoring.rst

.. _TOOLS_DOCUMENTATION_OVERVIEW:

.. toctree::
    :titlesonly:
    :numbered:
    :maxdepth: 1
    :caption: Tools Documentation

    ./tools/dbc.rst
    ./tools/log-parser.rst
    ./tools/waf-tools/waf-tools.rst
    ./tools/debugger/debug-application.rst
    ./tools/halcogen/halcogen.rst
    ./tools/static-analysis/axivion.rst
    ./tools/static-analysis/cppcheck.rst

.. _DEVELOPER_MANUAL:

.. toctree::
    :titlesonly:
    :numbered:
    :maxdepth: 1
    :caption: Developer Manual

    ./developer-manual/preface.rst
    ./developer-manual/style-guide/style-guide.rst
    ./developer-manual/software-developer-manual.rst
    ./developer-manual/hardware-developer-manual.rst

.. _MISCELLANEOUS_INFORMATION:

.. toctree::
    :titlesonly:
    :numbered:
    :maxdepth: 1
    :caption: Miscellaneous Information

    ./misc/acknowledgements.rst
    ./misc/indices-and-tables.rst
    ./misc/bibliography.rst
