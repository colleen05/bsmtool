# bsmtool - A command-line tool for creating and modifying BSM files.
**bsmtool** provides functionality for manipulating BSM files (see: [bsmlib on GitHub](https://github.com/colleen05/bsmlib)) with easy-to-use syntax.

## Syntax
Executing `bsm --help` will yield help on syntax:
```
$ bsm -h
bsmtool v1.0.0 by Colleen

Usage: bsm file (get [keys] | set [options])
    When using 'get', bsmtool will use subsequent arguments to find and list keys.

Options:
    -i <name> <value>   Set integer value.
    -f <name> <value>   Set float value.
    -s <name> <value>   Set string value.
    -r <name> <file>    Set raw value using bytes from given filename.
```