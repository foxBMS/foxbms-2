# Test Cache

## Purpose

This variant verifies the application with the MCU data and instruction cache
enabled. It is intended to catch integration problems that only occur when
cache support is active.

## Configuration

The cache setting is enabled in [`bms.json`](./bms.json):

```json
"mcu": {
    "use-cache": true
}
```

The remaining configuration is identical wit p_0020.

## Expected behavior

With the cache enabled, the application must continue to report valid battery
measurements and CAN messages. In particular, the cell voltage levels
must be transferred through CAN without stale values.

## Errors intended to catch

This variant is intended to catch cache-related regressions such as:

- incorrect voltage levels reported through CAN
- stale voltage values after a measurement update
