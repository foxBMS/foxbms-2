.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _TI_HALCOGEN_TOOL:

##################
|ti-halcogen| Tool
##################

.. note::
   A documentation of the tool |ti-halcogen| can be found in this section, the
   Waf tool wrapper for this project in :ref:`WAF_TOOL_HALCOGEN`, information
   on configuring |ti-halcogen| in the context of the toolchain of this project
   in :ref:`HAL_CONFIGURATION` and an example on how to use a pre-generated
   HAL instead of generating it in the compilation step in
   :ref:`HOW_TO_USE_GENERATED_SOURCES_FROM_HALCOGEN`.

This part of the manual summarizes the usage of |ti-halcogen| and references
important documents by TI.

.. _UPDATING_THE_STARTUP_ROUTINE:

Updating the Startup Routine
============================

|ti-halcogen| creates the source file ``HL_sys_startup.c`` which implements
(a weak implementation of) the function ``_c_int00`` (the system's startup
routine). |foxbms| provides its own **non**-weak implementation of ``_c_int00``
in ``fstartup.c``.
The |foxbms| implementation of ``_c_int00`` must be coupled to the the current
|ti-halcogen| configuration.

Most changes in the |ti-halcogen| project do not alter the startup behavior and
no further action needs to be taken into account.
However there are settings that alter the startup behavior.
Such settings need to be ported to ``fstartup.c`` as this non-weak
implementation of ``_c_int00`` outweighs the generated, new version of
``_c_int00`` in ``HL_sys_startup.c``.
Otherwise the startup function used by |foxbms| would not reflect the
|ti-halcogen| configuration.
The :ref:`WAF_TOOL_HALCOGEN` provides a mechanism to detected such changes.
The hash of the current ``HL_sys_startup.c`` implementation is stored in
``src/app/hal/app-startup.hash`` and compared to the actual hash of the
generated ``HL_sys_startup.c`` file.
If these are not the same, the build aborts with the following message:

.. literalinclude:: fstartup.c-check.txt
   :language: console
   :caption: Example error message on hash mismatch of ``HL_sys_startup.c``
   :name: halcogen-configuration-error

The build aborts as the expected hash is
``b'e2e61496edd65f44d7cc811b504ad1f2'`` while the actual hash is
``b'1something-other234'``.
Next, the function ``_c_int00`` in the two files (``fstartup.c``) and
``HL_sys_startup.c`` needs to be compared by the developer and the developer
needs to update the ``_c_int00`` implementation in the file ``fstartup.c`` to
reflect the |ti-halcogen| startup routine.
The concluding step is to update the hash value in
``src/app/hal/app-startup.hash`` with ``1something-other``.
Now the build toolchain knows, that the changes applied in the |ti-halcogen|
are reflected in the dependencies and the build will not abort after the HAL
sources are generated.

The following paths exists, after |ti-halcogen| has run and generated the
sources (``target`` is either ``app`` or ``bootloader`` respectively):

- The hash of
  ``build/<target>_embedded/src/<target>/hal/source/HL_sys_startup.c``
  matches the expected hash in ``<target>-startup.hash``.
  There is nothing to do and the build proceeds.
- The hash of
  ``build/<target>_embedded/src/<target>/hal/source/HL_sys_startup.c``
  does **not** match the expected hash in
  ``src/<target>/hal/<target>-startup.hash``.
  The build process is aborted.

  - The developer needs to check the generated ``HL_sys_startup.c`` source and
    diff it against the file against ``src/<target>/main/fstartup.c``.
  - The developer needs to decide which changes need to be ported to
    ``src/<target>/main/fstartup.c`` and apply them.
  - The developer needs to update the hash in
    ``src/<target>/hal/<target>-startup.hash``.
  - Re-run the build process.

Release Notes
=============

When using |ti-halcogen| it is mandatory to be aware of the official
`release notes SPNA203 <https://www.ti.com/lit/ml/spna203/spna203.pdf>`_.
Special care **SHALL** be taken when reading the section "Known issues and
limitations".
It has to be screened for issues affecting the particular implementation of the
BMS.

Moreover, the most recent revision of the
`Hercules Safety MCU Resource Guide <https://software-dl.ti.com/hercules/hercules_docs/latest/hercules/>`_
**SHALL** be read carefully.

Additional Known Issues
=======================

In addition to the known issues described in the release notes we have
encountered additional issues.
This section captures the issue, together with a reference to correspondence
with TI (not necessarily by us) and a description of work-around measures.
This section refers to |ti-halcogen| in version ``04.07.01``.

Incompatibility of enum-definitions with newer compilers in strict ANSI mode
----------------------------------------------------------------------------

The |ti-arm-cgt| compilers since version ``20.2.2.LTS`` (shipped with CCS
``10.2.0.00009``) contain a bugfix for the underlying data type of enums
described in `10334 <https://sir.ext.ti.com/jira/browse/EXT_EP-10334>`_.
The code that is generated by |ti-halcogen| heavily relies on this behavior
when implementing enums.

This issue affects compilation of the HAL in strict ANSI mode. In this mode,
the underlying data type of enums is *signed char* (as expected by the C
standard).
These enums, however, are used for comparison with *unsigned integers* in the
generated HAL.
Details can be found in a
`TI forum post <https://e2e.ti.com/support/tools/ccs/f/code-composer-studio-forum/993701/codecomposer-ccs10-2-0-20-2-2lts-cannot-compile-halcogen-04-07-01-generated-sources-anymore>`_.

The correct fix for this issue would be to extend all relevant generated enums
in the HAL with an entry ``dummy=UCHAR_MAX`` as last entry.
This tells the compiler to use *unsigned char* as underlying data type for
these enums.
Since we cannot modify the HAL without loosing the ability to generate it and
TI is not planning to provide a fix in |ti-halcogen| as stated in the linked
forum post, a second fix as described in the following paragraph is possible.

The compilation failure in this case comes from a diagnostic error ``2142``
that is raised to an error in our toolchain. Under the assumption that the
generated HAL is correct, these warnings can be disabled as described by TI by
removing ``--emit_warnings_as_errors`` and ``--diag_error=2142`` for the
compilation of the HAL.

This issue is fixed with the latter option in |foxbms| versions ``v1.1.0`` and
upwards.

Message Definitions in CAN4
---------------------------

|ti-halcogen| has a bug that prevents it from generating a complete set of
message definitions for the message boxes 33 to 64.
This issue is described in a
`TI forum post about HALCoGen v04.05.02 <https://e2e.ti.com/support/microcontrollers/other-microcontrollers-group/other/f/other-microcontrollers-forum/543081/halcogen-not-generating-code-for-can4-mailboxes-32>`_
and an additional
`TI forum post about HALCoGen v04.07.01 <https://e2e.ti.com/support/microcontrollers/other-microcontrollers-group/other/f/other-microcontrollers-forum/998737/halcogen-not-generating-code-for-can4-mailboxes-32-in-halcogen-04-07-01>`_.
TI plans to update the release note of |ti-halcogen| in the third quarter of
2021 with this information.

For the workaround, the configuration files of |ti-halcogen| have to be
modified.
To be precise, the file
``C:\ti\Hercules\HALCoGen\v04.07.01\drivers\TMS570LC4357ZWT\CAN4v000.xml``
(if |ti-halcogen| is installed into the standard directory) has to be opened
and the limit of the for-loop in line ``144`` has to be changed from ``32`` to
``64``.

The result should look like the content of :numref:`modify-can4-xml`.

.. code-block:: javascript
   :linenos:
   :lineno-start: 143
   :emphasize-lines: 2
   :caption: Modified line 144 in ``CAN4v000.xml``
   :name: modify-can4-xml

         var i=0
      for(i=1;i &lt;= 64;i++)
            {

Mailbox 42 Configuration in CAN1
--------------------------------

|ti-halcogen| has a bug that the initialization code of CAN1 mailbox 42 is not
generated whatever is configured in |ti-halcogen|.
To use this mailbox, the user needs to initialize this mailbox manually (see
`TI forum post <https://e2e.ti.com/support/microcontrollers/arm-based-microcontrollers-group/arm-based-microcontrollers/f/arm-based-microcontrollers-forum/905953/ccs-tms570lc4357-halcogen-can-message-configuration-bug-can1-message-41-42---can2-message-41-42>`).
