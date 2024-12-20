# `unit-hw` Directory Description

This directory includes some low-level test code for the bootloader of the
TMS570-based foxBMS BMS-Master.
The actual test binary is built through
[this wscript](./../../src/bootloader/main/wscript) and
[this directory's wscript](./wscript).

> **_NOTE:_**
>
> The test files in this directory do **not** contain the section markers for
> _test files_, but the on for _source files_.
> The section marker for _test files_ is just for _test files_ on the _host_;
> for _target test files_ the _source_ section marker is more appropiate.
