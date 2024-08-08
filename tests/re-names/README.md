# Unit Tests for Guidelines

Tests to verify that the regular expressions that are used to check function
names etc. are indeed correct.

These are relaxed tests, as the real naming tests is implemented in the Axivion
test (``tests/axivion/rule_config_names.py``).

## Running the Test

```pwsh
PS C:\Users\vulpes\Documents\foxbms-2> .\fox.ps1 run-script tests\re-names\test_names_regex.py
```

## Adding Tests

If a function, variable etc. is found in code by manual review, that did not
get mentioned in the Axivion report, then the regex for that case needs to to
fixed and this name should be added to the test case of the respective case.
