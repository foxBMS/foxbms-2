# CAN Callback Implementation Organization

- Messages that are meant to be received by the BMS are implemented in `rx`.
- Messages that are meant to be transmitted by the BMS **asynchronously** are
  implemented in `tc-async`.
- Messages that are meant to be transmitted by the BMS **cyclicly** are
  implemented in `tc-cyclic`.

All implementation files **SHALL** follow this naming schema:
  `can_cbs_<direction>_<message-type>_<manufacturer>-<model>-<message-name>.c`

- `<direction>` is either:
  - `tx` for transmit messages  or
  - `rx` for receive messages
- `<message-type>` is either:
  - `afe` for CAN-based BMS-Slaves
  - `as` for aerosol sensors
  - `cs` for current sensors
  - `f` for foxBMS messages
  - `imd` for insulation measurement devices.

  These infixes are the (lowercased) prefixes of the CAN messages defined in
  `tools/dbc/foxbms.dbc`.
- `<manufacturer>-<model>` is self explanatory
  however; for foxBMS messages it is left empty, i.e.,
  `_<manufacturer>-<model>.c` becomes `.c`.
- `<message-name>` is optional.
  If there is only one implementation file, that implements the entire message
  handling of the device, then it can omitted.
  If there is more than one implementation file, then each file **SHALL**
  contain only the implementation for one message, and final infix **SHALL** be
  the name of the message as defined in `tools/dbc/foxbms.dbc`.

This set of rules enables a simple maintenance, consider the following example:
There is a CAN-based current sensor model `foo` from manufacturer `bar` that
periodically transmit a state message.
- The test-macro is then defined as`FOXBMS_CS_BAR_FOO=1`
- The implementation file then
  `src/app/driver/can/cbs/rx/can_cbs_rx_cs_foo-bar.c`
- The CAN message is `tools/dbc/foxbms.dbc` is then named `CS_FOO_BAR`.

See also [tools/dbc/README.md](./../../../../../tools/dbc/README.md).
