.. include:: ../../../macros.txt
.. include:: ../../../units.txt

.. _HOW_TO_USE_ASSERTIONS:

How to Use assertions
=====================

When developing it is sensible and good practice to use assertions in order to
verify conditions that should be true. The following guide summarizes the
different styles of assertions that are available in this codebase.

#error directive
----------------

The C preprocessor allows to fail directly in the parsing of the C-files. This
is useful for cases such as when a certain configuration through defines is not
supported. Generally, defines should be used as less as possible.

.. code-block:: c
    :linenos:
    :caption: Using #error directives
    :name: error-directive

    #define NUMBER_OF_XYZ 3
    #if (NUMBER_OF_XYZ > 2)
    #error "Configurations with more than 2 XYZ are not supported"
    #endif


The downside of using the error-directive is that the C-preprocessor runs
before any compilation is done. Therefore it is impossible to e.g., check size
requirements of variables.

static assertions
-----------------

Newer C-standards such as C11 have an assert that is able to do assertions
during compilation. It is called ``static_assert``. We have emulated this
function with a construction that will fail during compilation if the condition
is false. This allows to check for example the alignment of variables or that
the size of a variable is not zero.

.. code-block:: c
    :linenos:
    :caption: Using static assertions
    :name: static-assert

    /* db_variable is defined in your code and should contain something */
    static_assert((db_variable != 0), "database may not be zero");
    /* This assertion will not compile if db_variable has size zero. */


assertions
----------

Whenever it is impossible to determine a condition at compile time, normal
assertions can be used. In this project a macro called ``FAS_ASSERT(condition)``
exists. It supports bringing the BMS to a full-stop in case of a failing
assertion. Please refer to the module on implementation and usage details.

Assertions should be used generously in your code in order to assert conditions
that should always be true. Examples for this are:

* The variable passed to a function is explicitly documented as being not
  null and your code expects this. Make sure with an assertion that no
  programming error has occurred.
* The default case of a switch-case-statement is not expected to be reached in
  the program flow ever. You can add a ``FAS_ASSERT(FAS_TRAP)`` into the default
  case in order to trip when this defect occurs.

However, there also exist cases where an assertion is definitely not the right
tool. Examples are:

* A sensor value is expected to be in a certain range, but a hardware defect
  might lead to larger values. This should be handled by error-handling code.
* The communication to a slave-board fails or returns an invalid CRC. This is
  also an error that is plausible to occur and not necessarily connected
  to a programming fault.

To sum it up, assertions should only be used for defects that are introduced by
incorrect parts of the program. They should not be used for "normal" failures
that might occur and just have to be handled gracefully.

Finally, care should be taken as code with assertions should be tested
thoroughly in order to make sure, that no reboot loops or similar problems
occur. Apart from that having assertions enabled in production code is OK, as
they often catch errors that might lead to completely unexpected behavior.
