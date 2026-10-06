# BMS Module

Prefix: `BMS`

## Main `BMS` Module

### Intention

The main BMS module operates only already validated database entries and
controls the battery system behavior based on these.
The relevant values are

- `DATA_BLOCK_ID_STATE_REQUEST`,
- `DATA_BLOCK_ID_MIN_MAX`, and
- `DATA_BLOCK_ID_PACK_VALUES`.

The main BMS module **sets** the follow database entries:

- `DATA_BLOCK_ID_ERROR_STATE`, and
- `DATA_BLOCK_ID_SYSTEM_STATE`.

## `BMS Values`

Prefix: `BMSVL`

### Intention

This module **SHALL** provide the following database values:

- `DATA_BLOCK_ID_MIN_MAX`,
- `DATA_BLOCK_ID_CELL_VOLTAGE`,
- `DATA_BLOCK_ID_CELL_TEMPERATURE`, and
- `DATA_BLOCK_ID_PACK_VALUES`.

The [`BMS`](#main-bms-module) application code **SHALL** work solely on these values.

The `BMS Values` module behavior is dependent on the setting in
[`conf/bms/bms.json`](../../../../conf/bms/bms.json)
`application:↳redundant-v-t-measurement`:

- if `true`: the [`REDUNDANCY`](../redundancy/README.md) module is used
  (implicitly uses the [`PLAUSIBILITY`](../plausibility/README.md) module).
- if `false`: the [`PLAUSIBILITY`](../plausibility/README.md) module is used
  directly.

## Further Reading

- [docs/software/modules/application/bms/bms.rst](../../../../docs/software/modules/application/bms/bms.rst)
