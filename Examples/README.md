# ARESCRIPT & ARES (A)utomated (EX)ecution

## ARESCRIPT Formatter Rulebooks

ARESCRIPT allows of you to use `\@WRITE FROM [INPUTFILE] WITH [file.arescript] TO [OUTPUT]` to format input data into strict formats like JSON.

### Version Compatibility
A .arescript file can be used across ARES.MON versions unless specified otherwise.

### Operands
`STRICT` - Optional - Defines that we will format strictly with RULES.

`GROUP`  - Defines the amount of words we will split per TOKEN (Defines what DELIM will split).

`DELIM`  - Specifies the splitting character that will be used to split WORDS/GROUPS.

`WDELIM` - Specifies the characters who will wrap WORDS. (QUOTE SINGLE, QUOTE DOUBLE, QUOTE QPERCENT are available at the moment)

### Examples

A JSON Formatter example can be found in [/Examples/json.arescript](json.arescript)


## ARES (A)utomated (EX)ecution
.ares files allow you to simplify using ARES by performing repetitive tasks in a single call, using the `\@AEX` Operand.

.ares files use the same Built-In commands as ARES, allowing you to automate extensive tasks (as seen in the [format.ares](format.ares) AEX Workflow.) this was made for testing, but can prove useful when repeating tasks.

### Version Compatibility

ARES Workflows are SPECIFIC of each ARES Version, since commands can be differently formatted or changed in future releases.

´´´plaintext
A script for ARES 0.0.12-alpha cannot be used with 0.0.13-alpha ARES.
´´´

### Notes
All these are still in development, bugs are to be spected.