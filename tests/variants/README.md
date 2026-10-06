<!-- cspell:ignore cfdl -->

# Variant Build Configuration

- Tests are organized in directories containing
  - `battery_cell_cfg.json`
  - `battery_cell_soc_lookup-table.csv`
  - `battery_cell_soe_lookup-table.csv`
  - `battery_system_cfg.json`
  - `bms.json`
  - `diag_array_cfg.json`
  files.
- Test cases may contain additional PowerShell scripts that are named after the
  test directory.
- The name of the test directory  is prefixed by `c_` if it is confidential or
  cannot be published for other reasons or `p_` if is published.
  Then a four-digits number follows, starting at `0000`.
- Each directory shall contain a `README.md` explaining the necessity of the
  test case.
- There is a helper script [`apply-test-case.ps1`](./apply-test-case.ps1) that
  copies the test configuration to the build-relevant location (`conf/bms`).

## Configuration file timestamps

The helper script refreshes each copied configuration file's `LastWriteTimeUtc`
instead of retaining the source timestamp preserved by `Copy-Item`.
This is required because Windows Git can consider a file unchanged when its
size and modification timestamp match the cached index metadata, even if its
contents differ.
In that case, `git diff` can report no changes and a subsequent checkout can
leave the variant configuration in place on a reused CI runner.

For example, the default and LTC6806 variant `battery_system_cfg.json` files
differ only in the cell count, `18` versus `36`, and have the same size.
A hidden variant configuration can therefore produce a freshly generated
36-cell header while the selected driver has already been restored to LTC6813.
Refreshing destination timestamps makes Git detect the configuration changes
so that subsequent checkouts restore the defaults.

## Validate directory names

Use [`validate-directory-names.py`](./validate-directory-names.py) to verify
that all variant directory names match the required format (`c_` or `p_`
prefix and the first 16 hash characters):

```powershell
python tests/variants/validate-directory-names.py
```

The script exits with status code 0 on success and 1 if mismatches are found.
