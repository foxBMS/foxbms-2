.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _C_CODING_GUIDELINES:

C Coding Guidelines
===================

These coding guidelines **MUST** be applied to all ``C`` source and header
files.

The source files **MUST** be successfully checked by running the waf command
``check_guidelines`` before files can be merged into the master branch of the
repository.

General Information
-------------------

Generally |foxbms| uses for the embedded code
`1TBS <https://en.wikipedia.org/wiki/Indentation_style#Variant:_1TBS_(OTBS)>`_.
This is checked by ``clang-format``. The ``clang-format`` configuration is
found in ``./.clang-format``. The ``C`` source and header files can be checked
by running ``waf check_guidelines``. Using VS Code files are automatically
saved correctly (see :ref:`CREATING_A_WORKSPACE`) when ``clang-format`` is
installed (see :ref:`software_prerequisites`) using :kbd:`Ctrl-S`.

.. warning::

    The style of third party sources (generated HAL sources in
    ``build/bin/src/hal/**`` and ``src/os``) should not be changed. To save
    without reformatting use :kbd:`Ctrl-K + Ctrl-Shift-S`.

The following list shows more detailed rules for |foxbms|. Every rule has some
context and/or rationale and notes that clearly state the rules, followed by a
correct examples. If it supports the clarification incorrect examples may also
be shown.

The following rules generally apply and follow the naming schema
``C:<ongoing-number>``.

.. _rule_c_filenames:

Filenames (``C:001``)
---------------------

Additional to the general file naming rules the following **MUST**
be applied.

.. admonition:: File name rules

    - The general file naming rules **MUST** be applied (see
      :numref:`rule_general_filenames`).
    - ``Assembler`` source files **MUST** use ``.asm`` as file extension.
    - ``C`` source files **MUST** use ``.c`` as file extension.
    - ``C`` header files **MUST** use ``.h`` as file extension.
    - Software functionalities (e.g., low level driver, algorithms, etc...)
      should be split in configuration files and the actual implementation.
      These configuration files **MUST** end with ``_cfg.c`` or ``_cfg.h``
      respectively.

For example the valid file names for an implementation of ``driver``, that is
split into a driver and a configuration part, is:

- ``driver.c``
- ``driver.h``
- ``driver_cfg.c``
- ``driver_cfg.h``

.. _rule_c_header:

Header (``C:002``)
------------------

.. admonition:: C file header

    C source and header files **MUST** start with the following header:

   .. literalinclude:: ./../../../conf/tpl/c.c
      :language: c
      :linenos:
      :lines: 1-40
      :caption: File header for ``.c`` and ``.h`` files.
      :name: file-header-c-and-h

.. _rule_c_linelength:

Linelength (``C:003``)
----------------------

.. admonition:: Linelength rules

   - Each line of text in your code **SHOULD** be at most 120 characters
     long. A line **MAY** exceed 120 characters if it is

     - a comment line which is not feasible to split without harming
       readability, ease of cut and paste or auto-linking, e.g., if a line
       contains an example command or a literal URL longer than 120 characters
       or
     - raw-string literal with content that exceeds 120 characters. Except for
       test code, such literals should appear near the top of a file.

   - Each line of doxygen comment in your code **SHOULD** be at most 80
     characters long. A line **MAY** exceed 80 characters if it is

     - a comment line which is not feasible to split without harming
       readability, auto-linking, e.g., a literal URL longer than 80 characters
       or
     - a raw-string literal with content that exceeds 80 characters.


.. _rule_c_file_level_doxygen:

File level doxygen (``C:004``)
------------------------------

`Doxygen <https://www.doxygen.nl/>`_ is used to create an API documentation.

.. admonition:: File level doxygen rule

    - Every file **MUST** be documented with doxygen style comments in order to
      be properly processed by doxygen.
    - The file level doxygen **MUST** come after the license header separated
      by a blank line.
    - The following doxygen parameters **MUST** be included in every file:
      ``@file``, ``@author``, ``@date``, ``@updated``, ``@ingroup``,
      ``@prefix``, ``@brief``, ``@details``
    - All doxygen parameter arguments **MUST** be whitespace aligned.
    - After ``@prefix``, ``@brief`` and ``@details`` there **MUST** be an blank
      line.
    - The ``@prefix`` argument **MUST** use between two and five
      uppercase alphanumericals starting with an uppercase character.
    - The ``@prefix`` argument **MAY** use abbreviations.
    - The ``@prefix`` and ``@ingroup`` arguments **MUST** use alphanumeric
      uppercase characters including underscores.
    - The ``@date`` argument **MUST** be an ISO 8601 date followed by
      ``(date of creation)``.
    - The ``@updated`` argument **MUST** be an ISO 8601 date followed by
      ``(date of last update)``.

:numref:`doxygen-test-h` shows how this looks for a file ``test.h``.

.. code-block:: c
   :linenos:
   :emphasize-lines: 2-7,9,11,12
   :caption: file level doxygen for test.h
   :name: doxygen-test-h

    /**
     * @file    test.h
     * @author  foxBMS Team
     * @date    2020-01-02 (date of creation)
     * @updated 2020-03-19 (date of last update)
     * @ingroup SOME_GROUP
     * @prefix  TST
     *
     * @brief   This is the header for the test driver
     *
     * @details This describes what is exactly implemented.
     *          And this may span multiple lines.
     *
     */

     /* here comes the actual code */

.. _rule_c_include_guard:

Include guard (``C:005``)
-------------------------

An include guard, is a construct used in ``C`` to avoid the problem
of multiple inclusion when dealing with the include directive.

.. admonition:: ``C:005`` Include guard rules

    - All header files **MUST** implement include guards to prevent multiple
      inclusions.
    - The include guard **MUST** come right after the file level doxygen
      comment.
    - The format of the include guard **MUST** consist of the prefix
      ``FOXBMS__``, followed by the  file name in uppercase and then followed
      by ``_H_``.
    - There **MUST NOT** be a blank line between ``#ifndef`` and ``#define``.
    - There **MUST** be a blank line after ``#define``.

:numref:`include-guard-abc-h` shows how this looks for a file ``abc.h``, where
the include guard would be ``FOXBMS__ABC_H_``.


.. code-block:: c
    :linenos:
    :emphasize-lines: 1,2,6
    :caption: Include guard for ``abc.h``
    :name: include-guard-abc-h

    #ifndef FOXBMS__ABC_H_
    #define FOXBMS__ABC_H_

    /* Some code */

    #endif /* FOXBMS__ABC_H_ */

.. _rule_c_sections:

C Sections (``C:006``)
----------------------

.. admonition:: C sections

    Every .c and .h file **MUST** contain all specific section comments in the
    correct order. There are different section comments for sources and headers
    for source fils (files in ``src/**``) and test files (files in
    ``tests/unit/**``)

:numref:`c_section_header` shows how this looks for a file ``header.h``.

.. code-block:: c
   :linenos:
   :emphasize-lines: 1-5
   :caption: c section for header.h
   :name: c_section_header

    /*========== Includes =======================================================*/
    /*========== Macros and Definitions =========================================*/
    /*========== Extern Constant and Variable Declarations ======================*/
    /*========== Extern Function Prototypes =====================================*/
    /*========== Externalized Static Functions Prototypes (Unit Test) ===========*/


:numref:`c_section_source` shows how this looks for a file ``source.c``.

.. code-block:: c
   :linenos:
   :emphasize-lines: 1-8
   :caption: c section for source.c
   :name: c_section_source

    /*========== Includes =======================================================*/
    /*========== Macros and Definitions =========================================*/
    /*========== Static Constant and Variable Definitions =======================*/
    /*========== Extern Constant and Variable Definitions =======================*/
    /*========== Static Function Prototypes =====================================*/
    /*========== Static Function Implementations ================================*/
    /*========== Extern Function Implementations ================================*/
    /*========== Externalized Static Function Implementations (Unit Test) =======*/


:numref:`c_section_test_header` shows how this looks for a file ``./test/header.h``.

.. code-block:: c
   :linenos:
   :emphasize-lines: 1-2
   :caption: c section for ./test/header.h
   :name: c_section_test_header

    /*========== Includes =======================================================*/
    /*========== Macros and Definitions =========================================*/

:numref:`c_section_test_source` shows how this looks for a file ``./test/source.c``.

.. code-block:: c
   :linenos:
   :emphasize-lines: 1-4
   :caption: c section for ./test/source.c
   :name: c_section_test_source

    /*========== Includes =======================================================*/
    /*========== Definitions and Implementations for Unit Test ==================*/
    /*========== Setup and Teardown =============================================*/
    /*========== Test Cases =====================================================*/


.. _rule_c_includes:

Includes (``C:007``)
--------------------

Includes are used to insert the contents of a second file into the
original file. There are two scenarios to be considered for file includes. They
distinguish if the software module provides configuration files or not.

.. admonition:: ``C:007``: Include rules

    - All includes **MUST** be listed after the ``includes`` marker.
    - Only required includes **MUST** be added.
    - Forward declarations **MUST NOT** be used. Instead, you **MUST**
      ``#include`` all the headers that declare functions you need.
    - Configuration header files **MUST** apply the following order of
      includes:

        #. Include ``general.h``
        #. A blank line
        #. Add required includes in alphabetical order according to rule
    - Other header files **MUST** apply the following order of includes:

        #. Include corresponding configuration header if it exists, otherwise
           include ``general.h``
        #. A blank line
        #. Add required includes in alphabetical order according to rule
    - Source files **MUST** apply the following order or includes:

        #. Include corresponding header file
        #. A blank line
        #. Add required includes in alphabetical order according to rule
    - The rule for sorting required includes is (omit block if empty):

        #. ``unity.h`` (for unit tests)
        #. A blank line
        #. Generated Mock-header (for unit tests)
        #. A blank line
        #. Any ``*_cfg.h`` that is included
        #. A blank line
        #. HAL-headers starting with ``HL_``-header files and ending with
           ``ti_``-header files
        #. A blank line
        #. FreeRTOS headers starting with ``FreeRTOS.h``
        #. A blank line
        #. All other required headers except Mocks


:numref:`includes-abc_cfg-h`, :numref:`includes-abc_cfg-c`,
:numref:`includes-abc-h` and :numref:`includes-abc-c` show how includes
**MUST** be added.

.. code-block:: c
    :linenos:
    :emphasize-lines: 2,4,5
    :caption: Include order for ``abc_cfg.h``
    :name: includes-abc_cfg-h

    /*================== Includes ===============================================*/
    #include "general.h"

    #include "required_additional_header.h"
    #include "required_additional_header2.h"

.. code-block:: c
    :linenos:
    :emphasize-lines: 2,4,5
    :caption: Include order for ``abc_cfg.c``
    :name: includes-abc_cfg-c

    /*================== Includes ===============================================*/
    #include "abc_cfg.h"

    #include "some_other_required_header.h"
    #include "some_other_required_header2.h"


.. code-block:: c
    :linenos:
    :emphasize-lines: 2,4,5
    :caption: Include oder for ``abc.h``
    :name: includes-abc-h

    /*================== Includes ===============================================*/
    #include "abc_cfg.h"

    #include "required_different_header.h"
    #include "required_very_different_header.h"

.. code-block:: c
    :linenos:
    :emphasize-lines: 2,4,5,6
    :caption: Include order for ``abc.c``
    :name: includes-abc-c

    /*================== Includes ===============================================*/
    #include "abc.h"

    #include "FreeRTOS.h"
    #include "task.h"

    #include "first_used_header_1.h"
    #include "second_used_header.h"
    #include "xyz.h"

.. _rule_c_scoping:

Scoping (``C:008``)
-------------------

.. admonition:: ``C:008``: Scope rules

    - All functions, variables, typedefs, macros etc. **MUST** be declared
      the narrowest scope possible (function, file, global).

       - If a function, macro, typedef or variable is used by multiple
         files/modules, declare it public.
       - If a function, macro, typedef or variable is only used within a file,
         declare it static.
       - If variable is only used within a function, declare it inside the
         function.

.. code-block:: c
    :linenos:
    :caption: Narrowest variable scope
    :name: narrowest-variable-scope

    /*================== Extern Constant and Variable Declarations ==============*/
    extern uint32_t abc_globalVariable;  /*!< Important global variable that is used by multiple files/modules */

    /*================== Extern Function Prototypes =============================*/
    /**
      * @brief   Function to do important stuff.
      */
    extern void ABC_GlobalImportantFunction(void);

    /*================== Static Constant and Variable Definitions ===============*/
    static uint32_t abc_staticVariable = 0;  /*!< File static variable that is used within multiple functions */

    /*================== Static Function Prototypes =============================*/
    /**
      * @brief   Function to do important stuff in this file/module.
      */
    static void ABC_StaticImportantFunction(void);

    /*================== Static Function Implementations ========================*/
    static void ABC_StaticImportantFunction(void) {
        uint32_t tempVariable = 0;  /* Temporary variable to do calculations in this function */
    }

.. _rule_c_function_names:

Function names (``C:009``)
--------------------------

.. admonition:: Function naming rules

    Function names **MUST** start with the uppercase module prefix followed by
    a capital letter with capital letters for each new word (Pascal Case). The
    only exception are the :ref:`unit-test-functions <UNIT_TESTS>` which start
    with ``test`` due to ceedling requiring it.

:numref:`function-names`, shows correctly named functions.

.. code-block:: c
    :linenos:
    :caption: Function names using the uppercase module prefix.
    :name: function-names

    void ABC_ImportantFunction(void);
    void ABC_EvenMoreImportantFunction(void);

.. _rule_c_function_scopes:

Function scopes (``C:010``)
---------------------------

.. admonition:: Function scope rules

    - Global and static functions **MUST** be declared with the respective
      keywords ``extern`` or ``static``. This keyword **MUST** be used for
      function prototype declaration and the function definition.
    - Global function prototypes **MUST** be declared in the header file.
    - Static function prototypes **MUST** be declared in the source file.

:numref:`function-prototype-and-doxygen` and
:numref:`function-implementation-and-doxygen` show correctly declared and
implemented functions on the header ``abc.h`` and respective source file
``abc.c``.

.. _rule_c_function_doxygen_documentation:

Function doxygen documentation (``C:011``)
------------------------------------------

.. admonition:: Function scope rules

    - The doxygen documentation describing each function **MUST** be placed
      above the prototype declaration.
    - All function prototype declarations and function implementations **MUST**
      be placed in the correct sections in the source/header files.
    - The following doxygen parameters **MUST** be documented for every
      function: ``@brief`` and ``@details``.
    - Doxygen parameter ``@return`` **MUST** be documented if the return type
      is not of type ``void``.
    - Function arguments **MUST** be documented using the doxygen parameter
      ``@param``.
    - All doxygen parameter arguments **MUST** be whitespace aligned.

:numref:`function-prototype-and-doxygen` and
:numref:`function-implementation-and-doxygen` show doxygen documented functions
in a file ``abc.c``.

.. code-block:: c
    :linenos:
    :caption: Global function declaration in ``abc.h``, placed in the ``Extern Function Prototypes`` section.
    :name: function-prototype-and-doxygen

    /*========== Extern Function Prototypes =====================================*/
    /**
     * @brief   Does this and that
     * @details Detailed description of this function
     * @return  some unit8_t which describes xyz
     */
    extern unit8_t ABC_ImportantFunction(void);

.. code-block:: c
    :linenos:
    :caption: Global function implementation in abc.c, placed in the ``Extern Function Implementations`` section.
    :name: function-implementation-and-doxygen

    /*========== Static Function Prototypes =====================================*/
    /**
     * @brief   Does something
     * @details Detailed description of this function
     * @return  some unit8_t which describes xyz
     */
    static unit8_t ABC_Another_Function(void);

    /*========== Static Function Implementations ================================*/
    static unit8_t ABC_Another_Function(void) {
        /* code */
        return 0;
    }

    /*========== Extern Function Implementations ================================*/
    extern unit8_t ABC_ImportantFunction(void) {
        /* code */
        return 0;
    }

.. _rule_c_function_return_value:

Function return statement (``C:012``)
-------------------------------------

.. admonition:: Return statement rules

    Parentheses surrounding the return expression **SHOULD NOT** be used. Use
    parentheses in return expression only in places where you would use them
    in normal assignments.

:numref:`return-parentheses-usage` shows how to correctly use parentheses and
the return statement.

.. code-block:: c
    :linenos:
    :caption: Correct usage of the ``return`` statement.
    :name: return-parentheses-usage

    return result;        /* No parentheses in the simple case. */
    /* Parentheses OK to make a complex expression more readable. */
    return (some_long_condition &&
            another_condition);

.. _rule_c_function_calls:

Function calls (``C:013``)
--------------------------

.. admonition:: Function call rules

    - Multiple arguments **SHOULD** be put on a single line to reduce the
      number of lines necessary for calling a function unless there is a
      specific readability problem. Some find that formatting with strictly one
      argument on each line is more readable and simplifies editing of the
      arguments. However, we prioritize for the reader over the ease of editing
      arguments, and most readability problems are better addressed with the
      following techniques.
    - If the arguments do not all fit on one line, they **MAY** be broken up
      onto multiple lines, with each subsequent line aligned with the first
      argument.
    - Arguments **MAY** also be placed on subsequent lines with an eight space
      indent.
    - If having multiple arguments in a single line decreases readability due
      to the complexity or confusing nature of the expressions that make up
      some arguments, it is **RECOMMENDED** to

        - create variables that capture those arguments in a descriptive name,
        - put the confusing argument on its own line with an explanatory
          comment.
        - If there is still a case where one argument is significantly more
          readable on its own line, then put it on its own line. The decision
          should be specific to the argument which is made more readable rather
          than a general policy.
        - Sometimes arguments form a structure that is important for
          readability. In those cases, it is **RECOMMENDED** to format the
          arguments according to that structure

Different correct ways to call functions with multiple parameters or long
function names are given in :numref:`function-call`.

.. code-block:: c
    :linenos:
    :caption: Correct formatting of function calls.
    :name: function-call


    /* use variable to capture value, before passing to function */
    int16_t myHeuristic = scores[x] * y + bases[x];
    int16_t result = ABC_DoSomething(myHeuristic, x, y, z);

    /* place confusing argument on own commented line and all other parameters also an own line */
    int16_t result = ABC_DoSomething(
        scores[x] * y + bases[x],  /* Score heuristic. */
        x,
        y,
        z);

    /* Its a matrix, and therefore it makes sense to format the arguments as a matrix */
    /* clang-format off */
    ABC_TransformMyWidget(x1, x2, x3,
                          y1, y2, y3,
                          z1, z2, z3);
    /* clang-format on */


.. _rule_c_additional_function_rules:

Additional function rules (``C:014``)
-------------------------------------

Most of the following rules are checked by the clang-format configuration of
the project. If a rule is not checked automatically it is indicated.

.. admonition:: Additional Function rules

    - The open parenthesis **MUST** be on the same line as the function name.
    - There **MUST NOT** be a space between the function name and the open
      parenthesis.
    - There **MUST NOT** be a space between the parentheses and the parameters.
    - The open curly brace **MUST** be on the end of the last line of the
      function declaration, not the start of the next line.
    - The close curly brace **MUST** be either on the last line by itself or on
      the same line as the open curly brace.
    - There **MUST** be a space between the close parenthesis and the open
      curly brace.
    - Spaces after the open or before the close parenthesis **MUST NOT** be
      added.
    - All parameters **SHOULD** be aligned if possible.
    - If you cannot fit the return type and the function name on a single line,
      you **MUST** break between them.
    - If you break after the return type of a function declaration or
      definition, you **MUST** not indent.
    - You **SHOULD** use describing parameter names. (Not checked by
      clang-format)


.. _rule_c_function_parameter_checking:

Function parameter checking (``C:015``)
---------------------------------------

.. admonition:: Function parameter checking

    - Input values of function parameters **SHOULD** be checked at the
      beginning of a function if possible.
    - Pointers passed as parameters **MUST** be checked against NULL.

Parameter checking is shown in :numref:`function-input-check`.

.. code-block:: c
    :linenos:
    :caption: Input check of function parameters
    :name: function-input-check

    extern uint32_t ABC_ArrayAverage(uint8_t *pArray, uint8_t arrayLength) {
        uint32_t sum = 0u;

        FAS_ASSERT(pArray != NULL_PTR)
        FAS_ASSERT(arrayLength > 0u)

        for (uint16_t i = 0u; i < arrayLength; i++) {
            sum += pArray[i];
        }
        sum /= arrayLength;
        return sum;
    }


.. _rule_c_variable_names:

Variables names (``C:016``)
---------------------------

.. admonition:: Variable naming rules

    - global and static variables **MUST** be commented with a doxygen style
      comment.
    - Variable names (including function parameters) **MUST** start with a
      lowercase letter and are written in "camel Case".
    - If the scope of the variable is at least file wide (more than function
      scope) or if the variable is declared static it **MUST** start with
      module prefix in lowercase letters. A variable representing a physical
      unit is followed by a suffix with the SI-unit symbol ``_<unit>`` (e.g.
      ``_mA`` for milliampere or ``K`` for Kelvin). Exceptions are non-ASCII
      symbols as ``_perc`` for ``%``, ``_degC`` for ``°C`` and ``u`` for ``μ``.
    - A doxygen comment explaining what this variable is used for **MUST** be
      added to all static and global variables.

.. code-block:: c
    :linenos:
    :caption: Different examples for correctly named variables.

    /*================== Extern Constant and Variable Declarations ==============*/
    extern uint32_t abc_globalVariable;  /*!< Global variable -> prefix + doxygen comment */

    /*================== Static Constant and Variable Definitions ===============*/
    static uint32_t abc_staticVariable = 0u;  /*!< Static variable -> prefix + doxygen comment */
    static int32_t abc_packSoc_perc = 0;  /*!< Static variable representing physical unit -> prefix + suffix + doxygen */

    /*================== Static Function Implementations ========================*/
    static void ABC_StaticImportantFunction(void) {
        uint8_t maxValue = 32u;  /* Temporary variable to do calculations in this function */
        uint32_t maximumTemporaryVoltage_mV = 0u;  /* Variable representing physical unit -> suffix */
        maxValue--;

        /* Local loop counter */
        for (uint8_t counter = 0u; counter < maxValue; counter++) {
            /* Do something.... */
        }
    }

.. _rule_c_constant_names:

Constant names (``C:017``)
--------------------------

.. admonition:: Constant rules

    - Constant variables **MUST** be commented with a doxygen style comment.
    - Constant variables **MUST** be named with a module prefix and a leading
      ``k`` followed by camel case.
    - Underscores **MAY** be used as separators in rare cases where
      capitalization cannot be used for separation.

.. code-block:: c
    :linenos:
    :caption: Correct examples for naming constant variables.

    const static uint16_t abc_kDaysInAWeek = 7;     /*!< Number of days in a week */

.. _rule_c_pointer_rules:

Pointer rules (``C:018``)
-------------------------

.. admonition:: Pointer rules

    - The general variable name rules apply (see :ref:`rule_c_variable_names`).
    - Variables used for pointers **MUST** be prefixed with a leading ``p`` in
      the case of a pointer to a variable and ``fp`` in the case of a function
      pointer, followed by camel Case. When declaring a pointer variable or
      argument, the asterisk **MUST** be placed adjacent to the variable name.
    - As function-pointer syntax can get complicated and lead to error, a
      function pointer **MUST** use a typedef. The typedef of a function has
      to use the suffix ``_f``.
    - Spaces around ``.`` or ``->`` **MUST NOT** be used when accessing
      pointers. The following are examples of correctly-formatted pointer and
      reference expressions:

.. code-block:: c
    :linenos:
    :caption: Correct usage of pointers.

    typedef uint8_t SOME_FUNCTION_TYPE_f (void);  /* typedef of a function type */
    uint32_t myVariable = 0;                      /* local uint32_t variable */
    static uint32_t *abc_pMyPointer = NULL_PTR;   /* static uint32_t pointer */
    static INIT_STRUCT_s abc_initStruct = {0};    /* static init struct */
    INIT_STRUCT_s *pInitStruct = NULL_PTR;        /* local pointer to some init struct */
    SOME_FUNCTION_TYPE_f * fpMyFunction;          /* local function pointer using a typedef */


    abc_pMyPointer = &myVariable;
    myVariable = *abc_pMyPointer;
    myVariable = abc_initStruct.value;
    myVariable = pInitStruct->value;

.. _rule_c_variable_initialization:

Variable initialization (``C:019``)
-----------------------------------

.. admonition:: Variable initialization rules

    - All variables **SHOULD** be initialized at the point of their definition.
      If this is not done it **MUST** be commented why it is not done.
    - All variables **MUST** be initialized with the correct type.
    - Only one variable **MUST** be initialized or declared per line.
    - No multi-definitions **MUST** be used.
    - For the initialization, the correct suffixes for unsigned, signed and
      floating-point types **MUST** be used. See
      :numref:`table-variable-initialization` for details.
    - Pointers **MUST** be initialized with ``NULL`` if no other valid
      initialization is possible.


.. _table-init-rules:

.. table:: Variable initialization suffixes
    :name: table-variable-initialization

    +----------+----------+
    | Type     | Suffix   |
    +==========+==========+
    | uint8_t  | ``u``    |
    +----------+----------+
    | uint16_t | ``u``    |
    +----------+----------+
    | uint32_t | ``u``    |
    +----------+----------+
    | uint64_t | ``uLL``  |
    +----------+----------+
    | int8_t   | ``none`` |
    +----------+----------+
    | int16_t  | ``none`` |
    +----------+----------+
    | int32_t  | ``none`` |
    +----------+----------+
    | int64_t  | ``LL``   |
    +----------+----------+

.. code-block:: c
    :linenos:
    :caption: Initialization examples for variables and complex types

    /* Initialization for variables */
    uint8_t myVar = 10u;
    uint16_t myVar = 10u;
    uint32_t myVar = 10u;
    uint64_t myVar = 10uLL;
    int8_t myVar = -10;
    int16_t myVar = -10;
    int32_t myVar = -10;
    int32_t myVar = -10LL;

    /* Initialization example for arrays */
    uint8_t myArrayA[] = {1,2,3};   /* Array has type uint8_t[3] and holds 1,2,3 */
    uint8_t myArrayC[5] = {0};      /* Array has type uint8_t[5] and holds 0,0,0,0,0 */

    uint8_t myArrayD[4][3] = {  /* array of 4 arrays of 3 uint8_t's each (4x3 matrix) */
        { 1 },                  /* row 0 initialized to {1, 0, 0} */
        { 0, 1 },               /* row 1 initialized to {0, 1, 0} */
        { [2]=1 },              /* row 2 initialized to {0, 0, 1} */
    };                          /* row 3 initialized to {0, 0, 0} */

    /* Initialization example for structs */
    typedef struct POINT {
        float x;
        float y;
        float z;
    } POINT_s;

    POINT_s myPoint = {1.2, 1.3}  /* p.x=1.2, p.y=1.3, p.z=0.0 */

    typedef struct PATH {
        POINT_S point;
        uint32_t timestamp;
        uint32_t prevTimestamp;
        uint8_t[16] name;
    } PATH_s;

    PATH_s myPath = {80.0f, 127.0f, -37.0f, 0, 1, "test"};  /* 80.0f initializes myPath.point.x  */
                                                            /* 127.0f initializes myPath.point.y */
                                                            /* -37.0f initializes myPath.point.z */
                                                            /* 0 initializes myPath.timestamp    */
                                                            /* 1 initializes ex.in_u.a8[3]       */
                                                            /* "test" initializes name[0-3]      */
                                                            /* name[4-15] are set to 0           */

.. _rule_c_hexadecimal:

Hexadecimal values (``C:020``)
------------------------------

.. admonition:: Hexadecimal values rules

    Hexadecimal digits **MUST** be written in uppercase letters.

.. code-block:: c
    :linenos:
    :caption: Correct usage of hexadecimal digits.

    uint8_t myVar = 0xFFu;
    int8_t myOtherVar = 0xAA;

.. _rule_c_floating_point:

Floating-point values (``C:021``)
---------------------------------

.. admonition:: Floating-point values rules

    - Floating-point literals **MUST** always have a radix point, with digits
      on BOTH sides, even if they use exponential notation. Readability is
      improved if all floating-point literals take this familiar form, as this
      helps ensure that they are not mistaken for integer literals, and that
      the E/e of the exponential notation is not mistaken for a hexadecimal
      digit.
    - ``float`` types **SHOULD** be used wherever possible as the float
      operations are performed in hardware while double operations are not.


.. _table-floating-point-init-rules:

.. table:: Floating-point literal initialization suffixes
    :name: table-floating-point-initialization

    +----------+----------+
    | Type     | Suffix   |
    +==========+==========+
    | float    | ``f``    |
    +----------+----------+
    | double   | ``none`` |
    +----------+----------+

.. code-block:: c
    :linenos:
    :caption: Usage of floating-point literals.

    const float kEuler = 2.71828f;
    const double kPi = 3.141592
    double number = 1248.0e6;
    long double largeNumber = -0.5L;

.. _rule_c_structs:

Structs (``C:022``)
-------------------

.. admonition:: Struct rules

    - Structs **MUST** be commented with a doxygen style comment.
    - Struct members **MUST** be commented with a doxygen style comment.
    - Structs **MUST** be declared as typedefs.
    - Struct names **MUST** be all uppercase with underscores ``(_)`` between
      each word starting with the module prefix and ending with suffix ``_s``.
    - Struct members **MUST** be named as ordinary variables.
    - Anonymous structs **MUST NOT** be used, instead the struct type without
      the suffix ``_s`` **MUST** be defined.
    - A trailing comma **MUST** be used after the last member.
    - A doxygen comment describing each struct **MUST** be added above the
      definition.
    - A doxygen comment describing each struct member **MUST** be added after
      the member.

Example:

.. code-block:: c
    :linenos:
    :caption: Correct struct implementation.

    /** Symbolic names for battery system state */
    typedef struct ABC_DATE {
        uint16_t year,  /*!< year */
        uint8_t month,  /*!< month (1: january ... 12: december) */
        uint8_t day,    /*!< day of the month */
    } ABC_DATE_s;

.. _rule_c_enums:

Enums (``C:023``)
-----------------

.. admonition:: Enum rules

    - Enums **MUST** be commented with a doxygen style comment.
    - Enum members **MUST** be commented with a doxygen style comment.
    - Enums **MUST** be declared as typedefs.
    - Enums **MUST** be named all uppercase with underscores ``(_)`` between each word starting with the module prefix
      and ending with suffix ``_e``.
    - Anonymous enums **MUST NOT** be used, instead the enum type without the suffix ``_e`` **MUST** be defined.
    - Values **MUST NOT** be assigned to specific enum members.
    - Members **MUST** be named all in uppercase beginning with the module prefix.
    - No trailing comma **MUST** be used after the last entry.
    - The last member **MUST** be named after the typedef struct replacing the ``_e`` with ``_E`` and appending the
      suffix ``_MAX``.
    - A doxygen comment describing each enum **MUST** be added above the definition.
    - A doxygen comment describing each enum member **MUST** be added after the member.

.. code-block:: c
    :linenos:
    :caption: Correct enum implementation.

    /** Symbolic names for weekdays */
    typedef enum ABC_DAYS {
        ABC_MONDAY,     /**< First day of the week */
        ABC_TUESDAY,    /**< Second day of the week */
        ...
        ...  /* more members */
        ABC_SUNDAY,     /**< Last day of the week */
        ABC_DAYS_E_MAX  /**< Max marker */
    } ABC_DAYS_e;


.. _rule_c_typedefs:

Typedefs (``C:024``)
--------------------

.. admonition:: Typedef general rules

    - Typedef names **MUST** be all uppercase with underscores ``(_)`` between
      each word.
    - Other typedef names **MUST** end with the suffix ``_t``.

.. code-block:: c
    :linenos:
    :caption: Correct example for usage of typedefs.

    /* Remapping of types */
    typedef SPI_HandleType_s    SPI_HANDLE_TYPEDEF_t;
    typedef uint32_t            TICK_TYPE_t;

    /* Using typedefs for the creation of function pointers */
    /* Pointer to functions with uint32_t return datatype and two function parameters of type uint32_t */
    typedef uint32_t (*CALLBACK_FUNCTION_POINTER_t)(uint32_t, float);

    /* Example */
    static uint32_t ABC_DummyFunction(uint32_t varA, uint32_t varB);  /* Function prototype */
    /* local variable of type callback_funcPtr_t */
    CALLBACK_FUNCTION_POINTER_t callback = &ABC_BuildSum;

    /* call of function ABC_BuildSum()*/
    uint32_t temp = callback(37, 42);

.. _rule_c_macros:

Macros (``C:025``)
------------------

.. admonition:: Macro rules

    - Macro names **MUST** be capitalized with underscores.
    - Macros **MUST** start with the module prefix
    - If macros define a physical value the name **MUST** be suffixed with the
      SI-unit or a SI-derived unit, e.g., use ``F`` for a capacity instead of
      SI units  ``s4_A2__m2_kg_1``.
    - If macros are used to define a value, this value **MUST** be put in
      parentheses.
    - It is **NOT RECOMMENDED** to use function-like macros

.. code-block:: c
    :linenos:
    :caption: Correct naming examples of macros.

    #define ABC_GET_MINIMUM_FLOAT(x, y)       DEF_CalculateMinimumFloat(x, y)
    #define ABC_PI_ROUNDED                    (3.14f)
    #define ABC_NUMBER_OF_DAYS_IN_A_WEEK      (7u)
    #define ABC_MAXIMUM_PACK_VOLTAGE_mV       (320000u)

.. _rule_c_conditionals:

Conditionals (``C:026``)
------------------------

.. admonition:: Conditionals rules

    - No spaces **MUST** be used between the parentheses and the condition
      statement.
    - The ``if`` and ``else`` keywords **MUST** be placed in separate lines.
    - A space between the ``if`` keyword and the open parenthesis and between
      the close parenthesis and the curly bracket **MUST** be placed.
    - Multiple statements in one condition **MUST** be placed into separate
      parenthesis. If you have a boolean expression that is longer than the
      standard line length, the logical operator **MUST** be at the end of the
      lines.

.. code-block:: c
    :linenos:
    :caption: Correct implementation of if-else statements.

    if (condition) {  /* no spaces inside parentheses */
        /* code */  /* 4 space indent. */
    } else if (other_condition) {  /* The else goes on the same line as the closing brace. */
      /* code */
    } else {
      /* code */
    }
    if ((this_one_thing > this_other_thing) &&
        (a_third_thing == a_fourth_thing) &&
        (yet_another && last_one)) {
        /* code */
    }

.. _rule_c_switch:

switch Statements (``C:027``)
-----------------------------

.. admonition:: Switch statement rules

    - Switch statements **MUST** use parenthesis for blocks.
    - Fall-throughs between cases **SHOULD NOT** be used but instead all cases
      are terminated with one single break-statement at the end of the case.
      The only exceptions for this rules are empty fall-throughs that **MUST**
      be treated within the next case. These deliberate fall-throughs **MUST**
      be annotated.
    - Case blocks in switch statements **MUST NOT** use brackets.
    - Furthermore, switch statements **MUST** have a default case.
    - If the default case should never be executed, this **MUST** be treated as
      an error.
    - There **MUST** be an empty line between ``break`` and the next ``case``
      directive.

.. code-block:: c
    :linenos:
    :caption: Correct implementation of switch-case statement.

    switch (var) {
        case 0:    /* 4 space indent */
            /* code */    /* 4 space indent */
            break;

        case 1:
            /* code */
            break;

        case 2:
            /* code */
            break;

        case 3:  /* Empty fall-through allowed as case 3 and 4 require the same treatment */
        case 4:
            /* case 3 and case 4 trigger the same response */
            /* code */
            break;

        default:
            FAS_ASSERT(FAS_TRAP);
            break;

    }

.. _rule_c_loop:

loop Statements (``C:028``)
-----------------------------

.. admonition:: loop rules

    - Parenthesis **MUST** be used for all loops, at all times. This is valid
      for single-statement loops.
    - Empty loop bodies **MUST** use an empty pair of brackets and explain why
      they are empty.

.. code-block:: c
    :linenos:
    :caption: Correct usage of spaces and parenthesis in loop statements.

    for (uint8_t i = 0u; i < kSomeNumber; ++i) {
      /* code */
    }
    while (condition) {
      /* This should never happen, since ... */
    }

There are three defines that are typically looped over: the number of strings
(``BS_NR_OF_STRINGS``), the number of modules (``BS_NR_OF_MODULES``) and the
number of batteries per module (``BS_NR_OF_CELLS_PER_MODULE``).

These loops **MUST** follow the pattern as shown in
:numref:`special-counter-variables` and
:numref:`special-counter-variables-example`.

.. table:: Special counter variables in for loops
    :name: special-counter-variables
    :widths: grid

    +-------------------------------+-------------------+
    | Define                        | Counter variable  |
    +===============================+===================+
    | ``BS_NR_OF_STRINGS``          | ``s``             |
    +-------------------------------+-------------------+
    | ``BS_NR_OF_MODULES``          | ``m``             |
    +-------------------------------+-------------------+
    | ``BS_NR_OF_CELLS_PER_MODULE`` | ``c``             |
    +-------------------------------+-------------------+

.. code-block:: c
    :linenos:
    :caption: Looping over BS_NR_OF_STRINGS, BS_NR_OF_MODULES, BS_NR_OF_CELLS_PER_MODULE
    :name: special-counter-variables-example

    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        /* string related code */
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES; m++) {
            /* module related code */
            for (uint8_t c = 0u; c < BS_NR_OF_CELLS_PER_MODULE; c++) {
                /* cell related code */
            }
        }
    }

.. _rule_c_comment_style:

C Comment style (``C:029``)
---------------------------

Comment style rule

.. _rule_c_hints:

General note and common mistakes (``C:030``)
--------------------------------------------

For both, *camel Case* and *Pascal Case*, the general rules of the english
language apply. This means that the correct version for a variable about the
cell voltage is ``abc_cellVoltage`` (**not** ``abc_cellvoltage``).


.. _rule_c_formatting:

C Formatting (``C:031``)
------------------------

Formatting rule


.. _rule_c_state_machines:

State machines (``C:032``)
--------------------------

If a driver (or similar) requires to be implemented in a state machine there
are some hints found in :ref:`HOW_TO_WRITE_STATE_MACHINES`.

File Templates
--------------

These file templates below show how these rules are correctly applied. They
**SHOULD** be used as basis for new files.

- C header file :download:`c.h <../../../conf/tpl/c.h>`
- C source file :download:`c.c <../../../conf/tpl/c.c>`
- C test header file :download:`test_c.h <../../../conf/tpl/test_c.h>`
- C test source file :download:`test_c.c <../../../conf/tpl/test_c.c>`

C Header Files
^^^^^^^^^^^^^^

.. literalinclude:: ../../../conf/tpl/c.h
   :language: C
   :linenos:
   :caption: File template for C header files
   :name: c-h-template

C Source Files
^^^^^^^^^^^^^^

.. literalinclude:: ../../../conf/tpl/c.c
   :language: C
   :linenos:
   :caption: File template for C source files
   :name: c-c-template


C Test Header Files
^^^^^^^^^^^^^^^^^^^

.. literalinclude:: ../../../conf/tpl/test_c.h
   :language: C
   :linenos:
   :caption: File template for C test header files
   :name: test-c-h-template

C Test Source Files
^^^^^^^^^^^^^^^^^^^

.. literalinclude:: ../../../conf/tpl/test_c.c
   :language: C
   :linenos:
   :caption: File template for C test source files
   :name: test-c-c-template
