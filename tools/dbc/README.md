# CAN Message Definitions for foxBMS 2

## Adding new Messages and Signals

- Always adapt the sym-file first and then export the dbc-file.
- Generally, if the messages/signals are from a third party first import these
  verbatim:
  - The message name needs to the adopted and use one of these prefixes:
    - `AFE` for CAN-based BMS-Slaves
    - `AS` for aerosol sensors
    - `CS` for current sensors
    - `IMD` for insulation measurement devices.
  - Signal names **SHALL** never be changed in third-party messages.
  - Third-party DBC files **SHALL** be included verbatim in
    ``tools/dbc/third-party``.
    See [tools/dbc/third-party/README.md](./third-party/README.md) for
    more information.
- foxBMS messages **SHALL** use the lowercase prefix ``f_`` (in contrast to
  upper case prefixes of third party messages).
- foxBMS message and signal names **SHALL** use a PascalCase naming schema.
- Try to always assign either a type/enum or unit to the signals.
- In the comment attribute of a message (in the sym file), the file
  implementing the embedded message handling needs to be specified, following
  this pattern:
  `<comment-1> (in:<file-name>:<function-name>, fv:<direction>, type:<message-type>)<comment-2>`
  - `<comment-1`: an optional comment
  - `<file-name>`: base name of implementation file
  - `<function-name>`: name of function (in file `<file-name>`)
  - `<direction>` is either:
    - `tx` for transmit messages  or
    - `rx` for receive messages
  - `<message-type>`: type of message (can be chosen arbitrary)
  - `<comment-2`: an optional comment

See also
[src/app/driver/can/cbs/README.md](./../../src/app/driver/can/cbs/README.md).
