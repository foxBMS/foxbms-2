
## Development

This file is automatically parsed by GitLab for its correctness so that it
can work.
However, the rules we want to enforce on this file are purely stylistic (
e.g., to ease the maintainability) or some testing logic (e.g., for every
binary that is tested on the HIL, a SPA build shall exists too).
As such things are notorious error prone, there is a script, that helps to
ensure, that everything is well.
This check script is of course run in CI.

Ignore some strings, that are only used in this file
cSpell:ignore cbtc, utnc, bccsv, utpb, spatt, spapb
cSpell:ignore Werror, pubkey
cSpell:ignore dbfile, confdir

## General setup

### `stages`

The job prefix is the derived from the stage name by the following rule:
split string by underscores and concatenate the first letter of each entry,
i.e., the prefix for the stage 'abc_def' becomes 'ad'.
Every stage needs to comment in a one-line comment what it does.

| stage name | job prefix |description |
| --- | --- | --- |
| fox_install | fi |check for basic software fox_installation |
| configure | c |check that the project can be configured |

### `c_win32_ps1`

Windows pwsh.exe configuration of the build system
