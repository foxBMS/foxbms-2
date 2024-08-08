# CAN Message Definitions for foxBMS 2

## Adding new Messages and Signals

- Always adapt the sym-file first and then export the dbc-file.
- Generally, if the messages/signals are from a third party import these
  verbatim.
  - In the rare cases, where the message names need to be change, the message
    names shall be suffixed (e.g., message ``Abc`` would then be
    ``Abc_String0``).
  - Signal names shall never be changed in third-party messages.
  - Third-party DBC files shall be included in ``tools/dbc/third-party``.
    See [tools/dbc/third-party/README.md](./third-party/README.md) for
    more information.
- foxBMS messages shall use the prefix ``f_``.
- foxBMS message and signal names shall use a PascalCase naming schema.
- Try to always assign either a type/enum or unit to the signals.
