# Tools

This directory contains a set of tools that support the development or usage
of foxBMS 2.

| Script                | Purpose                                                                                       |
|:----------------------|:----------------------------------------------------------------------------------------------|
| ``crc/*``             | Various [CRC](https://en.wikipedia.org/wiki/Cyclic_redundancy_check) helpers and scripts.     |
| ``dbc/``              | foxBMS 2 data base CAN.                                                                       |
| ``debugger/*``        | Configuration files for debugger settings.                                                    |
| ``gui/*``             | foxBMS 2 GUI.                                                                                 |
| ``ide/vscode/*``      | Configuration file templates for VS Code settings.                                            |
| ``utils/*``           | Helper scripts for batch and shellscript.                                                     |
| ``vendor/ceedling/*`` | Vendored ceedling files.                                                                      |
| ``waf-tools/*``       | Tools required for the build toolchain that do not come bundled with the default waf binary.  |
| ``waf``               | The build tool binary.                                                                        |
| ``waf-pubkey.asc``    | This is the public key that is used by waf to sign the waf binary ``waf``.                    |
| ``waf-verify-sig.py`` | Script to verify the signature of the waf binary.                                             |
