# Static Program Analysis

Do not use the cppcheck configuration in this directory for the Cppcheck GUI.
Instead after running ``waf build_static_analysis`` goto
``build/static_analysis`` and use the cppcheck configuration there.

## Usage in CI

The CI job uses the waf task which is intended to be a pass/fail criterion.
We are masking out quite a few rules (with the intention to reduce this
number).
The waf task prints only the failing rules that are NOT masked.

## Usage on your machine

For using cppcheck on your local machine, please take the cppcheck-GUI.
Together with the configuration file that is generated in
``build/static_analysis`` you should be able to comfortably have a complete
list of all rule violations.
The GUI allows you to filter rule violations.

You can configure cppcheck to send you directly into the editor of your choice
when double-clicking on a violation.
