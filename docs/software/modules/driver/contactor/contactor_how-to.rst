
.. _HOW_TO_USE_THE_CONTACTOR_MODULE:

How to use the Contactor module
===============================

How to change number of Contactors
----------------------------------

.. note::

    The minimal number of Contactors used is dependent on the number of Strings attached (+ and - for each)

The number of additional Contactors (precharge, etc.) can be changed by
editing ``BS_NR_OF_CONTACTORS_OUTSIDE_STRINGS`` in ``src/app/application/config/battery_system_cfg.h``

How to change the Contactor behaviour
-------------------------------------

Contactor behaviour is changed in file ``src/app/driver/config/contactor_cfg.c``
The variable ``cont_contactorStates`` holds a list of configurations. Each of the entries corresponds to one Contactor.

.. literalinclude:: ./contactor_how-to.h
   :language: C
   :linenos:
   :start-after: example-contactor-state-start
   :end-before: example-contactor-state-end
   :caption: The **structure** of configuration for one Contactor
   :name: structure-of-contactor-state

The options for each value along with a quick explanation are listed below.

+-----------------------+------------------------------+
|**currentSet/feedback**| **explanation**              |
+-----------------------+------------------------------+
| CONT_SWITCH_OFF       | Contactor is open            |
+-----------------------+------------------------------+
| CONT_SWITCH_ON        | Contactor is closed          |
+-----------------------+------------------------------+
| CONT_SWITCH_UNDEFINED | Contactor state is not known |
+-----------------------+------------------------------+


+-------------------------------+------------------------------------------+
|**feedbackPinType**            | **explanation**                          |
+-------------------------------+------------------------------------------+
| CONT_FEEDBACK_NORMALLY_OPEN   | feedback line is normally open           |
+-------------------------------+------------------------------------------+
| CONT_FEEDBACK_NORMALLY_CLOSED | feedback line is normally closed         |
+-------------------------------+------------------------------------------+
| CONT_FEEDBACK_THROUGH_CURRENT | Get feedback through the current flowing |
+-------------------------------+------------------------------------------+
| CONT_HAS_NO_FEEDBACK          | Feedback line is not used                |
+-------------------------------+------------------------------------------+


+----------------+
|**stringIndex** |
+----------------+
| BS_STRING0     |
+----------------+
| BS_STRING1     |
+----------------+
| BS_STRING2     |
+----------------+
| BS_STRING_MAX  |
+----------------+


+----------------+------------------------------------------------------------+
|**type**        | **explanation**                                            |
+----------------+------------------------------------------------------------+
| CONT_PLUS      | Contactor is placed in HV plus path                        |
+----------------+------------------------------------------------------------+
| CONT_MINUS     | Contactor is placed in HV minus path                       |
+----------------+------------------------------------------------------------+
| CONT_PRECHARGE | used as precharge Contactor, installed as a plus Contactor |
+----------------+------------------------------------------------------------+
| CONT_UNDEFINED | Undefined Contactor                                        |
+----------------+------------------------------------------------------------+


+----------------+
|**spsChannel**  |
+----------------+
| any number     |
+----------------+


+----------------------------+------------------------------------------------+
|**breakingDirection**       | **explanation**                                |
+----------------------------+------------------------------------------------+
| CONT_CHARGING_DIRECTION    | preferably open in charge current direction    |
+----------------------------+------------------------------------------------+
| CONT_DISCHARGING_DIRECTION | preferably open in discharge current direction |
+----------------------------+------------------------------------------------+
| CONT_BIDIRECTIONAL         | no preference                                  |
+----------------------------+------------------------------------------------+

Some Contactors are designed and optimized to open current that is flowing
in one direction to provide maximum current breaking capability. This
direction is then dependent on how the Contactors are installed withing the
battery system. If bidirectional Contactors are used, the main power
terminals can be connected in either direction and the current breaking
capability is the same for both directions.


Further Reading
---------------

Implementation details of the Contactor module are found in :ref:`CONTACTOR`.
