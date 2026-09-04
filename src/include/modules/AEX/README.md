# Automatic Execution (AEX)

AEX Is the scripting engine/language of ARES, providing Type-Explicit and verbosity(in errors).

## Operands
AEX Provides SEVERAL "Operands" from the GCOO Type such as:

- `\@COMM` : \@COMM Provides a method to comment(hence its name) blocks for your own understanding.
    COMM Syntax is as follows:
```ares script
    \@COMM Comment contents go here, these are automatically ignored by AEX on runtime.
```
- `\#IF <file/var> !|= <0|1>` : the \#IF Statement allows you to  build conditional loops and compare values to ensure proper execution flow instead of linear.
    IF Statements read as follows:
```ares script
    \@COMM If a file does not exist
    \#IF file.txt !0
        \@COMM Do something.
    \#ELSE
        \@COMM Otherwise, do something else.
```
> Alternatively, ELIF blocks are provided, these do the same as IF blocks, though it is recommended to use them within a chained check to avoid confusion.

- `\#REPORT` : REPORT is the way one can output errors on a session to avoid letting the user see nothing while your script runs.
        A REPORT call will return the errors on the current session, thus it is recommended its use when doing sensitive operations.
        REPORT's syntax is as follows:
```ares script
        \@COMM Report all current errors.
        \#REPORT
```
- `\#INTO` : With INTO, you can put commands(AEX) INSIDE of a file(though this require a ONE-Liner if not using \#BLOCK.)
```ares script
    \#INTO's Syntax reads as follows.
    \#INTO "something.ares" PUT "\@WRITE 'Something Cool from %SHELL '"
```
- `\#BLOCK` : BLOCK is a way to define LONG Block successions, this allows you to properly bring longform content(such as scripts, data types, HTML Files, Bash additions, etc) to ARES, allowing you to define longform content(and its reference environment variable) before writing.
```ares script
    \#BLOCK's symtax reads as follows
    \#BLOCK coolName LINES 4
    some
    good
    content
    goes here
    \#BLOCKEND
```
> Note, if BLOCKEND is missing, the setting of this content will fail, and anything using it will fail.

- `\@a  RET` : RET is still in development for its use in FUNCTIONS(\#DEF FUNCTION FUNCTIONNAME [argumentType argumentPointer]{body \@RET})

## AMEM, AVER, AEND?
AVER as well as its sister Descriptors AMEM and AEND are `Operation Descriptors`.

All they do is set **HARD LIMITS** on ARES' Execution.

- `AMEM` : Sets a memory cap(in bytes) for the current script BEFORE running, optional, but useful if you don't wanna be hit with ARES' Hard limits on memory consumption.
- `AVER` : VERY Important value, as this sets on WHICH Parser/AEX Engine scripts can run, note that, versions are NOT forward or backwards compatible.
> If you do a script for a version, it stays on that version. compatibility is something LDS Does not bother with for these use cases. all on pro of keeping execution reliable and predictable.

- `AEND` : VERY Important, as this tells the script to stop executing. without it, ARES could simply refuse to execute your script.

## Usable operands?
AEX uses the same very commands as ARES, so, if you can make it on ARES' REPL, you can do it here too.

You can run externals using `\@EXEC` or `@<cmd>`, or maybe write stuff to terminal using `\@WRITE "I love cats"`!

Limits are up to what YOU can imagine you can do with all the provided operands!

## How can you use it?
AEX is made to be a way to rapidly executing repetitive tasks(replacing content, running binaries with repetitive arguments, catt-ing from files.)

yet its uses can be many, and depend fully on YOUR capabilities.

so go make some cool stuff with LDS' AEX Operand Format!