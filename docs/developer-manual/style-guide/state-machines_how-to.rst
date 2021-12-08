.. include:: ../../macros.txt
.. include:: ../../units.txt

.. _HOW_TO_WRITE_STATE_MACHINES:

How to Write State Machines
===========================

This section demonstrates how state machines are implemented within the
|foxbms| project.
A simple, but fully functional, real-world implementation of this can be found
in the debug AFE driver (see :ref:`DEBUG_DEFAULT`).

The Example
-----------

This example implements a simple state machine with the following states:

- |state_uninitialized|
- |state_initialization|
- |state_running|
- |state_error|

An error in this example is an unrecoverable error. This gives the
state flow diagram in :numref:`state-diagram`.

.. graphviz:: state-machine-example/state-diagram-top-view.dot
    :caption: States and their transitions
    :name: state-diagram

The state |state_initialization| consists of three substates, which are
processed sequentially:

- |substate_initialization_0|: The first initialization substate
- |substate_initialization_1|: The second initialization substates
- |substate_initialization_exit|: The last initialization substate (e.g., for
  some cleanup)

The state |state_running| consists of three substate which are run in an
endless loop in the following order:

- |substate_running_0|: The first running substate
- |substate_running_1|: The second running substate
- |substate_running_2|: The third running substate

In any of the states  |state_initialization| and |state_running| and any of the
substates errors can occur. If this is the case, the state machine transitions
from the substate to the state |state_error|. The full state machine graph is
shown in :numref:`complete-state-diagram` and in a simplified way in
:numref:`complete-state-diagram-simplified`:

.. graphviz:: state-machine-example/state-diagram-complete.dot
    :caption: States and substates and their transitions
    :name: complete-state-diagram

.. graphviz:: state-machine-example/state-diagram-complete-simplified.dot
    :caption: States and substates and their transitions in a simplified graph
    :name: complete-state-diagram-simplified

Implementing the State Machine
------------------------------

The following describes the idea behind the state machine pattern and how it
is implemented for the described example.

This how to is written in a top-down approach, starting for an abstract state
machine interface to more detailed implementations of subfunctions. This makes
the global understanding simpler. But this also means, that functions are used
and only explained at a later point in the text.

.. note::

    In this example, the module prefix will be ``EG``. Sometimes in this how to
    it will a appropriate to use a variable reference for the module prefix.
    In that case ``{MODULE_PREFIX}`` is used.

.. note::

    In running text functions always use parentheses with no argument or three
    dots (``...``) to indicate that a function is referred to. Code examples of
    course always implement the full and correct function or function call.
    Below two simple examples are shown:

    - The function ``void noArguments()`` is referred to by ``noArguments()``.
    - The function ``uint8_t addTwoNumbers(uint8_t a, uint8_t b)`` is
      referred to by ``addTwoNumbers()``.

.. _implementation_basics:

Basics
^^^^^^

All states **MUST** be put into an enum describing the states. There are four
states in the example (|state_uninitialized|, |state_initialization|,
|state_running|, |state_error|) plus the boilerplate of the state machine (a
dummy state called ``Dummy`` and a state indicating that the state machine has
never run called ``Has_never_run``). The enum entries **MUST** use
``FSM_STATE`` as infix after the module prefix. Taking all these rules into
account, the enum for the states used in this example looks like this:


.. code-block:: c
   :linenos:
   :caption: Enumeration of the states
   :name: enum-for-states

    typedef enum EG_FSM_STATES {
        EG_FSM_STATE_DUMMY,          /*!< dummy state - always the first state */
        EG_FSM_STATE_HAS_NEVER_RUN,  /*!< never run state - always the second state */
        EG_FSM_STATE_UNINITIALIZED,  /*!< uninitialized state */
        EG_FSM_STATE_INITIALIZATION, /*!< initializing the state machine */
        EG_FSM_STATE_RUNNING,        /*!< operational mode of the state machine  */
        EG_FSM_STATE_ERROR,          /*!< state for error processing  */
    } EG_FSM_STATES_e;

A similar pattern has to be applied for the substates. For the boilerplate, a
dummy substate called ``Dummy`` (as in the state) and an additional substate
called ``Entry``  have to be defined. The enum entries **MUST** use
``FSM_SUBSTATE`` as infix after the module prefix. Taking all these rules into
account, the enum for the substates used in this example looks like this:

.. code-block:: c
   :linenos:
   :caption: Enumeration of the substates
   :name: substates

    typedef enum EG_FSM_SUBSTATES {
        EG_FSM_SUBSTATE_DUMMY,               /*!< dummy state - always the first substate */
        EG_FSM_SUBSTATE_ENTRY,               /*!< entry state - always the second substate */
        EG_FSM_SUBSTATE_INITIALIZATION_0,    /*!< fist initialization substate */
        EG_FSM_SUBSTATE_INITIALIZATION_1,    /*!< second initialization substate */
        EG_FSM_SUBSTATE_INITIALIZATION_EXIT, /*!< last initialization substate */
        EG_FSM_SUBSTATE_RUNNING_0,           /*!< fist running substate */
        EG_FSM_SUBSTATE_RUNNING_1,           /*!< second running substate */
        EG_FSM_SUBSTATE_RUNNING_2,           /*!< third running substate */
    } EG_FSM_SUBSTATES_e;

A struct named ``{MODULE_PREFIX}_STATE_s`` contains the general state of the
state machine, with variables like ``currentState`` and ``previousState``. In
this example this struct is named ``EG_STATE_s``.
This struct is typically extended by an additional struct that holds relevant
information or data (``EG_INFORMATION_s information``). In a real application
these are usually pointers to some database entries required (see
:ref:`DEBUG_DEFAULT`) or variables used within the module. In this example it
is just a struct holding three values.

.. code-block:: c
   :linenos:
   :caption: The state type
   :name: state-type

    typedef struct EG_STATE {
        uint16_t timer;                      /*!< timer of the state */
        uint8_t triggerEntry;                /*!< trigger entry of the state */
        EG_FSM_STATES_e nextState;           /*!< next state of the FSM */
        EG_FSM_STATES_e currentState;        /*!< current state of the FSM */
        EG_FSM_STATES_e previousState;       /*!< previous state of the FSM */
        EG_FSM_SUBSTATES_e nextSubstate;     /*!< next substate of the FSM */
        EG_FSM_SUBSTATES_e currentSubstate;  /*!< current substate of the FSM */
        EG_FSM_SUBSTATES_e previousSubstate; /*!< previous substate of the FSM */
        EG_INFORMATION_s information;        /*!< Some information to be stored */
    } EG_STATE_s;

With these lines of code, all types needed for the state machine are defined.
The next step is the implementation of the state machine.

The first thing to do is to declare a variable for the state machine state

.. code-block:: c
   :linenos:
   :caption: The state variable
   :name: introduction-of-state-variable

    extern EG_STATE_s eg_state;

and initialize it as shown in :numref:`initialization-of-state-variable`. The
members of the struct related to the state (``previousState``, ``currentState``
and ``nextState``) **MUST** be initialized with
``EG_FSM_STATE_HAS_NEVER_RUN`` to indicate that the state machine has not run
yet. The members of the struct related to the substate (``previousSubstate``,
``currentSubstate`` and ``nextSubstate``) **MUST** be initialized with the
dummy state ``EG_FSM_SUBSTATE_DUMMY``. The information struct can be anything
that is required by the application.

.. code-block:: c
   :linenos:
   :caption: The state variable
   :name: initialization-of-state-variable

    EG_STATE_s eg_state = {
        .timer             = 0,
        .triggerEntry      = 0,
        .nextState        = EG_FSM_STATE_HAS_NEVER_RUN,
        .currentState     = EG_FSM_STATE_HAS_NEVER_RUN,
        .previousState    = EG_FSM_STATE_HAS_NEVER_RUN,
        .nextSubstate     = EG_FSM_SUBSTATE_DUMMY,
        .currentSubstate  = EG_FSM_SUBSTATE_DUMMY,
        .previousSubstate = EG_FSM_SUBSTATE_DUMMY,
        .information.r0   = 0,
        .information.r1   = 0,
        .information.r2   = 0,
    };

A state machine always consists of a periodic *trigger* function. The trigger
function gets the state variable introduced above (``eg_state`` in this
example) as parameter. The trigger function **MUST** use ``Trigger`` as
function name infix. This example uses ``EG_Trigger()``. If needed, the name
can be extended (e.g., ``EG_TriggerAfe()``).

.. code-block:: c
   :linenos:
   :caption: The trigger function
   :name: introduction-of-trigger-function

    extern EG_Trigger(EG_STATE_s *pEgState)

The trigger function is then called somewhere in the application with
``EG_Trigger(&eg_state);``

The trigger function is always implemented as shown in
:numref:`implementation-of-trigger-function` where ``EG_RunStateMachine()``
is the actual state machine implementation. The base name of the function
**MUST** be ``{MODULE_PREFIX}_RunStateMachine``. The implementation of
``EG_CheckMultipleCalls()`` can be taken directly from the
example code. The detailed explanation of this function is found later in the
text in :numref:`function_check_reentrance`.

It is often necessary to wait a definite amount of time. This can be the case
for example when the state machine waits for a measurement to be finished
before continuing. Waiting is implemented via the variable ``timer`` which
is a member of the state variable. It must be decremented one time every time
the trigger function is called. Two cases can happen:

 - If it has the value zero, it stays at zero and the content of the state
   machine is processed further.
 - If is has a non-zero value, it is decremented and the trigger function
   exits without processing the state machine.

To wait a definite amount of time, the ``time`` variable must only be assigned
a non-zero value. The time to wait will depend on the periodicity with which
the state machine is processed via the trigger function. If ``timer`` is set
to ``N`` and the trigger function is called with a period ``T``, the wait time
before the state machine is processed further will be ``N*T``.

.. code-block:: c
   :linenos:
   :caption: Implementation of the trigger function
   :name: implementation-of-trigger-function

    extern STD_RETURN_TYPE_e EG_Trigger(EG_STATE_s *pEgState) {
        FAS_ASSERT(pEgState != NULL_PTR);
        bool earlyExit                = false;
        STD_RETURN_TYPE_e returnValue = STD_OK;

        /* Check re-entrance of function */
        if (EG_MULTIPLE_CALLS_YES == EG_CheckMultipleCalls(pEgState)) {
            returnValue = STD_NOT_OK;
            earlyExit   = true;
        }

        if (earlyExit == false) {
            if (pEgState->timer > 0u) {
                if ((--pEgState->timer) > 0u) {
                    pEgState->triggerEntry--;
                    returnValue = STD_OK;
                    earlyExit   = true;
                }
            }
        }

        if (earlyExit == false) {
            EG_RunStateMachine(pEgState);
            pEgState->triggerEntry--;
        }
        return returnValue;
    }

As stated above the actual state machine is processed by
``EG_RunStateMachine()``.

``EG_RunStateMachine()`` must process all states,
**except for the dummy state** (``EG_FSM_STATE_DUMMY``). A condensed version of
the state machine runner function looks like this:

.. code-block:: c
   :linenos:
   :caption: Condensed state machine runner function
   :name: condensed-state-machine-runner-function

    static STD_RETURN_TYPE_e EG_RunStateMachine(EG_STATE_s *pEgState) {
        STD_RETURN_TYPE_e ranStateMachine = STD_OK;
        EG_FSM_STATES_e nextState         = EG_FSM_STATE_DUMMY;
        switch (pEgState->currentState) {
            /********************************************** STATE: HAS NEVER RUN */
            case EG_FSM_STATE_HAS_NEVER_RUN:
                /* code goes here */
                break;

            /********************************************** STATE: UNINITIALIZED */
            case EG_FSM_STATE_UNINITIALIZED:
                /* code goes here */
                break;

            /********************************************* STATE: INITIALIZATION */
            case EG_FSM_STATE_INITIALIZATION:
                /* code goes here */
                break;

            /**************************************************** STATE: RUNNING */
            case EG_FSM_STATE_RUNNING:
                /* code goes here */
                break;
            /****************************************************** STATE: ERROR */
            case EG_FSM_STATE_ERROR:
                /* code goes here */
                break;

            /**************************************************** STATE: DEFAULT */
            default:
                /* all cases must be processed, trap if unknown state arrives */
                FAS_ASSERT(FAS_TRAP);
                break;
        }

        return ranStateMachine;
    }

It can now be seen why the ``EG_FSM_STATE_DUMMY`` state must never be processed
by the state machine: If a function irregularly sets the state to
``EG_FSM_STATE_DUMMY``, the state machine will switch to the default case and
the ``FAS_ASSERT()`` function will stop this undefined behavior.

Description of the Implementation of All Cases
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

At next the implementations of all cases are explained in detail.

``EG_FSM_STATE_HAS_NEVER_RUN``
""""""""""""""""""""""""""""""

If the state machine has never run, it needs to be transferred to known state,
the uninitialized state (``EG_FSM_STATE_UNINITIALIZED``).

.. note::

    This section uses the function ``EG_SetState()``. The detailed
    explanation of ``EG_SetState()`` is found later in the text in
    :numref:`function_set_state`.

.. code-block:: c
   :linenos:
   :caption: Transition from the first startup of the state machine
   :name: never-to-uninitialized

    switch (pEgState->currentState) {
        /********************************************** STATE: HAS NEVER RUN */
        case EG_FSM_STATE_HAS_NEVER_RUN:
            /* Nothing to do, just transfer */
            EG_SetState(pEgState, EG_FSM_STATE_UNINITIALIZED, EG_FSM_SUBSTATE_ENTRY, EG_FSM_SHORT_TIME);
            break;
        /* ... */
    }

``EG_FSM_STATE_UNINITIALIZED``
""""""""""""""""""""""""""""""

This is the first state that is present in the state machine example. In
the example there is nothing to do in the state |state_uninitialized|. For most
applications this will also be the case. However, if needed an application can
implement some behavior in this state before transferring to the state
|state_initialization| (``EG_FSM_STATE_INITIALIZATION``):

.. code-block:: c
   :linenos:
   :caption: Transition from the first startup of the state machine
   :name: uninitialized-to-initialization

    switch (pEgState->currentState) {
        /* ... */
        /********************************************** STATE: UNINITIALIZED */
        case EG_FSM_STATE_UNINITIALIZED:
            /* Nothing to do, just transfer */
            EG_SetState(pEgState, EG_FSM_STATE_INITIALIZATION, EG_FSM_SUBSTATE_ENTRY, EG_FSM_SHORT_TIME);
            break;
        /* ... */
    }

``EG_FSM_STATE_INITIALIZATION``
"""""""""""""""""""""""""""""""

The example  showed, that the state |state_initialization| consists of three
substates. Putting all code for all substates directly into the state
|state_initialization| would cause bad readability and bad maintainability.
Therefore all details of what happens in the state are implemented
in state processing functions. :ref:`state_processing_functions` explains what
state processing functions are and how they work. For now it is sufficient to
know that state processing functions need to exist.


If an error occurs in any of the substates of the state |state_initialization|
the state machine needs to transfer to the state |state_error|. The transitions
based on the states and substates would not be clearly visible in such a
implementation.
Therefore this logic is transferred into a state processing function
``EG_ProcessInitializationState()``. State processing functions **MUST** use
the naming pattern ``{MODULE_PREFIX}_Process{StateName}State`` where
``{StateName}`` is the state to be processed, e.g., for the
state |state_initialization| ``{StateName}`` needs to be replaced by
``Initialization``.

The state processing function (in this example
``EG_ProcessInitializationState()``) returns the state the state machine has
to transition to.

Generally three cases can happen:

- the state machine stays in the current state,
- the state machine transitions to another state or
- something went wrong and the state machine must process the error.

To reflect this, an ``if-else`` structure is used. The first ``if`` always
processes the current case, i.e. staying in the current state. The final
``else`` always processes the case if something unforeseen went wrong and
performs an assertion. Between the ``if`` and ``else`` all ``else if``
implement the state transitions to other states. For this example this
translates into the following code:

.. code-block:: c
   :linenos:
   :caption: The initialization state is processed by an own function
   :name: the-initialization-state

    switch (pEgState->currentState) {
        /* ... */
        /********************************************* STATE: INITIALIZATION */
        case EG_FSM_STATE_INITIALIZATION:
            nextState = EG_ProcessInitializationState(pEgState);
            if (nextState == EG_FSM_STATE_INITIALIZATION) {
                /* staying in state, processed by substate function */
            } else if (nextState == EG_FSM_STATE_ERROR) {
                EG_SetState(pEgState, EG_FSM_STATE_ERROR, EG_FSM_SUBSTATE_ENTRY, EG_FSM_SHORT_TIME);
            } else if (nextState == EG_FSM_STATE_RUNNING) {
                EG_SetState(pEgState, EG_FSM_STATE_RUNNING, EG_FSM_SUBSTATE_ENTRY, EG_FSM_SHORT_TIME);
            } else {
                FAS_ASSERT(FAS_TRAP); /* Something went wrong */
            }
            break;
        /* ... */
    }

``EG_FSM_STATE_RUNNING``
""""""""""""""""""""""""

After a successful initialization the state machine transfers into the
operational mode. As described above, the state machine stays in that state
until an error occurs. This state is also processed by the state function
``EG_ProcessRunningState()`` as it has more than one option to transfer to
(either staying in the state or going to an error state).

.. code-block:: c
   :linenos:
   :caption: The running state is also processed by a own function
   :name: the-running-state

    switch (pEgState->currentState) {
        /* ... */
        /**************************************************** STATE: RUNNING */
        case EG_FSM_STATE_RUNNING:
            nextState = EG_ProcessRunningState(pEgState);
            if (nextState == EG_FSM_STATE_RUNNING) {
                /* staying in state, processed by state function */
            } else if (nextState == EG_FSM_STATE_ERROR) {
                EG_SetState(pEgState, EG_FSM_STATE_ERROR, EG_FSM_SUBSTATE_ENTRY, EG_FSM_SHORT_TIME);
            } else {
                FAS_ASSERT(FAS_TRAP); /* Something went wrong */
            }
            break;
        /* ... */
    }

``EG_FSM_STATE_ERROR``
""""""""""""""""""""""

This state processes the error case. Errors can be recoverable, but in this
example, for the sake of simplicity, they are not.

.. code-block:: c
   :linenos:
   :caption: The error state
   :name: the-error-state

    switch (pEgState->currentState) {
        /* ... */
        /****************************************************** STATE: ERROR */
        case EG_FSM_STATE_ERROR:
            /* implement error processing here or trap */
            break;
        /* ... */
    }

In many cases an error is recoverable. Such a situation is described in
:ref:`recoverable-error`

.. _default_case:

``default``
"""""""""""

This case makes sure that all states are correctly processed and the dummy
state (``EG_FSM_STATE_DUMMY``) is not used. If this is not the case then
this function traps.

.. code-block:: c
   :linenos:
   :caption: The default state
   :name: the-default-state

    switch (pEgState->currentState) {
        /* ... */
        /**************************************************** STATE: DEFAULT */
        default:
            /* all cases must be processed, trap if unknown state arrives */
            FAS_ASSERT(FAS_TRAP);
            break;
    }


``EG_FSM_STATE_DUMMY``
""""""""""""""""""""""

As already stated in :ref:`default_case` processing the ``EG_FSM_STATE_DUMMY``
state is not required. The following describes the purpose of this *pseudo*
state. There are two reasons one additional state is needed.

The first reason is that ``EG_SetState()`` and ``EG_SetSubstate()``
needed some state to set the ``nextState`` and ``nextSubstate`` members
of the struct to some valid value after the ``nextState`` is transferred to
``currentState`` and ``currentSubstate`` member. This must be some value that
is not a real state the state machine could transfer to, but something to
indicate that ``nextState`` and ``nextSubstate`` were cleared.
``EG_FSM_STATE_DUMMY`` is used for that purpose.

The second reason comes from the initialization of variables in C.
All uninitialized struct variables are initialized with zero, therefore for
this example  also ``eg_state``, which is the state variable of this state
machine.
This is guaranteed by the C99 standard. For details see ISO C99 Standard
6.7.8.21 (Language/Declarations/Initialization/21).

State variables store all states. These states are defined by an enum. This was
described in :ref:`implementation_basics`. The first entry in an unnumbered
enum has the value zero. Not fully explicitly initializing the state variable
would implicitly initialize it with zero.

.. code-block:: c
   :linenos:
   :caption: No initialization of the state variable ``eg_state``
   :name: no-initialization-of-state

    EG_STATE_s eg_state;
    /* equals to: EG_STATE_s eg_state = {0}; */

In order to prevent **not** thinking about the initialization of the state
members, the first state is the second enum entry (in this example
``EG_FSM_STATE_HAS_NEVER_RUN``).
This equals integer value `1`, not `0`. This forces the developer to think
about initialization and think how the state variable (here ``eg_state``) needs
to be initialized. In combination with the implementation pattern of the
``EG_RunStateMachine()`` the state machine only starts if the initialization
is correctly done.

.. _state_processing_functions:

Description of the Implementation of State Processing Functions
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Functions that *process* a specific state are referred to as
*state processing functions*.

State processing functions **MUST** use
the naming pattern ``{MODULE_PREFIX}_Process{StateName}State`` where
``StateName`` is the state to be processed, e.g., for the state
|state_initialization| ``StateName`` needs to be replaced by
``Initialization``.

State processing functions always return the next state to transition to. A
variable called ``nextState`` **MUST** be defined locally in such functions.
This variable **MUST** always be initialized with the state this state
processing function implements.

Generally the ``nextState`` variables definition follows the following pattern
``EG_FSM_STATES_e nextState = EG_FSM_STATE_{SOME_STATE}`` where ``{SOME_STATE}``
needs to be replaced with the state this function is processing. For example,
as the function ``EG_ProcessInitializationState()`` process the state
|state_initialization| the correct state to initialize ``nextState`` with is
``EG_FSM_STATE_INITIALIZATION``. The example in
:numref:`example-of-next-state-initialization` shows this more detailed for
``EG_ProcessInitializationState()``:

.. code-block:: c
   :linenos:
   :caption: Example of ``nextState`` initialization
   :name: example-of-next-state-initialization

    static EG_FSM_STATES_e EG_ProcessInitializationState(EG_STATE_s *pEgState) {
        EG_FSM_STATES_e nextState = EG_FSM_STATE_INITIALIZATION; /* default behavior: stay in state */
        /* code */
        return nextState;
        }

At next the state processing functions ``EG_ProcessInitializationState()``
and ``EG_ProcessRunningState()`` are explained.


``EG_ProcessInitializationState()``
"""""""""""""""""""""""""""""""""""

.. note::

    This section uses the function ``EG_SetSubstate()``. The detailed
    explanation of ``EG_SetSubstate()`` is found later in the text in
    :numref:`function_set_substate`.

The initialization state has three substates (|substate_initialization_0|,
|substate_initialization_1|, |substate_initialization_exit|) that are run
sequentially.
The Entry substate (from the enums boilerplate) just transfers the state
machine in the first initialization substate |substate_initialization_0|.
There is no error handling required and code reads as simple as follows:

.. code-block:: c
   :linenos:
   :caption: Transferring from the Entry state in first initialization substate
             |substate_initialization_0|
   :name: initialization-transferring-from-entry-to-initialization_0
   :emphasize-lines: 6

    static EG_FSM_STATES_e EG_ProcessInitializationState(EG_STATE_s *pEgState) {
        EG_FSM_STATES_e nextState = EG_FSM_STATE_INITIALIZATION; /* default behavior: stay in state */
        switch (pEgState->currentSubstate) {
            case EG_FSM_SUBSTATE_ENTRY:
                /* Nothing to do, just transfer to next substate */
                EG_SetSubstate(pEgState, EG_FSM_SUBSTATE_INITIALIZATION_0, EG_FSM_SHORT_TIME);
                break;
            /* ... */
        }
    }

In the first substate |substate_initialization_0| some work needs to be done
(hypothetically for this example). This work is implemented in a function
``EG_SomeInitializationFunction0()`` that returns either ``true`` (if
successful) or ``false`` (if unsuccessful). If it was unsuccessful, the
substate |substate_initialization_0| failed and the state machine needs to
transfer into the state |state_error|. If this substate was successful the
|state_initialization| state should precede with the second substate
|substate_initialization_1|. The code below shows the implementation.

.. code-block:: c
   :linenos:
   :caption: Transferring from the first initialization substate
             |substate_initialization_0| to second initialization substate
             |substate_initialization_1|
   :name: initialization-transferring-from-initialization_0-to-initialization_1
   :emphasize-lines: 6,7,10

    static EG_FSM_STATES_e EG_ProcessInitializationState(EG_STATE_s *pEgState) {
        EG_FSM_STATES_e nextState = EG_FSM_STATE_INITIALIZATION; /* default behavior: stay in state */
        switch (pEgState->currentSubstate) {
            /* ... */
            case EG_FSM_SUBSTATE_INITIALIZATION_0:
                if (true == EG_SomeInitializationFunction0()) {
                    EG_SetSubstate(pEgState, EG_FSM_SUBSTATE_INITIALIZATION_1, EG_FSM_SHORT_TIME);
                } else {
                    /* Something might go wrong, so transition to error state */
                    nextState = EG_FSM_STATE_ERROR;
                }
                break;
            /* ... */
        }
    }

Transferring from initialization substate |substate_initialization_1| to
initialization substate |substate_initialization_exit| works similar, therefore
this implementation is left out. At next the transition from the initialization
substate |substate_initialization_exit| into the next state, the first running
substate |substate_running_0|, is shown. The function
``EG_SomeInitializationFunctionExit()`` behaves the same way
``EG_SomeInitializationFunction0()`` above does. This leads to the following
implementation:

.. code-block:: c
   :linenos:
   :caption: Transferring from the last initialization substate
             |substate_initialization_exit| to first running substate
             |substate_running_0|
   :name: initialization-transferring-from-initialization_exit-to-running_0
   :emphasize-lines: 6,8,11

    static EG_FSM_STATES_e EG_ProcessInitializationState(EG_STATE_s *pEgState) {
        EG_FSM_STATES_e nextState = EG_FSM_STATE_INITIALIZATION; /* default behavior: stay in state */
        switch (pEgState->currentSubstate) {
            /* ... */
            case EG_FSM_SUBSTATE_INITIALIZATION_EXIT:
                if (true == EG_SomeInitializationFunctionExit()) {
                    /* Initialization was successful, so transition to running state */
                    nextState = EG_FSM_STATE_RUNNING;
                } else {
                    /* Something might go wrong, so transition to error state */
                    nextState = EG_FSM_STATE_ERROR;
                }
                break;
            /* ... */

        }
    }


The ``default`` case is implemented to assert on illegal substates:

.. code-block:: c
   :linenos:
   :caption: Assertion on illegal substate
   :name: initialization-assert-on-illegal-substate
   :emphasize-lines: 6

    static EG_FSM_STATES_e EG_ProcessInitializationState(EG_STATE_s *pEgState) {
        EG_FSM_STATES_e nextState = EG_FSM_STATE_INITIALIZATION; /* default behavior: stay in state */
        switch (pEgState->currentSubstate) {
            /* ... */
        default:
            FAS_ASSERT(FAS_TRAP);
            break;
        }
    }

``EG_ProcessRunningState()``
""""""""""""""""""""""""""""

The state |state_running| consists of three substates that are looped in order
(
|substate_running_0|,
|substate_running_1|,
|substate_running_2|,
|substate_running_0|,
|substate_running_1|,
|substate_running_2|,
|substate_running_0|,
...) as long as no error occurs. If an error occurs in any |state_running|
state's substates the next state is the state |state_error|.

In all of the |state_running| state's substates some work needs to be done
(again, hypothetically for this example). This work is implemented in the
functions
``EG_SomeRunningFunction0()`` for substate |substate_running_0|,
``EG_SomeRunningFunction1()`` for substate |substate_running_1| and
``EG_SomeRunningFunction2()`` for substate |substate_running_2|
that return either ``true`` (if successful) or ``false`` (if unsuccessful).
If it was unsuccessful, the respective next state is the state |state_error|.
If it was successful, the respective next substate will be run. The
implementation is shown below:

.. code-block:: c
   :linenos:
   :caption: The |state_running| and its substates implemented in its state
             processing function
   :name: running-state-processing-function

    static EG_FSM_STATES_e EG_ProcessRunningState(EG_STATE_s *pEgState) {
        EG_FSM_STATES_e nextState = EG_FSM_STATE_RUNNING; /* default behavior: stay in state */
        switch (pEgState->currentSubstate) {
            case EG_FSM_SUBSTATE_ENTRY:
                /* Nothing to do, just transfer to next substate */
                EG_SetSubstate(pEgState, EG_FSM_SUBSTATE_RUNNING_0, EG_FSM_SHORT_TIME);
                break;

            case EG_FSM_SUBSTATE_RUNNING_0:
                if (true == EG_SomeRunningFunction0()) {
                    EG_SetSubstate(pEgState, EG_FSM_SUBSTATE_RUNNING_1, EG_FSM_SHORT_TIME);
                } else {
                    /* Something might go wrong, so transition to error state */
                    nextState = EG_FSM_STATE_ERROR;
                }
                break;

            case EG_FSM_SUBSTATE_RUNNING_1:
                if (true == EG_SomeRunningFunction1()) {
                    EG_SetSubstate(pEgState, EG_FSM_SUBSTATE_RUNNING_2, EG_FSM_SHORT_TIME);
                } else {
                    /* Something might go wrong, so transition to error state */
                    nextState = EG_FSM_STATE_ERROR;
                }
                break;

            case EG_FSM_SUBSTATE_RUNNING_2:
                if (true == EG_SomeRunningFunction2()) {
                    EG_SetSubstate(pEgState, EG_FSM_SUBSTATE_RUNNING_0, EG_FSM_SHORT_TIME);
                } else {
                    /* Something might go wrong, so transition to error state */
                    nextState = EG_FSM_STATE_ERROR;
                }
                break;

            default:
                FAS_ASSERT(FAS_TRAP);
                break;
        }
        return nextState;
    }

Generic Functions Used in the State Machine
-------------------------------------------

The following functions (``EG_CheckMultipleCalls``, ``EG_SetState``,
``EG_SetSubstate``) are needed for all state machines.

.. _function_check_reentrance:

``EG_CheckMultipleCalls()``
^^^^^^^^^^^^^^^^^^^^^^^^^^^

The state machine trigger function (here ``EG_Trigger``) **MUST** only be
called time or event triggered and **MUST NOT** be called multiple times (no
*reentrance*).

``EG_CheckMultipleCalls()`` checks based on ``triggerEntry`` if the function
is called only one time. The ``triggerEntry`` variable must be incremented once
in each call of this function. It must be decremented once in every call of the
trigger function, no matter what the trigger function does (this means even if
the timer has not elapsed).

.. _function_set_state:

``EG_SetState()``
^^^^^^^^^^^^^^^^^

This function sets the next state. The following steps are performed:

- setting the idle time of a state and
- setting the state and substate.

Function behavior:

If neither, the state or substate have changed, there is no action to be taken.
If the state has changed, the state **and** the substate need to change. The
state is set to the next state and the substate is set to the entry state
for substates (``EG_FSM_SUBSTATE_ENTRY``). After that the ``nextState`` and
``nextSubstate`` of state and substate can be cleared (set  to
``EG_FSM_STATE_DUMMY`` and ``EG_FSM_SUBSTATE_DUMMY`` respectively).
If the state has not changed, and only the substate has, the next substate
is set by ``EG_SetSubstate()``.

This implementation requires that every state has a defined entry for all
states and all states need to implement that entry. This also ensure no state
transitions from e.g.

- ``State A`` and ``third substate`` into
- ``State C`` and ``second substate``

are made, but a strict chain needs to be followed:

- ``State A`` and ``third substate`` into
- ``State C`` and ``first substate`` (``EG_FSM_SUBSTATE_ENTRY``) into
- ``State C`` and ``second substate``.

**What if there is no substate in a case?**: There might be states that do not
need substate, even this example has three states with no substates (
``EG_FSM_STATE_HAS_NEVER_RUN``, ``EG_FSM_STATE_UNINITIALIZED`` and
``EG_FSM_STATE_ERROR``). In this case just the transition(s) in the next
state(s) need to be implemented and no state processing function needs to be
implemented. Therefore setting the substate implicitly by using the
``EG_SetState`` is fine, as the substate is ignored in that case and it
is correctly set to entry (``EG_FSM_SUBSTATE_ENTRY``) for the next case, whether
this state implements substates or not.

.. _function_set_substate:

``EG_SetSubstate()``
^^^^^^^^^^^^^^^^^^^^

This function only sets the **sub**\ state.

When ``currentSubstate`` is set to the next substate, the ``nextSubstate`` can
be cleared. This is done by setting it to the dummy substate
(``EG_FSM_SUBSTATE_DUMMY``).

.. _recoverable-error:

Extended Example With Recoverable Error
---------------------------------------

There are cases where an error during the processing of the state machine can
occur and there are strategies to recover from them. The example from
:numref:`state-diagram` is extended as follows:

.. graphviz:: state-machine-example/state-diagram-recoverable-error.dot
    :caption: Example with recoverable error
    :name: state-diagram-recoverable

To implement this behavior, the error case needs to be changed to something
like shown in :numref:`the-recoverable-error-state`. There is a state function
``EG_ProcessErrorState()`` to process the error case and there might be an option
to re-initialize the state machine based on the type of error.

.. code-block:: c
   :linenos:
   :caption: The error state
   :name: the-recoverable-error-state

    switch (pEgState->currentState) {
        /* ... */
        /****************************************************** STATE: ERROR */
        case EG_FSM_STATE_ERROR:
            nextState = EG_ProcessErrorState(pEgState);
            if (nextState == EG_FSM_STATE_ERROR) {
                /* staying in error state, processed by state function */
            } else if (nextState == EG_FSM_STATE_UNINITIALIZED) {
                EG_SetState(pEgState, EG_FSM_STATE_UNINITIALIZED, EG_FSM_SUBSTATE_ENTRY, EG_FSM_SHORT_TIME);
            } else {
                FAS_ASSERT(FAS_TRAP); /* Something went wrong */
            }
            break;
        /* ... */
    }

Full Example Code
-----------------

The full implementation of this state machine is found in
:numref:`state-machine-h` and :numref:`state-machine-c`.

.. literalinclude:: ./state-machine-example/state-machine.h
    :language: C
    :linenos:
    :caption: The header of the state machine
    :name: state-machine-h

.. literalinclude:: ./state-machine-example/state-machine.c
    :language: C
    :linenos:
    :caption: The implementation of the state machine
    :name: state-machine-c

.. |state_uninitialized| replace:: **Uninitialized**
.. |state_initialization| replace:: **Initialization**
.. |state_running| replace:: **Running**
.. |state_error| replace:: **Error**
.. |substate_initialization_0| replace:: **I**\ :sub:`0`
.. |substate_initialization_1| replace:: **I**\ :sub:`1`
.. |substate_initialization_exit| replace:: **I**\ :sub:`exit`
.. |substate_running_0| replace:: **R**\ :sub:`0`
.. |substate_running_1| replace:: **R**\ :sub:`1`
.. |substate_running_2| replace:: **R**\ :sub:`2`
